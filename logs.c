#include "logs.h"

char *logMappingTab[MODULES_COUNT][8] = {{"server connected", "child connected", "child not created", "date not sent", "child task closed", "no available sockets", "log7", "log8"},};

//----------------------------------------

QueueHandle_t logsQueue;

static struct logs_context
{
	struct node *logs_list;
};

static struct logs_context logs_ctx; // context

//----------------------------------------

static uint32_t bcd2dec(uint32_t num);
static void log_task(void *params);

//----------------------------------------

int logsInit(void)
{	
	
	logsQueue = xQueueCreate(LOGS_QUEUE_LENGTH, sizeof(struct logStruct));
	if(logsQueue == NULL)
	{		
		return -1; // error: queue not created
	}
	
	return linkedListCreate(&(logs_ctx.logs_list));
}

//----------------------------------------

int logsAddNewModule(uint8_t moduleAddr)
{
	struct logStruct log_data = 
	{
		.moduleAddr = moduleAddr,
		.hourBcd = 0,
		.minBcd = 0,
		.log_code = 0,
	};
	
	return linkedListAdd(logs_ctx.logs_list, &log_data);
}

//----------------------------------------

int logsAddLog(uint8_t moduleAddr, uint8_t logNumber)
{
	if(logNumber > 7)
	{		
		return -1; // error: incorrect value (log number - <0, 7>)
	}
	
	struct logStruct log_data;
	
	if(linkedListFindModule(logs_ctx.logs_list, &log_data, moduleAddr) != 0) // get current module logs
	{		
		return -1; // error: linked list failure
	}
	
	log_data.log_code |= (1 << logNumber); // add log
	
	return linkedListSetModuleData(logs_ctx.logs_list, &log_data); // update module logs
}

//----------------------------------------

int logsSave(uint8_t moduleAddr)
{
	struct logStruct log_data;
	
	if(linkedListFindModule(logs_ctx.logs_list, &log_data, moduleAddr) != 0) // get current module logs
	{
		return -1; // error: linked list failure
	}
	
	rtcGetHourBcd(&(log_data.hourBcd));
	rtcGetMinBcd(&(log_data.minBcd));
	
	if(xQueueSend(logsQueue, (void *)&log_data, (TickType_t)10) == pdPASS)
	{		
		return 0;
	}
	else
	{		
		return -1; // error: couldn't push item to queue
	}
}

int logsPrintSaved(char *log_tab[MODULES_COUNT][8])
{
	struct logStruct log;
	
	for(uint8_t i = 0; i < 32; i++)
	{
		if((eepromReadBlock(i, &log) != 0) && (eepromReadBlock(i, &log) != EEPROM_INVALID_SECOND_BLOCK))
		{
			return 1; // error: block error
		}
	
		printf("log module: %d \n", log.moduleAddr);
		printf("log time: %d : %d \n", bcd2dec(log.hourBcd), bcd2dec(log.minBcd));
		
		for(int log_num = 0; log_num < 8; log_num++)
		{
			if(log.log_code & (1 << log_num))
			{
				printf("log: %s\n", log_tab[log.moduleAddr][log_num]);
			}			
		}
		
		printf("\n");
	}
		
	return 0;
}

int logsTaskCreate(void)
{
		
	if (pdPASS != xTaskCreate(log_task, "log", configMINIMAL_STACK_SIZE, NULL, 3, NULL))
	{
		printf("ERROR: Unable to create task!\n");
		
		return 1;
	}
	
	return 0;
}

//----------------------------------------
/* Private functions */

static uint32_t bcd2dec(uint32_t num)
{
	 return (((num >> 4) & 0x0F)*10) + (num & 0x0F);
}

static void log_task(void *params)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
	
	
	while(1)
	{
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET)
		{
			__disable_irq();
			logsPrintSaved(logMappingTab);
			__enable_irq();					
		}	
		
		vTaskDelay(500);	
	}
}

