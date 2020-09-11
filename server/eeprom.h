#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "i2c.h"
#include "retarget.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "logs.h"

#define EEPROM_I2C_ADDR 0x50
#define EEPROM_SIZE 128 /* 1 block of 128 bytes */

#define EEPROM_INVALID_FIRST_BLOCK -2
#define EEPROM_INVALID_SECOND_BLOCK -3

//----------------------------------------

int eepromInit(void);
int eepromReadPage(uint8_t readAddr, uint8_t *buffer, uint8_t length); //max read length 8 bytes
int eepromWritePage(uint8_t writeAddr, uint8_t *buffer, uint8_t length); //max write length 8 bytes
int eepromReadBlock(uint8_t block_number, struct logStruct *log);
int eepromClear(void);

int eepromTaskCreate(void);

//----------------------------------------

#endif /* __EEPROM_H__ */