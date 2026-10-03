/* STM32F411RE + Fastbit IoT Connect Shield. Register-level CMSIS.
 * IMPORTANT: shield A0/A1 expects UART4, which F411 does not have!
 * Jumper PA15 -> A0 (ESP GPIO6 AT_RX), PB7 <- A1 (ESP GPIO7 AT_TX).
 * PA0/PA1 remain analog inputs so they cannot drive these shared nets.
 * PA9/PA10 are NOT used: shield flash CS / OLED D-C occupy those pins.
 */
#include "stm32f4xx.h"
#include "uart.h"
#include "board_clock.h"

#define UART_BAUD_RATE 115200U

static void uart_pin_init(GPIO_TypeDef *port, uint32_t pin)
{
    uint32_t shift = pin * 2U;
    uint32_t af_shift = (pin % 8U) * 4U;
    /* Push-pull output, fast GPIO edges, idle-high pull-up, AF7.
     * MODER uses 2 bits per pin; AFR uses 4 bits per pin. */
    port->OTYPER &= ~(1U << pin);
    port->OSPEEDR = (port->OSPEEDR & ~(3U << shift)) | (2U << shift);
    port->PUPDR = (port->PUPDR & ~(3U << shift)) | (1U << shift);
    port->AFR[pin / 8U] = (port->AFR[pin / 8U] & ~(15U << af_shift))
                         | (7U << af_shift);
    port->MODER = (port->MODER & ~(3U << shift)) | (2U << shift);
}

static void uart_init(USART_TypeDef *uart, uint32_t pclk, uint32_t direction)
{
    /* Configure disabled: 8 data bits, no parity, 1 stop bit, OVER8=0,
     * no flow control, DMA or interrupts. BRR is rounded PCLK/baud
     * with oversampling by 16: at 16 MHz and 115200 it is 139 (0x8B). */
    uart->CR1 = 0U;
    uart->CR2 = 0U;
    uart->CR3 = 0U;
    uart->BRR = (pclk + UART_BAUD_RATE / 2U) / UART_BAUD_RATE;
    uart->CR1 = direction | USART_CR1_UE;
}

void usart1_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    (void)RCC->AHB1ENR; /* Readback gives clock enables time to propagate. */
    (void)RCC->APB2ENR;
    GPIOA->MODER |= (3U << 0) | (3U << 2); /* PA0/PA1 analog (high-Z). */
    GPIOA->PUPDR &= ~((3U << 0) | (3U << 2));
    uart_pin_init(GPIOA, 15U); /* TX. Replaces JTDI; debug using SWD. */
    uart_pin_init(GPIOB, 7U);  /* RX. */
    uart_init(USART1, board_pclk2_hz(), USART_CR1_TE | USART_CR1_RE);
}

static void uart2_init(uint32_t direction)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    (void)RCC->AHB1ENR;
    (void)RCC->APB1ENR;
    uart_pin_init(GPIOA, 2U); /* TX to ST-LINK virtual COM port. */
    if ((direction & USART_CR1_RE) != 0U) { uart_pin_init(GPIOA, 3U); }
    uart_init(USART2, board_pclk1_hz(), direction);
}
void uart2_tx_init(void) { uart2_init(USART_CR1_TE); }
void uart2_rxtx_init(void) { uart2_init(USART_CR1_TE | USART_CR1_RE); }

static int uart_try_read(USART_TypeDef *uart, uint8_t *byte)
{
    uint32_t status = uart->SR;
    uint32_t errors = USART_SR_ORE | USART_SR_NE | USART_SR_FE | USART_SR_PE;
    if ((status & (USART_SR_RXNE | errors)) == 0U) { return 0; }
    /* SR then DR clears RXNE and receive errors on STM32F4.
     * Discard errored data. Use only one consumer per USART. */
    uint8_t data = (uint8_t)uart->DR;
    if ((status & errors) != 0U) { return -1; }
    *byte = data;
    return 1;
}
int uart1_try_read(uint8_t *byte) { return uart_try_read(USART1, byte); }
int uart2_try_read(uint8_t *byte) { return uart_try_read(USART2, byte); }

char uart2_read(void)
{
    /* Compatibility helper: blocks indefinitely until a valid byte arrives. */
    uint8_t byte;
    while (uart2_try_read(&byte) != 1) { }
    return (char)byte;
}

/* TXE means DR is free, not that the last stop bit has left the wire.
 * Wait for TC before disabling/changing the UART. Call only after init. */
void uart1_write(int ch)
{
    while ((USART1->SR & USART_SR_TXE) == 0U) { }
    USART1->DR = (uint8_t)ch;
}
void uart2_write(int ch)
{
    while ((USART2->SR & USART_SR_TXE) == 0U) { }
    USART2->DR = (uint8_t)ch;
}
/* Existing syscalls.c routes printf/getchar through these hooks. */
int __io_putchar(int ch) { uart2_write(ch); return ch; }
int __io_getchar(void) { return (unsigned char)uart2_read(); }
