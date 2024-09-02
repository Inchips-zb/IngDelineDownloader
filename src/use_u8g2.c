#include <stdio.h>
#include <string.h>
#include "u8g2.h"
#include "delay.h"
#include "ingsoc.h"
#include "platform_api.h"
#include "iic.h"


#define OLED_SCL_PIN 	9
#define OLED_SDA_PIN 	8

static u8g2_t sta_u8g2;


static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
	case U8X8_MSG_DELAY_MILLI: // delay arg_int * 1 milli second
        delay_ms(arg_int);
        break;
#ifndef USE_HARD_IIC	
    case U8X8_MSG_DELAY_100NANO: // delay arg_int * 100 nano seconds
        __NOP();
        break;
    case U8X8_MSG_DELAY_10MICRO: // delay arg_int * 10 micro seconds
        for (uint16_t n = 0; n < 320; n++)
        {
            __NOP();
        }
        break;

    case U8X8_MSG_DELAY_I2C: // arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
        delay_us(1);
        break;                    // arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_I2C_CLOCK: // arg_int=0: Output low at I2C clock pin

	    GIO_WriteValue(OLED_SCL_PIN,arg_int);
        break;                    // arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA:  // arg_int=0: Output low at I2C data pin

	     GIO_WriteValue(OLED_SDA_PIN,arg_int);

        break;                    // arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, /* get menu select pin state */0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, /* get menu next pin state */0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, /* get menu home pin state */0);
        break;
#endif	
    default:
        u8x8_SetGPIOResult(u8x8, 1); // default return value
        break;
    }
    return 1;
}
void i2c0_send_by_dma(uint8_t addr,uint8_t *data,uint16_t len);

#ifdef USE_HARD_IIC	
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	static uint8_t buffer[32];		/* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
	static uint8_t buf_idx;
	uint8_t *data;

	switch(msg)
	{
		case U8X8_MSG_BYTE_SEND:
		  data = (uint8_t *)arg_ptr;   
		  while( arg_int > 0 ){
					buffer[buf_idx++] = *data;
					data++;
					arg_int--;
				}      
		break;
				
		case U8X8_MSG_BYTE_INIT:
		  /* add your custom code to init i2c subsystem */
		break;
			
		case U8X8_MSG_BYTE_START_TRANSFER:
		  buf_idx = 0;
		break;
			
		case U8X8_MSG_BYTE_END_TRANSFER:
#ifdef USE_DMA			
			i2c0_send_by_dma(u8x8_GetI2CAddress(u8x8) >> 1,buffer,buf_idx);
#else		
		    i2c_write(I2C_PORT_0,u8x8_GetI2CAddress(u8x8) >> 1,buffer,buf_idx);
#endif
		break;
			
		default:
		  return 0;
	}
	return 1;
}

#endif

u8g2_t *get_u8g2(void){

	return &sta_u8g2;
}



#ifdef USE_DMA	
static void setup_peripherals_dma_module(void)
{
    SYSCTRL_ClearClkGateMulti(1 << SYSCTRL_ClkGate_APB_DMA);
    DMA_Reset(1);
    DMA_Reset(0);
}

#define I2C_DMA_TX_CHANNEL   (0)//DMA channel 0

DMA_Descriptor descriptor __attribute__((aligned (8)));
void peripherals_i2c_dma_to_txfifo(int channel_id, void *src, int size)
{
    descriptor.Next = (DMA_Descriptor *)0;
    DMA_PrepareMem2Peripheral(&descriptor,SYSCTRL_DMA_I2C0,src,size,DMA_ADDRESS_INC,0);
}
static uint8_t dma_send[128] __attribute__((aligned (4)));
volatile uint8_t send_cmpl = 1;

void i2c0_send_by_dma(uint8_t addr,uint8_t *data,uint16_t len)
{

//  printf("tran:%d\r\n",len);
//    delay_ms(1);
    delay_us(600);
    while(0 == send_cmpl) delay_ms(1);
    send_cmpl = 0;
  
    memcpy(dma_send,data,len);
    descriptor.SrcAddr = (uint32_t)dma_send;
    descriptor.TranSize = len;
    DMA_EnableChannel(I2C_DMA_TX_CHANNEL, &descriptor);
//  peripherals_i2c_dma_to_txfifo(I2C_DMA_TX_CHANNEL, dma_send, len);
    I2C_Config(APB_I2C0,I2C_ROLE_MASTER,I2C_ADDRESSING_MODE_07BIT,addr);
    I2C_CtrlUpdateDirection(APB_I2C0,I2C_TRANSACTION_MASTER2SLAVE);
    I2C_CtrlUpdateDataCnt(APB_I2C0, len);
    I2C_DmaEnable(APB_I2C0,1);
    
    I2C_CommandWrite(APB_I2C0, I2C_COMMAND_ISSUE_DATA_TRANSACTION);
}

static uint32_t DMA_cb_isr(void *user_data)
{
    uint32_t state = DMA_GetChannelIntState(I2C_DMA_TX_CHANNEL);
    DMA_ClearChannelIntState(I2C_DMA_TX_CHANNEL, state);

//	printf("dma:%x\r\n",state);
    send_cmpl = 1;
    return 0;
}
#endif



void setup_oled_io_init(void)
{
	SYSCTRL_ClearClkGateMulti((1 << SYSCTRL_ITEM_APB_PinCtrl)
						#ifdef USE_HARD_IIC		
								| (1 << SYSCTRL_ITEM_APB_SysCtrl)
								| (1 << SYSCTRL_ITEM_APB_I2C0)
						#else	
								| (1 << SYSCTRL_ITEM_APB_GPIO1)
						#endif
	);
#ifdef USE_HARD_IIC	
	PINCTRL_SelI2cIn(I2C_PORT_0, OLED_SCL_PIN, OLED_SDA_PIN);
	PINCTRL_SetPadMux(OLED_SCL_PIN, IO_SOURCE_I2C0_SCL_OUT);
	PINCTRL_SetPadMux(OLED_SDA_PIN, IO_SOURCE_I2C0_SDA_OUT);
	SYSCTRL_ResetBlock(SYSCTRL_ITEM_APB_I2C0);
	SYSCTRL_ReleaseBlock(SYSCTRL_ITEM_APB_I2C0);	
	i2c_init(I2C_PORT_0);
	I2C_Enable(APB_I2C0,1);
	I2C_ConfigClkFrequency(APB_I2C0,I2C_CLOCKFREQUENY_FASTMODE_PLUS);
#ifdef USE_DMA	
    setup_peripherals_dma_module();
	platform_set_irq_callback(PLATFORM_CB_IRQ_DMA, DMA_cb_isr, 0);
    
    //DMA Init only use 3 len, burist size is 0
    peripherals_i2c_dma_to_txfifo(I2C_DMA_TX_CHANNEL, dma_send, 3);
  
#endif	
#else	
	PINCTRL_SetPadMux(OLED_SCL_PIN, IO_SOURCE_GPIO);
	PINCTRL_SetPadMux(OLED_SDA_PIN, IO_SOURCE_GPIO);
	GIO_SetDirection((GIO_Index_t)OLED_SCL_PIN, GIO_DIR_OUTPUT);
	GIO_WriteValue((GIO_Index_t)OLED_SCL_PIN, 1);
	GIO_SetDirection((GIO_Index_t)OLED_SDA_PIN, GIO_DIR_OUTPUT);
	GIO_WriteValue((GIO_Index_t)OLED_SDA_PIN, 1);
#endif	
	PINCTRL_Pull(OLED_SCL_PIN,PINCTRL_PULL_UP);
	PINCTRL_Pull(OLED_SDA_PIN,PINCTRL_PULL_UP);
	
}

void u8g2Init(u8g2_t *u8g2)
{
	
#ifdef USE_HARD_IIC		
	//u8g2_Setup_sh1107_i2c_seeed_128x128_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
	//u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
	u8g2_Setup_sh1106_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
#else
	//u8g2_Setup_sh1107_i2c_seeed_128x128_f(u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay);
	//u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay);
	u8g2_Setup_sh1106_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay);
#endif
	u8g2_InitDisplay(u8g2); 
	u8g2_SetPowerSave(u8g2, 0); 
	u8g2_ClearBuffer(u8g2);
	u8g2_SendBuffer(u8g2);
}


