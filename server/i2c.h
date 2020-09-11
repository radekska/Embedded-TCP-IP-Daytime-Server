#ifndef __I2C_H__
#define __I2C_H__


#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>

//----------------------------------------

int i2cInit(void);
int i2cWriteData(uint8_t addr, uint8_t *data, uint8_t length);
int i2cReadData(uint8_t addr, uint8_t *data, uint8_t length);
int i2cReadRegister(uint8_t addr, uint8_t reg_addr, uint8_t *data, uint8_t length);

//----------------------------------------

#endif /* __I2C_H__ */