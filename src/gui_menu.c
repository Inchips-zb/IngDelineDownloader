#include "gui_menu.h"
#include "delay.h"
#include "bsp_msg.h"
#include "platform_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart_buner.h"
#include "uart_driver.h"  
#include "use_fatfs.h" 
#include "bsp_buzzer.h"
#include "bmp.h"
#include <string.h>
#include "btstack_event.h"
#include "profile.h"
#include "ble_scan_adv_list.h"
#include "device_info.h"
#include "btstack_util.h"
#include "eeprom.h"

u8g2_t u8g2;        //显示器初始化结构体

#define BEER_FREQ_KEY_BACK   	2600
#define BEER_FREQ_KEY_CONFIRM   2900
#define BEER_FREQ_KEY_PSH       3100

typedef struct 
{
	char menuName[10];
	const bmp_t *pBmp;
}menu_struct_t;

const menu_struct_t stMenu[] = {
									{"BURN",&bmpBurn},
									{"CLOCK",&bmpClock},
									{"FILES",&bmpFiles},
									{"SET",&bmpSet},
									{"ERASE",&bmpErase},
									{"BLE",&bmpBle},
								};

#define  MENU_NUMS  (sizeof(stMenu)/sizeof(stMenu[0]))
	
static void drawLog(u8g2_t *u8g2)
{
#define START_X (0)
#define START_Y (1)
	
	u8g2_DrawXBMP(u8g2, 0, START_Y,  bmpIngLogo.xSize,  bmpIngLogo.ySize, bmpIngLogo.pBitmap);
    u8g2_SetFontMode(u8g2, 1); 
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_5x7_tr);
    u8g2_DrawStr(u8g2, START_X+6,START_Y+52,"http://www.ingchips.cn/");
}

static uint8_t drawProgress(u8g2_t *u8g2,uint8_t progress, char *text)
{
	#define	U8G2_SPACING	(2)		
	#define U8G2_BOX_WIDTH	(1)	
	#define U8G2_BOX_HEIGHT	(16)	
	#define	U8G2_BAR_HEITH	(12)	
	#define	U8G2_BAR_R		(0)		
	#define U8G2_BAR_MAX	(100)	

	float lcdWidth = u8g2_GetDisplayWidth(u8g2);
	float lcdHeight = u8g2_GetDisplayHeight(u8g2);

	char progressBuf[5] = {0};
	float setp = 0;				
	unsigned int barWidth = 0;	
	

	u8g2_DrawBox(u8g2, U8G2_SPACING, U8G2_SPACING, lcdWidth-U8G2_SPACING*2, U8G2_BOX_WIDTH);
	u8g2_DrawBox(u8g2, U8G2_SPACING, U8G2_SPACING, U8G2_BOX_WIDTH, U8G2_BOX_HEIGHT);
	u8g2_DrawBox(u8g2, lcdWidth-U8G2_SPACING-U8G2_BOX_WIDTH, U8G2_SPACING, U8G2_BOX_WIDTH, U8G2_BOX_HEIGHT);
	

	u8g2_DrawBox(u8g2, U8G2_SPACING, lcdHeight-U8G2_SPACING-U8G2_BOX_WIDTH, lcdWidth-U8G2_SPACING*2, U8G2_BOX_WIDTH);
	u8g2_DrawBox(u8g2, U8G2_SPACING, lcdHeight-U8G2_SPACING-U8G2_BOX_HEIGHT, U8G2_BOX_WIDTH, U8G2_BOX_HEIGHT);
	u8g2_DrawBox(u8g2, lcdWidth-U8G2_SPACING-U8G2_BOX_WIDTH, lcdHeight-U8G2_SPACING-U8G2_BOX_HEIGHT, 
		U8G2_BOX_WIDTH, U8G2_BOX_HEIGHT);
	

	u8g2_SetFont(u8g2, u8g2_font_amstrad_cpc_extended_8r);
	u8g2_DrawStr(u8g2, (lcdWidth-u8g2_GetStrWidth(u8g2, text))/2, lcdHeight*1/3, text);
	

	u8g2_DrawRFrame(u8g2, (lcdWidth-(lcdWidth*2/3))/2, (lcdHeight/2)-(U8G2_BAR_HEITH/2), (lcdWidth*2)/3, U8G2_BAR_HEITH, U8G2_BAR_R);

	setp = ((lcdWidth*2)/3)/U8G2_BAR_MAX;
	barWidth = setp*progress;
	if(progress <= U8G2_BAR_MAX)
	{

		u8g2_DrawRBox(u8g2, (lcdWidth-(lcdWidth*2/3))/2, ((lcdHeight/2)-(U8G2_BAR_HEITH/2)), 
			barWidth, U8G2_BAR_HEITH, U8G2_BAR_R);
		
		snprintf(progressBuf, sizeof(progressBuf), "%d%s", progress, "%");
		u8g2_DrawStr(u8g2, (lcdWidth-u8g2_GetStrWidth(u8g2, progressBuf))/2, lcdHeight*3/4, progressBuf);
	}
	else{
		return 1;
	}
	return 0;
}




static void burnPageDrawBackground(void)
{
	u8g2_FirstPage(&u8g2);
	do
	{
	  drawProgress(&u8g2,0,"Wait");
	   
	} while (u8g2_NextPage(&u8g2));	

}
	
static void ing_bootloader_trig(void)
{
     platform_printf("Trigger boot\r\n");
     GIO_WriteValue((GIO_Index_t)TRIG_BOOT, 1);
     vTaskDelay(pdMS_TO_TICKS(100));
     GIO_WriteValue((GIO_Index_t)TRIG_RST, 0);
     vTaskDelay(pdMS_TO_TICKS(30));
     GIO_WriteValue((GIO_Index_t)TRIG_RST, 1);
     vTaskDelay(pdMS_TO_TICKS(30));
     GIO_WriteValue((GIO_Index_t)TRIG_BOOT, 0);
}

void trig_boot_timerout_cb(void)
{
	UserQue_SendMsg(USER_MSG_BURN_STATE,NULL,BURN_STATE_TIMEROUT);
}

static fileNode* pageFileSerch(fileNode* fileList,uint16_t fileNums,uint16_t sel)
{
	fileNode* pList =  fileList;
	fileNode* rpList  = NULL;
	uint8_t index = 1;
	const bmp_t *pBmp;
	uint8_t total =  fileNums/3+1 - !(fileNums%3);
	char sc_buff[64];
	u8g2_SetFont(&u8g2, u8g2_font_t0_11_tr);
	u8g2_FirstPage(&u8g2);
	do
	{

		u8g2_DrawStr(&u8g2, 1, 60,"<");
		u8g2_DrawStr(&u8g2, 120, 60,">");
		
		sprintf(sc_buff,"%d/%d",index, total);	
		u8g2_DrawStr(&u8g2, 50, 60,sc_buff);

		if(NULL == fileList || 0 == fileNums)
		{
			u8g2_DrawStr(&u8g2, 1, 30,"No files ");
			return NULL;
	
		}
		while (pList != NULL) {
			
			if(0 == pList->isFile){
				pBmp = &bmpFolder;
			}else
			{
				pBmp = &bmpFile;
			}
			
			if(strlen(pList->name) > 10){
			
				sc_buff[0] = '.';
				sc_buff[1] = '.';
				sc_buff[2] = '.';
				extractLastCharacters( pList->name,10,sc_buff+3);
			}
			else
			{
			
				sprintf(sc_buff,"%s",  pList->name);	
			}	
			if(sel == (index-1)){
				u8g2_DrawStr(&u8g2, 120, index*17-4,"<");
				rpList = pList;
			}
			u8g2_DrawXBMP(&u8g2, 1, (index-1)*17,pBmp->xSize, pBmp->ySize, pBmp->pBitmap);			
			u8g2_DrawStr(&u8g2, pBmp->xSize+8, index*17-4,sc_buff);

		//	printf("%d,%d:%s,%s\n",sel+1, index,pList->name,sc_buff);
			index++;
			if(index > 3) break;
			pList = pList->next;
		}
	} while (u8g2_NextPage(&u8g2));
	return rpList;
}
char file_dir[256];	
uint8_t fi_offset = 0;
uint8_t pageBurningDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;

	int8_t  coder = 0;
	uint8_t progress = 0;
	uint8_t fileReady = 0;
	static uint8_t burning = 0;
	uint16_t maxFileNums = 0;
	fileNode *subfileList;
	fileNode *getfileList;
	fileNode *nowfileList;
	fi_offset = sprintf(file_dir,"1:");
    fileNode* fileList = files_scan("1:", &maxFileNums);
	getfileList = pageFileSerch(fileList,maxFileNums,0);
	nowfileList = fileList;	
    if(!use_fatfs_mount(1)) {
		freeFileList(getfileList);
		return PAGE_SHOW_MAIN;
	}
	
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
						if(burning) break;
						if(getfileList){
							freeFileList(getfileList);
							getfileList = NULL;
						}
						if(subfileList)
						{
							freeFileList(getfileList);
							getfileList = NULL;
						}
						use_fatfs_mount(0);
						return PAGE_SHOW_MAIN;
					}
					if(RecvMsg.length & 0x06) {
						apUART_BaudRateSet(APB_UART1,SYSCTRL_GetClk(SYSCTRL_ITEM_APB_UART1),115200);
						if(fileReady){
							if(burning) break;
							use_fatfs_mount(1);
							ing_bootloader_trig();
							platform_set_timer(trig_boot_timerout_cb,2000/0.625);
							burning = 1;
							key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
							u8g2_FirstPage(&u8g2);
							do
							{
								drawProgress(&u8g2,0,"Burning");
							   
							} while (u8g2_NextPage(&u8g2));
						}
						else
						{
							    fi_offset += sprintf(file_dir+fi_offset,"/%s",getfileList->name);
								printf("open:%s\r\n",file_dir);
							    if((1 == getfileList->isFile) && (0 ==strcasecmp(getfileList->name,"flash_download.ini"))) {
									
									if(getfileList){
										freeFileList(getfileList);
										getfileList = NULL;
									}
									if(subfileList)
									{
										freeFileList(getfileList);
										getfileList = NULL;
									}
									fileReady = 1;
									load_downloader_cfg(file_dir);
									burnPageDrawBackground();
									break;
								}
					
								subfileList = files_scan(file_dir, &maxFileNums);
								nowfileList = subfileList;
								coder = 0;
								getfileList = pageFileSerch(nowfileList,maxFileNums,abs(coder));						
						
							}
						printf("key confirm\n");
					}
				}break;

				case USER_MSG_BURN_STEP:{
	
					platform_set_timer(trig_boot_timerout_cb,0);
					u8g2_FirstPage(&u8g2);
				    do
				    {
						progress = RecvMsg.length;
						drawProgress(&u8g2,progress,"Burning");
					   
				    } while (u8g2_NextPage(&u8g2));			
				}break;
				
				case USER_MSG_BURN_STATE:{		
					printf("states:%d\n",RecvMsg.length);
					burning = 0;
					u8g2_FirstPage(&u8g2);
				    do
				    {
						switch(RecvMsg.length)
						{
							case BURN_STATE_OK :{
								drawProgress(&u8g2,100,"Burn ok");
							}break;
							case  BURN_STATE_FLASH_LOCK:{
								drawProgress(&u8g2,progress,"Flash lock");
								}break;
							case  BURN_STATE_TIMEROUT:{
								drawProgress(&u8g2,progress,"Burn timerout");
								}break;
							case  BURN_STATE_MEM_OVER:{
								 drawProgress(&u8g2,progress,"Mem over");
								}break;
							case  BURN_STATE_FILE_ERR:{
								drawProgress(&u8g2,progress,"Faile err");
								}break;
							case  BURN_STATE_NAK:{
								drawProgress(&u8g2,progress,"Burn noack");
								}break;
							case  BURN_STATE_UNKNOW_CMD:{
								drawProgress(&u8g2,progress,"Unknow cmd");
								}break;
							default:drawProgress(&u8g2,progress,"Burn fail");break;
						} 
					   progress = 0;
				    } while (u8g2_NextPage(&u8g2));	
					burn_cmpl_buzzer_open(3000);
				}break;		
				case USER_MSG_CODER:{
					key_coder_buzzer_open(3100,10);		
					coder += RecvMsg.length;	
			
                    coder = coder % maxFileNums;		
					getfileList = pageFileSerch(nowfileList,maxFileNums,abs(coder));					
					printf("coder:%d,%d\n",RecvMsg.length,coder);
				}break;				
				default:break;
			}
		}

    }
	return 0;
}



	
static void mainPageDrawBackground(uint8_t index,uint8_t x)
{
#define MAIN_MNUE_START_Y (10)	
	if(index >= MENU_NUMS) index = 0;
	uint8_t last_index = index ? (index-1):(MENU_NUMS-1);
	uint8_t next_index = (index == (MENU_NUMS-1)) ? (0):(index+1);
	printf("last:%d,curr:%d.next:%d\n",last_index,index,next_index);
	u8g2_FirstPage(&u8g2);
	do
	{
		u8g2_DrawXBMP(&u8g2, -18+x*9, MAIN_MNUE_START_Y, stMenu[last_index].pBmp->xSize, stMenu[last_index].pBmp->ySize, stMenu[last_index].pBmp->pBitmap);
		u8g2_DrawXBMP(&u8g2, 45+x*9, MAIN_MNUE_START_Y, stMenu[index].pBmp->xSize, stMenu[index].pBmp->ySize, stMenu[index].pBmp->pBitmap);
		u8g2_DrawStr(&u8g2, (128-u8g2_GetStrWidth(&u8g2, stMenu[index].menuName))/2+x*9, MAIN_MNUE_START_Y+50, stMenu[index].menuName);
	    u8g2_DrawXBMP(&u8g2, 110+x*9, MAIN_MNUE_START_Y, stMenu[next_index].pBmp->xSize, stMenu[next_index].pBmp->ySize, stMenu[next_index].pBmp->pBitmap);

	} while (u8g2_NextPage(&u8g2));

}

static int index_menu = 0;
uint8_t pageMainDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	u8g2_SetFont(&u8g2, u8g2_font_amstrad_cpc_extended_8r);
	mainPageDrawBackground(index_menu,0);
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
					}
					if(RecvMsg.length & 0x02 ) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						
						printf("page entry:%d\n",index_menu);
						return (index_menu+PAGE_SHOW_BURN);
					}
				}break;
				
				case USER_MSG_CODER:{

					index_menu += RecvMsg.length;
					
					if(index_menu < 0) index_menu = MENU_NUMS-1;
					
					if(index_menu >= MENU_NUMS) index_menu = 0;
					
					index_menu = index_menu % (MENU_NUMS);
					mainPageDrawBackground(index_menu,0);	
					key_coder_buzzer_open(3100,10);						
					printf("coder:%d,%d\n",RecvMsg.length,index_menu);
				}break;
								
				default:break;
			}
		}

    }
	return 0;
}

static void uDiskPageDrawBackground(void)
{
#define UDISK_START_Y (1)	

	u8g2_FirstPage(&u8g2);
	do
	{
		u8g2_DrawXBMP(&u8g2, 40, UDISK_START_Y, bmpUdisk.xSize, bmpUdisk.ySize, bmpUdisk.pBitmap);
		u8g2_DrawStr(&u8g2, (128-u8g2_GetStrWidth(&u8g2,"Key BACK reset" ))/2, UDISK_START_Y+60,"Key BACK reset");
	} while (u8g2_NextPage(&u8g2));

}

uint8_t pageUdiskDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	static int index_menu = 0;
	u8g2_SetFont(&u8g2, u8g2_font_amstrad_cpc_extended_8r);
	uDiskPageDrawBackground();
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
						platform_reset();
					}
					if(RecvMsg.length & 0x06) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						
						printf("key confirm\n");
						
					}
				}break;
									
				default:break;
			}
		}

    }
	return 0;
}


volatile uint8_t bleState = 0;



static void showAdvLinkedList(advNode* startNode,uint8_t showNums,uint16_t total) {
#define BLE_DISPLAY_START_Y (1)	   
	advNode* currentNode = startNode;
	char sc_buff[64];
	const bmp_t *pBmp = bleState ? &bmpOn : &bmpOff;
	u8g2_FirstPage(&u8g2);
	do
	{   
		u8g2_SetFont(&u8g2, u8g2_font_amstrad_cpc_extended_8r);
		u8g2_DrawXBMP(&u8g2, 100, BLE_DISPLAY_START_Y,pBmp->xSize, pBmp->ySize, pBmp->pBitmap);
		u8g2_DrawStr(&u8g2, 4, BLE_DISPLAY_START_Y+12,"BLE");
		u8g2_SetFont(&u8g2, u8g2_font_5x7_tr);
	    for (int i = 0; i < showNums; i++) {
			if (currentNode != NULL) {
				if(currentNode->name[0]){
					if(strlen(currentNode->name) <= 17){
						int sz = sprintf(sc_buff,"%s", currentNode->name);
						for(int j = sz-1;j <= 17;j++)
							sc_buff[j] = ' ';
					}
					else
					{
						extractLastCharacters(currentNode->name,17,sc_buff);
					}
					sprintf(sc_buff+17," %ddBm", currentNode->rssi);
						
				}
				else{
					sprintf(sc_buff,"%02x:%02x:%02x:%02x:%02x:%02x %ddBm", currentNode->address[0],currentNode->address[1],currentNode->address[2],currentNode->address[3],currentNode->address[4],currentNode->address[5],currentNode->rssi);
				}
				currentNode = currentNode->next;
			} 
			else 
			{
			  //printf("No more nodes\n");
				
			  break;
			}
			u8g2_DrawStr(&u8g2, 0, 30+i*8,sc_buff);
	  }
  } while (u8g2_NextPage(&u8g2));
}

#include "platform_api.h"
extern void platform_get_heap_status(platform_heap_status_t *status);
uint8_t pageBleDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	platform_heap_status_t status;
	uint8_t bleNextState = !bleState;
	advListHead = createAdvLinkedList();
	scrollAdvLinkedList(advListHead,5,0,showAdvLinkedList);
	platform_get_heap_status(&status);
	printf("Mem start:%d,%d\n",status.bytes_free,status.bytes_minimum_ever_free);
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
						freeAdvLinkedList(advListHead);
						advListHead = NULL;
					    return (PAGE_SHOW_MAIN);
					}
					if(RecvMsg.length & 0x06) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						if(advListHead)
							btstack_push_user_msg(USER_MSG_BLE_STATE_SET, (void *)NULL, bleNextState);
						printf("key confirm:%d\n",bleNextState);
					}
				}break;
	
				case USER_MSG_BLE_STATE:
				{
					if(0 == RecvMsg.length)
					{
						bleState = bleNextState;
						bleNextState = !bleNextState;
						
						scrollAdvLinkedList(advListHead,5,0,showAdvLinkedList);	
					}
				}break;
				case USER_MSG_BLE_REFRESH:{
					if(0 == RecvMsg.length)
					{
						//printf("Mem:%d,%d\n",status.bytes_free,status.bytes_minimum_ever_free);
						scrollAdvLinkedList(advListHead,5,0,showAdvLinkedList);	
					}
				}break;

				case USER_MSG_CODER:{
					key_coder_buzzer_open(3100,10);		
					scrollAdvLinkedList(advListHead, 5, RecvMsg.length,showAdvLinkedList);					
					printf("coder:%d\n",RecvMsg.length);
				}break;
										
				
									
				default:break;
			}
		}

    }
	return 0;
}


uint8_t pageFileBrowse(void *user_data){

	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	static int index_menu = 0;
	uint16_t maxFileNums = 0;
	fileNode *subfileList;
	fileNode *getfileList;
	fileNode *nowfileList;
    fileNode* fileList = files_scan("1:", &maxFileNums);
	getfileList = pageFileSerch(fileList,maxFileNums,0);
	nowfileList = fileList;
	printf("File:%d\n", maxFileNums);
    for (;;)
    {

		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
						if(subfileList){
							freeFileList(subfileList);
							subfileList = NULL;
							coder = 0;
							nowfileList = fileList;
							getfileList = pageFileSerch(nowfileList,maxFileNums,abs(coder));		
						}else
						{
							freeFileList(fileList);
							fileList = NULL;
							return (PAGE_SHOW_MAIN);
						}
						
					}
					if(RecvMsg.length & 0x06) {
						char sc_buff[64];
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						printf("key confirm:%d\n",getfileList->isFile);
						if(1 == getfileList->isFile) break;
						sprintf(sc_buff,"1:/%s",getfileList->name);
						subfileList = files_scan(sc_buff, &maxFileNums);
						nowfileList = subfileList;
						coder = 0;
						getfileList = pageFileSerch(nowfileList,maxFileNums,abs(coder));
						
					}
				}break;
				
				case USER_MSG_CODER:{
					key_coder_buzzer_open(3100,10);		
					coder += RecvMsg.length;	
			
                    coder = coder % maxFileNums;		
					getfileList = pageFileSerch(nowfileList,maxFileNums,abs(coder));					
					printf("coder:%d,%d\n",RecvMsg.length,coder);
				}break;
								
				default:break;
			}
		}

    }
	return 0;
}

typedef struct setPage_s
{
	uint8_t id;
	char itemName[20];	
	uint8_t type;
} setPage_t;

const setPage_t setPage[] = {

	{0,"Beer",0},
	{1,"Mac", 1},
	{2,"Name",1},
	{3,"Ver",1},
};
static void setPageDrawBackground(dev_info_t dev,uint8_t sel)
{
	char sc_buff[64];
	const bmp_t *pBmpOnOff;
#define SET_DISPLAY_START_Y (1)	
#define SET_DISPLAY_START_X (4)	
#define	SET_SPACING	(2)		
#define SET_BOX_WIDTH	(128)	
#define SET_BOX_HEIGHT	(16)	
	
	u8g2_FirstPage(&u8g2);
	do
	{
		
		for(uint8_t i = 0; i< sizeof(setPage)/sizeof(setPage[0]);i++){
			
			int len = sprintf(sc_buff,"%s",setPage[i].itemName);
			if(0 == setPage[i].type)
			{
				pBmpOnOff = dev.beer_en ? &bmpOn : &bmpOff;
				u8g2_DrawXBMP(&u8g2, 128-SET_DISPLAY_START_X-pBmpOnOff->xSize,  SET_BOX_HEIGHT*i+SET_DISPLAY_START_Y,pBmpOnOff->xSize, pBmpOnOff->ySize, pBmpOnOff->pBitmap);
			}
			else
			{
				if(i == 1){
					sprintf(sc_buff+len,": %s",bd_addr_to_str(dev.mac));
				}else if(i == 2)
				{
					sprintf(sc_buff+len,": %s",dev.blename);
				}else if(i == 3)
				{
					sprintf(sc_buff+len,": %d.%d.%d",dev.ver.major,dev.ver.minor,dev.ver.patch);
				}
	
			}
			if(i == sel)
				u8g2_DrawRFrame(&u8g2,0, SET_BOX_HEIGHT*i+SET_DISPLAY_START_Y-1, SET_BOX_WIDTH, SET_BOX_HEIGHT,SET_BOX_HEIGHT/2);
			
			u8g2_DrawStr(&u8g2, SET_DISPLAY_START_X, SET_BOX_HEIGHT*i+ SET_DISPLAY_START_Y+10,sc_buff);
		}
		
	} while (u8g2_NextPage(&u8g2));

}
uint8_t pageSetDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	static int index_menu = 0;
    dev_info_t *pDev = get_device_informs();
	dev_info_t tDevice;
	memcpy(&tDevice,pDev,sizeof(dev_info_t));
	u8g2_SetFont(&u8g2, u8g2_font_spleen5x8_mf);
	setPageDrawBackground(tDevice,0);
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						printf("key back\n");
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						memcpy(pDev,&tDevice,sizeof(dev_info_t));
						storage_device_informs();
						return (PAGE_SHOW_MAIN);
					}
					if(RecvMsg.length & 0x06) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						tDevice.beer_en = !tDevice.beer_en;
						setPageDrawBackground(tDevice,coder);
						printf("key confirm\n");
					}
				}break;
				
				case USER_MSG_CODER:{
					key_coder_buzzer_open(3100,10);		
					coder += RecvMsg.length;
					coder = (coder)%(sizeof(setPage)/sizeof(setPage[0]));
					setPageDrawBackground(tDevice,coder);
					printf("coder:%d\n",coder);
				}break;
								
				default:break;
			}
		}

    }
	return 0;
}
//typedef struct 
//{
//	char subMenuName[20];
//	const unsigned char *pMnue;
//	
//}pageErase_struct_t;
const char *pageEraseItem[] = {"Series:","Start sector:","Stop sector:","Start erase"};
static void eraseDrawBackground(uint8_t index,uint8_t x)
{
#define MAIN_START_Y (12)	
	uint8_t last_index = index ? (index-1):(MENU_NUMS-1);
	uint8_t next_index = (index == (MENU_NUMS-1)) ? (0):(index+1);
	printf("last:%d,curr:%d.next:%d\n",last_index,index,next_index);
	uint8_t i = 0;
	u8g2_FirstPage(&u8g2);
	do
	{

		u8g2_SetFont(&u8g2, u8g2_font_t0_11_tr);
		u8g2_DrawStr(&u8g2, 1, MAIN_START_Y+i*12, pageEraseItem[i]);
		i++;
		u8g2_DrawStr(&u8g2, 1, MAIN_START_Y+i*12, pageEraseItem[i]);
		i++;
		u8g2_DrawStr(&u8g2, 1, MAIN_START_Y+i*12, pageEraseItem[i]);
		i++;
		u8g2_DrawStr(&u8g2, 1, MAIN_START_Y+i*12, pageEraseItem[i]);
	} while (u8g2_NextPage(&u8g2));

}

uint8_t eraseMainDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	static int index_menu = 0;

	eraseDrawBackground(0,0);
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
						return (PAGE_SHOW_MAIN);
					}
					if(RecvMsg.length & 0x06) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						
						printf("key confirm\n");
						
					}
				}break;
				
				case USER_MSG_CODER:{

					index_menu += RecvMsg.length;
					
					if(index_menu < 0) index_menu = MENU_NUMS-1;
					
					if(index_menu == MENU_NUMS) index_menu = 0;
					
					index_menu = index_menu % (MENU_NUMS);
				//	mainDrawBackground(index_menu,0);	
					key_coder_buzzer_open(3100,10);						
					printf("coder:%d,%d\n",RecvMsg.length,index_menu);
				}break;
								
				default:break;
			}
		}

    }
	return 0;
}


uint8_t pageDrowLogDisply(void *user_data)
{

	u8g2_FirstPage(&u8g2);
	
	do
	{
	   drawLog(&u8g2);
	   
	} while (u8g2_NextPage(&u8g2));
	
	vTaskDelay(pdMS_TO_TICKS(1000));

	return PAGE_SHOW_MAIN;
}

struct xy_map{
	uint8_t x;
	uint8_t y;
};

enum CLOCK{
	CLOCK_POINTER_SEC = 0,
	CLOCK_POINTER_MIN,
	CLOCK_POINTER_HOUR,
	CLOCK_POINTER_NUM
};

struct xy_map map[3][60];

static void drawClockInit(void)
{
	int radius = u8g2.height / 2.1;  // 半径
	int x = u8g2.width / 2;
	int y = u8g2.height / 2;
	for(uint16_t i = 0;i<60 ;i++){
	
	    float angle = 6*i * 3.1415 / 180.0;
		map[CLOCK_POINTER_SEC][i].x = x + (radius - 6) * sin(angle);
		map[CLOCK_POINTER_SEC][i].y = y - (radius - 6) * cos(angle);
		
		map[CLOCK_POINTER_MIN][i].x = x + (radius - 12) * sin(angle);
		map[CLOCK_POINTER_MIN][i].y = y - (radius - 12) * cos(angle);
		
		map[CLOCK_POINTER_HOUR][i].x = x + (radius - 18) * sin(angle);
		map[CLOCK_POINTER_HOUR][i].y = y - (radius - 18) * cos(angle);
	}

}

static void clockDrawBackground(int x,int y,int radius)
{

    // 仪表盘背景
    u8g2_SetFont(&u8g2, u8g2_font_amstrad_cpc_extended_8r);
    // 绘制仪表盘外框
 //   u8g2_DrawCircle(&u8g2, x, y, radius, U8G2_DRAW_ALL);
	u8g2_DrawCircle(&u8g2, x, y, 1, U8G2_DRAW_ALL);
    u8g2_DrawCircle(&u8g2, x, y, radius+1, U8G2_DRAW_ALL);
    // 绘制刻度
    for (int angle = 0; angle < 360; angle += 6) 
	{
        float radian = angle * 3.14 / 180.0;
		uint8_t te = 5;
		if(angle%30) te = 2;
        int startX = x + (radius - te) * cos(-radian);
        int startY = y - (radius - te) * sin(-radian);
        int endX = x + radius * cos(-radian);
        int endY = y - radius * sin(-radian);
		if(0 == angle){
			
			u8g2_DrawStr(&u8g2, startX-8, startY+6, "3");
		}
		if(90 == angle){
			
			u8g2_DrawStr(&u8g2, startX-5, startY-1, "6");
		}
		
		if(180 == angle){
			
			u8g2_DrawStr(&u8g2, startX+1, startY+5, "9");
		}
		if(270 == angle){
			
			u8g2_DrawStr(&u8g2, startX-8, startY+10, "12");
		}		
        u8g2_DrawLine(&u8g2, startX, startY, endX, endY);
    }

}

static void drawClock(RTC_TimeTypeDef Time) {

    // 仪表盘半径
    int radius = u8g2.height / 2.1;  // 半径
	int x = u8g2.width / 2;
	int y = u8g2.height / 2;	
	
    u8g2_ClearBuffer(&u8g2);
	clockDrawBackground( x, y, radius);
    // 绘制指针
    u8g2_DrawLine(&u8g2, x, y, map[CLOCK_POINTER_SEC][Time.Seconds].x, map[CLOCK_POINTER_SEC][Time.Seconds].y);
	u8g2_DrawLine(&u8g2, x, y, map[CLOCK_POINTER_MIN][Time.Minutes].x, map[CLOCK_POINTER_MIN][Time.Minutes].y);
    u8g2_DrawLine(&u8g2, x, y, map[CLOCK_POINTER_HOUR][Time.Hours%12*5+Time.Minutes/12].x, map[CLOCK_POINTER_HOUR][Time.Hours%12*5+Time.Minutes/12].y);
	u8g2_SendBuffer(&u8g2);
}

uint8_t pageClockDisply(void *user_data)
{
	RTC_TimeTypeDef Time;
    RTC_DateTypeDef Date;	
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	drawClockInit();
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						return PAGE_SHOW_MAIN;
						printf("key back\n");
					}
				}break;
				

				case USER_MSG_RTC_S:{
					u8g2_FirstPage(&u8g2);
					do
					{
						get_rtc_time(&Time,&Date);
						drawClock(Time);   
					} while (u8g2_NextPage(&u8g2));
				}break;
				default:break;
			}
		}

	}
	return 0;
}

uint8_t pageSetClockDisply(void)
{
	RTC_TimeTypeDef Time;
    RTC_DateTypeDef Date;	
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	drawClockInit();
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) printf("key back\n");
					if(RecvMsg.length & 0x02) printf("key confirm\n");
				}break;
				
				case USER_MSG_CODER:{
					coder += RecvMsg.length;
					if(coder <0 )coder = 0;
					if(coder > 100) coder = 100;
					printf("coder:%d\n",coder);
				}break;
				case USER_MSG_RTC_S:{
					u8g2_FirstPage(&u8g2);
					do
					{
						get_rtc_time(&Time,&Date);
						drawClock(Time);   
					} while (u8g2_NextPage(&u8g2));
				}break;
				default:break;
			}
		}

	}
	return 0;
}

 