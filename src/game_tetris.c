#include <stdio.h>
#include <string.h>
#include "ingsoc.h"
#include "platform_api.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "bsp_msg.h"
#include "game_tetris.h"

#include "FreeRTOS.h"
#include "task.h"
#include "delay.h"

static void interface(u8g2_t *u8g2,game_teris_t *teris);
static void remove_block(game_teris_t *teris);
static void block_fixed(game_teris_t *teris);     /*�жϷ����Ƿ��ܹ��ƶ�*/
static void create_box_next(uint8_t a, game_teris_t *teris); /*������ʾ��һ�����飬������Ĳ���һ��*/
static void create_box(uint8_t a, game_teris_t *teris);  /*����block�����е�ֵ������Ϊ0��ʱ�����������Ϊ1ʱд��*/
static void block_go(game_teris_t *teris)  ;

static void draw_block(u8g2_t *u8g2,game_teris_t *teris)     /*���Ʒ���*/
{
	uint8_t block_x , block_y;
	for (block_x = 0 ; block_x < 10 ; block_x ++)
		for (block_y = 0 ; block_y < 20 ; block_y ++)
		{
		  if (teris->block[block_x][block_y] == 1)
			u8g2_DrawBox(u8g2,2 * block_x + 28 , 2 * block_y + 4 , 2 , 2);
		 
		}
}

static void draw_frame(u8g2_t *u8g2)          /*������Ϸ�߿�*/
{
  uint8_t frame_x , frame_y ;
  for (frame_y = 0 ; frame_y <= 64 ; frame_y += 4)
  {
    u8g2_DrawFrame(u8g2, 0, frame_y, 4, 4);
    u8g2_DrawFrame(u8g2, 100, frame_y, 4, 4);
  }
  for (frame_x = 0 ; frame_x <= 100 ; frame_x += 4)
  {
    u8g2_DrawFrame(u8g2, frame_x, 0, 4, 4);
    u8g2_DrawFrame(u8g2, frame_x, 60, 4, 4);
  }
}

void game_begin(u8g2_t *u8g2,game_teris_t *teris) 
{

  u8g2_FirstPage(u8g2);
  do
  {
    draw_frame(u8g2);   /*�߿�*/
//    draw_block(u8g2,teris);
//    interface(u8g2,teris);
  } while (u8g2_NextPage(u8g2));
//  block_go(teris);
//  remove_block(teris);

}
static void block_go(game_teris_t *teris)             /*������ƶ��ͱ���*/
{
  block_fixed(teris);
  if (teris->move_down == 1 )    /*���鲻���½���ʱ�������µķ���*/
  {
    for (int m = 0; m < 10; m++)
    {
      if (teris->block[m][3] == 1) /*�ж���Ϸ�Ƿ����*/
      {
      //  game_over();
        break;
      }
    }
    teris->block_coor.x = 5 ,  teris->block_coor.y = 2;    /*���ó�ʼ���ĵ�λ��*/
    teris->block_type_now = teris->block_type_next ;
	
    teris->block_type_next = platform_rand()%19 +1; 
    teris->move_right = 0 ;
    teris->move_left = 0;
    teris->move_down = 0;
    create_box(0,teris);
  }

  if ( teris->move_down == 0)  /*����״̬Ϊ0ʱ�����½�*/
  {
    create_box(0,teris);         /*�����һ���������Ϣ����ֹ���µķ������Ӱ��*/
     teris->block_coor.y++;
    create_box(1,teris);        /*�������ĵ����������д�뷽����Ϣ*/
  }
  /*�ж��Ƿ��ܹ��ƶ�*/
  switch (teris->dir)
  {
    case 1:           /*�任������״*/
      create_box(0,teris);
      if ( teris->block_type_now >= 1 && teris->block_type_now <= 4)   /*��ֹ�任��ʱ�򷽿���״�����任*/
      {
        teris->block_type_now++;
        if (teris->block_type_now > 4)
          teris->block_type_now = 1;
      }
      else if ( teris->block_type_now >= 5 && teris->block_type_now <= 8)
      {
        teris->block_type_now++;
        if (teris->block_type_now > 8)
          teris->block_type_now = 5;
      }
      else if ( teris->block_type_now >= 9 && teris->block_type_now <= 12)
      {
        teris->block_type_now++;
        if (teris->block_type_now > 12)
          teris->block_type_now = 9;
      }
      else if ( teris->block_type_now >= 13 && teris->block_type_now <= 14)
      {
        teris->block_type_now++;
        if (teris->block_type_now > 14)
          teris->block_type_now = 13;
      }
      else if ( teris->block_type_now >= 15 && teris->block_type_now <= 16)
      {
        teris->block_type_now++;
        if (teris->block_type_now > 16)
          teris->block_type_now = 15;
      }
      else if ( teris->block_type_now >= 17 && teris->block_type_now <= 18)
      {
        teris->block_type_now++;
        if (teris->block_type_now > 18)
          teris->block_type_now = 17;
      }
      else if ( teris->block_type_now == 19)
      {
        teris->block_type_now = 19;
      }
      teris->dir = 0;
      create_box(1,teris);
      delay_ms(50);
      break;
    case 2:          /*��������*/
      if ( teris->move_right == 0)
      {
        create_box(0,teris);   /*����ϸ�������Ϣ*/
         teris->block_coor.x++;
        create_box(1,teris);
        teris->dir  = 0;
      }
      delay_ms(50);
      break;
    case 4:         /*��������*/
      if ( teris->move_left == 0)
      {
        create_box(0,teris);
         teris->block_coor.x--;
        create_box(1,teris);
        teris->dir  = 0;
      }
      delay_ms(50);
      break;
  }
}


static void create_box(uint8_t a, game_teris_t *teris)  /*����block�����е�ֵ������Ϊ0��ʱ�����������Ϊ1ʱд��*/
{
  switch (teris->block_type_now)
  {
    case 1:
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break ;
    case 2:
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 3:
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 4:
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 5:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 2] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      break;
    case 6:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x + 2][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      break;
    case 7:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 2] = a;
      break;
    case 8:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 2][teris->block_coor.y] = a;
      break;
    case 9:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 2] = a;
      break;
    case 10:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x + 2][teris->block_coor.y] = a;
      break;
    case 11:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 2] = a;
      break;
    case 12:
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 2][teris->block_coor.y] = a;
      break;
    case 13:
      teris->block[teris->block_coor.x][teris->block_coor.y + 2] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 14:
      teris->block[teris->block_coor.x + 2][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 15:
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 16:
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 17:
      teris->block[teris->block_coor.x - 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 18:
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
    case 19:
      teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x + 1][teris->block_coor.y] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y + 1] = a;
      teris->block[teris->block_coor.x][teris->block_coor.y] = a;
      break;
  }
}

static void create_box_next(uint8_t a, game_teris_t *teris) /*������ʾ��һ�����飬������Ĳ���һ��*/
{
  switch (teris->block_type_next)
  {
    case 1:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break ;
    case 2:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 3:
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 4:
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 5:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 2] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      break;
    case 6:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x + 2][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      break;
    case 7:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 2] = a;
      break;
    case 8:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 2][teris->block_coor_next.y] = a;
      break;
    case 9:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 2] = a;
      break;
    case 10:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x + 2][teris->block_coor_next.y] = a;
      break;
    case 11:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 2] = a;
      break;
    case 12:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 2][teris->block_coor_next.y] = a;
      break;
    case 13:
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 2] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 14:
      teris->block_next[teris->block_coor_next.x + 2][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 15:
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 16:
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 17:
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y - 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 18:
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x - 1][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
    case 19:
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x + 1][teris->block_coor_next.y] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y + 1] = a;
      teris->block_next[teris->block_coor_next.x][teris->block_coor_next.y] = a;
      break;
  }
}

static void block_fixed(game_teris_t *teris)      /*�жϷ����Ƿ��ܹ��ƶ�*/
{
  switch (teris->block_type_now)
  {
    case 1:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 2:
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 3:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 4:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 5:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 2] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 2] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 6:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 3][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 7)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 2][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 7:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 2] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 2] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 3] == 1 || teris->block_coor.y >= 17)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 8:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 3][teris->block_coor.y] == 1 || teris->block_coor.x <= 2)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 2][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 9:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 2] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 2] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 10:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 3][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block_coor.x >= 7)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 2][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 11:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 2] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 2] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 3] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 17)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 12:
      if (teris->block[teris->block_coor.x - 3][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 2)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 2][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 13:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 2] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 2] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 3] == 1 || teris->block_coor.y >= 17)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 14:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 3][teris->block_coor.y] == 1 || teris->block_coor.x >= 7)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 2][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 15:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y - 1] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 2][teris->block_coor.y - 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 16:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y - 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 19)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 17:
      if (teris->block[teris->block_coor.x - 2][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x - 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y - 1] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 9)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 18:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 2][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 1)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 2] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 1] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
    case 19:
      if (teris->block[teris->block_coor.x - 1][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x - 1][teris->block_coor.y + 1] == 1 || teris->block_coor.x <= 0)
        teris->move_left = 1;
      else
        teris->move_left = 0;
      if (teris->block[teris->block_coor.x + 2][teris->block_coor.y] == 1 || teris->block[teris->block_coor.x + 2][teris->block_coor.y + 1] == 1 || teris->block_coor.x >= 8)
        teris->move_right = 1;
      else
        teris->move_right = 0;
      if (teris->block[teris->block_coor.x][teris->block_coor.y + 2] == 1 || teris->block[teris->block_coor.x + 1][teris->block_coor.y + 2] == 1 || teris->block_coor.y >= 18)
        teris->move_down = 1;
      else
        teris->move_down = 0;
      break;
  }
}

static void remove_block(game_teris_t *teris) 
{
  uint8_t sum = 0, m, n, i = 0 , h;       /*i����Ҫ������������h:��¼����һ����Ҫ����*/
  /*�ж��ж�������Ҫ����*/
  for ( m = 19 ; m > 4; m--)
  {
    for ( n = 0 ; n < 10 ; n++)
      sum += teris->block[n][m];
    if (sum == 10)
    {
      i++;
      teris->score += 10;
      h = m;
      teris->move_down = 1 ;
    }
    else
      sum = 0;
  }
  /*���������е����������ÿһ�ж������ƶ�i����λ*/
  for ( ; i > 0; i--)
    for ( m = h; m >= 3; m--)
      for ( n = 0; n < 10; n++)
        teris->block[n][m] = teris->block[n][m - 1];
}

static void interface(u8g2_t *u8g2,game_teris_t *teris) 
{
//  u8g2_SetFont(u8g_font_timR08);
//  u8g2_DrawStr(57, 10, "Score");
//  u8g2_DrawStr(57, 30, "Grade");
//  u8g2_DrawStr(0, 10, "Next");
//  u8g2_SetPrintPos(70, 20);
//  /*��ʾ����*/
//  u8g.print(score);
//  /*��ʾ�ȼ�*/
//  if ( move_speed <= 300)
//    u8g.drawBox(60, 42, 3, 6);
//  if ( move_speed <= 250)
//    u8g.drawBox(65, 39, 3, 9);
//  if ( move_speed <= 200)
//    u8g.drawBox(70, 36, 3, 12);
//  if ( move_speed <= 150)
//    u8g.drawBox(75, 33, 3, 15);
//  if (move_speed <= 100)
//    u8g.drawBox(80, 30, 3, 18);
  /*��ʾ��һ������*/
  create_box_next(1,teris);
  for (int m = 0 ; m < 5 ; m++)
    for (int n = 0 ; n < 5 ; n++)
    {
      if ( teris->block_next[m][n] == 1)
        u8g2_DrawBox(u8g2, 3 * m + 5, 3 * n + 15, 3, 3);
    }
  create_box_next(0,teris);
}

