#include "ui.h"
#include "app.h"
#include "string.h"
#include "stdio.h"
#include "page.h"
#include "rtc.h"

static const uint16_t color_bg_time = mkcolor(248,248,248);
static const uint16_t color_bg_inner = mkcolor(136,217,234);
static const uint16_t color_bg_outdoor = mkcolor(254,135,75);



//默认情况---非连接数据下的初始界面
void main_page_display(void){
	
	
		const uint16_t color_bg=mkcolor(0,0,0);
		ui_fill_color(0, 0, UI_WIDTH - 1, UI_HEIGHT - 1, color_bg);
		
	
	
		do{
			ui_fill_color(15, 15, 225 - 1, 155 - 1, color_bg_time);
			ui_write_image(23,20,&image_icon_wifi);
			main_page_redraw_wifi_ssid(WIFI_SSID);
			
			//(240-((76*5)/2))/2=25---时间
			ui_write_string(25,37,"--:--",mkcolor(0, 0, 0),color_bg_time,&font76_mapple_extrabold);
			ui_write_string(207,96,"--",mkcolor(0, 0, 0),color_bg_time,&font16_mapple);
			
			//(240-((20*17)/2))/2=---日期
			ui_write_string(35,121,"----/--/-- 星期-",mkcolor(143, 143, 143),color_bg_time,&font20_mapple_bold);
			
			
		
		}while(0);
		
		
		
		
		
		
		do{
			ui_fill_color(15, 165, 115 - 1, 305 - 1, mkcolor(136,217,234));
		
			ui_write_string(19,170,"室内环境",mkcolor(0, 0, 0),color_bg_inner,&font20_mapple_bold);
			
			ui_write_string(83,191,"℃",mkcolor(0, 0, 0),color_bg_inner,&font32_mapple_bold);
			ui_write_string(91,262,"%",mkcolor(0, 0, 0),color_bg_inner,&font32_mapple_bold);
			main_page_redraw_inner_temperature(-100.00);
			main_page_redraw_inner_humidity(-100.00);

		
		}while(0);
		
		
		
		
		 
		
		do{
			ui_fill_color(125, 165, 225 - 1, 305 - 1, color_bg_outdoor);
			ui_write_image(139,239,&image_icon_wenduji);
			main_page_redraw_outdoor_weather_icon(0);	
			
			ui_write_string(127,170,"吕梁",mkcolor(0, 0, 0),color_bg_outdoor,&font20_mapple_bold);
			
			ui_write_string(192,190,"℃",mkcolor(0, 0, 0),color_bg_outdoor,&font32_mapple_bold);
			main_page_redraw_outdoor_temperature(-100.00);
			
		
		
		}while(0);


}

//ssid的redraw
void main_page_redraw_wifi_ssid(const char *ssid){

		
			//这里采用的是格式化打印的方式
			//%20s会自动将打印的字符在20位中居右，然后超出20位的会低位截断
			//这里的21和%20s是有讲究的，其实就是模块1的背景宽是210，那么减去wifi标志的50宽就是最多容纳160个像素
			//而这里的字符的半宽占8个像素，所以最多20个字符，所以是%20s，str[21]表示末尾的'\0'
			char str[21];
			snprintf(str,sizeof(str),"%20s",ssid);//
			ui_write_string(50,23 ,str,mkcolor(143, 143, 143),color_bg_time,&font16_mapple);

}








//时间---redraw
void main_page_redraw_time(rtc_date_time_t *time){

		char str[6]={'-','-',':','-','-'};
		char mmsa= (time->second % 2 == 0) ? ':':' ';
		snprintf(str,sizeof(str),"%02u%c%02u",time->hour,mmsa ,time->minute);
		
		ui_write_string(25,37,str,mkcolor(0, 0, 0),color_bg_time,&font76_mapple_extrabold);
		
		char str_sec[3]={'-','-'};
		snprintf(str_sec,sizeof(str_sec),"%02u",time->second);
		ui_write_string(207,96,str_sec,mkcolor(0, 0, 0),color_bg_time,&font16_mapple);

}

//日期---redraw
void main_page_redraw_date(rtc_date_time_t *date){

		char str[18]={0};
		
		snprintf(str,sizeof(str),"%04u/%02u/%02u 星期%s",date->year,date->month,date->day,
			date->weekday == 1 ? "一" :
        date->weekday == 2 ? "二" :
        date->weekday == 3 ? "三" :
        date->weekday == 4 ? "四" :
        date->weekday == 5 ? "五" :
        date->weekday == 6 ? "六" :
        date->weekday == 7 ? "天" : "-"
		);
		
		ui_write_string(35,121,str,mkcolor(143, 143, 143),color_bg_time,&font20_mapple_bold);

}




//室内环境的温度redraw
void main_page_redraw_inner_temperature(float temperature){

		char str[4]={'-','-'};
		if(temperature >= -10.0f && temperature <= 100.0f){
				snprintf(str,sizeof(str),"%2.0f",temperature);//允许存入负数
		}
		ui_write_string(30,192,str,mkcolor(0, 0, 0),color_bg_inner,&font54_mapple_bold);


}

//室内环境的湿度。注意湿度不能小于0
void main_page_redraw_inner_humidity(float humidity){

		char str[4]={'-','-'};
		if(humidity >= 0.0f && humidity <= 100.0f){
				snprintf(str,sizeof(str),"%2.0f",humidity);//不允许存入负数
		}
		ui_write_string(25,239,str,mkcolor(0, 0, 0),color_bg_inner,&font64_mapple_extrabold);


}

//城市的更新
void main_page_redraw_outdoor_city(const char *city){

		char str[10]="北京";//注意汉字占两个字节，如果是内蒙古等就超6字节了所以得设置大点
		if(city){
				snprintf(str,sizeof(str),"%s",city);//不允许存入负数
		}
		ui_write_string(127,170,str,mkcolor(0, 0, 0),color_bg_outdoor,&font20_mapple_bold);

}


//天气
void main_page_redraw_outdoor_temperature(float temperature){

	char str[4]={'-','-'};
		if(temperature >= -10.0f && temperature <= 100.0f){
				snprintf(str,sizeof(str),"%2.0f",temperature);//允许存入负数
		}
		ui_write_string(140,189 ,str,mkcolor(0, 0, 0),color_bg_outdoor,&font54_mapple_bold);


}

//icon使用传回来的code去显示对应的天气icon
void main_page_redraw_outdoor_weather_icon(const int code){
		
		const image_t *icon;
		if (code == 0 || code == 2 || code == 38)
        icon = &image_icon_qing;
    else if (code == 1 || code == 3)
        icon = &image_icon_yueliang;
    else if (code == 4 || code == 9)
        icon = &image_icon_yintian;
    else if (code == 5 || code == 6 || code == 7 || code == 8)
        icon = &image_icon_duoyun;
    else if (code == 10 || code == 13 || code == 14 || code == 15 || code == 16 || code == 17 || code == 18 || code == 19)
        icon = &image_icon_zhongyu;
    else if (code == 11 || code == 12)
        icon = &image_icon_leizhenyu;
    else if (code == 20 || code == 21 || code == 22 || code == 23 || code == 24 || code == 25)
        icon = &image_icon_zhongxue;
    else // 扬沙、龙卷风等
        icon = &image_icon_na;
	


		ui_write_image(166,240,icon);
}

