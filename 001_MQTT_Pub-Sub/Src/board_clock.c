#include "stm32f4xx.h"
#include "board_clock.h"

void board_tick_init(void);

void board_clock_init(void)
{
    /* Call before peripheral setup. HSI needs no external crystal.
     * Switch to 16 MHz HSI BEFORE removing any existing bus prescalers. */
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U) { }
    RCC->CFGR &= ~RCC_CFGR_SW;
    while ((RCC->CFGR & RCC_CFGR_SWS) != 0U) { }
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    SystemCoreClockUpdate(); /* HCLK = PCLK1 = PCLK2 = 16 MHz. */
    board_tick_init();
}

static uint32_t peripheral_clock(uint32_t shift)
{
    /* APB encoding: 0xx=/1, 100=/2, 101=/4, 110=/8, 111=/16.
     * USART/I2C use PCLK, NOT the doubled timer clock. */
    uint32_t div = (RCC->CFGR >> shift) & 7U;
    SystemCoreClockUpdate();
    return SystemCoreClock >> ((div < 4U) ? 0U : div - 3U);
}
uint32_t board_pclk1_hz(void) { return peripheral_clock(10U); }
uint32_t board_pclk2_hz(void) { return peripheral_clock(13U); }

void board_tick_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t board_ticks_per_ms(void)
{
    return SystemCoreClock / 1000U;
}

uint32_t board_millis(void)
{
    return DWT->CYCCNT / board_ticks_per_ms();
}

void board_delay_ms(uint32_t ms)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t delta = ms * board_ticks_per_ms();
    while ((DWT->CYCCNT - start) < delta) { }
}
