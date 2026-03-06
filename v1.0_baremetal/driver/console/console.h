#ifndef __CONSOLE_H
#define __CONSOLE_H

#include "stdint.h"

typedef void (*console_received_func_t)(uint8_t data);


void console_init(void);

void console_write(const char c[]);

void console_received_register(console_received_func_t func);


#endif
