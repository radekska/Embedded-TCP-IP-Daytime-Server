#include "rtc.h"

//----------------------------------------

static int readSec(uint8_t *second);
static int readMin(uint8_t *minute);
static int readHour(uint8_t *hour);
static int readDay(uint8_t *day);
static int readMonth(uint8_t *month);
static int readYear(uint8_t *year);
static int dec2bcd(uint32_t num);

//----------------------------------------

int rtcInit(void)
{
	i2cInit();
	
	return 0;
}

//----------------------------------------

int rtcReadDate(struct date_struct *date)
{	
	volatile uint8_t data = 0;
	
	for(int i = 0; i < 0xFFFF; i++); //temporary
	
	readSec(&data);
	date->sec = data;
	
	readMin(&data);
	date->min = data;
	
	readHour(&data);
	date->hour = data;
	
	readDay(&data);
	date->day = data;
	
	readMonth(&data);
	date->month = data;
	
	readYear(&data);
	date->year = data;

    __enable_irq();

	return 0;
}

//----------------------------------------

int rtcSetDate(struct date_struct *date)
{
	volatile uint8_t data[2];
	data[0] = 0;
	data[1] = (uint8_t)dec2bcd(date->sec);
	
	i2cWriteData(RTC_I2C_ADDR, data, 2);
	
	data[0] = 1;
	data[1] = (uint8_t)dec2bcd(date->min);
	
	i2cWriteData(RTC_I2C_ADDR, data, 2);
	
	data[0] = 2;
	data[1] = (uint8_t)dec2bcd(date->hour);
	
	i2cWriteData(RTC_I2C_ADDR, data, 2);
	
	data[0] = 4;
	data[1] = (uint8_t)dec2bcd(date->day);
	
	i2cWriteData(RTC_I2C_ADDR, data, 2);
	
	data[0] = 5;
	data[1] = (uint8_t)dec2bcd(date->month);
	
	i2cWriteData(RTC_I2C_ADDR, data, 2);
	
	data[0] = 6;
	data[1] = (uint8_t)dec2bcd(date->year);
	
	i2cWriteData(RTC_I2C_ADDR, data, 2);	
	
}

int rtcPrintDate(void)
{
	struct date_struct date;
	
	printf("read date\n");
	
	rtcReadDate(&date);	
	
	printf("time: %d : %d : %d \n", date.hour, date.min, date.sec);
	
	return 0;
}

int rtcGetHourBcd(uint8_t *hourBcd)
{
	return i2cReadRegister(RTC_I2C_ADDR, 2, hourBcd, 1);
}

int rtcGetMinBcd(uint8_t *minBcd)
{
	return i2cReadRegister(RTC_I2C_ADDR, 1, minBcd, 1);
}

//----------------------------------------
/* Private functions */

static int readSec(uint8_t *second)
{
	uint8_t data;
	
	i2cReadRegister(RTC_I2C_ADDR, 0, &data, 1);
	
	*second = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}

static int readMin(uint8_t *minute)
{
	uint8_t data;
	
	i2cReadRegister(RTC_I2C_ADDR, 1, &data, 1);
	
	*minute = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static int readHour(uint8_t *hour)
{
	uint8_t data;
	
	i2cReadRegister(RTC_I2C_ADDR, 2, &data, 1);
	
	*hour = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static int readDay(uint8_t *day)
{
	uint8_t data;
	
	i2cReadRegister(RTC_I2C_ADDR, 4, &data, 1);
	
	*day = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static int readMonth(uint8_t *month)
{
	uint8_t data;
	
	i2cReadRegister(RTC_I2C_ADDR, 5, &data, 1);
	
		*month = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}
	
static int readYear(uint8_t *year)
{
	uint8_t data;
	
	i2cReadRegister(RTC_I2C_ADDR, 6, &data, 1);
	
	*year = (((data >> 4) & 0x0F)*10) + (data & 0x0F);
	
	return 0;
}

static int dec2bcd(uint32_t num)
{
    unsigned int ones = 0;
    unsigned int tens = 0;
    unsigned int temp = 0;

    ones = num%10; 
    temp = num/10; 
    tens = temp<<4;  
                     
    return (tens + ones);
}
	