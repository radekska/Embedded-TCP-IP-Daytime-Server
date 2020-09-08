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

//----------------------------------------