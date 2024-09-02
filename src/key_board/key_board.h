#ifndef __KEY_BOARD_H
#define __KEY_BOARD_H

#include "key_board_config.h"

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define GET_ARRAY_SIZE(x)    (sizeof(x) / sizeof(*(x)))

#define KEY_FLAG_NONE                                 (0u << 0)

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
#define KEY_FLAG_PRESS_LONG                           (1u << 0)
#define KEY_FLAG_RELEASE_LONG                         (1u << 1)
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
#define KEY_FLAG_PRESS_MULTI                          (1u << 2)
#define KEY_FLAG_RELEASE_MULTI                        (1u << 3)
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
#define KEY_FLAG_PRESS_CONTINUOUS                     (1u << 4)
#endif

#define KEY_PUBLIC_SIG_DEF(_id, _pin_desc, _get, _function) \
    {                                                       \
        .id = _id,                                          \
        .pin_desc = _pin_desc,                              \
        .get = _get,                                        \
        .function = _function                               \
    }

#define KEY_PUBLIC_CTRL_DEF(_pin_desc, _set)                \
    {                                                       \
        .pin_desc = _pin_desc,                              \
        .set = _set,                                        \
    }

enum key_state_t {
    KEY_NONE                  = 0u << 0,        /*NONE KEY*/
    KEY_RELEASE               = 1u << 0,        /*RELEASE*/
    KEY_PRESS                 = 1u << 1,        /*PRESS*/
    KEY_PRESSING              = 1u << 2,        /*PRESSING*/
    KEY_PRESS_DEBOUNCING      = 1u << 3,        /*PRESS DEBOUNCING*/
    KEY_PRESSING_DEBOUNCING   = 1u << 4,        /*PRESSING DEBOUNCING*/

#if (KEY_LONG_SUPPORT == KEY_ENABLE)
    KEY_PRESS_LONG            = 1u << 5,        /*Long press (automatically triggered after reaching the long press time)*/
    KEY_RELEASE_LONG          = 1u << 6,        /*Long press (triggered after reaching the long press time and bouncing)*/
#endif

#if (KEY_MULTI_SUPPORT == KEY_ENABLE)
    KEY_PRESS_MULTI           = 1u << 7,        /*Multiple hits (multiple presses)*/
    KEY_RELEASE_MULTI         = 1u << 8,        /*Multiple hits (multiple bounces)*/
#endif

#if (KEY_CONTINUOUS_SUPPORT == KEY_ENABLE)
    KEY_PRESS_CONTINUOUS      = 1u << 9,        /*Multiple click*/
#endif
};

typedef bool (*pin_level_get_callback)(const void *);
typedef void (*pin_level_set_callback)(const void *, bool);
typedef void (*print_debug_callback)(const char *);

typedef enum  {
    KB_HARD_KEY_QDEC= 0, 
    KB_HARD_KEY_BACK, 
    KB_HARD_KEY_CONFIRM, 
    KB_HARD_END
}key_id_hard_e;

enum key_board_type_t {
    KEY_BOARD_NORMAL,       //Common single IO control button
    KEY_BOARD_MATRIX        //Matrix keyboard for row and column scanning
};

struct key_public_sig_t {
    unsigned int id;            //Pin id (Note: cannot repeat!!! It is better to set it as a continuous value)
    const void *pin_desc;       //Pin Description
    pin_level_get_callback get; //Signal pin level status acquisition
    unsigned int function;      //Key function sign
};

struct key_public_ctrl_t {
    const void *pin_desc;       //Pin Description
    pin_level_set_callback set; //Control pin level state setting
};

struct key_combine_t {
    key_id_hard_e id;            //hard Key ID
    enum key_state_t state;     //Key state
};

/**
* @brief    init key board
* @retval   Successfully returned 0,failed return -1
*/
int key_board_init(void);

/**
* @brief    Register a keyboard
* @param    type: keyboard type
* @param    sig: Relevant information of signal line
* @param    key_sig_n: Number of signal lines
* @param    ctrl: Relevant information of control line (only required when the type is matrix keyboard)
* @param    key_ctrl_n: Number of control lines (only required when the type is matrix keyboard)
* @retval   Keyboard object pointer registered successfully, and NULL returned in case of failure
*/
struct key_board_t *key_board_register(enum key_board_type_t type, const struct key_public_sig_t sig[], unsigned int key_sig_n, const struct key_public_ctrl_t ctrl[], unsigned int key_ctrl_n,void (*fun)(void));

/**
* @brief    Uninstall a registered keyboard
* @param    obj: Registered keyboard object
* @retval   void
*/
void key_board_unregister(struct key_board_t *obj);

/**
* @brief    uninit key board
* @retval   void
*/
void key_board_destroy(void);

/**
* @brief    Check the specified key status
* @param    id: key id
* @param    state: Key status to check
* @retval   If the key status to be checked is multi hit and is in multi hit status, 
            the current number of multi hit will be returned; otherwise, 0 will be returned;
            If the key status to be checked is other and in the status to be checked, it returns 1; 
            otherwise, it returns 0
*/
unsigned int key_check_state(unsigned int id, enum key_state_t state);

/**
* @brief    get key board press count
* @retval   void
*/
unsigned int key_press_count_get(void);

/**
* @brief    Register the specified combination status
* @param    Combination information to check
* @param    Number of combined information groups to check
* @retval   An ID is returned if the registration succeeds, and 0 is returned if the registration fails
*/
int key_combine_register(const struct key_combine_t c[], unsigned int n);

/**
* @brief    Uninstall the specified combined state
* @retval   void
*/
void key_combine_unregister(unsigned int id);

/**
* @brief    Check the specified combination status
* @param    id Combination id to check
* @retval   If the combined status to be checked meets the conditions, it returns 1; otherwise, it returns 0
*/
unsigned int key_check_combine_state(int id);

/**
* @brief    Keyboard Scan Function(1ms proid)
* @retval   void
*/
void key_check(void);

/**
* @brief    Register debugging information output function (it is better not to register when not in use, and it should be called before all other interfaces)
* @param    print_debug: Debug information output function to be registered
* @retval   void
*/
void key_board_debug_register(print_debug_callback print_debug);

#endif/*__KEY_BOARD_H*/
