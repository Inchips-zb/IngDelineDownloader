#include "platform_api.h"
#include "peripheral_gpio.h"
#include "bsp_rtc.h"
#include <stdio.h>
#include <string.h>
#include "bsp_msg.h"
#include "bsp_key.h"



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
   if(v) UserQue_SendMsg(USER_MSG_KEY,NULL,v);

    GIO_ClearAllIntStatus();
    return 0;
}

