#ifndef __CHIP_INIT_H__
#define __CHIP_INIT_H__

#include "ioLibrary_Driver/Ethernet/socket.h"
#include "wizchip_conf.h"
#include <stm32f4xx_hal.h>

/* socket sizes in kB */
#define SOCK_1_BUFSIZE 2
#define SOCK_2_BUFSIZE 2
#define SOCK_3_BUFSIZE 2
#define SOCK_4_BUFSIZE 2

extern wiz_NetInfo netInfoConfig;
extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart2;

extern int8_t initHardware();


#endif /* __CHIP_INIT_H__ */