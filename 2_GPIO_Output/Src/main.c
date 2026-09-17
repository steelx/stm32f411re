
#include <stdint.h>
#include <stdio.h>
#include <stm32f4xx.h>

static void uart_set_baudrate(USART_TypeDef* USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);
void uart2_write(int ch);
void uart2_tx_init();

int __io_putchar(int ch)
{
	uart2_write(ch);
	return ch;
}

static void delay_cycles(volatile uint32_t count)
{
    while (count--) {
        __asm("nop");
    }
}

#define SYS_FREQ		 16000000
#define APB1_CLK		 SYS_FREQ
#define UART_BAUDERATE 115200


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
#define UART2EN	(1U << 17)

#define PIN5	 	(1U<<5)
#define PIN13	 	(1U<<13)

#define LED_PIN 	PIN5
#define USER_BUTTON PIN13


int main(void)
{
	uart2_tx_init();

	while(1)
	{
		printf("Hello from STM32F4 UART. \n\r");
	}
}

void uart2_tx_init()
{
	/**** Configure UART GPIO Pin ****/
	RCC->AHB1ENR |= GPIOAEN;

	// GPIO Pin 2 --> MODER2  sets to [1:0] alternate function mode
	GPIOA->MODER &= ~(1U<<4); // set bit 4 to 0
	GPIOA->MODER |= (1U<<5); // set bit 5 to 1 as per AF

	// Set the AF mode to AF7 Type [0 1 1 1]
	// negated 15U (1111 0000 1111 1111)
	// 7U on the 8th Bit position (0111)
	GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(15U << 8)) | (7U << 8);


	/**** Configure UART module ****/
	// enable clock access to USART2
	RCC->APB1ENR |= UART2EN;
	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDERATE);

	// Set Transfer Direction & enable UART module
	USART2->CR1 |= USART_CR1_TE;
	USART2->CR1 |= USART_CR1_UE;
}

void uart2_write(int ch)
{
	// Wait for transmit data register is empty
	while (!(USART2->SR & USART_SR_TXE)) {}

	// Write to transmit data register
	USART2->DR = (ch & 0xFF);
}

static void uart_set_baudrate(USART_TypeDef* USARTx, uint32_t PeriphClk, uint32_t BaudRate)
{
	USARTx->BRR = compute_uart_bd(PeriphClk, BaudRate);
}

static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate)
{
	return (PeriphClk + (BaudRate/2U)) / BaudRate;
}









