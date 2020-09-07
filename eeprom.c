#include "eeprom.h"

//----------------------------------------

static uint32_t eeprom_check_if_busy(void);

//----------------------------------------

uint32_t eeprom_init(void)
{
	i2c_init();
	
	return 0;
}

//----------------------------------------

uint32_t eeprom_read_page(uint8_t read_addr, uint8_t *buffer, uint8_t length)
{
	if(length > 8)
	{
		length = 8; // max single read
	}
	
	//while(eeprom_check_if_busy()); //TODO
//	
//	i2c_write_data(EEPROM_I2C_ADDR, &read_addr, 1); // set read address
//	
//	return i2c_read_data(EEPROM_I2C_ADDR, buffer, length);
	return i2c_read_register(EEPROM_I2C_ADDR, read_addr, buffer, length);
}

//----------------------------------------

uint32_t eeprom_write_page(uint8_t read_addr, uint8_t *buffer, uint8_t length)
{
	if(length > 8)
	{
		length = 8; // max single write
	}
	
	//while(eeprom_check_if_busy()); //TODO
	
	uint8_t tmp_buffer[length + 1];
	tmp_buffer[0] = read_addr; // attach read address at the begining
	memcpy(&(tmp_buffer[1]), buffer, length);
	
	return i2c_write_data(EEPROM_I2C_ADDR, tmp_buffer, length + 1);
}

//----------------------------------------
/* Private functions */

static uint32_t eeprom_check_if_busy(void)
{	
	I2C1->DR = EEPROM_I2C_ADDR << 1;
  if(I2C1->SR1 & I2C_SR1_ADDR)
	{
		return 1; // eeprom busy
	}
	
	return 0; // eeprom not busy
}

    