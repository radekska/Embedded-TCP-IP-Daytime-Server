#include "eeprom.h"

#define EEPROM_SYNC (0b00 << 6)
#define LOG_moduleAddr_MASK 0x3F

//----------------------------------------

enum isInitialized
{
	NOT_INITIALIZED,
	INITIALIZED,
};

struct eepromContext
{
	enum isInitialized isInit;
	uint8_t write_pointer; // next write address in eeprom
};

static struct eepromContext eepromCtx; // context

extern QueueHandle_t logsQueue; // queue from logs files
extern SemaphoreHandle_t saveLogs_semaphore; // semaphore from logs files

//----------------------------------------

static int eepromCheckIfBusy(void);
static int eepromTask(void *params);
static int saveLog(struct logStruct *log);
static int findLastLog(void);

//----------------------------------------

int eepromInit(void)
{
	if(eepromCtx.isInit == INITIALIZED)
	{
		return -1; // error: eeprom is already initialized
	}
	
	i2cInit();
	
	eepromCtx.write_pointer = findLastLog();
		
	eepromCtx.isInit = INITIALIZED;
	
	return 0;
}

//----------------------------------------

int eepromReadPage(uint8_t readAddr, uint8_t *buffer, uint8_t length)
{
	if(length > 8)
	{
		length = 8; // max single read
	}
	
	return i2cReadRegister(EEPROM_I2C_ADDR, readAddr, buffer, length);
}

//----------------------------------------

int eepromWritePage(uint8_t writeAddr, uint8_t *buffer, uint8_t length)
{
	if(length > 8)
	{
		length = 8; // max single write
	}
		
	uint8_t tmp_buffer[length + 1];
	tmp_buffer[0] = writeAddr; // attach read address at the begining
	memcpy(&(tmp_buffer[1]), buffer, length);
	
	return i2cWriteData(EEPROM_I2C_ADDR, tmp_buffer, length + 1 );
}

//----------------------------------------

int eepromReadBlock(uint8_t block_number, struct logStruct *log)
{
	if(log == NULL)
	{
		return -1; // error: invalid argument
	}
	
	uint8_t buffer[8];
	
	if(eepromReadPage((block_number * 8), buffer, 8) != 0)
	{	
		return -1; // error: eepromReadPage failed
	}

	if((buffer[0] & (3 << 6)) != 0) // if there is no sync symbol
	{			
		return EEPROM_INVALID_FIRST_BLOCK; // error: invalid first block
	}
	
	log->moduleAddr = buffer[0];
	log->hourBcd = buffer[1];
	log->minBcd = buffer[2];
	log->log_code = buffer[3];
	
		if((buffer[5] & (3 << 6)) != 0) // if there is no sync symbol
	{			
		return EEPROM_INVALID_SECOND_BLOCK; // error: invalid second block
	}
	
	log->moduleAddr = buffer[5];
	log->hourBcd = buffer[6];
	log->minBcd = buffer[7];
	log->log_code = buffer[8];
	
	return 0;
}

//----------------------------------------

int eepromTaskCreate(void)
{
	
	if (pdPASS != xTaskCreate(eepromTask, "eeprom", configMINIMAL_STACK_SIZE, NULL, 3, NULL))
	{
		printf("ERROR: Unable to create task!\n");
		
		return -1;
	}
	
	return 0;
}

//----------------------------------------

int eepromClear(void)
{
	uint8_t buffer[8] = {255, 255, 255, 255, 255, 255, 255, 255};
	
	for(int addr = 0; addr < EEPROM_SIZE; addr+=8)
	{
		if(eepromWritePage(addr, buffer, 8))
		{
			return -1; // error: write failed
		}
	}

	return 0;
}

//----------------------------------------
/* Private functions */

static int eepromCheckIfBusy(void)
{	
	I2C1->DR = EEPROM_I2C_ADDR << 1;
  if(I2C1->SR1 & I2C_SR1_ADDR)
	{
		return -1; // eeprom busy
	}
	
	return 0; // eeprom not busy
}

static int eepromTask(void *params)
{
	if(eepromCtx.isInit = NOT_INITIALIZED)
	{
		eepromInit();
	}
	
	while(1)
	{		
		struct logStruct log_data;

		while(xQueueReceive(logsQueue, &log_data, 1000) == pdTRUE)
		{		
			if(eepromCheckIfBusy() == 1)
			{
				vTaskDelay(10);
			}
			
			__disable_irq();
			saveLog(&log_data);
			__enable_irq();
		} 
	}
}

static int saveLog(struct logStruct *log)
{
	uint8_t buffer[8];
	
	if((eepromCtx.write_pointer % 8) == 0) // if write first 4 bytes of page
	{
		buffer[0] = (EEPROM_SYNC) | (log->moduleAddr & LOG_moduleAddr_MASK); // sync and module address
		buffer[1] = log->hourBcd;
		buffer[2] = log->minBcd;
		buffer[3] = log->log_code;
		buffer[4] = 255;
		buffer[5] = 255;
		buffer[6] = 255;
		buffer[7] = 255;	
	}
	else // if write last 4 bytes of page
	{		
		if(eepromReadPage((eepromCtx.write_pointer - (eepromCtx.write_pointer % 8)), buffer, 8) != 0)
		{	
			return -1; // error: eepromReadPage failed
		}
		
		buffer[4] = (EEPROM_SYNC) | (log->moduleAddr & LOG_moduleAddr_MASK); // sync and module address
		buffer[5] = log->hourBcd;
		buffer[6] = log->minBcd;
		buffer[7] = log->log_code;	
	}

	
	if(eepromWritePage(eepromCtx.write_pointer, buffer, 8) == 0)
	{
		eepromCtx.write_pointer += 8;
		eepromCtx.write_pointer %= EEPROM_SIZE;
		
		return 0;
	}
	else
	{	
		printf("eeprom save failed\n");
		
		return -1; // error: eeprom write failed
	}
}

static int findLastLog(void)
{
	uint8_t buffer[8];
	
	for(uint8_t i = 0; i < EEPROM_SIZE; i+=8)
	{
		if(eepromReadPage(i, buffer, 8) != 0)
		{
			return -1; // error: eepromReadPage failed
		}
		
		if((buffer[0] & (3 << 6)) != 0) // if there is no sync symbol
		{
			eepromCtx.write_pointer = i;
			
			return 0;
		}
		
		if((buffer[5] & (3 << 6)) != 0) // if there is no sync symbol
		{
			eepromCtx.write_pointer = i + 4;
			
			return 0;
		}
	}	
	
	eepromCtx.write_pointer = 0; // if memory is full
	
	return 0;
}




    