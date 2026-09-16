/* OLED Display
 * NUCLEO-F411RE + Pimoroni PIM374 (I2C, SH1107).
 * CMSIS only. Use one checkpoint as Src/main.c.
 * HSI = HCLK = PCLK1 = PCLK2 = 16 MHz. No HAL.
 */
#include "stm32f4xx.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define GPIOBEN (1U << 1)
#define I2C1EN  (1U << 21)
#define OLED_ADDR7 0x3CU /* 0x3D only if address trace is cut */
#define WAIT_LIMIT 1000000U

volatile uint32_t fault_code;

static void fail(uint32_t code)
{
    fault_code = code;
    for (;;) { __NOP(); } /* breakpoint here */
}

static void wait_bits(volatile uint32_t *reg, uint32_t mask, uint32_t value, uint32_t code)
{
    uint32_t remaining = WAIT_LIMIT;
    while ((*reg & mask) != value) {
        if (--remaining == 0U) fail(code);
    }
}

static void i2c_pins_init(void)
{
    RCC->AHB1ENR |= GPIOBEN;
    (void)RCC->AHB1ENR;
    GPIOB->OTYPER |= (1U << 8) | (1U << 9);
    GPIOB->PUPDR &= ~((3U << 16) | (3U << 18)); /* Clear bits */
    GPIOB->PUPDR |=  ((1U << 16) | (1U << 18)); /* Enable Pull-ups (01b) for PB8 & PB9 */
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR &
        ~((3U << 16) | (3U << 18))) | (1U << 16) | (1U << 18);
    GPIOB->AFR[1] = (GPIOB->AFR[1] & ~0xFFU) | 0x44U;
    GPIOB->MODER = (GPIOB->MODER &
        ~((3U << 16) | (3U << 18))) | (2U << 16) | (2U << 18);
}

static void clock_init(void)
{
    RCC->CR |= RCC_CR_HSION;
    wait_bits(&RCC->CR, RCC_CR_HSIRDY, RCC_CR_HSIRDY, 1);
    RCC->CFGR &= ~RCC_CFGR_SW; /* choose HSI */
    wait_bits(&RCC->CFGR, RCC_CFGR_SWS, 0, 2);
    RCC->CFGR &= ~(RCC_CFGR_HPRE |
                   RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    SystemCoreClockUpdate();
    if (SystemCoreClock != 16000000U) fail(3);
}

/* This blocking function owns SysTick; no IRQ handler. */
static void delay_ms(uint32_t ms)
{
    SysTick->CTRL = 0;
    SysTick->LOAD = SystemCoreClock / 1000U - 1U;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    while (ms--) {
        while (!(SysTick->CTRL &
                 SysTick_CTRL_COUNTFLAG_Msk)) {}
    }
    SysTick->CTRL = 0;
}

static void i2c1_init(void)
{
    RCC->APB1ENR |= I2C1EN;
    (void)RCC->APB1ENR;
    RCC->APB1RSTR |= I2C1EN;
    RCC->APB1RSTR &= ~I2C1EN;
    I2C1->CR1 = 0; /* PE disabled while setting timing */
    I2C1->CR2 = 16U; /* APB1 MHz */
    I2C1->OAR1 = 1U << 14; /* required reserved bit */
    I2C1->CCR = 80U; /* 16 MHz / (2 * 100 kHz) */
    I2C1->TRISE = 17U; /* 16 MHz * 1 us + 1 */
    I2C1->CR1 = I2C_CR1_PE;
}

static void bus_fail(uint32_t code)
{
    /* Do not generate STOP after losing arbitration. */
    if (!(I2C1->SR1 & I2C_SR1_ARLO) &&
         (I2C1->SR2 & I2C_SR2_MSL))
        I2C1->CR1 |= I2C_CR1_STOP;
    fail(code); /* halt; power-cycle after fixing the cause */
}

static void i2c_wait(volatile uint32_t *reg, uint32_t mask, uint32_t value, uint32_t timeout_code)
{
    uint32_t remaining = WAIT_LIMIT;
    for (;;) {
        uint32_t sr = I2C1->SR1;
        if (sr & I2C_SR1_AF)   bus_fail(10); /* NACK */
        if (sr & I2C_SR1_BERR) bus_fail(11);
        if (sr & I2C_SR1_ARLO) bus_fail(12);
        if (sr & I2C_SR1_OVR)  bus_fail(13);
        if ((*reg & mask) == value) return;
        if (--remaining == 0U) bus_fail(timeout_code);
    }
}

static void i2c_byte(uint8_t byte)
{
    i2c_wait(&I2C1->SR1, I2C_SR1_TXE, I2C_SR1_TXE, 7);
    I2C1->DR = byte;
}

static void oled_write(uint8_t is_data, const uint8_t *bytes, size_t n)
{
    if (n == 0U) return;
    i2c_wait(&I2C1->SR2, I2C_SR2_BUSY, 0, 4);
    I2C1->CR1 |= I2C_CR1_START;
    i2c_wait(&I2C1->SR1, I2C_SR1_SB, I2C_SR1_SB, 5);
    I2C1->DR = OLED_ADDR7 << 1; /* 0x78: address plus write bit */
    i2c_wait(&I2C1->SR1, I2C_SR1_ADDR, I2C_SR1_ADDR, 6);
    (void)I2C1->SR1;
    (void)I2C1->SR2; /* this read sequence clears ADDR */
    i2c_byte(is_data ? 0x40U : 0x00U);
    for (size_t i = 0; i < n; ++i) i2c_byte(bytes[i]);
    i2c_wait(&I2C1->SR1, I2C_SR1_BTF, I2C_SR1_BTF, 8);
    I2C1->CR1 |= I2C_CR1_STOP;
    i2c_wait(&I2C1->SR2, I2C_SR2_BUSY, 0, 9);
}

static void oled_command(uint8_t command)
{
    oled_write(0, &command, 1);
}

/* SH1107 Initialization Sequence */
static const uint8_t init_cmds[] = {
    0xAE,       /* Display OFF */
    0xDC, 0x00, /* Set display start line to 0 */
    0x81, 0x80, /* Set contrast control */
    0x20,       /* Set memory addressing mode to Page */
    0xA1,       /* Segment remap */
    0xC8,       /* COM scan direction */
    0xA8, 0x7F, /* Set multiplex ratio (128 for 1.12" OLED) */
    0xD3, 0x00, /* Set display offset to 0 */
    0xD5, 0x50, /* Set display clock divide ratio */
    0xD9, 0x22, /* Set pre-charge period */
    0xDB, 0x35, /* Set VCOMH deselect level */
    0xAD, 0x8B, /* DC-DC Control: Enable internal charge pump (CRITICAL) */
    0xA4,       /* Entire display ON (resume to RAM content) */
    0xA6,       /* Normal display (non-inverted) */
    0xAF        /* Display ON */
};

static void oled_init(void)
{
    for (size_t i = 0; i < sizeof(init_cmds); i++) {
        oled_command(init_cmds[i]);
    }
}

volatile uint32_t bus_levels;

int main(void)
{
    i2c_pins_init();
    clock_init();
    i2c1_init();

    /* 1. Wake up and configure the SH1107 controller */
    oled_init();

	/* 2. Hardware test: Force all pixels ON (ignores RAM)
	 * If the screen fills with white, your I2C and hardware are perfect. */
    oled_command(0xA5);

    for (;;) {
        bus_levels = (GPIOB->IDR >> 8) & 3U;
        delay_ms(100); /* expect 3: both lines idle high */
    }
}
