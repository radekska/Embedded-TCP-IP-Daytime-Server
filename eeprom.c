#include "eeprom.h"

#define EEPROM_SYNC (0b00 << 6)
#define LOG_MODULE_ADDR_MASK 0x3F

//----------------------------------------

static enum is_initialized
{
	NOT_INITIALIZED,
	INITIALIZED,
};

static struct eeprom_context
{
	enum is_initialized is_init;
	uint8_t write_pointer; // next write address in eeprom
};

static struct eeprom_context eeprom_ctx; // context

extern QueueHandle_t logs_queue; // queue from logs files
extern SemaphoreHandle_t save_logs_semaphore; // semaphore from logs files

//----------------------------------------

static uint32_t eeprom_check_if_busy(void);
static uint32_t eeprom_task(void *params);
static uint32_t save_log(struct log_struct *log);
static uint32_t find_last_log(void);

//----------------------------------------

uint32_t eeprom_init(void)
{
	if(eeprom_ctx.is_init == INITIALIZED)
	{
		return 1; // error: eeprom is already initialized
	}
	
	i2c_init();
	
	//eeprom_ctx.write_pointer = find_last_log();
		
	eeprom_ctx.is_init = INITIALIZED;
	
	return 0;
}

//----------------------------------------

uint32_t eeprom_read_page(uint8_t read_addr, uint8_t *buffer, uint8_t length)
{
	if(length > 8)
	{
		length = 8; // max single read
	}
	
	return i2c_read_register(EEPROM_I2C_ADDR, read_addr, buffer, length);
}

//----------------------------------------

uint32_t eeprom_write_page(uint8_t read_addr, uint8_t *buffer, uint8_t length)
{
	if(length > 8)
	{
		length = 8; // max single write
	}
		
	uint8_t tmp_buffer[length + 1];
	tmp_buffer[0] = read_addr; // attach read address at the begining
	memcpy(&(tmp_buffer[1]), buffer, length);
	
	return i2c_write_data(EEPROM_I2C_ADDR, tmp_buffer, length + 1 );
}

//----------------------------------------

uint32_t eeprom_read_block(uint8_t block_number, struct log_struct *log)
{
	if(log == NULL)
	{
		return 1; // error: invalid argument
	}
	
	uint8_t buffer[8];
	
	if(eeprom_read_page((block_number * 4), buffer, 8) != 0)
	{
		printf(" eeprom_read_page error\n");
		
		return 1; // error: eeprom_read_page failed
	}

	if((buffer[0] & (3 << 6)) != 0) // if there is no sync symbol
	{	
		printf(" sync error\n");
		
		return 1; // error: invalid block
	}
	
	log->module_addr = buffer[0];
	log->hour_bcd = buffer[1];
	log->min_bcd = buffer[2];
	log->log_code = buffer[3];
	
	return 0;
}

//----------------------------------------

uint32_t eeprom_task_create(void)
{
	
	if (pdPASS != xTaskCreate(eeprom_task, "eeprom", configMINIMAL_STACK_SIZE, NULL, 3, NULL))
	{
		printf("ERROR: Unable to create task!\n");
		
		return 1;
	}
	
	printf("eeprom task created\n");
	
	return 0;
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

static uint32_t eeprom_task(void *params)
{
	if(eeprom_ctx.is_init = NOT_INITIALIZED)
	{
		eeprom_init();
	}
	
	while(1)
	{
		//if(pdTRUE == xSemaphoreTake(save_logs_semaphore, 100))
		{
			//printf("eeprom: semaphore taken successfully\n");
			
			// semaphore taken successfully
			
			struct log_struct log_data;
			
			while(xQueueReceive(logs_queue, &log_data, 1000) == pdTRUE)
			{		
				if(eeprom_check_if_busy() == 1)
				{
					vTaskDelay(10);
				}
				
				__disable_irq();
				save_log(&log_data);
				__enable_irq();
			}
		} 
//		else 
//		{
//			// timeout occurred
//		}
		
	}

}

static uint32_t save_log(struct log_struct *log)
{
	uint8_t buffer[4];
	
	buffer[0] = (EEPROM_SYNC) | (log->module_addr & LOG_MODULE_ADDR_MASK); // sync and module address
	buffer[1] = log->hour_bcd;
	buffer[2] = log->min_bcd;
	buffer[3] = log->log_code;
	
	if(eeprom_write_page(eeprom_ctx.write_pointer, buffer, 4))
	{
		eeprom_ctx.write_pointer += 4;
		eeprom_ctx.write_pointer %= EEPROM_SIZE;
		
		printf("data saved\n");
		
		return 0;
	}
	else
	{
			printf("data save failed\n");
		
		return 1; // error: eeprom write failed
	}
}

static uint32_t find_last_log(void)
{
	uint8_t buffer[4];
	
	for(uint8_t i = 0; i < 127; i+=4)
	{
		if(eeprom_read_page(i, buffer, 4) != 0)
		{
			return 1; // error: eeprom_read_page failed
		}
		
		if((buffer[0] & (3 << 6)) != 0) // if there is no sync symbol
		{
			eeprom_ctx.write_pointer = i;
			
			return 0;
		}
	}	
	
	eeprom_ctx.write_pointer = 0; // if memory is full
	
	return 0;
}




    