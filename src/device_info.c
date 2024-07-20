#include <string.h>
#include <stdio.h>
#include "device_info.h"
#include "kv_storage.h"
#include "ingsoc.h"
#include "btstack_util.h"
#include "platform_api.h"



enum
{
    KV_USER_DATA_STORAGE_KEY = KV_USER_KEY_START,
};

dev_info_t dev_info =
{
	.beer_en = 0,
	.mac = {0xc0,0x0c,0x00,0x01,0x02,0x00},
	.blename = "Buner",
	.ver = {1,0,4}
};

const dev_info_t dev_info_default =
{
	.beer_en = 0,
	.mac = {0xc0,0x0c,0x00,0x01,0x02},
	.blename = "Buner",
	.ver = {1,0,4}
};



static int kv_put_if_changed(const kvkey_t key, const void *data, int16_t len)
{
    int16_t cur_len = 0;
    const uint8_t *cur = kv_get(key, &cur_len);
    if ((cur_len == len) && (memcmp(cur, data, len) == 0)) return 1;
    kv_put(key, data, len);
    kv_commit(1);
    return 0;
}


void storage_device_informs(void)
{
   kv_put_if_changed(KV_USER_DATA_STORAGE_KEY,(uint8_t*)&dev_info,sizeof(dev_info_t));
}

void load_device_informs(void)
{
    dev_info_t *pDev = get_device_informs();    
    if (kv_get(KV_USER_DATA_STORAGE_KEY, NULL) == NULL)
    {
        // init device info
        kv_put(KV_USER_DATA_STORAGE_KEY, NULL, sizeof(dev_info_t));
        
        storage_device_informs();
        platform_printf("Init device info\n");
    }
    else{
        memcpy(pDev, kv_get(KV_USER_DATA_STORAGE_KEY, NULL), sizeof(dev_info_t));
     //   platform_printf("Load device info:0x%x\n",pDev->hasBond);
    }
}

void reset_device_informs(void)
{
    dev_info_t *pDev = get_device_informs();    
    memcpy(pDev, &dev_info_default, sizeof(dev_info_t));
    storage_device_informs();
//    platform_printf("Reset device info:0x%x\n",pDev->hasBond);
}

void printf_device_info(void)
{
    dev_info_t *pDev = get_device_informs();
    platform_printf("Load device info:\n");

}

dev_info_t *get_device_informs(void)
{
   return &dev_info;
}


