#ifndef __IMAGE_H
#define __IMAGE_H
#include "stdint.h"



typedef struct{

	const uint16_t width;
	const uint16_t height;
	const uint8_t *data;

}image_t;

extern const image_t image_meihua180;
extern const image_t image_error160;
extern const image_t image_wifi180;

extern const image_t image_icon_duoyun;
extern const image_t image_icon_leizhenyu;
extern const image_t image_icon_qing;
extern const image_t image_icon_wenduji;
extern const image_t image_icon_wifi;
extern const image_t image_icon_yintian;
extern const image_t image_icon_yueliang;
extern const image_t image_icon_zhongxue;
extern const image_t image_icon_zhongyu;
extern const image_t image_icon_na;




#endif
