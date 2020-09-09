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

//----------------------------------------

uint32_t eeprom_init(void);
uint32_t eeprom_read_page(uint8_t read_addr, uint8_t *buffer, uint8_t length); //max read length 8 bytes
uint32_t eeprom_write_page(uint8_t read_addr, uint8_t *buffer, uint8_t length); //max write length 8 bytes
uint32_t eeprom_read_block(uint8_t block_number, struct log_struct *log);

uint32_t eeprom_task_create(void);

//----------------------------------------

#endif /* __EEPROM_H__ */