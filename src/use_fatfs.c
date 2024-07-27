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

#include "ff.h"


#define print_err(fun,line,code) platform_printf("[ERR] %s(%d): %d\n",fun, line, code)
FATFS *fs = NULL;                         /* FatFs�ļ�ϵͳ���� */


uint8_t use_fatfs_mount(uint8_t mo)
{

    //f_mount()->find_volume()->disk_initialize->SPI_FLASH_Init()
	uint8_t res = 0;
	if(mo){
		if(NULL == fs)
			fs = (FATFS*) malloc(sizeof(FATFS));
		else
			printf("Fs maybe has exists!");
		if(NULL == fs)
			goto END;
		
		FRESULT fs_res = f_mount(fs,"1:",1);
	 
		if (res!=FR_OK)
		{
			free(fs);
			fs = NULL;
			print_err(__FUNCTION__,__LINE__,fs_res);
		}
		else
		{
			res = 1;
			printf("FATFS Mount ok!\r\n");
		}		
	}
	else
	{

		if(NULL == fs)
			goto END;
		
		FRESULT fs_res = f_mount(NULL,"1:",1);
	 
		if (res!=FR_OK)
		{
			print_err(__FUNCTION__,__LINE__,fs_res);
		}
		else
		{
			free(fs);
			fs = NULL;
			res = 1;
			printf("FATFS remove ok!\r\n");
		}
	
	}

END:
	return res;
}

FRESULT use_fatfs_mkfs(void){
	/* ��ʽ�� */
	FRESULT res = FR_INVALID_PARAMETER;
	BYTE *work = (BYTE *)malloc(FF_MAX_SS);
	if(NULL == work || NULL == fs) goto END;
	printf("Start make faftfs!\r\n");
	res = f_mkfs("1:", NULL, work, FF_MAX_SS);
	if (res == FR_OK)
	{
		/* ��ʽ������ȡ������ */
		res = f_mount(NULL,"1:",1);
		/* ���¹��� */
		res = f_mount(fs,"1:",1);
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


//const char inifile[] = "1:/918x/flash_download.ini";
void extractFolderPath(const char* filePath, char* folderPath);
void extractFileName(const char *filepath, char *filename);
void load_downloader_cfg(const char *inifile)
{
	char str[100] = {0};
	char fileName[100];
	downdloader_cfg_t *cfg = get_downloader_cfg();
    FRESULT RET;
	if(!use_fatfs_mount(1)) { printf("Fatfs mount fail\n"); return ; }
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
	platform_printf("Chips family:%d\r\n",cfg->family);
	cfg->verify = (int)ini_getl("options", "verify", -1, inifile);
	
	cfg->set_entry =  (int)ini_getl("options", "set-entry", -1, inifile);
	cfg->entry_addr = (uint32_t)ini_getl("options", "entry", 0, inifile);
	platform_printf("entry:%x\r\n",cfg->entry_addr);
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
		if(1 == cfg->mate.blocks[i].check)
		{
			cfg->mate.block_num += 1;
			cfg->mate.blocks[i].loadaddr = (uint32_t)ini_getl(sect, "Address", -1, inifile);	
		    ini_gets(sect, "FileName", "dummy", str, 100, inifile);
			extractFileName(str,fileName);
		    FILINFO fno;
			extractFolderPath(inifile,str);
			sprintf(cfg->mate.blocks[i].filename,"%s/%s",str,fileName);
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
    if(!use_fatfs_mount(0)) { printf("Fatfs remove fail\n"); return ; }
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


void extractFolderPath(const char* filePath, char* folderPath) {
    char* lastSlash = strrchr(filePath, '/');
    if (lastSlash != NULL) {
        size_t pathLength = lastSlash - filePath + 1;
        strncpy(folderPath, filePath, pathLength);
        folderPath[pathLength-1] = '\0';
    } else {
        strcpy(folderPath, "");  // ���δ�ҵ�б���ַ������ļ���·������Ϊ���ַ���
    }
}

fileNode* createFileNode(const char* name) {
    fileNode* newNode = (fileNode*)malloc(sizeof(fileNode));
    strcpy(newNode->name, name);
    newNode->next = NULL;
    return newNode;
}

static void insertFileNode(fileNode** head, fileNode* newNode) {
    if (*head == NULL) {
        *head = newNode;
    } else {
        fileNode* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void freeFileList(fileNode* head) {
    fileNode* current = head;
    while (current != NULL) {
        fileNode* temp = current;
        current = current->next;
        free(temp);
    }
}


fileNode* files_scan(char *path, uint16_t *fileNums)
{
    FATFS fs;
    FILINFO fileInfo;
    DIR dir;
    FRESULT res;
	uint8_t i = 0;
	
    fileNode* head = NULL;
	if(!use_fatfs_mount(1)) return NULL;
    // �򿪸�Ŀ¼
    res = f_opendir(&dir, path);
    if (res != FR_OK) {

        return NULL;
    }
	
    // ������Ŀ¼�µ������ļ�

		while (1)
		{
			res = f_readdir(&dir, &fileInfo);
			if (res != FR_OK || fileInfo.fname[0] == 0) {
				// ��ȡĿ¼��ʧ�ܻ��ߵ���Ŀ¼ĩβ���˳�ѭ��
				break;
			}
			if (fileInfo.fattrib & AM_DIR) {
				// ��һ��Ŀ¼
				if (strcmp(fileInfo.fname, ".") != 0 && strcmp(fileInfo.fname, "..") != 0) {
					if(0 == strcasecmp(fileInfo.fname,"System Volume Information")) continue;
					fileNode* newNode = createFileNode(fileInfo.fname);
					if(newNode){
						insertFileNode(&head, newNode);
						newNode->isFile = 0;
						*fileNums += 1;
					}else
					{
						printf("Creat faile\n");
					}
				}
			} else {
				fileNode* newNode = createFileNode(fileInfo.fname);
				if(newNode){
					insertFileNode(&head, newNode);
					newNode->isFile = 1;
					*fileNums += 1;
				}else
				{
					printf("Creat faile\n");
				}
			}		
		}
    // �ر�Ŀ¼
    f_closedir(&dir);
	use_fatfs_mount(0);
	return head;
}




