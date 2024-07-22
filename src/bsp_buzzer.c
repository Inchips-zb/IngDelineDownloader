#include "platform_api.h"
#include "peripheral_gpio.h"
#include "bsp_buzzer.h"
#include <stdio.h>
#include <string.h>
#include "bsp_msg.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device_info.h"

#define BUZZ_PIN        GIO_GPIO_6
volatile uint8_t buzzer_is_buse = 0;

void setup_buzzer(void)
{
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_PWM));
    PINCTRL_SetGeneralPadMode(BUZZ_PIN, IO_MODE_PWM, 4, 0);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    SYSCTRL_ClearClkGateMulti( (1 << SYSCTRL_ClkGate_APB_PinCtrl)
                                    | (1 << SYSCTRL_ClkGate_APB_PWM));
    SYSCTRL_SelectPWMClk(SYSCTRL_CLK_32k);
    PINCTRL_SetPadMux(BUZZ_PIN, IO_SOURCE_PWM0_A);
    PWM_SetMode(0, PWM_WORK_MODE_UP_WITHOUT_DIED_ZONE);
    PWM_SetMask(0, 0, 0);
	PWM_HaltCtrlEnable(0, 1);
	PWM_Enable(0, 0);
#else
    #error unknown or unsupported chip family
#endif
}



static void set_buzzer_freq(uint16_t freq)
{
	dev_info_t *pDev = get_device_informs();
	if(1 != pDev->beer_en) return;
	if(freq)
	{
		uint32_t pera = PWM_CLOCK_FREQ / freq;
		uint32_t high = pera > 1000 ? pera / 100 * 40 : pera * 40 / 100;
		PINCTRL_SetPadMux(BUZZ_PIN, IO_SOURCE_PWM0_A);
		PWM_HaltCtrlEnable(0, 1);
		PWM_Enable(0, 0);
		PWM_SetPeraThreshold(0, pera);
		PWM_SetHighThreshold(0, 0, high);
		PWM_HaltCtrlEnable(0, 0); 
		PWM_Enable(0, 1);
	}
	else
	{
	    PINCTRL_SetPadMux(BUZZ_PIN, IO_SOURCE_GPIO);
	    GIO_SetDirection((GIO_Index_t)BUZZ_PIN, GIO_DIR_OUTPUT);  
		GIO_WriteValue((GIO_Index_t)BUZZ_PIN,0);
	}
}


void key_coder_buzzer_open(uint16_t freq,uint16_t time)
{

	set_buzzer_freq(freq);
	vTaskDelay(pdMS_TO_TICKS(time));
	set_buzzer_freq(0);
}


void burn_cmpl_buzzer_open(uint16_t freq)
{
	set_buzzer_freq(freq);
	vTaskDelay(pdMS_TO_TICKS(50));
	set_buzzer_freq(0);
	vTaskDelay(pdMS_TO_TICKS(40));
	set_buzzer_freq(freq);
	vTaskDelay(pdMS_TO_TICKS(50));
	set_buzzer_freq(0);
}
