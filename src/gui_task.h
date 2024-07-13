#ifndef __GUI_TASK_H
#define __GUI_TASK_H

#include "ingsoc.h"


typedef uint8_t (*f_page_show)(void *user_data);
typedef struct{     /* 菜单按键 */
    uint8_t index;  /* 索引 */
    uint8_t left;
    uint8_t right;
    uint8_t ok;
    f_page_show fun;
}menu_btn;

enum{   /* 按键类型 */
    OK_KEY = 1,
    LEFT_KEY,
    RIGHT_KEY,
};

extern uint8_t guc_key_state;
uint8_t guc_now_index;
void gui_main_task(void *pdata);


#endif  /* __GUI_TASK_H */

