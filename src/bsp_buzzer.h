#ifndef __BSP_BUZZER_H__
#define __BSP_BUZZER_H__

#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */
#include <stdint.h>
#include "ingsoc.h"
    
extern  void setup_buzzer(void);
extern  void set_buzzer_freq(uint16_t freq);
extern  void key_coder_buzzer_open(uint16_t freq,uint16_t time);
extern  void burn_cmpl_buzzer_open(uint16_t freq);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif

