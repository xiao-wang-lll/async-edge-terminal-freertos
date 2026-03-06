#ifndef __APP_H
#define __APP_H
#include "stdint.h"


#define APP_VERSION "v1.0"
#define WIFI_SSID   "vivo"
#define WIFI_PASSWD "wangshuanger12345"
#define WEATHER_URL "https://api.seniverse.com/v3/weather/now.json?key=S75RkjmBtcG-hHVnn&location=Lvliang&language=en&unit=c"

void wifi_init(void);
void wifi_wait_connect(void);

void main_loop(void);
void main_loop_init(void);

#endif
