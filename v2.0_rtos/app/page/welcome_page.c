#include "ui.h"

void welcome_page_display(void){
	
	
		const uint16_t color_bg=mkcolor(0,0,0);
		ui_fill_color(0, 0, UI_WIDTH - 1, UI_HEIGHT - 1, color_bg);
		ui_write_image(30,10,&image_meihua180);

		//ui_write_string(48,205,"梅花嵌入式",mkcolor(),color_bg,&font32);//如果直接按照position就是错了
		ui_write_string(40,205,"梅花嵌入式",mkcolor(237,128,147),color_bg,&font32_mapple_bold);//x1=(240-(32*5))/2
		ui_write_string(56,233,"天气时钟",mkcolor(86,165,255),color_bg,&font32_mapple_bold);//x1=(240-(32*4))/2=56


		//（240 -（（24*10）/2）/2）=60  //这里的字符要使用24高的字体
		ui_write_string(60,285,"loading...",mkcolor(255, 255, 255),color_bg,&font24_mapple_bold);//因为是字符所以是半宽

}

