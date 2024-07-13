#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void delay_ms(int ms);
void delay_us(int us);	

#ifdef __cplusplus
}
#endif

#endif