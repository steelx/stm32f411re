/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Ajinkya Borade
 * @brief          : STM32 Button Matrix
 ******************************************************************************
 */
#include "stm32f4xx.h"
#include <stdint.h>

//#define LED_ONLY_TEST 1  // First test the LED. Then change to 0.

volatile uint32_t k1_pressed;

int main(void)
{
    /* Enable GPIOB for the LED and GPIOC for the keypad. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    (void)RCC->AHB1ENR;

    /* D1 -> PB0: push-pull output.
       High = LED off; low = LED on. */
    GPIOB->BSRR = 1U << 0; // Preload LED off
    GPIOB->OTYPER &= ~(1U << 0);
    GPIOB->PUPDR &= ~(3U << 0);
    GPIOB->MODER = (GPIOB->MODER & ~(3U << 0)) | (1U << 0);

    /* L1 -> PC2: open-drain output.
       High releases the row; low activates it. */
    GPIOC->BSRR = 1U << 2;       // Initially release L1
    GPIOC->OTYPER |= 1U << 2;
    GPIOC->PUPDR &= ~(3U << 4);
    GPIOC->MODER = (GPIOC->MODER & ~(3U << 4)) | (1U << 4);

    /* R1 -> PC6: input with pull-up. */
    GPIOC->MODER &= ~(3U << 12);
    GPIOC->PUPDR = (GPIOC->PUPDR & ~(3U << 12)) | (1U << 12);

    /* Keep L1 active for this single-key test. */
    GPIOC->BSRR = 1U << (2 + 16);

    for (;;) {
        k1_pressed = (GPIOC->IDR & (1U << 6)) == 0U;

        if (k1_pressed) GPIOB->BSRR = 1U << (0 + 16); // D1 on
        else GPIOB->BSRR = 1U << 0; // D1 off
    }
}
