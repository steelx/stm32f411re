/*
 * utils.c
 *
 *  Created on: 16 Oct 2026
 *      Author: Ajinkya
 */
#include "utils.h"

/* ESP32_STATION_MODE, ESP32_OK and esp32_get_ip(). */
#include "esp32_at.h"

/* LogInfo() and LogError(). */
#include "application_config.h"

/**
 * @brief Ask the ESP32-C6 for its station IP address and print it through
 *        the project's OpenOCD logging system.
 *
 * @note  Call this only after esp32_join_ap() returns ESP32_OK.
 */
void ESP32_Print_IP_Address(void)
{
    /* Maximum IPv4 text is "255.255.255.255" plus the null terminator. */
    uint8_t ip_address[16] = {0};

    /*
     * Use the ESP32 driver's public API. It already sends AT+CIFSR and
     * receives the response through the UART4 DMA code initialized by
     * esp32_init(). Direct HAL_UART_Receive() calls here would compete with
     * that DMA receiver for the same UART bytes.
     */
    if (esp32_get_ip(ESP32_STATION_MODE, ip_address) != ESP32_OK)
    {
        LogError(("Failed to read the ESP32 station IP address"));
        return;
    }

    LogInfo(("ESP32 IP address: %s", (char *)ip_address));
}
