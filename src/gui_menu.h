#ifndef _GUI_MENU_H
#define _GUI_MENU_H
 
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "u8g2.h"
#include "bsp_rtc.h"
 
#define TRIG_RST          GIO_GPIO_23
#define TRIG_BOOT         GIO_GPIO_36

enum{
	PAGE_SHOW_LOG = 0,
	PAGE_SHOW_MAIN,
	PAGE_SHOW_UDISK,
	PAGE_SHOW_BURN,
	PAGE_SHOW_CLOCK,
	PAGE_SHOW_FILES,
	PAGE_SHOW_SET,
	PAGE_SHOW_ERASE,
	PAGE_SHOW_BLE,
	PAGE_SHOW_END
};

extern u8g2_t u8g2;  

extern uint8_t pageDrowLogDisply(void *user_data);	
extern uint8_t pageLoadingDisply(void *user_data);
extern uint8_t pageClockDisply(void *user_data);
extern uint8_t pageBurningDisply(void *user_data);
extern uint8_t pageMainDisply(void *user_data);
extern uint8_t eraseMainDisply(void *user_data);
extern uint8_t pageVoidDisply(void *user_data);
extern uint8_t pageFileBrowse(void *user_data);
extern uint8_t pageUdiskDisply(void *user_data);
#endif // !_GUI_MENU_H