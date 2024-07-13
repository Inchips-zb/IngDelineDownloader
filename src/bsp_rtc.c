#include "platform_api.h"
#include "peripheral_rtc.h"
#include "bsp_rtc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bsp_msg.h"
#include "gui_menu.h"

#define COMPILE_TIME (__DATE__ " " __TIME__)

const char* compileTime = COMPILE_TIME;
#define COMPILE_YEAR    atoi(COMPILE_TIME + 7)
#define COMPILE_MON     get_mon()
#define COMPILE_DAY     atoi(COMPILE_TIME + 4)
#define COMPILE_HOUR    atoi(COMPILE_TIME + 11)
#define COMPILE_MIN     atoi(COMPILE_TIME + 15)
#define COMPILE_SEC     atoi(COMPILE_TIME + 18)

static uint32_t rtc_isr(void *user_data);
static int dayOfYear(int year, int month, int day) ;
static uint8_t is_leap_year(uint16_t year);
static uint8_t date_get_week(uint16_t year,uint8_t month,uint8_t day);
static uint8_t get_mon(void);
// 计算给定天数后的日期
static RTC_DateTypeDef calculateDateFromDays(int days,RTC_DateTypeDef StartD) ;
static int calculateDateDifference(RTC_DateTypeDef date1, RTC_DateTypeDef date2);

static const uint8_t table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};



static RTC_DateTypeDef StartDate = {.Year = 2000,.Month = 1,.Day = 1};

static RTC_DateTypeDef Back;

uint8_t get_rtc_time(RTC_TimeTypeDef *pTime,RTC_DateTypeDef *pDate)
{
    static uint16_t day_last = 0;
    uint16_t  days = 0;

    if(NULL == pTime || NULL == pDate) return 1;

    days = RTC_GetTime(& pTime->Hours,&pTime->Minutes,&pTime->Seconds);
    if(day_last != days)
    {
        day_last = days;
        Back = calculateDateFromDays(days,StartDate);
        Back.WeekDay = date_get_week(Back.Year, Back.Month,Back.Day);
    };
    memcpy(pDate,&Back,sizeof(RTC_DateTypeDef));
    return 0;
}



uint8_t modify_rtc_time(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t sumday;
    RTC_DateTypeDef SetDate = {.Year = syear ,.Month = smon ,.Day = sday};
	if(SetDate.Year < StartDate.Year || SetDate.Year > StartDate.Year+179) return 1;	     
    sumday = calculateDateDifference(StartDate,SetDate);

    RTC_ModifyTime(sumday, hour, min, sec);
#ifndef SOFTWARE_RTC_DHMS
    int i = 100000;
    while(i--);
#else    
    while(1 != RTC_IsModificationDone())
    {
        ;
    }    
#endif
	return 0;	    
}

void setup_rtc(void)
{
    printf("Compilation time: %s\n", COMPILE_TIME);
	RTC_Enable(0);
    modify_rtc_time(COMPILE_YEAR,COMPILE_MON,COMPILE_DAY,COMPILE_HOUR,COMPILE_MIN,COMPILE_SEC+15);	
	RTC_Enable(1);
	RTC_EnableIRQ(0 | 0 | 0 | RTC_IRQ_SECOND | 0 | 0);
	platform_set_irq_callback(PLATFORM_CB_IRQ_RTC, (f_platform_irq_cb)rtc_isr, NULL);
}
static uint8_t get_mon(void)
{
    uint8_t mon = 0;
    char monthStr[4];   
    strncpy(monthStr, compileTime, 3);
    monthStr[3] = '\0';
    while (strcmp(monthStr, months[mon]) != 0) {
        if(++mon > 12)
            break;
    }    
    mon += 1;
    return mon;
}

static uint8_t date_get_week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}	
static uint8_t is_leap_year(uint16_t year)
{			  
	if(year%4==0)
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;
			else return 0;   
		}else return 1;   
	}else return 0;	
}

// 计算给定日期是该年的第几天
static int dayOfYear(int year, int month, int day) {
    uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // 对于闰年，2月有29天
    if (is_leap_year(year)) {
        daysInMonth[1] = 29;
    }

    int totalDays = 0;
    int i = 0;
    for (i = 0; i < month - 1; i++) {
        totalDays += daysInMonth[i];
    }

    totalDays += day;

    return totalDays;
}

static int calculateDateDifference(RTC_DateTypeDef date1, RTC_DateTypeDef date2) {
    // 先计算起始日期到该年末的天数
    int days1 = dayOfYear(date1.Year, 12, 31) - dayOfYear(date1.Year, date1.Month, date1.Day);

    // 计算终止日期到该年初的天数
    int days2 = dayOfYear(date2.Year, date2.Month, date2.Day);

    // 计算整年的天数
    int totalDays = 0;
    int year;
    for ( year = date1.Year + 1; year < date2.Year; year++) {
        totalDays += is_leap_year(year) ? 366 : 365;
    }

    // 计算日期差值
    int difference = totalDays + days1 + days2;

    return difference;
}

// 计算给定天数后的日期
static RTC_DateTypeDef calculateDateFromDays(int days,RTC_DateTypeDef StartD) {
    RTC_DateTypeDef date = {.Year = StartD.Year, .Month = StartD.Month, .Day = StartD.Day}; 
    uint8_t  daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // 每个月的天数

    while (days > 0) {
        int daysInYear = is_leap_year(date.Year) ? 366 : 365; // 当年的天数
        int remainingDays = daysInYear - dayOfYear(date.Year, date.Month, date.Day) + 1; // 当年剩余的天数

        if (days < remainingDays) {
            date.Day += remainingDays;
            break;
        } else {
            days -= remainingDays;
            date.Year++;
            date.Month = 1;
            date.Day = 1;
        }
    }


    if (is_leap_year(date.Year)) {
        daysInMonth[1] = 29; 
    }        
    while (days > 0) {

        int remainingDays = daysInMonth[date.Month - 1]; // 当月剩余的天数

        if (days < remainingDays) {
            date.Day += days;
            break;
        } else {
            days -= remainingDays;
            date.Month++;
            date.Day = 1;
        }
    }
 
    return date;
}

extern uint8_t page_index;
static uint32_t rtc_isr(void *user_data){
	uint32_t states = RTC_GetIntState();
	if((RTC_IRQ_SECOND & states)&&(PAGE_SHOW_CLOCK == page_index))
		UserQue_SendMsg(USER_MSG_RTC_S,NULL,0);
	
	RTC_ClearIntState(states);
	return 0;
}
