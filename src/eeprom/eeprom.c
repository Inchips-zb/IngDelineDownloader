#include "eeprom.h"
#include "iic.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>


At24cObjectType At24C128CMoudle;

void I2C_SET_CLK_SPEED(I2C_TypeDef *BASE, uint32_t clock_hz)
{
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)	
    uint32_t clock_count = OSC_CLK_FREQ / clock_hz;
    BASE->I2C_TIMING0.CLR = (0x3ff << 16) | (0x3ff);
    BASE->I2C_TIMING0.SET = (clock_count << 16) | (clock_count >> 1);
    BASE->I2C_TIMING1.CLR = (0x3ff << 16) | (0x3ff);
    BASE->I2C_TIMING1.SET = (clock_count << 16) | (clock_count >> 1);
#endif
}

#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
#define I2C_SCL         GIO_GPIO_12
#define I2C_SDA         GIO_GPIO_13
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
#define I2C_SCL         GIO_GPIO_14
#define I2C_SDA         GIO_GPIO_13
#else
    #error unknown or unsupported chip family
#endif
static void setup_peripherals_i2c_pin(void)
{
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
    SYSCTRL_ClearClkGateMulti( (1 << SYSCTRL_ClkGate_APB_I2C1)
                              |(1 << SYSCTRL_ClkGate_APB_GPIO)
                              |(1 << SYSCTRL_ClkGate_APB_PinCtrl));
    PINCTRL_SetPadMux(I2C_SCL, IO_SOURCE_I2C0_SCL_O);
    PINCTRL_SetPadMux(I2C_SDA, IO_SOURCE_I2C0_SDO);
    PINCTRL_SelI2cSclIn(I2C_PORT_1, I2C_SCL);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    SYSCTRL_ClearClkGateMulti(    (1 << SYSCTRL_ITEM_APB_I2C1)
                                  | (1 << SYSCTRL_ITEM_APB_SysCtrl)
                                  | (1 << SYSCTRL_ITEM_APB_PinCtrl)
                                  | (1 << SYSCTRL_ITEM_APB_GPIO1)
                                  | (1 << SYSCTRL_ITEM_APB_GPIO0));

    PINCTRL_SelI2cIn(I2C_PORT_1, I2C_SCL, I2C_SDA);
	PINCTRL_Pull(I2C_SCL,PINCTRL_PULL_UP);
	PINCTRL_Pull(I2C_SDA,PINCTRL_PULL_UP);
#else
    #error unknown or unsupported chip family
#endif
}

static void setup_peripherals_i2c_module(void)
{
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)    
    i2c_init(I2C_PORT_1);
    I2C_SET_CLK_SPEED(APB_I2C1,400);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    //init I2C module
    I2C_Config(APB_I2C1,I2C_ROLE_MASTER,I2C_ADDRESSING_MODE_07BIT,EEPROM_CHIP_ADDR);
    I2C_ConfigClkFrequency(APB_I2C1,I2C_CLOCKFREQUENY_FASTMODE_PLUS);
    I2C_Enable(APB_I2C1,1);
#endif
}

static void Eeprom_Delay(volatile uint32_t nMs)
{
    vTaskDelay(nMs);
//int i = 180000;
//while(i--);
}

static int At24x_read(struct At24cObject *at,uint16_t regAddress,uint8_t *rData,uint16_t rSize)
{
    uint16_t cSize;
    uint8_t cmd[2];
       
    if(at->memAddLength==AT24C8BitMemAdd)
    {
        cSize=1;
        cmd[0]=(uint8_t)regAddress;
    }
    else
    {
        cSize=2;
        cmd[0]=(uint8_t)(regAddress>>8);
        cmd[1]=(uint8_t)regAddress;
    }

    return i2c_read(EEPROM_I2C_PORT, at->devAddress,cmd, cSize, rData, rSize) == 0 ? 0 : -1;
}
static int At24x_write(struct At24cObject *at,uint16_t regAddress,uint8_t *wData,uint16_t wSize)
{
    uint8_t data[wSize + 2];
    uint16_t tSize;
    if(at->memAddLength == AT24C8BitMemAdd)
    {
        data[0] = (uint8_t)regAddress;
        tSize = wSize + 1;
        memcpy(data + 1, wData, wSize);
    }
    else
    {
        data[0]=(uint8_t)(regAddress>>8);
        data[1]=(uint8_t)regAddress;
        tSize = wSize + 2;
        memcpy(data + 2, wData, wSize);
    }
   
    return i2c_write(EEPROM_I2C_PORT, at->devAddress, data, tSize) == 0 ? 0 : -1;
}

void eeprom_init(void)
{
    setup_peripherals_i2c_pin();
    setup_peripherals_i2c_module();

    At24cxxInitialization(
                            &At24C128CMoudle,
                            EEPROM_CHIP_ADDR,
                            AT24C128C,
                            At24x_read,
                            At24x_write,
                            Eeprom_Delay);                            
}

int eeprom_app_reads(At24cObjectType *at,int start_addr,unsigned char *p_buf,int len)
{
    uint16_t i = 0;
	uint16_t head;
	uint16_t left;
	uint16_t tail;
	uint32_t r_num = 0;
    uint32_t page_saze = at->pageSize;
    if((len + start_addr)/page_saze == start_addr/page_saze)
    {
        return (len == ReadBytesFromAT24CXX( at,start_addr,p_buf,len))?0:1;
    }
    else
    {
        head = (start_addr / page_saze+1) * page_saze - start_addr;	//开始页剩余待写入字节数
		left = len - head;													//除去开始页剩下字节数
		tail=left-left/page_saze*page_saze;				    //末页待写入字节数
	//	printf("page:%d,head:%d,left:%d,tail:%d\n",at->pageSize,head,left,tail);
        r_num =  ReadBytesFromAT24CXX( at,start_addr,p_buf,head);
        for(i = 0;i < left/page_saze;i++)
        {
             r_num +=  ReadBytesFromAT24CXX(at,start_addr+head+i*page_saze,p_buf+head+i*page_saze,page_saze);
        }
		if(tail) r_num += ReadBytesFromAT24CXX(at,start_addr+head+i*page_saze,p_buf+head+i*page_saze,tail);
    }	
    return (len ==r_num)?0:1;
}

int eeprom_app_writes(At24cObjectType *at,int start_addr,unsigned char *p_buf,int len)
{
    uint16_t i = 0;
	uint16_t head;
	uint16_t left;
	uint16_t tail;
    uint32_t page_saze = at->pageSize;
    if((len + start_addr)/page_saze == start_addr/page_saze)
    {
	//	printf("w1\n");
        return (len == WriteBytesToAT24CXX( at,start_addr,p_buf,len))?0:1;
    }
    else
    {
        head = (start_addr / page_saze+1) * page_saze - start_addr;	//开始页剩余待写入字节数
		left = len - head;													//除去开始页剩下字节数
		tail=left-left/page_saze*page_saze;				    //末页待写入字节数
	//	printf("page:%d,head:%d,left:%d,tail:%d\n",at->pageSize,head,left,tail);
        WriteBytesToAT24CXX( at,start_addr,p_buf,head);
        for(i = 0;i < left/page_saze;i++)
        {
             WriteBytesToAT24CXX(at,start_addr+head+i*page_saze,p_buf+head+i*page_saze,page_saze);
        }
		if(tail) WriteBytesToAT24CXX(at,start_addr+head+i*page_saze,p_buf+head+i*page_saze,tail);
    }
    return 0;
}

