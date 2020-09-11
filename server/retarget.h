#ifndef _RETARGET_H
#define _RETARGET_H

#include <stdio.h>
#include "stm32f446xx.h"
#include <stm32f4xx_hal.h>

//----------------------------------------

uint32_t retargetInit(void);

int fputc(int ch, FILE *f);
int ferror(FILE *f);

//----------------------------------------

#endif /* _RETARGET_H */