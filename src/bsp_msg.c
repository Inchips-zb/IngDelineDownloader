#include <stdio.h>
#include <string.h>
#include "ingsoc.h"
#include "platform_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_msg.h"


static xQueueHandle UserQueHandle = NULL;
static xQueueHandle KeyQueHandle = NULL;
#define MSG_SIZE        32
UserQue_msg_t DisplySendMsg;
UserQue_msg_t KeySendMsg;

int UserQue_SendMsg(uint32_t msg_id, const void *param, int length)
{
    int8_t err = 0;
    DisplySendMsg.msg_id = msg_id;
    DisplySendMsg.length = length;
    DisplySendMsg.param = param;
    if(NULL == UserQueHandle) return 1;
    if (IS_IN_INTERRUPT()){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        err = xQueueSendToBackFromISR(UserQueHandle, &DisplySendMsg, &xHigherPriorityTaskWoken);
    } else {
        err = xQueueSendToBack(UserQueHandle, &DisplySendMsg, portMAX_DELAY);
    }
    return err == pdPASS ? 0 : 1;
}

int UserQueMsgGet(UserQue_msg_t *Msg)
{
	return (xQueueReceive(UserQueHandle, Msg, portMAX_DELAY) == pdPASS);
}

int KeyQue_SendMsg(uint32_t msg_id, const void *param, int length)
{
    int8_t err = 0;


    KeySendMsg.msg_id = msg_id;
    KeySendMsg.length = length;
    KeySendMsg.param = param;
    if(NULL == KeyQueHandle) return 1;
    if (IS_IN_INTERRUPT()){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        err = xQueueSendToBackFromISR(KeyQueHandle, &KeySendMsg, &xHigherPriorityTaskWoken);
    } else {
        err = xQueueSendToBack(KeyQueHandle, &KeySendMsg, portMAX_DELAY);
    }
    return err == pdPASS ? 0 : 1;
}

int KeyQueMsgGet(UserQue_msg_t *Msg)
{
	return (xQueueReceive(KeyQueHandle, Msg, portMAX_DELAY) == pdPASS);
}
void UserQueInit(void)
{
    UserQueHandle = xQueueCreate( MSG_SIZE, sizeof(UserQue_msg_t)); 
    KeyQueHandle =  xQueueCreate( MSG_SIZE, sizeof(UserQue_msg_t)); 
}

