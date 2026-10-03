/*
 * sh1106.c
 *
 * Created on: Oct 2, 2024
 * Author: Shreyas Acharya, BHARATI SOFTWARE
 *
 * STUB: the framebuffer drawing API is preserved, but the I2C/SPI transport
 * to the SH1106 panel is not implemented for the register-level F411 project.
 * The display will not update until a real I2C/SPI driver is wired in here.
 */

#include <sh1106.h>
#include <stdlib.h>
#include <string.h>
#include "board_clock.h"

// SH1106 object
static SH1106_t SH1106;

// Initialization commands
static const uint8_t init_cmds[] = {
    0xAE,         /* Display off */
    0xD5, 0x80,   /* Set display clock divide ratio, suggested value 0x80 */
    0xA8, 0x3F,   /* Set multiplex, 1/64 duty */
    0xD3, 0x00,   /* Set display offset, no offset */
    0x40,         /* Set start line at 0 */
    0x8D, 0x14,   /* Charge pump, enable charge pump */
    0x20, 0x00,   /* Memory mode, horizontal addressing mode */
    0xA1,         /* Set segment remap */
    0xC8,         /* Set COM output scan direction */
    0xDA, 0x12,   /* Set COM pins hardware configuration */
    0x81, 0x7F,   /* Set contrast control */
    0xD9, 0xF1,   /* Set pre-charge period */
    0xDB, 0x40,   /* Set VCOMH deselect level */
    0xA4,         /* Enable display all on */
    0xA6,         /* Normal display */
    0xAF          /* Display on */
};

// Transport stubs
sh1106_t sh1106_write_command(void* sh_handle, uint8_t cmd) {
    (void)sh_handle;
    (void)cmd;
    return SH1106_OK;
}

static sh1106_t sh1106_write_data(void* sh_handle, uint8_t* data, uint16_t len) {
    (void)sh_handle;
    (void)data;
    (void)len;
    return SH1106_OK;
}

static sh1106_t sh1106_send_commands(void* sh_handle, const uint8_t* cmds, uint8_t cmd_count) {
    for (uint8_t i = 0; i < cmd_count; i++) {
        if (sh1106_write_command(sh_handle, cmds[i]) != SH1106_OK) {
            return SH1106_ERR;
        }
    }
    return SH1106_OK;
}

sh1106_t sh1106_init(void* sh_handle) {
    board_delay_ms(100);

    SH1106.CurrentX = 0;
    SH1106.CurrentY = 0;
    SH1106.Inverted = 0;
    SH1106.Initialized = 1;

    return sh1106_send_commands(sh_handle, init_cmds, sizeof(init_cmds));
}

sh1106_t sh1106_clear(void) {
    memset(SH1106.Buffer, 0, sizeof(SH1106.Buffer));
    SH1106.CurrentX = 0;
    SH1106.CurrentY = 0;
    return SH1106_OK;
}

sh1106_t sh1106_update_screen(void* sh_handle) {
    for (uint8_t page = 0; page < 8; page++) {
        if (sh1106_write_command(sh_handle, 0xB0 + page) != SH1106_OK) return SH1106_ERR;
        if (sh1106_write_command(sh_handle, 0x02) != SH1106_OK) return SH1106_ERR;
        if (sh1106_write_command(sh_handle, 0x10) != SH1106_OK) return SH1106_ERR;

        if (sh1106_write_data(sh_handle, &SH1106.Buffer[SH1106_WIDTH * page], SH1106_WIDTH) != SH1106_OK) {
            return SH1106_ERR;
        }
    }
    return SH1106_OK;
}

sh1106_t sh1106_draw_pixel(uint8_t x, uint8_t y, sh1106_color_t color) {
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
        return SH1106_ERR;
    }

    if (color == SH1106_COLOR_WHITE) {
        SH1106.Buffer[x + (y / 8) * SH1106_WIDTH] |= (1 << (y % 8));
    } else {
        SH1106.Buffer[x + (y / 8) * SH1106_WIDTH] &= ~(1 << (y % 8));
    }
    return SH1106_OK;
}

sh1106_t sh1106_gotoXY(uint8_t x, uint8_t y) {
    if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) {
        return SH1106_ERR;
    }

    SH1106.CurrentX = x;
    SH1106.CurrentY = y;
    return SH1106_OK;
}

sh1106_t sh1106_draw_line(void* sh_handle, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, sh1106_color_t color) {
    (void)sh_handle;
    int16_t dx = abs(x2 - x1);
    int16_t dy = -abs(y2 - y1);
    int16_t sx = x1 < x2 ? 1 : -1;
    int16_t sy = y1 < y2 ? 1 : -1;
    int16_t err = dx + dy;

    while (1) {
        sh1106_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int16_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
    return SH1106_OK;
}

sh1106_t sh1106_draw_rect(void* sh_handle, uint8_t x, uint8_t y, uint8_t w, uint8_t h, sh1106_color_t color) {
    (void)sh_handle;
    sh1106_draw_line(sh_handle, x, y, x + w, y, color);
    sh1106_draw_line(sh_handle, x + w, y, x + w, y + h, color);
    sh1106_draw_line(sh_handle, x + w, y + h, x, y + h, color);
    sh1106_draw_line(sh_handle, x, y + h, x, y, color);
    return SH1106_OK;
}

sh1106_t sh1106_draw_circle(void* sh_handle, uint8_t x0, uint8_t y0, uint8_t r, sh1106_color_t color) {
    (void)sh_handle;
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    sh1106_draw_pixel(x0, y0 + r, color);
    sh1106_draw_pixel(x0, y0 - r, color);
    sh1106_draw_pixel(x0 + r, y0, color);
    sh1106_draw_pixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        sh1106_draw_pixel(x0 + x, y0 + y, color);
        sh1106_draw_pixel(x0 - x, y0 + y, color);
        sh1106_draw_pixel(x0 + x, y0 - y, color);
        sh1106_draw_pixel(x0 - x, y0 - y, color);
        sh1106_draw_pixel(x0 + y, y0 + x, color);
        sh1106_draw_pixel(x0 - y, y0 + x, color);
        sh1106_draw_pixel(x0 + y, y0 - x, color);
        sh1106_draw_pixel(x0 - y, y0 - x, color);
    }
    return SH1106_OK;
}

char sh1106_putc(char ch, FontDef_t* Font, sh1106_color_t color) {
    uint32_t i, b, j;

    if (SH1106_WIDTH <= (SH1106.CurrentX + Font->FontWidth) ||
        SH1106_HEIGHT <= (SH1106.CurrentY + Font->FontHeight)) {
        return 0;
    }

    for (i = 0; i < Font->FontHeight; i++) {
        b = Font->data[(ch - 32) * Font->FontHeight + i];
        for (j = 0; j < Font->FontWidth; j++) {
            if ((b << j) & 0x8000) {
                sh1106_draw_pixel(SH1106.CurrentX + j, SH1106.CurrentY + i, (sh1106_color_t)color);
            } else {
                sh1106_draw_pixel(SH1106.CurrentX + j, SH1106.CurrentY + i, (sh1106_color_t)!color);
            }
        }
    }

    SH1106.CurrentX += Font->FontWidth;
    return ch;
}

char sh1106_puts(char* str, FontDef_t* Font, sh1106_color_t color) {
    while (*str) {
        if (sh1106_putc(*str, Font, color) != *str) {
            return *str;
        }
        str++;
    }
    return *str;
}

void sh1106_draw_bitmap(int16_t x, int16_t y, const unsigned char* bitmap, int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8;
    uint8_t byte = 0;

    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7) {
                byte <<= 1;
            } else {
                byte = (*(const unsigned char *)(&bitmap[j * byteWidth + i / 8]));
            }
            if (byte & 0x80) sh1106_draw_pixel(x + i, y, color);
        }
    }
}
