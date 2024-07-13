/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "qspi_flash.h"
#include "platform_api.h"
/* Definitions of physical drive number for each drive */
#define ATA         0     // 
#define SPI_FLASH   1     // SPI Flash


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{

    DSTATUS status = STA_NOINIT;

    switch (pdrv) {
    case ATA: /* SD CARD */
        break;

    case SPI_FLASH:
       if (sFLASH_ID == ReadFlashId()) 
		{
            status &= ~STA_NOINIT;
        } 
		else 
		{
 
            status = STA_NOINIT;;
        }
        break;

    default:
        status = STA_NOINIT;
    }
    return status;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    uint16_t i;
    DSTATUS status = STA_NOINIT;
    switch (pdrv) {
    case ATA:          /* SD CARD */
        break;

    case SPI_FLASH:    /* SPI Flash */
        status=disk_status(SPI_FLASH);
        break;

    default:
        status = STA_NOINIT;
    }
    return status;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    DRESULT status = RES_PARERR;
    switch (pdrv) {
    case ATA: /* SD CARD */
        break;

    case SPI_FLASH:

		ext_flash_read(sector <<12,(uint8_t*)buff, count<<12);
        status = RES_OK;
        break;

    default:
        status = RES_PARERR;
    }
    return status;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    uint32_t write_addr;
    DRESULT status = RES_PARERR;
    if (!count) {
        return RES_PARERR;    /* Check parameter */
    }

    switch (pdrv) {
    case ATA: /* SD CARD */
        break;

    case SPI_FLASH:

	    ext_flash_program(sector<<12,(uint8_t*)buff,count<<12);
	    TMR_WatchDogRestart();

        status = RES_OK;
        break;

    default:
        status = RES_PARERR;
    }
    return status;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	
	switch (pdrv) {
	case ATA :

		return res;

	case SPI_FLASH :

			switch(cmd)
			{
				//返回扇区个数
				case GET_SECTOR_COUNT:
				//所使用的扇区数量，4096*4096/1024/1024 = 16(MB)
						*(DWORD *)buff = 4096 ;   
				break;
				
				//返回扇区大小
				case GET_SECTOR_SIZE:
						*(WORD *)buff = 4096 ;
				break;
				//返回擦除扇区的最小个数
				case GET_BLOCK_SIZE:
						*(WORD *)buff = 1 ;   //每次擦除一个扇区
				break;
			}
		res = RES_OK;         //由此看出，此处的数据返回值不是通过return返回的，如之前的讲解是通过指针返回
		return res;

	}

	return RES_PARERR;
}


DWORD get_fattime(void)
{
    return    ((DWORD)(2015 - 1980) << 25)  /* Year 2015 */
            | ((DWORD)1 << 21)        /* Month 1 */
            | ((DWORD)1 << 16)        /* Mday 1 */
            | ((DWORD)0 << 11)        /* Hour 0 */
            | ((DWORD)0 << 5)         /* Min 0 */
            | ((DWORD)0 >> 1);        /* Sec 0 */
}
