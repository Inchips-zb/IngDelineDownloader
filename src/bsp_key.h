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

void setup_peripherals_key(void);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif

