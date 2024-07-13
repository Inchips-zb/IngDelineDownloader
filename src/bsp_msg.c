#include <stdio.h>
#include <string.h>
#include "ingsoc.h"
#include "platform_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_msg.h"


static xQueueHandle UserQueHandle = NULL;
#define MSG_SIZE        32


int UserQue_SendMsg(uint32_t msg_id, const void *param, int length)
{
    int8_t err = 0;
    UserQue_msg_t SendMsg;

    SendMsg.msg_id = msg_id;
    SendMsg.length = length;
    SendMsg.param = param;
    if(NULL == UserQueHandle) return 1;
    if (IS_IN_INTERRUPT()){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        err = xQueueSendToBackFromISR(UserQueHandle, &SendMsg, &xHigherPriorityTaskWoken);
    } else {
        err = xQueueSendToBack(UserQueHandle, &SendMsg, portMAX_DELAY);
    }
    return err == pdPASS ? 0 : 1;
}

int UserQueMsgGet(UserQue_msg_t *Msg)
{
	return (xQueueReceive(UserQueHandle, Msg, portMAX_DELAY) == pdPASS);
}

void UserQueInit(void)
{
    UserQueHandle = xQueueCreate( MSG_SIZE, sizeof(UserQue_msg_t));   
}

