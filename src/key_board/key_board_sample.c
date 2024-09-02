#include "key_board_sample.h"
#include "platform_api.h"
#include "btstack_event.h"
#include "bsp_msg.h"

typedef struct
{
    key_id_hard_e keyHardId;
    enum key_state_t state;
    key_t key;
    eClickState multiType;  //if KEY_PRESS_CONTINUOUS state
    void (*cbFun)(void);
} keyFuncMap_t;

#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
typedef struct
{
    int combineKeyId;
    const struct key_combine_t *pCombineTab;
    uint8_t size;
    key_t key;
    void (*cbFun)(void);
} keyCombineMap_t;

static const struct key_combine_t key_ir_learn[] = {
    { .id = KB_HARD_KEY_QDEC,   .state = KEY_PRESS_LONG },
    { .id = KB_HARD_KEY_BACK,   .state = KEY_PRESS_LONG },
};
static const struct key_combine_t key_adv_start[] = {
    { .id = KB_HARD_KEY_QDEC,   .state = KEY_PRESS_LONG },
    { .id = KB_HARD_KEY_CONFIRM,  .state = KEY_PRESS_LONG },
};
static const struct key_combine_t key_adv_stop[] = {
    { .id = KB_HARD_KEY_BACK,   .state = KEY_PRESS_LONG },
    { .id = KB_HARD_KEY_CONFIRM,   .state = KEY_PRESS_LONG },
};

static void cmbinetest1(void)
{
    platform_printf("cmbinetest1\n");
}
static void cmbinetest2(void)
{
    platform_printf("cmbinetest2\n");
}
static void cmbinetest3(void)
{
    platform_printf("cmbinetest3\n");
}

// Number of members limited to the macro definition KEY_COMBINE_NUM.
static keyCombineMap_t keyCombineMap[] = 
{
    {0,  key_ir_learn,  GET_ARRAY_SIZE(key_ir_learn),    KB_NULL,  cmbinetest1}, // combine1
    {1,  key_adv_start, GET_ARRAY_SIZE(key_adv_start),   KB_NULL,  cmbinetest2}, // combine2
    {2,  key_adv_stop,  GET_ARRAY_SIZE(key_adv_stop),    KB_NULL,  cmbinetest3}, // combine3

};
#endif
static void key1_press_once(void)
{
    platform_printf("%s\n",__FUNCTION__);
}

static void key1_press_long(void)
{
    platform_printf("%s\n",__FUNCTION__);
}


static void key2_press_once(void)
{
    platform_printf("%s\n",__FUNCTION__);
}

static void key2_press_long(void)
{
    platform_printf("%s\n",__FUNCTION__);
}


static void key3_press_once(void)
{
    platform_printf("%s\n",__FUNCTION__);
}

static void key3_press_long(void)
{
    platform_printf("%s\n",__FUNCTION__);
}


static const keyFuncMap_t keyFuncMap[] = {
   //hard id        		trigger type     key_event id        multi key type     event callback     
    {KB_HARD_KEY_QDEC,		KEY_PRESS,       KB_MENU,     	MULTI_CLICK_NONE,  key1_press_once     }, // J1
    {KB_HARD_KEY_QDEC,   	KEY_PRESS_LONG,    KB_POWER_OFF,	MULTI_CLICK_NONE,  key1_press_long     }, // J2

    {KB_HARD_KEY_BACK,    	KEY_PRESS,       KB_EXIT, 		MULTI_CLICK_NONE,  key2_press_once     }, // J1
    {KB_HARD_KEY_BACK,    	KEY_PRESS_LONG,    KB_NULL,       	MULTI_CLICK_NONE,  key2_press_long     }, // J2

    {KB_HARD_KEY_CONFIRM,	KEY_PRESS,       KB_ENTER,		MULTI_CLICK_NONE,  key3_press_once     }, // J1
    {KB_HARD_KEY_CONFIRM,	KEY_PRESS_LONG,    KB_NULL,			MULTI_CLICK_NONE,  key3_press_long     }, // J2

};

const struct key_pin_t key_pin_sig[] = {
    {
        .pin = GIO_GPIO_10,
        .valid = GPIO_PIN_RESET,
        .invalid = GPIO_PIN_SET
    },
    {
        .pin = GIO_GPIO_12,
        .valid = GPIO_PIN_RESET,
        .invalid = GPIO_PIN_SET
    },
    {
        .pin = GIO_GPIO_7,
        .valid = GPIO_PIN_RESET,
        .invalid = GPIO_PIN_SET
    }
};

#if (USER_KEY_BOARD_MATRIX)
const struct key_pin_t key_pin_ctrl[] = {
    {
        .pin = GIO_GPIO_8,
        .valid = GPIO_PIN_SET,
        .invalid = GPIO_PIN_RESET
    },
    {
        .pin = GIO_GPIO_9,
        .valid = GPIO_PIN_SET,
        .invalid = GPIO_PIN_RESET
    },
    {
        .pin = GIO_GPIO_10,
        .valid = GPIO_PIN_SET,
        .invalid = GPIO_PIN_RESET
    }, 
    {
        .pin = GIO_GPIO_34,
        .valid = GPIO_PIN_SET,
        .invalid = GPIO_PIN_RESET
    },    
};
#endif

static inline bool pin_level_get(const void *desc)
{
    struct key_pin_t *pdesc;

    pdesc = (struct key_pin_t *)desc;
    return GIO_ReadValue(pdesc->pin) == pdesc->valid;
}

#if (USER_KEY_BOARD_MATRIX)
static inline void pin_level_set(const void *desc, bool flag)
{
    struct key_pin_t *pdesc;

    pdesc = (struct key_pin_t *)desc;
    //GIO_SetDirection(pdesc->pin, GIO_DIR_OUTPUT);
    GIO_WriteValue(pdesc->pin, flag ? pdesc->valid : pdesc->invalid);
}
#endif

const struct key_public_sig_t key_public_sig[] = {
#if (USER_KEY_BOARD_MATRIX)    
    KEY_PUBLIC_SIG_DEF(KB_HARD_K1, &key_pin_sig[0], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K5, &key_pin_sig[1], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K9, &key_pin_sig[2], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K13, &key_pin_sig[3], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),

    KEY_PUBLIC_SIG_DEF(KB_HARD_K2, &key_pin_sig[0], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K6, &key_pin_sig[1], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K10, &key_pin_sig[2], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K14, &key_pin_sig[3], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),

    KEY_PUBLIC_SIG_DEF(KB_HARD_K3, &key_pin_sig[0], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K7, &key_pin_sig[1], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K11, &key_pin_sig[2], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K15, &key_pin_sig[3], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),

    KEY_PUBLIC_SIG_DEF(KB_HARD_K4, &key_pin_sig[0], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K8, &key_pin_sig[1], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K12, &key_pin_sig[2], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_K16, &key_pin_sig[3], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
#else
    KEY_PUBLIC_SIG_DEF(KB_HARD_KEY_QDEC,    &key_pin_sig[0], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_KEY_BACK,    &key_pin_sig[1], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
    KEY_PUBLIC_SIG_DEF(KB_HARD_KEY_CONFIRM, &key_pin_sig[2], pin_level_get, KEY_FLAG_PRESS_CONTINUOUS | KEY_FLAG_PRESS_LONG | KEY_FLAG_RELEASE_LONG | KEY_FLAG_PRESS_MULTI | KEY_FLAG_RELEASE_MULTI),
#endif
};

#if (USER_KEY_BOARD_MATRIX)
const struct key_public_ctrl_t key_public_ctrl[] = {
    KEY_PUBLIC_CTRL_DEF(&key_pin_ctrl[0], pin_level_set),
    KEY_PUBLIC_CTRL_DEF(&key_pin_ctrl[1], pin_level_set),
    KEY_PUBLIC_CTRL_DEF(&key_pin_ctrl[2], pin_level_set),
    KEY_PUBLIC_CTRL_DEF(&key_pin_ctrl[3], pin_level_set),    
};
#endif

static bool repeat_flag = 0;
static void kb_check_event_callback(void)
{
    bool bFlag = false;
    static uint8_t com_count = 0;
    BaseType_t xReturn = pdPASS;
    for(int i = 0;i < GET_ARRAY_SIZE(keyFuncMap);i++)
    {
        if(KEY_PRESS_MULTI == keyFuncMap[i].state)
        {
            if(keyFuncMap[i].multiType == key_check_state(keyFuncMap[i].keyHardId, keyFuncMap[i].state))
            {
                bFlag = true;
            }
        }
        else
        {
            if(key_check_state(keyFuncMap[i].keyHardId,KEY_RELEASE))
            {
                com_count--;
            }
            if(key_check_state(keyFuncMap[i].keyHardId, keyFuncMap[i].state))
            {
                bFlag = true;  
                if(KEY_PRESS == keyFuncMap[i].state) com_count++;
            }
        }
       if(bFlag)
       {
            if(CB_FUN_NULL != keyFuncMap[i].cbFun)
            {
               // KeyCbFunQueuePush(keyFuncMap[i].cbFun);
				keyFuncMap[i].cbFun();
            }   
            if(KB_NULL != keyFuncMap[i].key.keyId)
            {
				platform_printf("Key id:%x\n",keyFuncMap[i].key.keyId);
				
				UserQue_SendMsg(USER_MSG_KEY,NULL,keyFuncMap[i].key.keyId);

            }
            bFlag = false;
       }
    
    }
    
    repeat_flag = (1 == com_count) ?  true : false; 

    for(int i = 0;i < GET_ARRAY_SIZE(keyCombineMap);i++)
    {
        if(keyCombineMap[i].combineKeyId && key_check_combine_state(keyCombineMap[i].combineKeyId))
        {
            if(CB_FUN_NULL != keyCombineMap[i].cbFun)
            {
               // KeyCbFunQueuePush(keyCombineMap[i].cbFun);
				 keyCombineMap[i].cbFun();
            }
            if(KB_NULL != keyCombineMap[i].key.keyId) 
            {           

				UserQue_SendMsg(USER_MSG_KEY,NULL,keyCombineMap[i].key.keyId);

				platform_printf("Com Key id:%d\n",keyFuncMap[i].key.keyId);
            }    
        }
    
    }
}


static void combine_register(void)
{
    for(int i = 0;i < GET_ARRAY_SIZE(keyCombineMap);i++)
    {
        if(NULL != keyCombineMap[i].pCombineTab)
        {
             keyCombineMap[i].combineKeyId = key_combine_register(keyCombineMap[i].pCombineTab, keyCombineMap[i].size);
        }
    }
}

#if USER_KEY_DEBUG
void key_print_debug_callback(const char *str)
{
    platform_printf("%s\n", str);
}
#endif



/*key timer init*/
static uint32_t timer2_isr(void *user_data);
static void init_timer2(void)
{
    SYSCTRL_ClearClkGateMulti(0
                                | (1 << SYSCTRL_ClkGate_APB_TMR2));
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
    // setup timer 1: 40us (25kHz)
    TMR_SetCMP(APB_TMR1, TMR_CLK_FREQ / KEY_SCAN_RATE);
    TMR_SetOpMode(APB_TMR1, TMR_CTL_OP_MODE_WRAPPING);
    TMR_IntEnable(APB_TMR1);
    TMR_Reload(APB_TMR1);
    TMR_Enable(APB_TMR1);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    TMR_SetOpMode(APB_TMR2, 0, TMR_CTL_OP_MODE_32BIT_TIMER_x1, TMR_CLK_MODE_APB, 0);
    TMR_SetReload(APB_TMR2, 0, TMR_GetClk(APB_TMR2, 0) / KEY_SCAN_RATE);
    TMR_Enable(APB_TMR2, 0, 0xf);
    TMR_IntEnable(APB_TMR2, 0, 0xf);
	TMR_PauseEnable(APB_TMR2,1);
#else
    #error unknown or unsupported chip family
#endif
	platform_set_irq_callback(PLATFORM_CB_IRQ_TIMER2, timer2_isr, NULL);
}

static uint32_t timer2_isr(void *user_data)
{
    key_check();
#if (INGCHIPS_FAMILY == INGCHIPS_FAMILY_918)
    TMR_IntClr(APB_TMR1);
#elif (INGCHIPS_FAMILY == INGCHIPS_FAMILY_916)
    TMR_IntClr(APB_TMR2, 0, 0xf);
#else
    #error unknown or unsupported chip family
#endif
    return 0;
}

void GPIO_Key_Board_Init(void)
{
    unsigned int i;
    SYSCTRL_ClearClkGateMulti(0 | (1 << SYSCTRL_ClkGate_APB_GPIO0)
                                | (1 << SYSCTRL_ClkGate_APB_GPIO1)
                                | (1 << SYSCTRL_ClkGate_APB_PinCtrl));

    for(i = 0;i < GET_ARRAY_SIZE(key_pin_sig);i++)
    {
        PINCTRL_SetPadMux(key_pin_sig[i].pin, IO_SOURCE_GPIO);
        GIO_SetDirection(key_pin_sig[i].pin, GIO_DIR_INPUT);        
    }

#if (USER_KEY_BOARD_MATRIX)
    for(i = 0;i < GET_ARRAY_SIZE(key_pin_ctrl);i++)
    {
        PINCTRL_SetPadMux(key_pin_ctrl[i].pin, IO_SOURCE_GPIO);        
        GIO_SetDirection(key_pin_ctrl[i].pin, GIO_DIR_OUTPUT);
        GIO_WriteValue(key_pin_ctrl[i].pin, 1);
    }
#endif
    key_board_init();
    init_timer2();
#if (USER_KEY_BOARD_MATRIX)
    #if (KEY_EVENT_TRIG_MODE == KEY_TRIG_REPORT)
    key_board_register(KEY_BOARD_MATRIX, key_public_sig, GET_ARRAY_SIZE(key_public_sig), key_public_ctrl, GET_ARRAY_SIZE(key_public_ctrl),kb_check_event_callback);
    #else
    key_board_register(KEY_BOARD_MATRIX, key_public_sig, GET_ARRAY_SIZE(key_public_sig), key_public_ctrl, GET_ARRAY_SIZE(key_public_ctrl),CB_FUN_NULL);
    #endif
#else
    key_board_register(KEY_BOARD_NORMAL, key_public_sig, GET_ARRAY_SIZE(key_public_sig), NULL, 0,kb_check_event_callback);
#endif
#if (KEY_COMBINE_SUPPORT == KEY_ENABLE)
    combine_register();
#endif    
#if USER_KEY_DEBUG    
    key_board_debug_register(key_print_debug_callback);
#endif    

#if(KEY_TRIG_QUERY == KEY_EVENT_TRIG_MODE)  
    cbFunSwTimer = kb_check_event_callback;   
#endif


}
