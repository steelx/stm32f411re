#include <stm32f4xx.h>
#include <stdint.h>

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

static void delay_cycles(volatile uint32_t count)
{
    while (count--) {
        __asm("nop");
    }
}


/**
 * Bit mask to enable GPIO Port A clock in RCC AHB1ENR (Bit 0).
 *
 * 1U in 32-bit binary:
 * 0b0000 0000 0000 0000 0000 0000 0000 0001  (Bit 0 is set)
 *
 * (1U << 0) shifts by 0 positions (remains at Bit 0):
 * 0b 0000 0000 0000 0000 0000 0000 0000 0001
 *
 * For comparison, (1U << 8) shifts 8 positions left (moves to Bit 8):
 * 0b 0000 0000 0000 0000 0000 0001 0000 0000
 */
#define GPIOAEN 	(1U << 0)
#define GPIOCEN 	(1U << 2)
#define USART2EN	(1U << 17)

#define PIN5	 	(1U<<5)
#define PIN13	 	(1U<<13)

#define LED_PIN 	PIN5
#define USER_BUTTON PIN13


int main(void)
{
	// Enable clock access
	RCC->AHB1ENR |= GPIOAEN;
	RCC->AHB1ENR |= GPIOCEN;

	/* 2. Configure PA5 as general-purpose output (Mode 01b) */
	GPIOA->MODER &= ~(3U << (5 * 2)); /* Shifts 0b 11 to bits 11:10 */
	GPIOA->MODER |=  (1U << (5 * 2)); /* Set bit 10 to 1 */

	/* Set PC13 as Input pin */
	GPIOC->MODER &= ~(3U << (13 * 2)); // clears Bit 27:26 to 0 which is Input Mode

    /* Loop forever */
	while(1)
	{
		if (GPIOC->IDR & USER_BUTTON){
			/// 8.4.7 GPIOx_BSRR Bit set and reset register
			GPIOA->BSRR = LED_PIN; // BS5 Set High
		} else {
			GPIOA->BSRR = (LED_PIN << 16);// BR5 (1U<<21) Reset
		}
	}
}
