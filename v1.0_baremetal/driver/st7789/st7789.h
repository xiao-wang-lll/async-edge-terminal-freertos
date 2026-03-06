#ifndef __ST7789_H
#define __ST7789_H
#include "font.h"
#include "image.h"

#include "stdint.h"
#define ST7789_WIDTH 240
#define ST7789_HEIGHT 320
#define mkcolor(r,g,b) (((r & 0xF8) <<8 )|((g &0xFC )<<3) | (b>>3))


void st7789_init(void);
void st7789_init_display(void);
void st7789_write_register(uint8_t reg,uint8_t arr[],uint16_t length);
void st7789_fill_color(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
void st7789_write_string(uint16_t x, uint16_t y,const char *str, uint16_t color, uint16_t bg_color, const font_t *font);
void st7789_write_image(uint16_t x1,uint16_t y1,const image_t *image);


//新增的映射方法，映射方法成功的话走if否则就正常的走ascii的字符集偏移
static const uint8_t *ascii_get_model(const char ch,const font_t *font);

#endif
