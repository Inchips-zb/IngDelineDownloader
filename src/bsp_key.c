#include "platform_api.h"
#include "peripheral_gpio.h"
#include "bsp_rtc.h"
#include <stdio.h>
#include <string.h>
#include "bsp_msg.h"
#include "bsp_key.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"


typedef struct{
	 uint8_t pin;
	 uint8_t last_value;
	 uint16_t pressed_evt_cnt;
	 uint16_t duration;
}key_process_t;

typedef struct{

	key_process_t key_process[3];
	f_key_event_cb key_event_cb;
}key_detect_t;

static const uint8_t pin_io[] = {GIO_GPIO_12,GIO_GPIO_7,GIO_GPIO_10};
static key_detect_t keyDetector;
static uint32_t gpio_isr(void *user_data);
	
void setup_peripherals_key(void)
{
    SYSCTRL_ClearClkGateMulti(0 |(1 << SYSCTRL_ITEM_APB_PinCtrl)
                                |(1 << SYSCTRL_ITEM_APB_GPIO0));   
	
    PINCTRL_SetPadMux(KB_KEY_BACK, IO_SOURCE_GPIO);
	GIO_SetDirection((GIO_Index_t)KB_KEY_BACK, GIO_DIR_INPUT);  
	PINCTRL_Pull(KB_KEY_BACK, PINCTRL_PULL_UP);
    PINCTRL_SetPadMux(KB_KEY_CONFIRM, IO_SOURCE_GPIO);
	GIO_SetDirection((GIO_Index_t)KB_KEY_CONFIRM, GIO_DIR_INPUT);  
	PINCTRL_Pull(KB_KEY_CONFIRM, PINCTRL_PULL_UP);	
	PINCTRL_SetPadMux(KB_KEY_PSH, IO_SOURCE_GPIO);
	GIO_SetDirection((GIO_Index_t)KB_KEY_PSH, GIO_DIR_INPUT);  
	PINCTRL_Pull(KB_KEY_PSH, PINCTRL_PULL_UP);
	GIO_ConfigIntSource(KB_KEY_BACK, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE, GIO_INT_EDGE);
	GIO_ConfigIntSource(KB_KEY_CONFIRM, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE, GIO_INT_EDGE);
    GIO_ConfigIntSource(KB_KEY_PSH, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE, GIO_INT_EDGE);		
	GIO_DebounceCtrl((1 << SYSCTRL_ITEM_APB_GPIO0), 4, GIO_DB_CLK_32K);
    GIO_DebounceEn(KB_KEY_BACK, 1);
	GIO_DebounceEn(KB_KEY_CONFIRM, 1);
	GIO_DebounceEn(KB_KEY_PSH, 1);
	platform_set_irq_callback(PLATFORM_CB_IRQ_GPIO0, gpio_isr, NULL);
	platform_enable_irq(PLATFORM_CB_IRQ_GPIO0,0);
}

static uint32_t gpio_isr(void *user_data)
{
    uint16_t v = 0;
    // report which keys are pressed
   if ((1 == GIO_GetIntStatus(KB_KEY_BACK)) && (0 == GIO_ReadValue(KB_KEY_BACK)))
        v |= 1;
   if ((1 == GIO_GetIntStatus(KB_KEY_CONFIRM)) && (0 == GIO_ReadValue(KB_KEY_CONFIRM)))
        v |= 2;
   if ((1 == GIO_GetIntStatus(KB_KEY_PSH)) && (0 == GIO_ReadValue(KB_KEY_PSH)))
        v |= 4;
//   printf("back:%d,%d\r\n",GIO_GetIntStatus(KB_KEY_BACK),GIO_ReadValue(KB_KEY_BACK));
//   printf("conf:%d,%d\r\n",GIO_GetIntStatus(KB_KEY_CONFIRM),GIO_ReadValue(KB_KEY_CONFIRM));
//      printf("psh:%d,%d\r\n",GIO_GetIntStatus(KB_KEY_PSH),GIO_ReadValue(KB_KEY_PSH));
   if(v) KeyQue_SendMsg(USER_MSG_KEY,NULL,v);

    GIO_ClearAllIntStatus();
    return 0;
}

static int sampling_timer_cb(uint16_t trig)
{
	for(uint8_t i=0;i<3;i++){
		if(!(0x01 & (trig>>i)))continue;
		uint8_t  value = GIO_ReadValue(keyDetector.key_process[i].pin);
		if (value == keyDetector.key_process[i].last_value)
		{
			keyDetector.key_process[i].duration++;
			if ((keyDetector.key_process[i].duration > 20) && (value != VALUE_PRESSED))
			{
				if (keyDetector.key_process[i].pressed_evt_cnt > 0)
					keyDetector.key_event_cb(keyDetector.key_process[i].pin, (key_press_event_t)keyDetector.key_process[i].pressed_evt_cnt);
				return 0;
			}
			return 1;
		}

		if (value != VALUE_PRESSED)
		{
			if (keyDetector.key_process[i].duration > 250)
			{
				keyDetector.key_event_cb(keyDetector.key_process[i].pin,KEY_LONG_PRESSED);
				keyDetector.key_process[i].pressed_evt_cnt = 0;
				keyDetector.key_process[i].duration = 0;
			}
			else
				keyDetector.key_process[i].pressed_evt_cnt++;
		}
		else
			keyDetector.key_process[i].duration = 0;

		keyDetector.key_process[i].last_value = value;
	}
    return 1;
}

static void timer_wakeup_task(void *pdata)
{
	UserQue_msg_t RecvMsg;
    for (;;)
    {
	    if(KeyQueMsgGet(&RecvMsg)) 
		if(0 == RecvMsg.length) break;
		platform_enable_irq(PLATFORM_CB_IRQ_GPIO0,0);
		for(uint8_t i=0;i<3;i++){
			keyDetector.key_process[i].last_value = VALUE_PRESSED;
			keyDetector.key_process[i].pressed_evt_cnt = KEY_LONG_PRESSED;
			keyDetector.key_process[i].duration = 0;
		}

        while (sampling_timer_cb(RecvMsg.length))
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
		platform_enable_irq(PLATFORM_CB_IRQ_GPIO0,1);
    }
}

void key_detect_init(f_key_event_cb cb)
{

	keyDetector.key_event_cb = cb;
	for(uint8_t i=0;i<3;i++){
		keyDetector.key_process[i].pin = pin_io[i];
		keyDetector.key_process[i].last_value = VALUE_PRESSED;
		keyDetector.key_process[i].pressed_evt_cnt = KEY_LONG_PRESSED;
		keyDetector.key_process[i].duration = 0;
	}	
    xTaskCreate(timer_wakeup_task,
               "b",
               150,
               NULL,
               (configMAX_PRIORITIES - 1),
               NULL);
}