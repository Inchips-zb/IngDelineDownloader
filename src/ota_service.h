/*
* Copyright (C) INGCHIPS. All rights reserved.
* This code is INGCHIPS proprietary and confidential.
* Any use of the code for whatever purpose is subject to
* specific written permission of INGCHIPS.
*/

#ifndef _ota_service_h
#define _ota_service_h

#include <stdint.h>
#include "eflash.h"


typedef struct prog_ver
{
    short major;
    char  minor;
    char  patch;
} prog_ver_t;

typedef struct ota_ver
{
    prog_ver_t platform;
    prog_ver_t app;
} ota_ver_t;

#pragma pack (push, 1)
typedef struct ota_meta
{
    uint16_t crc_value;
    uint32_t entry;
    fota_update_block_t blocks[0];
} ota_meta_t;
#pragma pack (pop)

#define OTA_CTRL_START              0xAA // param: no
#define OTA_CTRL_PAGE_BEGIN         0xB0 // param: page address (32 bit), following DATA contains the data
#define OTA_CTRL_PAGE_END           0xB1 // param: size (16bit), crc (16bit)
#define OTA_CTRL_READ_PAGE          0xC0 // param: page address, following DATA reading contains the data
#define OTA_CTRL_SWITCH_APP         0xD0 // param: no
#define OTA_CTRL_METADATA           0xE0 // param: ota_meta_t
#define OTA_CTRL_REBOOT             0xFF // param: no

#define OTA_STATUS_DISABLED         0
#define OTA_STATUS_OK               1
#define OTA_STATUS_ERROR            2
#define OTA_STATUS_WAIT_DATA        3
extern prog_ver_t prog_ver;
void ota_init_handles(const uint16_t handle_ver, const uint16_t handle_ctrl, const uint16_t handle_data);
int ota_write_callback(uint16_t connection_handle, uint16_t att_handle,
    uint16_t transaction_mode, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size);

int  ota_read_callback(uint16_t connection_handle, uint16_t att_handle,
    uint16_t offset, uint8_t * buffer, uint16_t buffer_size);

#endif
