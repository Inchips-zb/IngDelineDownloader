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
#include "image.h"
#include <string.h>
u8g2_t u8g2;        //显示器初始化结构体

#define BEER_FREQ_KEY_BACK   	2600
#define BEER_FREQ_KEY_CONFIRM   2900
#define BEER_FREQ_KEY_PSH       3100

static void drawLog(u8g2_t *u8g2)
{
#define START_X (0)
#define START_Y (1)
	
	u8g2_DrawXBMP(u8g2, 0, START_Y, 128, 32, logo);
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
     vTaskDelay(pdMS_TO_TICKS(10));
     GIO_WriteValue((GIO_Index_t)TRIG_RST, 1);
     vTaskDelay(pdMS_TO_TICKS(10));
     GIO_WriteValue((GIO_Index_t)TRIG_BOOT, 0);
}


uint8_t pageBurningDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;

	burnPageDrawBackground();
    for (;;)
    {
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						printf("key back\n");
						return PAGE_SHOW_MAIN;
					}
					if(RecvMsg.length & 0x06) {
						apUART_BaudRateSet(APB_UART1,SYSCTRL_GetClk(SYSCTRL_ITEM_APB_UART1),115200);
						ing_bootloader_trig();
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						printf("key confirm\n");
					}
				}break;

				case USER_MSG_BURN_STEP:{
	
					u8g2_FirstPage(&u8g2);
				    do
				    {
					  drawProgress(&u8g2,RecvMsg.length,"Burning");
					   
				    } while (u8g2_NextPage(&u8g2));			
				}break;
				
				case USER_MSG_BURN_STATE:{		
					printf("ack:%d\n",RecvMsg.length);
					u8g2_FirstPage(&u8g2);
				    do
				    {
						if(0 == RecvMsg.length)
							drawProgress(&u8g2,100,"Burn ok");
						else
							drawProgress(&u8g2,0,"Burn fail");
					   
				    } while (u8g2_NextPage(&u8g2));	
					burn_cmpl_buzzer_open(3000);
				}break;				
				default:break;
			}
		}

    }
	return 0;
}


typedef struct 
{
	char subMenuName[10];
	const unsigned char *pMnue;
	
}menu_struct_t;

const menu_struct_t stMenu[] = {
									{"BURN",bmpBurn37x37},
									{"CLOCK",bmpClock37x37},
									{"FILES",bmpFiles37x37},
									{"SET",bmpSet37x37},
									{"ERASE",bmpErase37x37},
								};

#define  MENU_NUMS  (sizeof(stMenu)/sizeof(stMenu[0]))
	
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
		u8g2_DrawXBMP(&u8g2, -18+x*9, MAIN_MNUE_START_Y, 37, 37, stMenu[last_index].pMnue);
		u8g2_DrawXBMP(&u8g2, 45+x*9, MAIN_MNUE_START_Y, 37, 37, stMenu[index].pMnue);
		u8g2_DrawStr(&u8g2, (128-u8g2_GetStrWidth(&u8g2, stMenu[index].subMenuName))/2+x*9, MAIN_MNUE_START_Y+50, stMenu[index].subMenuName);
	    u8g2_DrawXBMP(&u8g2, 110+x*9, MAIN_MNUE_START_Y, 37, 37, stMenu[next_index].pMnue);

	} while (u8g2_NextPage(&u8g2));

}
char sc_buff[128];
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
					
					if(index_menu == MENU_NUMS) index_menu = 0;
					
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
		u8g2_DrawXBMP(&u8g2, 40, UDISK_START_Y, 48, 48, bmpUdisk48x48);
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

const char *voidPageShowStr = "Function not implemented, please press the back button to return to the main menu!";

static void voidPageDrawBackground(void)
{
	const char *p = voidPageShowStr;
    for (;;)
    {
		u8g2_FirstPage(&u8g2);
		do
		{
			if('\0' == *p) p = voidPageShowStr;
			u8g2_DrawStr(&u8g2, 0, 30 , p++);

		} while (u8g2_NextPage(&u8g2));
		vTaskDelay(pdMS_TO_TICKS(200));

  }
}

void extractLastCharacters(const char *filename, int numChars, char *result) {
    int length = strlen(filename);
    if (length <= numChars) {
        strcpy(result+4, filename);
    } else {
        strcpy(result+4, filename + (length - numChars));
    }
}

typedef struct Node {
    char name[50];
	uint8_t isFile;
    struct Node* next;
} Node;

static Node* createNode(const char* name) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    strcpy(newNode->name, name);
    newNode->next = NULL;
    return newNode;
}

static void insertNode(Node** head, Node* newNode) {
    if (*head == NULL) {
        *head = newNode;
    } else {
        Node* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

static void freeList(Node* head) {
    Node* current = head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
}


static Node* files_scan(char *path, uint16_t *fileNums)
{
    FATFS fs;
    FILINFO fileInfo;
    DIR dir;
    FRESULT res;
	uint8_t i = 0;
   Node* head = NULL;
    // 打开根目录
    res = f_opendir(&dir, path);
    if (res != FR_OK) {

        return NULL;
    }
	
	u8g2_SetFont(&u8g2, u8g2_font_t0_11_tr);
    // 遍历根目录下的所有文件

		while (1)
		{
			res = f_readdir(&dir, &fileInfo);
			if (res != FR_OK || fileInfo.fname[0] == 0) {
				// 读取目录项失败或者到达目录末尾，退出循环
				break;
			}
			if (fileInfo.fattrib & AM_DIR) {
				// 是一个目录
				if (strcmp(fileInfo.fname, ".") != 0 && strcmp(fileInfo.fname, "..") != 0) {
					Node* newNode = createNode(fileInfo.fname);
					if(newNode){
						insertNode(&head, newNode);
						newNode->isFile = 0;
					}else
					{
						printf("Creat faile\n");
					}
				}
			} else {
				Node* newNode = createNode(fileInfo.fname);
				if(newNode){
					insertNode(&head, newNode);
					newNode->isFile = 1;
					*fileNums += 1;
				}else
				{
					printf("Creat faile\n");
				}
			}		
		}
    // 关闭目录
    f_closedir(&dir);
	return head;
}

static void pageFileDrawBackground(Node* fileList,uint16_t fileNums)
{
	Node* pList =  fileList;
	uint8_t index = 1;
	uint8_t total =  fileNums/4+1 - !(fileNums%4);
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
	
		}
		while (pList != NULL) {
			
			if(0 == pList->isFile){
				pList = pList->next;
				continue;
			}
			
			if(strlen(pList->name) > 15){
				sc_buff[0] = '0'+index;
				sc_buff[1] = ':';
				sc_buff[2] = ' ';
				sc_buff[3] = '.';

				extractLastCharacters( pList->name,15,sc_buff);
			}
			else
			{
			
				sprintf(sc_buff,"%d: %s",index,  pList->name);	
			}					
			u8g2_DrawStr(&u8g2, 1, index*12,sc_buff);

			printf("%d:%s\n", index,pList->name);
			index++;
			pList = pList->next;
		}
	} while (u8g2_NextPage(&u8g2));
	freeList(fileList);
}

uint8_t pageFileBrowse(void *user_data){

	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	static int index_menu = 0;
	uint16_t maxFileNums = 0;
    Node* fileList = files_scan("1:", &maxFileNums);
	pageFileDrawBackground(fileList,maxFileNums);
	printf("File:%d\n", maxFileNums);
    for (;;)
    {

		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						return (PAGE_SHOW_MAIN);
						printf("key back\n");
					}
					if(RecvMsg.length & 0x06) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						
						printf("key confirm\n");
					}
				}break;
				
				case USER_MSG_CODER:{
					key_coder_buzzer_open(3100,10);						
					printf("coder:%d,%d\n",RecvMsg.length,index_menu);
				}break;
								
				default:break;
			}
		}

    }
	return 0;
}
uint8_t pageVoidDisply(void *user_data)
{
	UserQue_msg_t RecvMsg;
	int8_t  coder = 0;
	static int index_menu = 0;
	u8g2_SetFont(&u8g2, u8g2_font_amstrad_cpc_extended_8r);
	//voidPageDrawBackground();
	const char *p = voidPageShowStr;
    for (;;)
    {
		{
			u8g2_FirstPage(&u8g2);
			do
			{
				if('\0' == *p) p = voidPageShowStr;
				u8g2_DrawStr(&u8g2, 0, 30 , p++);

			} while (u8g2_NextPage(&u8g2));
			vTaskDelay(pdMS_TO_TICKS(100));

		}
		if(UserQueMsgGet(&RecvMsg)) 
		{
			switch(RecvMsg.msg_id){
				
				case USER_MSG_KEY:{
					
					if(RecvMsg.length & 0x01) {
						key_coder_buzzer_open(BEER_FREQ_KEY_BACK,80);
						return (PAGE_SHOW_MAIN);
						printf("key back\n");
					}
					if(RecvMsg.length & 0x06) {
						key_coder_buzzer_open(BEER_FREQ_KEY_CONFIRM,80);
						
						printf("key confirm\n");
					}
				}break;
				
				case USER_MSG_CODER:{
					key_coder_buzzer_open(3100,10);						
					printf("coder:%d,%d\n",RecvMsg.length,index_menu);
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

 