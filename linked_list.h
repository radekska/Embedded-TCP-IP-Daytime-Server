#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>
#include <string.h>
#include "logs.h"

//----------------------------------------

struct node
{
	struct logStruct *data;
	struct node *next;
};

//----------------------------------------

int linkedListCreate(struct node **list);
int linkedListAdd(struct node *list, struct logStruct *log);
int linkedListFindModule(struct node *list, struct logStruct *log /* OUT */, uint8_t moduleAddr);
int linkedListSetModuleData(struct node *list, struct logStruct *log);

//----------------------------------------

#endif /* __LINKED_LIST_H__ */