#include <stdio.h>
#include <stdlib.h>
#include "use_fatfs.h"
#include "delay.h"
#include "ingsoc.h"
#include "platform_api.h"
#include "iic.h"
#include "FreeRTOS.h"
#include "string.h"
#include "uart_buner.h"
#include "minIni.h"


#define print_err(fun,line,code) platform_printf("[ERR] %s(%d): %d\n",fun, line, code)
FATFS fs;                         /* FatFs文件系统对象 */


FRESULT use_fatfs_mount(void)
{

    //f_mount()->find_volume()->disk_initialize->SPI_FLASH_Init()
    FRESULT res = f_mount(&fs,"1:",1);
 
    if (res!=FR_OK)
    {
		print_err(__FUNCTION__,__LINE__,res);
		/*----------------------- 格式化测试 -----------------*/
		/* 如果没有文件系统就格式化创建创建文件系统 */
	//if (res == FR_NO_FILESYSTEM) use_fatfs_mkfs();
	}
    else
    {
        printf("FATFS Mount ok!\r\n");
    }
END:
	return res;
}

FRESULT use_fatfs_mkfs(void){
	/* 格式化 */
	FRESULT res = FR_INVALID_PARAMETER;
	BYTE *work = (BYTE *)malloc(FF_MAX_SS);
	if(NULL == work) goto END;
	printf("Start make faftfs!\r\n");
	res = f_mkfs("1:", NULL, work, FF_MAX_SS);
	if (res == FR_OK)
	{
		/* 格式化后，先取消挂载 */
		res = f_mount(NULL,"1:",1);
		/* 重新挂载 */
		res = f_mount(&fs,"1:",1);
	}
	else
	{

		print_err(__FUNCTION__,__LINE__,res);
	}
	free(work);
	work = NULL;
END:	
	return res;
}


const char inifile[] = "1:flash_download.ini";
void extractFileName(const char *filepath, char *filename);
void load_downloader_cfg(void)
{
	char str[100] = {0};
	char fileName[100];
	downdloader_cfg_t *cfg = get_downloader_cfg();
	FRESULT RET = use_fatfs_mount();
	if(FR_OK != RET) { printf("Fatfs mount fail：(%d)\n",RET); return ; }
    ini_gets("main", "family", "dummy", str, 100, inifile);
	if(strcmp("ing916",str) == 0)
	{
		cfg->family = INGCHIPS_FAMILY_916;
		cfg->mate.sector_size = 4096;
		cfg->mate.page_size = 256;
		
	}else if(strcmp("ing918",str) == 0)
	{
	    cfg->family = INGCHIPS_FAMILY_918;
		cfg->mate.sector_size = 8192;
		cfg->mate.page_size = 8192;
	}
	else
	{
		cfg->family = 0xff;
	}
	
	cfg->verify = (int)ini_getl("options", "verify", -1, inifile);
	cfg->protect_enable = (int)ini_getl("options", "protection.enabled", -1, inifile);
	cfg->un_lock = (int)ini_getl("options", "protection.unlock", -1, inifile);
	cfg->timerout = (uint32_t)ini_getl("uart", "Timeout", -1, inifile);
	cfg->baud = (uint32_t)ini_getl("uart", "Baud", 115200, inifile);
	/******bin*********/
	
	cfg->mate.blocks[1].check = (int)ini_getl("bin-1", "Checked", -1, inifile);	
	cfg->mate.blocks[2].check = (int)ini_getl("bin-2", "Checked", -1, inifile);	
	cfg->mate.blocks[3].check = (int)ini_getl("bin-3", "Checked", -1, inifile);	
	cfg->mate.blocks[4].check = (int)ini_getl("bin-4", "Checked", -1, inifile);	
	cfg->mate.blocks[5].check = (int)ini_getl("bin-5", "Checked", -1, inifile);	
	
	cfg->mate.block_num = 0;
	cfg->mate.total_size = 0;
	for(uint8_t i = 0;i < BURN_BIN_NUM_MAX;i++)
	{
		char sect[10] = {0};
		sprintf(sect,"bin-%d",i);
		
		cfg->mate.blocks[i].check = (int)ini_getl(sect, "Checked", -1, inifile);	
		if(cfg->mate.blocks[i].check)
		{
			cfg->mate.block_num += 1;
			cfg->mate.blocks[i].loadaddr = (uint32_t)ini_getl(sect, "Address", -1, inifile);	
		    ini_gets(sect, "FileName", "dummy", str, 100, inifile);
			extractFileName(str,fileName);
		    FILINFO fno;
			sprintf(cfg->mate.blocks[i].filename,"1:%s",fileName);
			RET = f_stat(cfg->mate.blocks[i].filename,&fno);
			if(FR_OK == RET)
			{
				printf("file <%s> size %d \r\n",fno.fname, fno.fsize);
				cfg->mate.blocks[i].size = fno.fsize;
				cfg->mate.total_size += cfg->mate.blocks[i].size;
			}
			else
			{
				printf("<%s> is  non-existent\r\n",cfg->mate.blocks[i].filename);
			
			}
	
		}
	
	}
	
}


void extractFileName(const char *filepath, char *filename) {
    char *lastSlash = strrchr(filepath, '/');
    char *lastBackslash = strrchr(filepath, '\\');
    char *lastSeparator = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;

    if (lastSeparator != NULL) {
        strcpy(filename, lastSeparator + 1);
    } else {
        strcpy(filename, filepath);
    }
}


#include <stdio.h>
#include <string.h>
#include "ff.h"

void listRootDirectoryFiles() {
    FATFS fs;
    FILINFO fileInfo;
    DIR dir;
    FRESULT res;

    // 打开根目录
    res = f_opendir(&dir, "1:");
    if (res != FR_OK) {

        return;
    }

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
			if(strcmp("System Volume Information",fileInfo.fname))
				printf("dir：%s\n", fileInfo.fname);
        } else {
            // 是一个文件
            printf("file：%s\n", fileInfo.fname);
        }
    }

    // 关闭目录
    f_closedir(&dir);

}



void test_spiflash_fafts(void){
	
//	static FILINFO fno;
//	char str[100] = {0};

//	downdloader_cfg_t *cfg = get_downloader_cfg();

//	FRESULT RET = use_fatfs_mount();
//	  /* value reading */
//    long n = ini_getl("bin-0", "Address", -1, inifile);
//	printf("Address:0x%lx\n",n);
//    n = ini_gets("main", "family", "dummy", str, 100, inifile);
//  /* browsing through the file */
//	printf("family:%s\n",str);
//	
	
//	if(FR_OK == RET)
//	{
//		RET = f_open(fnew, inifile, FA_OPEN_EXISTING | FA_READ);
//	        /* 获取文件信息，必须确保文件存在*/
//        RET = f_stat(inifile,&fno);
//		printf("file <%s> size %d \r\n",fno.fname, fno.fsize);
//		if(FR_OK == RET)
//		{
//			uint8_t *buffer = (uint8_t *)malloc(fno.fsize+1); 
//			if(!buffer) return;
//			RET = f_read(fnew, buffer, fno.fsize, &fnum);
//			if (RET == FR_OK)
//			{
//				*(buffer+fnum) = 0;
//				
//				printf("FATFS Read(%d)\r\n%s \r\n", fnum,buffer);
//				
//				free(buffer);
//			}
//			else
//			{
//				printf("！！文件读取失败：(%d)\n",RET);
//			}
//	   }
//	   else
//	   {
//			printf("！！文件打开失败：(%d)\n",RET);
//	   }
// }
//		
//	f_close(fnew);
//	free(fnew);
//  
}

