#ifndef __RTC_H__
#define __RTC_H__

#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "i2c.h"
#include "retarget.h"

#define RTC_I2C_ADDR 0x68

//----------------------------------------

struct date_struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
};

//----------------------------------------

int rtcInit(void);
int rtcReadDate(struct date_struct *date);
int rtcSetDate(struct date_struct *date);
int rtcPrintDate(void);
int rtcGetHourBcd(uint8_t *hourBcd);
int rtcGetMinBcd(uint8_t *minBcd);

//----------------------------------------

#endif /* __RTC_H__ */