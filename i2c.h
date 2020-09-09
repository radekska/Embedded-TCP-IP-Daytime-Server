#ifndef __I2C_H__
#define __I2C_H__


#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>

//----------------------------------------

uint32_t i2c_init(void);
uint32_t i2c_write_data(uint8_t addr, uint8_t *data, uint8_t length);
uint32_t i2c_read_data(uint8_t addr, uint8_t *data, uint8_t length);
uint32_t i2c_read_register(uint8_t addr, uint8_t reg_addr, uint8_t *data, uint8_t length);

//----------------------------------------

#endif /* __I2C_H__ */