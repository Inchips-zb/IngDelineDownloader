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