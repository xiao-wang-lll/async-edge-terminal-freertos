#include "page.h"
#include "app.h"
#include "board.h"
#include "dht11.h"
#include "cpu_tick.h"
#include "stdio.h"

extern float rec_data[4];

int main(){

	board__lowlevel_init();
	board_init();
	
	
	welcome_page_display();
	
	wifi_init();
	wifi_page_display();
	wifi_wait_connect();
	


	main_page_display();
	
	
	while(1){

	main_loop();
	

	}


}







