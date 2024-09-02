#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */
#include <stdint.h>
#include "ingsoc.h"

#define KB_KEY_BACK           GIO_GPIO_12
#define KB_KEY_CONFIRM        GIO_GPIO_7
#define KB_KEY_PSH        	  GIO_GPIO_10

#ifndef VALUE_PRESSED
#define VALUE_PRESSED     0
#endif

typedef enum
{
	KEY_LONG_PRESSED = 0,
    KEY_PRESSED_SINGLE,
    KEY_PRESSED_DOUBLE,
    KEY_PRESSED_TRIPPLE
	
} key_press_event_t;

typedef void (*f_key_event_cb)(uint8_t key, key_press_event_t evt); 

extern void key_detect_init(f_key_event_cb cb);
extern void setup_peripherals_key(void);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif

