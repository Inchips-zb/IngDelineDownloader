#include "platform_api.h"
#include "peripheral_timer.h"
#include "bsp_timer.h"
#include <stdio.h>
#include <string.h>
#include "bsp_qdec.h"
#include "bsp_buzzer.h"
static uint32_t timer1_isr(void *user_data);

void setup_timer1(void)
{
    SYSCTRL_ClearClkGate(SYSCTRL_ClkGate_APB_TMR1);
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
    // setup timer 1: 10Hz
    TMR_SetCMP(APB_TMR1, TMR_CLK_FREQ / 10);
    TMR_SetOpMode(APB_TMR1, TMR_CTL_OP_MODE_WRAPPING);
    TMR_IntEnable(APB_TMR1);
    TMR_Reload(APB_TMR1);
    TMR_Enable(APB_TMR1);

#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    // setup channel 0 of timer 1: 10Hz
    TMR_SetOpMode(APB_TMR1, 0, TMR_CTL_OP_MODE_32BIT_TIMER_x1, TMR_CLK_MODE_APB, 0);
    TMR_SetReload(APB_TMR1, 0, TMR_GetClk(APB_TMR1, 0) / 80);
    TMR_Enable(APB_TMR1, 0, 0xf);
    TMR_IntEnable(APB_TMR1, 0, 0xf);

#else
    #error unknown or unsupported chip family
#endif
    platform_set_irq_callback(PLATFORM_CB_IRQ_TIMER1, timer1_isr, NULL);
}
uint16_t count = 0;
static uint32_t timer1_isr(void *user_data)
{
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
    TMR_IntClr(APB_TMR1);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    TMR_IntClr(APB_TMR1, 0, 0xf);
#else
    #error unknown or unsupported chip family
#endif
	QDEC_data_process();
    return 0;
}