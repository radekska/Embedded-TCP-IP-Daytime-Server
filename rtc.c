#include "rtc.h"

//----------------------------------------

static uint32_t read_sec(uint8_t *second);
static uint32_t read_min(uint8_t *minute);
static uint32_t read_hour(uint8_t *hour);
static uint32_t read_day(uint8_t *day);
static uint32_t read_month(uint8_t *month);
static uint32_t read_year(uint8_t *year);
static uint32_t dec2bcd(uint32_t num);

//----------------------------------------

uint32_t rtc_init(void)
{
    //	struct date_struct date_set =
//	{
//		.year = 2020,
//		.month = 9,
//		.day = 8,
//		.hour = 22,
//		.min = 10,
//		.sec = 30,
//	};
//
//	rtc_set_date(&date_set);

	i2c_init();
	
	return 0;
}

//----------------------------------------

uint32_t rtc_read_date(struct date_struct *date)
{
    __disable_irq();
	volatile uint8_t data = 0;
	
	for(int i = 0; i < 0xFFFF; i++); //temporary
	
	read_sec(&data);
	date->sec = data;
	
	read_min(&data);
	date->min = data;
	
	read_hour(&data);
	date->hour = data;
	
	read_day(&data);
	date->day = data;
	
	read_month(&data);
	date->month = data;
	
	read_year(&data);
	date->year = data;

    __enable_irq();

	return 0;
}

//----------------------------------------

uint32_t rtc_set_date(struct date_struct *date)
{
	volatile uint8_t data[2];
	data[0] = 0;
	data[1] = (uint8_t)dec2bcd(date->sec);
	
	i2c_write_data(RTC_I2C_ADDR, data, 2);
	
	data[0] = 1;
	data[1] = (uint8_t)dec2bcd(date->min);
	
	i2c_write_data(RTC_I2C_ADDR, data, 2);
	
	data[0] = 2;
	data[1] = (uint8_t)dec2bcd(date->hour);
	
	i2c_write_data(RTC_I2C_ADDR, data, 2);
	
	data[0] = 4;
	data[1] = (uint8_t)dec2bcd(date->day);
	
	i2c_write_data(RTC_I2C_ADDR, data, 2);
	
	data[0] = 5;
	data[1] = (uint8_t)dec2bcd(date->month);
	
	i2c_write_data(RTC_I2C_ADDR, data, 2);
	
	data[0] = 6;
	data[1] = (uint8_t)dec2bcd(date->year);
	
	i2c_write_data(RTC_I2C_ADDR, data, 2);	
	
}

uint32_t rtc_print_date(void)
{
	struct date_struct date;
	
	rtc_read_date(&date);
	printf("time:\n");
	
	printf("time: %d : %d : %d \n", date.hour, date.min, date.sec);
	
	return 0;
}

//----------------------------------------
/* Private functions */

static uint32_t read_sec(uint8_t *second)
{
	uint8_t data;
	
	i2c_read_register(RTC_I2C_ADDR, 0, &data, 1);
	
	*second = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}

static uint32_t read_min(uint8_t *minute)
{
	uint8_t data;
	
	i2c_read_register(RTC_I2C_ADDR, 1, &data, 1);
	
	*minute = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static uint32_t read_hour(uint8_t *hour)
{
	uint8_t data;
	
	i2c_read_register(RTC_I2C_ADDR, 2, &data, 1);
	
	*hour = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static uint32_t read_day(uint8_t *day)
{
	uint8_t data;
	
	i2c_read_register(RTC_I2C_ADDR, 4, &data, 1);
	
	*day = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static uint32_t read_month(uint8_t *month)
{
	uint8_t data;
	
	i2c_read_register(RTC_I2C_ADDR, 5, &data, 1);
	
		*month = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static uint32_t read_year(uint8_t *year)
{
	uint8_t data;
	
	i2c_read_register(RTC_I2C_ADDR, 6, &data, 1);
	
	*year = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}

static uint32_t dec2bcd(uint32_t num)
{
    unsigned int ones = 0;
    unsigned int tens = 0;
    unsigned int temp = 0;

    ones = num%10; 
    temp = num/10; 
    tens = temp<<4;  
                     
    return (tens + ones);
}
	