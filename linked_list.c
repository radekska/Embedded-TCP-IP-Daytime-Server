#include "linked_list.h"

//----------------------------------------

int linkedListCreate(struct node **list)
{	
	*list = malloc(sizeof(struct node));	
	
	if(*list == NULL)
	{
		return -1; // error: mallock failed
	}
	
	(*list)->data = NULL;
	(*list)->next = NULL;
		
	return 0;
}

//----------------------------------------

int linkedListAdd(struct node *list, struct logStruct *log)
{
	if((list == NULL) || (log == NULL))
	{
		printf("linkedListAdd failed0\n");
		
		return -1; // error: incorrect arguments
	}
	
	if((list->next == NULL) && (list->data == NULL)) // if list is empty
	{	
		list->data = malloc(sizeof(struct logStruct));		
		if(list->data == NULL)
		{
			printf("linkedListAdd failed1\n");
			
			return -1; // error: mallock failed
		}
		
		list->next = NULL;
		
		memcpy(list->data, log, sizeof(struct logStruct)); // fill linked list item
		
		return 0;
	}
	
	volatile struct node element = *list;	
	
	while(element.next != NULL) // find last element
	{
		element = *(element.next);
	}
	
		element.next = malloc(sizeof(struct node)); // create new element	
	if(element.next == NULL)
	{
		printf("linkedListAdd failed2\n");
		
		return -1; // error: mallock failed
	}
	
	element.next->data = malloc(sizeof(struct logStruct));	
	if(element.next->data == NULL)
	{
		printf("linkedListAdd failed2\n");
		
		return -1; // error: mallock failed
	}
		
	element.next->next = NULL;
		
	memcpy(element.next->data, log, sizeof(struct logStruct));	// fill linked list item
	
	return 0;
}

//----------------------------------------

int linkedListFindModule(struct node *list, struct logStruct *log, uint8_t moduleAddr)
{
	if((list == NULL) || (log == NULL))
	{
		return -1; // error: incorrect arguments
	}	
	
	volatile struct node element = *list;
	
	do // scan list
	{
		if(element.data->moduleAddr == moduleAddr) // if module is in the list
		{
			memcpy(log, element.data, sizeof(struct logStruct));
			
			return 0;
		}
	}	
	while(element.next != NULL);
		
	return -1; // error: module not found
}

//----------------------------------------

int linkedListSetModuleData(struct node *list, struct logStruct *log)
{
	if((list == NULL) || (log == NULL))
	{
		return -1; // error: incorrect arguments
	}	
	
	struct node element = *list;
	
	do // scan list
	{
		if(element.data->moduleAddr == log->moduleAddr) // if module is in the list
		{
			memcpy(element.data, log, sizeof(struct logStruct)); // modify list item
			
			return 0;
		}
	}	
	while(element.next != NULL); 
		
	return -1; // error: module not found
}