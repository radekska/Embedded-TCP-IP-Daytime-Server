#ifndef _RETARGET_H
#define _RETARGET_H

#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>

//----------------------------------------

struct retarget_config
{
	UART_HandleTypeDef uart_config;
	GPIO_InitTypeDef uart_gpio_config;
};

//----------------------------------------

uint32_t retarget_init(struct retarget_config *config);

int fputc(int ch, FILE *f);
int ferror(FILE *f);

//----------------------------------------

#endif /* _RETARGET_H */