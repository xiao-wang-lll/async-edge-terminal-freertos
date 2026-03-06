#include "st7789.h"
#include "font.h"
#include "string.h"
#include "image.h"
#include "app.h"


void wifi_page_display(void){
	
		static const char *ssid=WIFI_SSID;
		uint16_t x1=0;
		uint8_t len=strlen(ssid) * font20_mapple_bold.height /2;
		if(len<ST7789_WIDTH){
			x1=(ST7789_WIDTH - len)/2;
		}
	

		const uint16_t color_bg=mkcolor(0,0,0);
		st7789_fill_color(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, color_bg);
		
		st7789_write_image(30,15,&image_wifi180);
		//(240-((32*4)/2))/2
		st7789_write_string(88,191,"WiFi",mkcolor(0, 255, 234),color_bg,&font32_mapple_bold);
												
		st7789_write_string(x1,231,ssid,mkcolor(255, 255, 255),color_bg,&font20_mapple_bold);
		//(240-(24*3))/2								
		st7789_write_string(84,263,"Á¬½ÓÖÐ",mkcolor(148, 198, 255),color_bg,&font24_mapple_bold);
			

}


