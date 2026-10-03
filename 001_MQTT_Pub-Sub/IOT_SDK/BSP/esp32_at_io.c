/*
 * esp32_at_io.c
 *
 * Created on: Dec 27, 2024
 * Author: Shreyas Acharya, BHARATI SOFTWARE
 *
 * Ported to the register-level STM32F411RE drivers in this project.
 * The ESP32 AT interface is on USART1 (PA15 TX / PB7 RX).
 */

/* Includes ------------------------------------------------------------------*/
#include "esp32_at_io.h"
#include "uart.h"
#include "board_clock.h"
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define RECV_NB_TIMEOUT_MS 200U

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static int32_t esp32_io_recv_timeout(uint8_t *buffer, uint32_t length,
                                     uint32_t timeout_ms);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief  Initialize the ESP32 UART interface.
 * @retval 0 on success, -1 otherwise.
 */
int8_t esp32_io_init(void) {
  /* The host needs ~2 s before the ESP32 AT firmware is ready. */
  board_delay_ms(2000);
  return 0;
}

/**
 * @brief  Deinitialize the ESP32 UART interface.
 * @retval None.
 */
void esp32_io_deinit(void) {
  /* No action required for the register-level driver. */
}

/**
 * @brief  Reset the IO receive buffer state.
 * @retval None.
 */
void io_buff_reset(void) {
  /* Polling driver has no background buffer to reset. */
}

/**
 * @brief  Restart UART DMA reception.
 * @retval 0 on success, -1 otherwise.
 */
int8_t uart_dma_restart(void) {
  /* DMA is not used in this port. */
  return 0;
}

/**
 * @brief  Send data to the ESP32 module over UART.
 * @param  Buffer: Pointer to the data buffer to send.
 * @param  Length: Length of the data buffer.
 * @retval 0 on success, -1 otherwise.
 */
int8_t esp32_io_send(uint8_t *Buffer, uint32_t Length) {
  if (Buffer == NULL) {
    return -1;
  }
  for (uint32_t i = 0; i < Length; i++) {
    uart1_write((int)Buffer[i]);
  }
  return 0;
}

/**
 * @brief  Receive data from the ESP32 module over UART (blocking).
 * @param  Buffer: Pointer to the buffer to store received data.
 * @param  Length: Maximum length of the buffer.
 * @retval Number of bytes received.
 */
int32_t esp32_io_recv(uint8_t *Buffer, uint32_t Length) {
  return esp32_io_recv_timeout(Buffer, Length, DEFAULT_TIME_OUT);
}

/**
 * @brief  Non-blocking receive data from the ESP32 module over UART.
 * @param  buffer: Pointer to the buffer to store received data.
 * @param  length: Maximum length of the buffer.
 * @retval Number of bytes received.
 */
int32_t esp32_io_recv_nb(uint8_t *buffer, uint32_t length) {
  return esp32_io_recv_timeout(buffer, length, RECV_NB_TIMEOUT_MS);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Receive up to Length bytes, with a per-byte timeout.
 * @retval Number of bytes actually received.
 */
static int32_t esp32_io_recv_timeout(uint8_t *buffer, uint32_t length,
                                     uint32_t timeout_ms) {
  uint32_t read_data = 0;

  if (buffer == NULL) {
    return 0;
  }

  while (length--) {
    uint8_t byte;
    uint32_t tick_start = board_millis();
    int rc;
    do {
      rc = uart1_try_read(&byte);
      if (rc == 1) {
        *buffer++ = byte;
        read_data++;
        break;
      }
    } while ((board_millis() - tick_start) < timeout_ms);

    if (rc != 1) {
      /* Timeout on this byte: return what we have so far. */
      break;
    }
  }
  return (int32_t)read_data;
}
