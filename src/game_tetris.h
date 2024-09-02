/** @file eeprom.h
*  @brief API for eeprom init by ingchips 91xx
*
*  INGCHIPS confidential and proprietary.
*  COPYRIGHT (c) 2018-2023 by INGCHIPS
*
*  All rights are reserved. Reproduction in whole or in part is
*  prohibited without the written consent of the copyright owner.
*
*
*/

#ifndef _GAME_TETRIS_H_
#define _GAME_TETRIS_H_

#include "stdint.h"
#include "u8g2.h"

struct coordinate_t
{
	uint8_t x;
	uint8_t y;
};

typedef struct 
{
    uint8_t	block[10][20];
    uint8_t	block_next[5][5];
	uint8_t block_type_now;
	uint8_t block_type_next;
	struct coordinate_t block_coor;
	struct coordinate_t block_coor_next;
    uint8_t	start;
	uint8_t life;
	uint8_t score;
	uint8_t move_right;
	uint8_t move_left;
	uint8_t move_down;
	uint8_t move_speed;
	uint8_t dir;
}game_teris_t;

extern void game_begin(u8g2_t *u8g2,game_teris_t *teris) ;
#endif
