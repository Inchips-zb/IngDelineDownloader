#include "platform_api.h"
#include "peripheral_qdec.h"
#include "bsp_qdec.h"
#include <stdio.h>
#include <string.h>
#include "bsp_msg.h"

#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
#ifndef SIMULATION
// make sure that PClk is <= slow_clk
static void QDEC_PclkCfg(void)
{
    uint32_t hclk = SYSCTRL_GetHClk();
    uint32_t slowClk = SYSCTRL_GetSlowClk();
    uint8_t div = hclk / slowClk;
    if (hclk % slowClk)
        div++;
    if (div <= 15)      // This div has 4 bits at most
        SYSCTRL_SetPClkDiv(div);
    else;               // ERROR
}

#define PSH        GIO_GPIO_10

static uint32_t gpio0_isr(void *user_data);
	
void setup_peripherals_psh(void)
{
    SYSCTRL_ClearClkGateMulti(0 |(1 << SYSCTRL_ITEM_APB_PinCtrl)
                                |(1 << SYSCTRL_ITEM_APB_GPIO0));   
	
    PINCTRL_SetPadMux(PSH, IO_SOURCE_GPIO);
	GIO_SetDirection((GIO_Index_t)PSH, GIO_DIR_INPUT);  
	PINCTRL_Pull(PSH, PINCTRL_PULL_DOWN);

	GIO_ConfigIntSource(PSH, GIO_INT_EN_LOGIC_LOW_OR_FALLING_EDGE, GIO_INT_EDGE);

	platform_set_irq_callback(PLATFORM_CB_IRQ_GPIO0, gpio0_isr, NULL);
}

static uint32_t gpio0_isr(void *user_data)
{
    uint16_t v = 0;
    // report which keys are pressed
	if ((1 == GIO_GetIntStatus(PSH)) && (0 == GIO_ReadValue(PSH)))
        v |= 1;

	printf("PSH:%d,%d\r\n",GIO_GetIntStatus(PSH),GIO_ReadValue(PSH));

    GIO_ClearAllIntStatus();
    return 0;
}


void QDEC_Setup(void)
{
    SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ClkGate_APB_PinCtrl)
                              | (1 << SYSCTRL_ClkGate_APB_QDEC));
    uint8_t div = SYSCTRL_GetPClkDiv();
    SYSCTRL_ClearClkGate(SYSCTRL_ITEM_APB_QDEC);
    PINCTRL_SelQDECIn(31, 11);
    SYSCTRL_SelectQDECClk(SYSCTRL_CLK_SLOW, 25);

    // PClk must be <= slow_clk
    // when configuring QDEC
    QDEC_PclkCfg();
    QDEC_EnableQdecDiv(QDEC_DIV_1024);
    QDEC_QdecCfg(63, 0);
    QDEC_ChannelEnable(1);
    // restore PClk
    SYSCTRL_SetPClkDiv(div);
	setup_peripherals_psh();
}

static uint16_t StepCal(uint16_t preData, uint16_t data, int8_t *dir)
{
    uint16_t step = data - preData;
    *dir = -1;
    if (step > 32768)
    {
        step = preData - data;
        *dir = 1;
    }
	
    return step;
}

int8_t QDEC_data_process(void)
{
	static uint16_t preData = 0;
	static uint16_t data = 0;
	static int last_step = 0;
	int8_t dir;
	data = QDEC_GetData();
	int step = StepCal(preData, data, &dir);
	preData = data;
	
	if (0 == step) return 0;
	int8_t ret = (int8_t)(dir);
	if (step <= 127)
	{	last_step += step;
		if(0 == last_step % 2)
			UserQue_SendMsg(USER_MSG_CODER,NULL,ret);
		return ret;
	}
	return 0;
}
#endif
#endif