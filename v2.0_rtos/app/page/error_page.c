#include "ui.h"
#include "string.h"


void error_page_display(const char *msg){
	
	
		const uint16_t color_bg=mkcolor(0,0,0);
		ui_fill_color(0, 0, UI_WIDTH - 1, UI_HEIGHT - 1, color_bg);
		
		ui_write_image(40,37,&image_error160);

		//Ê¹ÓÃ20ºÅ×ÖÌå
		uint16_t x1=0;
		uint8_t len=strlen(msg) * font20_mapple_bold.height /2;
		if(len<UI_WIDTH){
			x1=(UI_WIDTH - len)/2;
		}
		ui_write_string(x1,254,msg,mkcolor(237,128,147),color_bg,&font20_mapple_bold);//x1=(240-£¨(20*strlen(err))/2£©)/2
		

}


