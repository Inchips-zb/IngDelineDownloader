#ifndef _qspi_flash_h
#define _qspi_flash_h

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define EXT_FLASH_PAGE_SIZE        256
#define EXT_FLASH_SECTOR_SIZE      4096
#define EXT_FLASH_ERASABLE_SIZE    EXT_FLASH_SECTOR_SIZE

#define DummyByte  0xA5
#define DummyWord  0xA5A5A5A5

#define	DEF_FLASH_DEVICE_ERR				((s8)-1)
#define	DEF_FLASH_NONE_ERR					((s8)0)

#define W8_2(addr)   (addr>>16)&0xFFU
#define W8_1(addr)   (addr>>8)&0xFFU
#define W8_0(addr)    addr&0xFFU

#define sFLASH_ID    0x4018
// FLASH Command
#define RdStatusReg1CMD 0x05
#define RdIDCMD        0x90
#define RdGEDECIDCMD        0x9F
#define PowerDownCMD   0xB9
#define WriteEnableCMD 0x06
#define WriteDisableCMD 0x04
#define PageWriteCMD  	0x02
#define ByteReadCMD  		0x03
#define RdStatusReg2CMD   0x35
#define WrStatusRegCMD   0x01
// Datasheet 7.1
#define Buffer_1_WrCMD  0x84
#define Buffer_2_WrCMD  0x87
// Datasheet 7.2
#define Buffer_1_ToMemWithEraseCMD  0x83
#define Buffer_2_ToMemWithEraseCMD  0x86
// Datasheet 7.3
#define Buffer_1_ToMemWithoutEraseCMD  0x88
#define Buffer_2_ToMemWithoutEraseCMD  0x89
// Datasheet 7.4
#define PageEraseCMD  0xFF
// Datasheet 7.5
#define BlockEraseCMD 0xD8
// Datasheet 7.6
#define SectorEraseCMD 0x20
// Datasheet 7.6
#define PageEraseCMD 	0xFF
// Datasheet 7.7
#define ChipEraseCMD1 0xC7
#define ChipEraseCMD2 0x60

#define ChipEraseCMD3 0x80
#define ChipEraseCMD4 0x9A

uint32_t ReadFlashId(void);
signed ext_flash_sector_erase(uint32_t addr);

int ext_flash_program(uint32_t addr, const uint8_t *data, int size);

uint16_t ext_flash_read_status_reg(void);

void ext_flash_set_status_reg(uint16_t data);

void ext_flash_chip_erase(void);

void ext_flash_read(uint32_t addr, uint8_t *buf, uint16_t len);
#ifdef __cplusplus
}
#endif

#endif
