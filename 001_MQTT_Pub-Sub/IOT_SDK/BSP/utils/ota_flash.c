/*
 * flash.c
 *
 *  Created on: Feb 18, 2025
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 *
 *  STUB: compiles without HAL, but flash write/erase operations are not
 *  implemented.  Port to the register-level FLASH interface if OTA is needed.
 */

#include "ota_flash.h"
#include "application_config.h"

flash_status_t flash_erase(uint32_t start_sector)
{
    (void)start_sector;
    return FLASH_ERROR;
}

flash_status_t flash_write(__IO uint32_t* flash_address, uint32_t* data, uint16_t data_length)
{
    (void)flash_address;
    (void)data;
    (void)data_length;
    return FLASH_ERROR;
}
