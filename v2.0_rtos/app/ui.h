#ifndef __UI_H
#define __UI_H
#include "stdint.h"
#include "image.h"
#include "font.h"
#include "st7789.h"

#define UI_WIDTH ST7789_WIDTH
#define UI_HEIGHT ST7789_HEIGHT
#define mkcolor(r,g,b) (((r & 0xF8) <<8 )|((g &0xFC )<<3) | (b>>3))

void ui_init(void);
void ui_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

void ui_write_string(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bg_color, const font_t *font);
void ui_write_image(uint16_t x, uint16_t y, const image_t *image);


#endif
