#include "stm32f4xx.h"
#include "board_clock.h"
#include "i2c.h"

int i2c1_init(void)
{
    uint32_t pclk = board_pclk1_hz();
    /* F411 APB1 max is 50 MHz; CR2.FREQ is a whole number of MHz. */
    if (pclk < 2000000U || pclk > 50000000U || pclk % 1000000U != 0U) {
        return -1;
    }
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    (void)RCC->AHB1ENR;
    (void)RCC->APB1ENR;
    /* PB8/PB9 use AFR[1]. Open-drain allows either device to pull low.
     * External resistors pull high; internal weak pulls are not sufficient.
     * The supplied IoT schematic does not show SCL/SDA pull-up resistors. */
    GPIOB->OTYPER |= (1U << 8) | (1U << 9);
    GPIOB->PUPDR &= ~((3U << 16) | (3U << 18));
    GPIOB->OSPEEDR = (GPIOB->OSPEEDR & ~((3U << 16) | (3U << 18)))
                     | (2U << 16) | (2U << 18);
    GPIOB->AFR[1] = (GPIOB->AFR[1] & ~0xFFU) | 0x44U;
    GPIOB->MODER = (GPIOB->MODER & ~((3U << 16) | (3U << 18)))
                   | (2U << 16) | (2U << 18);
    /* Reset only this peripheral. Call on an idle bus before transfers;
     * this does not recover an external slave holding SDA low. */
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;
    I2C1->CR1 = 0U; /* PE=0 while programming timing. */
    I2C1->CR2 = pclk / 1000000U;
    /* Standard mode: SCL=PCLK1/(2*CCR); round up for <=100 kHz.
     * TRISE corresponds to the 1000 ns standard-mode rise-time limit.
     * At 16 MHz: FREQ=16, CCR=80, TRISE=17. */
    I2C1->CCR = (pclk + 199999U) / 200000U;
    I2C1->TRISE = pclk / 1000000U + 1U;
    I2C1->OAR1 = 1U << 14; /* RM0383 requires bit 14 set; 7-bit mode. */
    I2C1->OAR2 = 0U;
    I2C1->CR1 = I2C_CR1_PE;
    return 0;
}
