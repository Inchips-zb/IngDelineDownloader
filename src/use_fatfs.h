#ifndef __USE_FATFS_H
#define __USE_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ff.h" 
FRESULT use_fatfs_mount(void);
FRESULT use_fatfs_mkfs(void);

extern void load_downloader_cfg(void);
extern void listRootDirectoryFiles();
#ifdef __cplusplus
}
#endif

#endif