#ifndef __USE_U8G2_H
#define __USE_U8G2_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "u8g2.h"

u8g2_t *get_u8g2(void);
void setup_oled_io_init(void);
void u8g2Init(u8g2_t *u8g2);

#ifdef __cplusplus
}
#endif

#endif