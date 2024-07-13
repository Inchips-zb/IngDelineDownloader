/**
***************************************************************************
* @file     gui_task.c
* @author   BJX
* @version  V1.0
* @date     2024-01-30
* @brief    gui ��������
***************************************************************************
* @attention
*
*   ƽ̨: F103RC + u8g2 ʵ��OLED�༶�˵�
*
***************************************************************************
*/

#include "gui_task.h"
#include "gui_menu.h"


uint8_t guc_now_index = 0;       /* ��ǰ���� */
uint8_t guc_last_index = 0;      /* �ϸ����� */
uint8_t guc_key_state = 0;      /* �˵�����״̬ */

const menu_btn t_menu_btn[] = {
/* ������  ���  �Ҽ�  ȷ����  �˵����� */
	{0,     3,    4,    5,    pageDrowLogDisply},
    {1,     3,    4,    5,    pageLoadingDisply},
    {2,     3,    4,    5,    pageClockDisply},
};

void (*show_func)(void);
f_page_show fGobalShow;
/**
 * @brief  gui ������
 * @param  ��
 * @retval ��
 */
void gui_main_task(void *pdata)
{
    guc_last_index = guc_now_index;
    
    switch (guc_key_state)
    {
    case OK_KEY:
        
        guc_now_index = t_menu_btn[guc_now_index].ok;
    break;
    
    case LEFT_KEY:;
        guc_now_index = t_menu_btn[guc_now_index].left;

    break;
        
    case RIGHT_KEY:
        guc_now_index = t_menu_btn[guc_now_index].right;
    break;
        
    default:
        break;
    }
    guc_key_state = 0;
    
    fGobalShow = t_menu_btn[guc_now_index].fun;
    fGobalShow(NULL);
}

/****************************** END OF FILE ******************************/
