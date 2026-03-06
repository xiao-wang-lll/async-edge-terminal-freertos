#include "st7789.h"
#include "font.h"
#include "string.h"
#include "image.h"


void error_page_display(const char *msg){
	
	
		const uint16_t color_bg=mkcolor(0,0,0);
		st7789_fill_color(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, color_bg);
		
		st7789_write_image(40,37,&image_error160);

		//Ê¹ÓÃ20ºÅ×ÖÌå
		uint16_t x1=0;
		uint8_t len=strlen(msg) * font20_mapple_bold.height /2;
		if(len<ST7789_WIDTH){
			x1=(ST7789_WIDTH - len)/2;
		}
		st7789_write_string(x1,254,msg,mkcolor(237,128,147),color_bg,&font20_mapple_bold);//x1=(240-£¨(20*strlen(err))/2£©)/2
		

}


