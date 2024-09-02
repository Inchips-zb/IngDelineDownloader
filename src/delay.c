#include <stdio.h>
#include <string.h>
#include "profile.h"
#include "ingsoc.h"
#include "platform_api.h"
#include "FreeRTOS.h"
#include "task.h"

#define US_RTOS_DELAY

static void nop(uint32_t n)
{
    uint32_t i;
    for(i=0;i<n;i++){
        __asm volatile ("nop");
    }
}

void delay_us(int us){
	
	uint64_t t = us + platform_get_us_time();
	while(t > platform_get_us_time())
	{
		__asm volatile ("nop");
	}
}

#ifdef US_RTOS_DELAY

void delay_ms(int ms){

  vTaskDelay(pdMS_TO_TICKS(ms));
}

#else

void delay_ms(int ms){

	uint64_t t = ms*1000 + platform_get_us_time();
	while(t > platform_get_us_time())
	{
		__asm volatile ("nop");
	}
}

#endif
