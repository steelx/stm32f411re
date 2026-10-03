#include "stm32f4xx.h"
#include "board_clock.h"

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
