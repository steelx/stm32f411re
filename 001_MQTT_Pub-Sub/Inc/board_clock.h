#ifndef BOARD_CLOCK_H_
#define BOARD_CLOCK_H_
#include <stdint.h>
void board_clock_init(void);
uint32_t board_pclk1_hz(void);
uint32_t board_pclk2_hz(void);
void board_tick_init(void);
uint32_t board_millis(void);
void board_delay_ms(uint32_t ms);
#endif
