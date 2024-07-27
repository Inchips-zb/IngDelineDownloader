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

#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "stdint.h"
#include "at24cxxfunction.h"

#ifndef EEPROM_I2C_PORT
#define EEPROM_I2C_PORT        I2C_PORT_1
#endif


//选择地址引脚的实际连接情况
//0_下拉，1_上拉
#define A0  0
#define A1  0
#define A2  0

//以下部分为条件编译无需修改

#define EEPROM_CHIP_ADDR                 0x50|(A2<<3)|(A1<<2)|(A0<<1)

extern At24cObjectType At24C128CMoudle;
void eeprom_init(void);
int eeprom_app_writes(At24cObjectType *at,int start_addr,unsigned char *p_buf,int len);
int eeprom_app_reads(At24cObjectType *at,int start_addr,unsigned char *p_buf,int len);

#endif
