/*
 * uart.c
 *
 *  Created on: 18 Sept 2026
 *      Author: Ajinkya
 */
#include "uart.h"


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


#define SYS_FREQ		 16000000
#define APB1_CLK		 SYS_FREQ

#define UART_BAUDERATE 115200


static void uart_set_baudrate(USART_TypeDef* USARTx, uint32_t PeriphClk, uint32_t BaudRate);
static uint16_t compute_uart_bd(uint32_t PeriphClk, uint32_t BaudRate);

void uart2_write(int ch);

int __io_putchar(int ch)
{
	uart2_write(ch);
	return ch;
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


void uart2_rxtx_init()
{
	/**** Configure UART GPIO Pin ****/
	RCC->AHB1ENR |= GPIOAEN;

	/** Enable Alternate Function Mode **/
	// GPIO Pin 2 --> MODER2  sets Bit 5:4 to [1:0] alternate function mode
	// 3U = 0111b
	GPIOA->MODER = (GPIOA->MODER & ~(3U << 4)) | (2U << 4);

	// GPIO Pin 3 --> MODER3 PA3: bits 7:6 to [1:0]
	GPIOA->MODER = (GPIOA->MODER & ~(3U << 6)) | (2U << 6);


	// Set PA2 Mode to AF mode; AF7 Type [0 1 1 1] UART_TX
	// 15u = 1111b negated 15U binary at 8th bit (1111 0000 1111 1111)
	// 7U  = 0111b on the 8th Bit position
	GPIOA->AFR[0] &= ~(15U << 8);
	GPIOA->AFR[0] |= (7U << 8);

	// Set PA3 to AF7 (UART_RX)
	GPIOA->AFR[0] &= ~(15U << 12);
	GPIOA->AFR[0] |= (7U << 12);


	/**** Configure UART module ****/

	RCC->APB1ENR |= UART2EN; // enable clock access to USART2
	uart_set_baudrate(USART2, APB1_CLK, UART_BAUDERATE);

	// Set Transfer Direction & enable UART module
	USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);
	USART2->CR1 |= USART_CR1_UE;
}

char uart2_read(void)
{
	// Make sure the Received data register is NOT empty
	while (!(USART2->SR & USART_SR_RXNE)) {}

	// Read data
	return USART2->DR;
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

