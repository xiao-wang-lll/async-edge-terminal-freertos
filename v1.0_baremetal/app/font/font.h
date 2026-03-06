#ifndef __FONT_H
#define __FONT_H

#include "stdint.h"
#include "stdbool.h"


typedef struct{

	const char *name;
	const uint8_t *model;

}font_chinese_t;



typedef struct{

	uint16_t height;
	const uint8_t *ascii_model;
	const	char *ascii_map;
	const font_chinese_t *chinese_model;

}font_t;






extern const font_t font16_mapple;
extern const font_t font20_mapple_bold;
extern const font_t font24_mapple_bold;
extern const font_t font32_mapple_bold;
extern const font_t font54_mapple_bold;
extern const font_t font54_mapple_semibold;
extern const font_t font64_mapple_extrabold;
extern const font_t font76_mapple_extrabold;

#endif
