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
#define PIN5	 	(1U<<5)
#define LED_PIN 	PIN5

int main(void)
{
	RCC->AHB1ENR |= GPIOAEN;

	/* 2. Configure PA5 as general-purpose output (Mode 01b) */
	GPIOA->MODER &= ~(3U << (5 * 2)); /* Shifts 0b 11 to bits 11:10 */
	GPIOA->MODER |=  (1U << (5 * 2)); /* Set bit 10 to 1 */

    /* Loop forever */
	while(1)
	{
		/// 8.4.7 GPIOx_BSRR Bit set and reset register
		GPIOA->BSRR = PIN5; // BS5
		delay_cycles(250000*10);

		GPIOA->BSRR = (PIN5 << 16);// BR5 (1U<<21)
		delay_cycles(250000*10);
	}
}
