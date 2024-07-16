#ifndef __USE_FATFS_H
#define __USE_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ff.h" 
	
typedef struct fileNode {
    char name[50];
	uint8_t isFile;
    struct fileNode* next;
} fileNode;

extern uint8_t use_fatfs_mount(uint8_t mo);
extern void load_downloader_cfg(void);
extern void listRootDirectoryFiles();
extern fileNode* files_scan(char *path, uint16_t *fileNums);
extern void freeFileList(fileNode* head);
#ifdef __cplusplus
}
#endif

#endif