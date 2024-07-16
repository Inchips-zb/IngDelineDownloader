#ifndef _UART_BURNER_H
#define _UART_BURNER_H
#include <stdint.h>

#define BURN_BIN_NUM_MAX  6
#pragma pack (push, 1)
typedef struct burn_block
{
	int check;
	char filename[64];
    uint32_t loadaddr;
    uint32_t size;
} burn_block_t;

typedef struct burn_meta
{
    uint16_t sector_size;
    uint16_t page_size;
	uint32_t total_size;
    uint8_t  block_num;    
    burn_block_t blocks[BURN_BIN_NUM_MAX];
} burn_meta_t;

typedef struct downdloader_cfg
{
	uint8_t family;
    uint32_t baud;
    uint32_t timerout;
	int  verify;
    int  protect_enable;    
    int  un_lock;
    int  sn_burn_enable;
    uint8_t  sn_code_len;
    uint32_t sn_code_addr;
    uint32_t sn_start;
    uint16_t sn_step;
	burn_meta_t mate;
} downdloader_cfg_t;

#pragma pack (pop)

extern  void uart_buner_rx_data(const char *d, uint8_t len,uint8_t cmpl);
extern  void uart_buner_start(void);
extern  downdloader_cfg_t *get_downloader_cfg(void);

#endif
