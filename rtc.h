#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "i2c.h"
#include "retarget.h"

//#define RTC_I2C_ADDR 0x68
#define RTC_I2C_ADDR 0xD0 // 0x68 << 1

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

struct rtc_config
{
	I2C_HandleTypeDef i2c_config;
	GPIO_InitTypeDef gpio_config;
};

//----------------------------------------

uint32_t rtc_init(struct rtc_config *config);
uint32_t rtc_read_date(struct date_struct *date);
uint32_t rtc_set_date(struct date_struct *date);

uint32_t rtc_print_date(void);

//----------------------------------------