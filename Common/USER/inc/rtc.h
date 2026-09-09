#ifndef _RTC_H
#define _RTC_H

#include "System.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "LED.h"
#include "i2c.h"

#define DS3231_addr     0x68 // DS3231 slave address
#define DS3231_seconds  0x00 // DS3231 seconds address
#define DS3231_control  0x0E // DS3231 control register address
#define DS3231_tmp_MSB  0x11 // DS3231 temperature MSB

// All DS3231 registers
typedef struct {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t alarm1_secconds;
	uint8_t alarm1_minutes;
	uint8_t alarm1_hours;
	uint8_t alarm1_day;
	uint8_t alarm1_date;
	uint8_t alarm2_minutes;
	uint8_t alarm2_hours;
	uint8_t alarm2_day;
	uint8_t alarm2_date;
	uint8_t control;
	uint8_t status;
	uint8_t aging;
	uint8_t msb_temp;
	uint8_t lsb_temp;
} DS3231_registers_TypeDef;

// DS3231 date
typedef struct {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t dow;
	uint8_t day;
	uint8_t month;
	uint8_t year;
} RAW_date_TypeDef;

// Human Readable Format date
typedef struct {
	uint8_t  sec;
	uint8_t  min;
	uint8_t  hour;
	uint8_t  day;
	uint8_t  month;
	uint16_t year;
	uint8_t  dow;
} DS3231_date_TypeDef;

extern DS3231_date_TypeDef DS3231_dTime;

float DS3231_ReadTemp(void);
void checkUpdateDS3231(uint8_t weekday,uint8_t d,uint8_t m,uint8_t y,uint8_t localH,uint8_t localM,uint8_t localS);
void getRTCDateTime(void);
float DS3231_ReadTemp(void);
void initDS3231(void);

#endif
