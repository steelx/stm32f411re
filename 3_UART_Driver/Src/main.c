// UART (RealTerm press 1 for LED On)
// ADC
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>
#include "uart.h"


#define GPIOAEN	(1U<<0)
#define GPIOA_5	(1U<<5)

char key;

int main(void)
{
	/// Enable clock access on GPIOA
	RCC->AHB1ENR |= GPIOAEN;

	// PA5 as Output Mode
	GPIOA->MODER &= ~(3U << 10);// Clear bits 11 and 10 to 00
	GPIOA->MODER |= (1U << 10);// Set bits 10 to 1 [11:10] = [0:1]


	uart2_rxtx_init();

	while(1)
	{
		key = uart2_read();
		if (key == '1')
		{
			GPIOA->ODR |= GPIOA_5;// Turn on LED
		}
		else
		{
			GPIOA->ODR &= ~GPIOA_5;// Turn off LED
		}
	}
}






