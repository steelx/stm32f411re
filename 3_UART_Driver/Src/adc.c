/*
 * adc.c
 *
 *  Created on: 26 Sept 2026
 *      Author: Ajinkya
 */


/// ADC1 is on APB2 on bit 8
#define ADC1EN		(1U<<8)
#define GPIOAEN	(1U<<0)

void pa1_adc_init(void)
{
	/** Enable clock access on GPIOA **/
	RCC->AHB1ENR |= GPIOAEN;

	// PA1 set to Analog mode
	GPIOA->MODER = (GPIOA->MODER & ~(0x3U << 2)) | (3U << 2);

	/** Configure ADC Module **/
	RCC->APB2ENR |= ADC1EN;
}
