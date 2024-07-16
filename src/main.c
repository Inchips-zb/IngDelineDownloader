#include <stdio.h>
#include <string.h>
#include "profile.h"
#include "ingsoc.h"
#include "platform_api.h"
#include "port_gen_os_driver.h"
#include "FreeRTOS.h"
#include "task.h"
#include "use_u8g2.h"
#include "delay.h"
#include "iic.h"
#include "qspi_flash.h"
#include "use_fatfs.h" 
#include "gui_menu.h"
#include "bsp_rtc.h"
#include "bsp_qdec.h"
#include "bsp_timer.h"
#include "bsp_msg.h"
#include "bsp_key.h"
#include "bsp_usb_msc.h"
#include "bsp_buzzer.h"
#include "uart_driver.h" 
#include "uart_buner.h"
#include "../data/setup_soc.cgen"

static uint32_t cb_hard_fault(hard_fault_info_t *info, void *_)
{
    platform_printf("HARDFAULT:\nPC : 0x%08X\nLR : 0x%08X\nPSR: 0x%08X\n"
                    "R0 : 0x%08X\nR1 : 0x%08X\nR2 : 0x%08X\nR3 : 0x%08X\n"
                    "R12: 0x%08X\n",
                    info->pc, info->lr, info->psr,
                    info->r0, info->r1, info->r2, info->r3, info->r12);
	while (apUART_Check_TXFIFO_EMPTY(APB_UART0) == 0) ;
	platform_reset();
    for (;;);
}

static uint32_t cb_assertion(assertion_info_t *info, void *_)
{
    platform_printf("[ASSERTION] @ %s:%d\n",
                    info->file_name,
                    info->line_no);
	while (apUART_Check_TXFIFO_EMPTY(APB_UART0) == 0) ;
	platform_reset();
    for (;;);
}

static uint32_t cb_heap_out_of_mem(uint32_t tag, void *_)
{
    platform_printf("[OOM] @ %d\n", tag);
	while (apUART_Check_TXFIFO_EMPTY(APB_UART0) == 0) ;
	platform_reset();
    for (;;);
}

#define PRINT_PORT    APB_UART0

uint32_t cb_putc(char *c, void *dummy)
{
    while (apUART_Check_TXFIFO_FULL(PRINT_PORT) == 1);
    UART_SendData(PRINT_PORT, (uint8_t)*c);
    return 0;
}

int fputc(int ch, FILE *f)
{
    cb_putc((char *)&ch, NULL);
    return ch;
}
static void setupBurnTrigIo(void);
void setup_peripherals(void)
{
    cube_setup_peripherals();
	setup_rtc();
	setup_oled_io_init();
	QDEC_Setup();
	setup_timer1();
	setup_peripherals_key();
	setup_buzzer();
	setupBurnTrigIo();

}

uint32_t on_lle_init(void *dummy, void *user_data)
{
    (void)(dummy);
    (void)(user_data);
    cube_on_lle_init();
    return 0;
}

uint32_t on_deep_sleep_wakeup(void *dummy, void *user_data)
{
    (void)(dummy);
    (void)(user_data);
    setup_peripherals();
    return 0;
}

uint32_t query_deep_sleep_allowed(void *dummy, void *user_data)
{
    (void)(dummy);
    (void)(user_data);
    // TODO: return 0 if deep sleep is not allowed now; else deep sleep is allowed
    return 0;
}

static const platform_evt_cb_table_t evt_cb_table =
{
    .callbacks = {
        [PLATFORM_CB_EVT_HARD_FAULT] = {
            .f = (f_platform_evt_cb)cb_hard_fault,
        },
        [PLATFORM_CB_EVT_ASSERTION] = {
            .f = (f_platform_evt_cb)cb_assertion,
        },
        [PLATFORM_CB_EVT_HEAP_OOM] = {
            .f = (f_platform_evt_cb)cb_heap_out_of_mem,
        },
        [PLATFORM_CB_EVT_PROFILE_INIT] = {
            .f = setup_profile,
        },
        [PLATFORM_CB_EVT_LLE_INIT] = {
            .f = on_lle_init,
        },
        [PLATFORM_CB_EVT_ON_DEEP_SLEEP_WAKEUP] = {
            .f = (f_platform_evt_cb)on_deep_sleep_wakeup,
        },
        [PLATFORM_CB_EVT_QUERY_DEEP_SLEEP_ALLOWED] = {
            .f = query_deep_sleep_allowed,
        },
        [PLATFORM_CB_EVT_PUTC] = {
            .f = (f_platform_evt_cb)cb_putc,
        },
    }
};


static void setupBurnTrigIo(void)
{
	SYSCTRL_ClearClkGateMulti(0 | (1 << SYSCTRL_ITEM_APB_GPIO1)
								 | (1 << SYSCTRL_ITEM_APB_PinCtrl));
	PINCTRL_SetPadMux(TRIG_RST, IO_SOURCE_GPIO);
	PINCTRL_SetPadMux(TRIG_BOOT, IO_SOURCE_GPIO);
	GIO_SetDirection((GIO_Index_t)TRIG_RST, GIO_DIR_OUTPUT);
	GIO_WriteValue((GIO_Index_t)TRIG_RST, 1);
	GIO_SetDirection((GIO_Index_t)TRIG_BOOT, GIO_DIR_OUTPUT);
	GIO_WriteValue((GIO_Index_t)TRIG_BOOT, 0);
}


uint8_t page_index = PAGE_SHOW_LOG;
static void u8g2_task(void *pdata)
{
	u8g2Init(&u8g2);
#ifdef TEST_FAFTS
	if((1 == GIO_ReadValue(KB_KEY_PSH)))
	{
	    page_index = PAGE_SHOW_LOG;
		load_downloader_cfg();
	}
	else{
		page_index = PAGE_SHOW_UDISK;
		bsp_usb_init();	
	    platform_printf("INIT USB\n");
	}
#endif
	platform_enable_irq(PLATFORM_CB_IRQ_GPIO0,1);//enable key isr
	uart_buner_start();
	uart_driver_init(APB_UART1, NULL, uart_buner_rx_data);
	setup_uart1();
	key_coder_buzzer_open(2700,1000);
    for (;;)
    {
		switch(page_index){
		
			case PAGE_SHOW_LOG:
			{
				page_index = pageDrowLogDisply(NULL);

			}break;
			case PAGE_SHOW_UDISK:
			{
				page_index = pageUdiskDisply(NULL);

			}break;
			case PAGE_SHOW_MAIN:
			{
				page_index = pageMainDisply(NULL);

			}break;			

			case PAGE_SHOW_BURN:
			{        
				page_index = pageBurningDisply(NULL);
			}break;		
			
			case PAGE_SHOW_CLOCK:
			{        
				page_index = pageClockDisply(NULL);
			}break;
			
			case PAGE_SHOW_FILES:
			{        
				page_index = pageFileBrowse(NULL);
			}break;	
			
			case PAGE_SHOW_SET:
			{        
				page_index = pageVoidDisply(NULL);
			}break;		
			
			case PAGE_SHOW_ERASE:
			{        
				page_index = eraseMainDisply(NULL);
			}break;		
			case PAGE_SHOW_BLE:
			{        
				page_index = pageVoidDisply(NULL);
			}break;		
			default:break;
		}
	}
}

// TODO: add RTOS source code to the project.
uintptr_t app_main()
{
	SYSCTRL_Init();
    cube_soc_init();
	platform_rt_rc_auto_tune2(32768);//Perform a low-speed clock calibration and calibrate the frequency to around 32768 Hz
    // setup event handlers
    platform_set_evt_callback_table(&evt_cb_table);
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
	
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    platform_config(PLATFORM_CFG_DEEP_SLEEP_TIME_REDUCTION, 4500);
#endif
    setup_peripherals();    

	UserQueInit();
	
    xTaskCreate(u8g2_task, 
				"u8g2", 
				configMINIMAL_STACK_SIZE*3, 
				NULL,
				5, 
				NULL);  	
	
	return (uintptr_t)os_impl_get_driver();
}

