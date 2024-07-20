#ifndef __DEVICE_INFO_H__
#define __DEVICE_INFO_H__


#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */
#include <stdint.h>
#include "ingsoc.h"

#pragma pack (push, 1)


typedef struct prog_ver_s
{
    short major;
    char  minor;
    char  patch;
} prog_ver_t;



typedef struct dev_info_s
{
	uint8_t beer_en;
	uint8_t mac[6];
	uint8_t blename[32];
	prog_ver_t ver;
} dev_info_t;


#pragma pack (pop)

#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
#define BLE_MAC_FLASH_ADDRESS  0x82000
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
#define BLE_MAC_FLASH_ADDRESS  0x2082000
#else
#error unknown or unsupported chip family
#endif

#define SOFT_PRO_VER_INVAD       {.major = 2, .minor = 0, .patch = 1}
#define HARD_PRO_VER_INVAD       {.major = 3, .minor = 0, .patch = 1}


dev_info_t *get_device_informs(void);
void storage_device_informs(void);
void load_device_informs(void);
void printf_device_info(void);
void reset_device_informs(void);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif

