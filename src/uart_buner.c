#include "uart_buner.h"
#include "bsp_msg.h"
#include "ingsoc.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "platform_api.h"
#include "uart_driver.h"
#include "port_gen_os_driver.h"

#define  LOAD_BAUD_RATE     (1000000u)

#ifndef SHAKE_BAUD_RATE
#define  SHAKE_BAUD_RATE    (115200u)
#endif

#define STR_CMD_SET_BAUD          "#$sbaud"
#define STR_CMD_CHIP_ERASE        "#$chera"
#define STR_CMD_SECTOR_ERASE      "#$stera"
#define STR_CMD_CHIP_LOCK         "#$lockk"
#define STR_CMD_CHIP_UNLOCK       "#$unlck"
#define STR_CMD_BURN_START        "#$start"
#define STR_CMD_BURN_STATE        "#$state"
#define STR_CMD_BURN_FLASHST      "#$fshst"
#define STR_CMD_PROM_FLASH        "#$u2fsh"
#define STR_CMD_PROM_EFUES        "#$u2efs"
#define STR_CMD_READ_EFUES        "#$efs2u"

enum
{
    EVENT_ID_UART_CMD_INPUT  = 0x0001,
    EVENT_ID_UART_BURN_START,
    EVENT_ID_UART_BURN_ULK,
    EVENT_ID_UART_BURN_LOCK,    
    EVENT_ID_UART_ACK,
    EVENT_ID_UART_NACK,
    EVENT_ID_UART_UNKNOW_CMD

};
#define GEN_OS          ((const gen_os_driver_t *)platform_get_gen_os_driver())
    
static gen_handle_t     cmd_event = NULL;
typedef struct
{
    uint8_t busy;
    uint16_t size;
    char buf[256];
} str_buf_t;

str_buf_t input = {0};

typedef void (*f_cmd_handler)(const char *param);

typedef struct
{
    const char *cmd;
    f_cmd_handler handler;
} cmd_t;


downdloader_cfg_t downdloader = {
	.family = INGCHIPS_FAMILY_916,
	.baud = LOAD_BAUD_RATE,
	.timerout = 1000,
	.verify = 0,
	.protect_enable = 0,
	.un_lock = 1,
	.sn_burn_enable = 1,
	.sn_code_len = 6,
	.sn_code_addr = 0x2074000,
	.sn_start = 1234,
	.sn_step = 1,
	.mate.sector_size = 4096,
	.mate.page_size = 256,
	.mate.block_num = 0,
	.mate.blocks = {0}
};



static void uart_event_set(uint16_t event_id);
static void uart_buner_tx_data(const char *cmd, const uint16_t cmd_len,uint8_t *param,uint16_t param_len);

downdloader_cfg_t *get_downloader_cfg(void)
{
    return &downdloader;
}

void cmd_uart_burn_sart(const char *param)
{
    uart_event_set(EVENT_ID_UART_BURN_START);
}

void cmd_uart_ack(const char *param)
{
    uart_event_set(EVENT_ID_UART_ACK);
}

void cmd_uart_unknow_cmd(const char *param)
{
    uart_event_set(EVENT_ID_UART_UNKNOW_CMD);
}

void cmd_uart_nack(const char *param)
{
    uart_event_set(EVENT_ID_UART_NACK);
}

void cmd_uart_burn_ulk(const char *param)
{
    uart_event_set(EVENT_ID_UART_BURN_ULK);
}

void cmd_uart_burn_lock(const char *param)
{
    uart_event_set(EVENT_ID_UART_BURN_LOCK);
}

static cmd_t cmds[] =
{
    {
        .cmd = "UartBurnStart916",
        .handler = cmd_uart_burn_sart
    },
    {
        .cmd = "#$ack\n",
        .handler = cmd_uart_ack
    },
    {
        .cmd = "#$nak\n",
        .handler = cmd_uart_nack
    },
    {
        .cmd = "#$ulk\n",
        .handler = cmd_uart_burn_ulk
    },
    {
        .cmd = "#$lck\n",
        .handler = cmd_uart_burn_lock
    }    

};
const unsigned char auchCRCHi[] =
{
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01,0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41,0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81,0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;

const unsigned  char auchCRCLo[] =
{
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,0x04,
    0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,0x08, 0xC8,
    0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,0x11, 0xD1, 0xD0, 0x10,
    0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
    0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
    0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB,0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
    0xB4, 0x74, 0x75, 0xB5,0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91,0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
    0x9C, 0x5C,0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
    0x88,0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,0x40
} ;

unsigned short check_crc16(char *puchMsg , unsigned short usDataLen)
{
    unsigned char uchCRCHi = 0xFF ; /* high byte of CRC initialized */
    unsigned char uchCRCLo = 0xFF ; /* low byte of CRC initialized */
    unsigned char uIndex ;          /* will index into CRC lookup table */

    while (usDataLen--)     /* pass through message buffer */
    {
        uIndex = uchCRCHi ^ *puchMsg++ ; /* calculate the CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex] ;
    }

    return (uchCRCHi << 8 | uchCRCLo) ;
}

void handle_command(char *cmd_line)
{
    static const char unknow_cmd[] =  "unknown command\n";
    char *param = cmd_line;
    int i;
    while (*param)
    {
        if (*param == ' ')
        {
            *param = '\0';
            param++;
            break;
        }
        else
            param++;
    }

    for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++)
    {    
        if (strcasecmp(cmds[i].cmd, cmd_line) == 0)
            break;
    }
    if (i >= sizeof(cmds) / sizeof(cmds[0]))
        goto show_help;

    cmds[i].handler(param);
    return;

show_help:

    cmd_uart_unknow_cmd((void*) NULL);

}




#include "FreeRTOS.h"
#include "task.h"
#include "btstack_util.h"

typedef struct burner_s
{
	uint8_t buner_work_mode;
	burn_block_t block;
    uint8_t  binIndex;
	uint16_t total_burn_sec_nums;
	uint32_t total_burn_data_size;
	uint8_t uart_burn_step;
	uint16_t block_index;    
	uint16_t sector_index ;
	uint16_t pag_index ;	
	uint8_t param[32];	
} burner_t;

burner_t ing_buner;

static void burn_ctrl_restart(burner_t *pBurner)
{
	memset(pBurner,0,sizeof(burner_t));
}


static void uart_buner_task_entry(void *_)
{

    FIL *fp = NULL;
	uint8_t *Buf = NULL;
	UINT bytesRead = 0;
    FRESULT res;
	burn_ctrl_restart(&ing_buner);
   // platform_printf("family:%d,sector_size:%d,page_size:%d,block_num:%d\r\n",downdloader.family ,downdloader.mate.sector_size,downdloader.mate.page_size,downdloader.mate.block_num);
    for(;;)
    {
        GEN_OS->event_wait(cmd_event);

        switch(*(uint16_t*)cmd_event)
        {
            case EVENT_ID_UART_CMD_INPUT:
            {
                // platform_printf("CMD_INPUT\r\n");
                handle_command(input.buf);
				memset(input.buf,0,input.size);
                input.size = 0;
                input.busy = 0;   
            }break;
            case EVENT_ID_UART_BURN_START:
            {
	
				burn_ctrl_restart(&ing_buner);
				for(ing_buner.binIndex = 0;ing_buner.binIndex < BURN_BIN_NUM_MAX;ing_buner.binIndex++)
				{
					if(downdloader.mate.blocks[ing_buner.binIndex].check)
					{
						memcpy(&ing_buner.block,&downdloader.mate.blocks[ing_buner.binIndex],sizeof(burn_block_t));
						break;
					}
				
				}
				fp = (FIL*)malloc(sizeof(FIL));
				Buf = (uint8_t *)malloc(downdloader.mate.page_size);
				
				if(NULL == fp || NULL == Buf) {
					platform_printf("mem malloc failed\r\n"); 
					free(fp);
					free(Buf);
					UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,1);
					break;
				}
			    res = f_open(fp, ing_buner.block.filename, FA_OPEN_EXISTING | FA_READ);				
				if (res != FR_OK) {
					platform_printf("%s open failed\r\n",ing_buner.block.filename); 
					free(fp);
					free(Buf);
					UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,1);
					break;
				}
				platform_printf("Uart burn start #bin%d - %s\r\n",ing_buner.binIndex,ing_buner.block.filename);
				if(ing_buner.binIndex < BURN_BIN_NUM_MAX) uart_buner_tx_data(STR_CMD_BURN_STATE,strlen(STR_CMD_BURN_STATE),NULL,0);
            }
            break;     
            case EVENT_ID_UART_ACK:
            {
                switch(ing_buner.uart_burn_step)
                {
					
                    case 1:
                    {
                        platform_printf("Set baud :%d\r\n",downdloader.baud);
                        apUART_BaudRateSet(APB_UART1,SYSCTRL_GetClk(SYSCTRL_ITEM_APB_UART1),downdloader.baud);
                        ing_buner.param[0] = 00;
                        ing_buner.param[1] = 0x20;
                        vTaskDelay(pdMS_TO_TICKS(10));
           
                        platform_printf("set flash qspi mode\r\n");
                        uart_buner_tx_data(STR_CMD_BURN_FLASHST,strlen(STR_CMD_BURN_FLASHST),ing_buner.param,2);
                        ing_buner.uart_burn_step = 2;
                        ing_buner.block_index = 0;
                        ing_buner.sector_index = 0;
                        ing_buner.total_burn_sec_nums = 0;
                        ing_buner.pag_index = 0;                             
                    }break;
                    case 2:
                    {                              
                        if(ing_buner.sector_index * downdloader.mate.sector_size >= ing_buner.block.size)
                        {
                            ing_buner.block_index += 1;
                            ing_buner.sector_index = 0;   
							ing_buner.binIndex += 1;
							for(;ing_buner.binIndex < BURN_BIN_NUM_MAX; ing_buner.binIndex++)
							{
								if(downdloader.mate.blocks[ing_buner.binIndex].check)
								{
									memcpy(&ing_buner.block,&downdloader.mate.blocks[ing_buner.binIndex],sizeof(burn_block_t));
									break;
								}
							
							}
							f_close(fp);
						    res = f_open(fp, ing_buner.block.filename, FA_OPEN_EXISTING | FA_READ);				
							if (res != FR_OK) {
								platform_printf("%s open failed\r\n",ing_buner.block.filename); 
								free(fp);
								free(Buf);
								UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,1);
								break;
							}							
							if(ing_buner.binIndex<BURN_BIN_NUM_MAX)
							platform_printf("Uart burn start #bin%d - %s\r\n",ing_buner.binIndex,ing_buner.block.filename);							
                        }

                        if(ing_buner.block_index >= downdloader.mate.block_num) 
                        {
                            ing_buner.uart_burn_step = 0;
                            ing_buner.block_index = 0;
                            ing_buner.sector_index = 0;
                            ing_buner.total_burn_sec_nums = 0;
                            if(downdloader.protect_enable)
                            {
                                uart_buner_tx_data(STR_CMD_CHIP_LOCK,strlen(STR_CMD_CHIP_LOCK),NULL,0);  
                                ing_buner.uart_burn_step = 6;                                
                            }
							UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,0);
                            platform_printf("Burn complete\r\n");
                			f_close(fp);
							free(fp);
							free(Buf);
                            break;                        
                        }      
             
                        ing_buner.pag_index = 0;                        
                        uint32_t *p = (uint32_t *)ing_buner.param;
                        *p = ing_buner.block.loadaddr + ing_buner.sector_index*downdloader.mate.sector_size;
						uint8_t *p8 = (uint8_t*)p;
                        ing_buner.sector_index += 1;
                        ing_buner.total_burn_sec_nums += 1;
                        uint8_t val = 99*(ing_buner.total_burn_sec_nums-1)*downdloader.mate.sector_size/downdloader.mate.total_size;
                        UserQue_SendMsg(USER_MSG_BURN_STEP, NULL, val);
                        uart_buner_tx_data(STR_CMD_SECTOR_ERASE,strlen(STR_CMD_SECTOR_ERASE),(uint8_t*)p,4);     
                        ing_buner.uart_burn_step = 3;

                    }break;
                    case 3:
                    {                
                        uint32_t *p = (uint32_t *)ing_buner.param;
                        *p = ing_buner.block.loadaddr + (ing_buner.sector_index-1)*downdloader.mate.sector_size + ing_buner.pag_index*downdloader.mate.page_size;
                        *(p+1) = downdloader.mate.page_size-1;
                        ing_buner.pag_index += 1;                                              
                        uart_buner_tx_data(STR_CMD_PROM_FLASH,strlen(STR_CMD_PROM_FLASH),(uint8_t*)p,5);    
                        ing_buner.uart_burn_step = 4;
                    }break;      
                    case 4:
                    {          
                        uint16_t  crc = 0;  

						res = f_read(fp, Buf, downdloader.mate.page_size, &bytesRead);
						if (res != FR_OK) {
						   platform_printf("File read err\r\n");
							ing_buner.uart_burn_step = 0;
							f_close(fp);
							free(fp);
							free(Buf);
							UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,1);
							break;
						}
						else
						{
							
							ing_buner.total_burn_data_size += bytesRead;
							crc = check_crc16((char*)Buf,downdloader.mate.page_size);

							driver_append_tx_data(Buf, downdloader.mate.page_size);
							driver_append_tx_data((uint8_t *)&crc, 2);
							//platform_printf("w:%d,%d\r\n",bytesRead,total_burn_data_size);
							if(downdloader.mate.page_size * (ing_buner.pag_index) >= downdloader.mate.sector_size)
							{
								ing_buner.uart_burn_step = 2;
											   
							}else
							{                        
								ing_buner.uart_burn_step = 3;   
							}
						}

                        
                    }break;    
                    case 5:
                    {   
                        ing_buner.uart_burn_step = 1;
                        uint32_t *p = (uint32_t *)ing_buner.param;
                        *p = downdloader.baud;
//                        printf_hexdump((uint8_t*)p,4);
//                        vTaskDelay(pdMS_TO_TICKS(100));
                        uart_buner_tx_data(STR_CMD_SET_BAUD,strlen(STR_CMD_SET_BAUD),(uint8_t*)p,4);     
                                
                    }break;     
                    case 6:
                    {   
                        ing_buner.uart_burn_step = 0;
                        platform_printf("Lock complete\r\n");
                                
                    }break;                       
                    default :break;
                
                }            
            }
            break;    
             case EVENT_ID_UART_NACK:
             {              
                platform_printf("nack\r\n");
                 
                UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,1);
				f_close(fp);
				free(fp);
				free(Buf);
               // platform_reset();
             }break;
             case EVENT_ID_UART_BURN_ULK:
             {
                 platform_printf("flash unlock\r\n");
                ing_buner.uart_burn_step = 1;
                uint32_t *p = (uint32_t *)ing_buner.param;
				
                *p = downdloader.baud;
                printf_hexdump((uint8_t*)p,4);
                vTaskDelay(pdMS_TO_TICKS(100));
                uart_buner_tx_data(STR_CMD_SET_BAUD,strlen(STR_CMD_SET_BAUD),(uint8_t*)p,4);                    

             }break;   
              case EVENT_ID_UART_BURN_LOCK:
             {
                 platform_printf("Flash was locked\r\n");
                 if(downdloader.un_lock)
                 {
                    ing_buner.uart_burn_step = 5;
                    uart_buner_tx_data(STR_CMD_CHIP_UNLOCK,strlen(STR_CMD_CHIP_UNLOCK),NULL,0);   
                 }
                else
                {  
                    ing_buner.uart_burn_step = 0;
                }
               
             }break;              
             case EVENT_ID_UART_UNKNOW_CMD:
             {
                 if(ing_buner.uart_burn_step)
                 {
                    ing_buner.block_index = 0;
                    ing_buner.sector_index = 0;
                    ing_buner.total_burn_sec_nums = 0;
					ing_buner.uart_burn_step = 0;
					f_close(fp);
					free(fp);
					free(Buf);
                    UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,2);
                 }
             }
                break;                 
            default:break;            
    }        
    }
}

void uart_buner_start(void)
{
    cmd_event = GEN_OS->event_create();
    GEN_OS->task_create("buner",
        uart_buner_task_entry,
        NULL,
        1024,
        GEN_TASK_PRIORITY_LOW);
}

static void append_data(str_buf_t *buf, const char *d, const uint16_t len)
{
    if (buf->size + len > sizeof(buf->buf))
        buf->size = 0;

    if (buf->size + len <= sizeof(buf->buf))
    {
        memcpy(buf->buf + buf->size, d, len);
        buf->size += len;
    }
}

void uart_buner_rx_data(const char *d, uint8_t len,uint8_t cmpl)
{

    if(input.busy || len == 0) return;
	
    append_data(&input, d, len);

    if (!(len%16) || cmpl)
    {       
        if(1 >=  input.size)
        {
            input.busy = 0;
            input.size = 0;
            return;
        }
        input.buf[input.size] = '\0';
        input.busy = 1;

        uart_event_set(EVENT_ID_UART_CMD_INPUT);     
    }
}

static void uart_event_set(uint16_t event_id)
{
    *(uint16_t*)cmd_event = event_id;
  return  GEN_OS->event_set(cmd_event);

}


static void uart_buner_tx_data(const char *cmd, const uint16_t cmd_len,uint8_t *param,uint16_t param_len)
{
    if(cmd)
		driver_append_tx_data(cmd, cmd_len);
    if(param)
		driver_append_tx_data(param, param_len);

}


