#ifndef __KEY_BOARD_SAMPLE_H
#define __KEY_BOARD_SAMPLE_H

#include "ingsoc.h"
#include "key_board.h"
#include "trace.h"
#include "FreeRTOS.h"
#include "queue.h"
typedef uint8_t KeyId_t;

//Enable this macro if it is a matrix keyboard
#define USER_KEY_BOARD_MATRIX 0

#define USER_KEY_DEBUG        1
//hz 1ms

#define COMPLEX_TIMER_INTERVAL     20
#define CB_FUN_NULL             (void*)NULL

#define KB_NULL         0
#define KB_POWER        0x66
#define KB_PAGE_UP      0x4B
#define KB_PAGE_DOWN    0x4E
#define KB_MENU         0x76
#define KB_HOME         0x4A
#define KB_VOICE        0x3E
#define KB_ENTER        0x28        
#define KB_EXIT         0x29
#define KB_LEFT         0x50     
#define KB_RIGHT        0x4F
#define KB_UP           0x52
#define KB_DOWN         0x51
#define KB_VOLUME_MUTE  0x7F
#define KB_VOLUME_UP    0x80
#define KB_VOLUME_DOWN  0x81
#define KB_VOICE_STOP   0x3F

#define KB_POWER_OFF     0xFE
typedef enum
{   
    GPIO_PIN_RESET = 0u,
    GPIO_PIN_SET = 1u,    
}GPIO_PinState;
typedef enum
{   
    MULTI_CLICK_NONE = 0,
    DOUBLE_CLICK = 2u   ,
    TRIPLE_CLICK        , 
    FOUR_CLICK          ,   
    FIVE_CLICK          ,
    SIX_CLICK           ,
    SEVEN_CLICK         ,
    EIGHT,CLICK    
} eClickState;

struct key_pin_t {
    GIO_Index_t pin;
    GPIO_PinState valid;
    GPIO_PinState invalid;
};
typedef struct 
{
    KeyId_t keyId;
}key_t;

extern QueueHandle_t xKeyQueue;
/*key bord init*/
void GPIO_Key_Board_Init(void);
key_t key_id_get(void);
#endif/*__KEY_BOARD_SAMPLE_H*/
