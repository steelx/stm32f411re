/*
 * uart.h
 * */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>
#include <stm32f4xx.h>

void uart2_tx_init();
void uart2_rxtx_init();
char uart2_read();

#endif /* UART_H_ */
