#ifndef UART_H_
#define UART_H_
#include <stdint.h>
void usart1_init(void);
void uart2_tx_init(void);
void uart2_rxtx_init(void);
void uart1_write(int ch);
void uart2_write(int ch);
char uart2_read(void);
/* Valid output pointer required. Result: 1=byte, 0=empty, -1=RX error. */
int uart1_try_read(uint8_t *byte);
int uart2_try_read(uint8_t *byte);
#endif
