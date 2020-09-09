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

uint32_t rtc_init(void);
uint32_t rtc_read_date(struct date_struct *date);
uint32_t rtc_set_date(struct date_struct *date);

uint32_t rtc_print_date(void);

uint32_t rtc_get_hour_bcd(uint8_t *hour_bcd);
uint32_t rtc_get_min_bcd(uint8_t *min_bcd);

//----------------------------------------

#endif /* __RTC_H__ */