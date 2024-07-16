#ifndef __USE_FATFS_H
#define __USE_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "ff.h" 
uint8_t use_fatfs_mount(uint8_t mo);

extern void load_downloader_cfg(void);
extern void listRootDirectoryFiles();
#ifdef __cplusplus
}
#endif

#endif