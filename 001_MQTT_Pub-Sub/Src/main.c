#include "stm32f4xx.h"
#include "board_clock.h"
#include "uart.h"
#include "i2c.h"

#include "esp32_at.h"

int main(void)
{
    board_clock_init();
    uart2_rxtx_init(); /* PC: PA2 TX / PA3 RX, 115200 8-N-1. */
    usart1_init();     /* ESP: PA15 TX / PB7 RX. See jumper instructions. */
    if (i2c1_init() != 0) {
        while (1) { } /* Clock configuration error: inspect with debugger. */
    }

    /* Shield D2 LED = PB0. Do NOT toggle PA5 with the shield attached:
     * PA5 is its shared SPI clock. Keep PA9/PA10 free of USART1 too. */
    GPIOB->BSRR = 1U << 16; /* LED initially off. */
    GPIOB->OTYPER &= ~1U;
    GPIOB->PUPDR &= ~3U;
    GPIOB->MODER = (GPIOB->MODER & ~3U) | 1U;
    const char *banner = "\r\nF411 ready. USART1/2:115200 I2C1:100kHz. 1=shield LED on, 0=off\r\n";
    while (*banner != '\0') { uart2_write(*banner++); }

    printf("Initializing ESP32 Wifi Module... \n");
    if (esp32_init() != ESP32_OK)
    {
    	printf("Failed to start ESP32 Wifi Module \n");
    	Error_Handler();
    }

    printf("Successfully initialized ESP32 Wifi Module \n");

    while (1) {
        uint8_t key;
        /* Nonblocking receive; CR/LF must not undo the LED command. */
        if (uart2_try_read(&key) == 1) {
            if (key == '1') { GPIOB->BSRR = 1U; }
            else if (key == '0') { GPIOB->BSRR = 1U << 16; }
        }
        /* Hardware initialization only. Add AT protocol handling and an
         * interrupt/DMA receive buffer before running the course MQTT SDK.
         * No ESP commands or I2C transfers are sent by this demo. */
    }
}
