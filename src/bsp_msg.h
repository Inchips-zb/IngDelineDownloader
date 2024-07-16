#ifndef __BSP_MSG_H__
#define __BSP_MSG_H__


#ifdef	__cplusplus
extern "C" {	/* allow C++ to use these headers */
#endif	/* __cplusplus */
#include <stdint.h>
#include "ingsoc.h"

typedef struct UserQue_msg
{
    uint32_t        msg_id;
    int       		length;
    const void      *param;
}UserQue_msg_t;

enum{
    USER_MSG_KEY = 0,
	USER_MSG_CODER,
	USER_MSG_RTC_S,
	USER_MSG_BURN_STEP,
	USER_MSG_BURN_STATE,
	USER_MSG_BLE_STATE,
	USER_MSG_BLE_REFRESH,
	USER_MSG_END
};

extern  int UserQue_SendMsg(uint32_t msg_id, const void *param, int length);
extern  int UserQueMsgGet(UserQue_msg_t *Msg);
extern  void UserQueInit(void);

#ifdef __cplusplus
} /* allow C++ to use these headers */
#endif	/* __cplusplus */

#endif

