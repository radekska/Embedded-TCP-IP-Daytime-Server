#ifndef __LOGS_H__
#define __LOGS_H__

#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include "retarget.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "linked_list.h"
#include "rtc.h"
#include "semphr.h"

#define LOGS_QUEUE_LENGTH 10
#define MODULES_COUNT 1

#define MODULE_SERVER 0

extern char *logMappingTab[MODULES_COUNT][8];

//----------------------------------------

//enum socket_states

struct logStruct
{
	uint8_t moduleAddr;
	uint8_t hourBcd;
	uint8_t minBcd;
	uint8_t log_code;
};

//----------------------------------------

int logsInit(void);
int logsAddNewModule(uint8_t moduleAddr);
int logsAddLog(uint8_t moduleAddr, uint8_t logNumber);
int logsSave(uint8_t moduleAddr);
int logsPrintSaved(char *log_tab[MODULES_COUNT][8]);

int logsTaskCreate(void);

//----------------------------------------

#endif /* __LOGS_H__ */