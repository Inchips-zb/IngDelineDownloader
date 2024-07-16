#ifndef __BMP_H__
#define __BMP_H__


#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */
#include <stdint.h>
#include "ingsoc.h"

typedef struct bmp_s
{
	uint16_t xSize;
	uint16_t ySize;
	const unsigned char *pBitmap;
}bmp_t;

extern const bmp_t  bmpIngLogo;
extern const bmp_t  bmpErase;
extern const bmp_t  bmpClock;
extern const bmp_t  bmpFiles;
extern const bmp_t  bmpSet;
extern const bmp_t  bmpBurn;
extern const bmp_t  bmpBle;
extern const bmp_t  bmpUdisk;
#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif
