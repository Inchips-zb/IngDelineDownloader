/*
* Copyright (C) INGCHIPS. All rights reserved.
* This code is INGCHIPS proprietary and confidential.
* Any use of the code for whatever purpose is subject to
* specific written permission of INGCHIPS.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ingsoc.h"
#include "platform_api.h"
#include "att_db.h"
#include "ota_service.h"
#include "rom_tools.h"
#include "gap.h"
#include "bluetooth.h"
#include "FreeRTOS.h"

extern  prog_ver_t prog_ver;

#define PAGE_SIZE   EFLASH_ERASABLE_SIZE

#include "../data/gatt.const"
#define ATT_OTA_HANDLE_VER          HANDLE_FOTA_VERSION
#define ATT_OTA_HANDLE_DATA         HANDLE_FOTA_DATA
#define ATT_OTA_HANDLE_CTRL         HANDLE_FOTA_CONTROL

typedef struct{  
     volatile uint8_t  ota_ctrl;
     volatile uint8_t  ota_downloading;
     volatile uint32_t ota_start_addr;
     volatile uint32_t ota_page_offset;
     uint8_t  *pBuf;
}ota_t;


void ota_init_handles(const uint16_t handle_ver, const uint16_t handle_ctrl, const uint16_t handle_data)
{

	
}

static ota_t ota_s = { 
                .ota_ctrl           = OTA_STATUS_DISABLED,
                .ota_downloading    = 0,
                .ota_start_addr     = 0,
                .ota_page_offset    = 0,
                .pBuf        = (void*)NULL
};

static uint8_t  ota_addr_compliance_judgment(const uint32_t flash_addr)
{
    if ((flash_addr & (PAGE_SIZE - 1)) != 0)
         return 2; 
    if ((flash_addr & 0x3) != 0)
         return 3;    
    return 0;
}

int ota_write_callback(uint16_t connection_handle, uint16_t att_handle,
    uint16_t transaction_mode, uint16_t offset, const uint8_t *buffer, uint16_t buffer_size)
{
    if (transaction_mode != ATT_TRANSACTION_MODE_NONE) goto Fail;

    if (att_handle == ATT_OTA_HANDLE_CTRL)
    {
        if (OTA_CTRL_START == buffer[0])
        {
            ota_s.ota_ctrl = OTA_STATUS_OK;
            ota_s.ota_start_addr = 0;
            ota_s.ota_downloading = 0;
            return 0;
        }

        switch (buffer[0])
        {
        case OTA_CTRL_PAGE_BEGIN:
            ota_s.ota_start_addr = *(uint32_t *)(buffer + 1);
            platform_printf("load: 0x%08x\n",  ota_s.ota_start_addr);
            if (ota_addr_compliance_judgment(ota_s.ota_start_addr))
            {
                platform_printf("address disallowed: 0x%08x.\n",  ota_s.ota_start_addr);
                goto Fail;
            }
            else
                ota_s.ota_ctrl = OTA_STATUS_OK;
            ota_s.ota_downloading = 1;
            ota_s.ota_page_offset = 0;
            break;
        case OTA_CTRL_PAGE_END:
            if(ota_s.pBuf)  
            {                
                program_flash(ota_s.ota_start_addr, ota_s.pBuf, ota_s.ota_page_offset);
                ota_s.ota_downloading = 0;
                do
                {
                    uint16_t len = *(uint16_t *)(buffer + 1);
                    uint16_t crc_value = *(uint16_t *)(buffer + 3);
                    if (ota_s.ota_page_offset < len)
                    {
                        ota_s.ota_ctrl = OTA_STATUS_WAIT_DATA;
                        break;
                    }

                    if (crc((uint8_t *)ota_s.ota_start_addr, len) != crc_value)
                       goto Fail;
                    else
                        ota_s.ota_ctrl = OTA_STATUS_OK;
                }while(0);
            }
            else
            {            
                goto Fail;
            }
            break;
        case OTA_CTRL_READ_PAGE:
            if (ota_s.ota_downloading)
            {
                goto Fail;
            }
            else
            {
                ota_s.ota_start_addr = *(uint32_t *)(buffer + 1);
                ota_s.ota_ctrl = OTA_STATUS_OK;
            }
            break;
        case OTA_CTRL_METADATA:
            if (OTA_STATUS_OK != ota_s.ota_ctrl) goto Fail;
        
            if ((0 == ota_s.ota_downloading) || (buffer_size < 1 + sizeof(ota_meta_t)))
            {
                const ota_meta_t  *meta = (const ota_meta_t *)(buffer + 1);
                int s = buffer_size - 1;
                if (crc((uint8_t *)&meta->entry, s - sizeof(meta->crc_value)) != meta->crc_value)
                {
                    goto Fail;
                }
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
                program_fota_metadata(meta->entry,
                                      (s - sizeof(ota_meta_t)) / sizeof(meta->blocks[0]),
                                      meta->blocks);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
                flash_do_update((s - sizeof(ota_meta_t)) / sizeof(meta->blocks[0]),
                                meta->blocks,
                                ota_s.pBuf);
#endif
            }
            else
            {
                    goto Fail;
            }
            break;
        case OTA_CTRL_REBOOT:
            if (OTA_STATUS_OK == ota_s.ota_ctrl)
            {
                if (ota_s.ota_downloading)
                {
                    goto Fail;
                }
                else
                    platform_reset();
            }
            break;
        default:    
                    goto Fail;
        }
    }
    else if (att_handle == ATT_OTA_HANDLE_DATA)
    {
        if (OTA_STATUS_OK == ota_s.ota_ctrl)
        {
            if (   (buffer_size & 0x3) || (0 == ota_s.ota_downloading)
                || (ota_s.ota_page_offset + buffer_size > PAGE_SIZE))
            {        
                goto Fail;
            }
            if(ota_s.pBuf)
            {
                memcpy(ota_s.pBuf + ota_s.ota_page_offset,
                       buffer, buffer_size);
                ota_s.ota_page_offset += buffer_size;
            }
            else
            {
                goto Fail;
            }
        }
    }
    return 0;
Fail:
    platform_printf("OTA Fail.\n");
    ota_s.ota_ctrl = OTA_STATUS_ERROR;
    if(ota_s.pBuf)
    {       
        free(ota_s.pBuf);
        ota_s.pBuf = (void*)NULL;      
    }    
    return 0;
}

int ota_read_callback(uint16_t connection_handle, uint16_t att_handle,
    uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    if (buffer == NULL)
    {
        if (att_handle == ATT_OTA_HANDLE_CTRL)
            return 1;
        else if (att_handle == ATT_OTA_HANDLE_VER)
            return sizeof(ota_ver_t);
        else
            return 0;
    }

    if (att_handle == ATT_OTA_HANDLE_CTRL)
    {
        buffer[0] = ota_s.ota_ctrl;
		ota_s.ota_ctrl = OTA_STATUS_DISABLED;
    }
	 
    /*
     * To use blow code: add "dynamic" property to VERSION characteristics.  */
    else if (att_handle == ATT_OTA_HANDLE_VER)
    {
        ota_ver_t *this_version = (ota_ver_t *)buffer;
        const platform_ver_t * v = platform_get_version();

        this_version->platform.major = v->major;
        this_version->platform.minor = v->minor;
        this_version->platform.patch = v->patch;
        this_version->app = prog_ver;
        if(NULL == ota_s.pBuf){
            ota_s.pBuf = (uint8_t*)malloc(PAGE_SIZE);
            platform_printf("OTA buffer malloc %s.\n",ota_s.pBuf ? "success":"fail");    
        }            
          
    }
  
    return buffer_size;
}

