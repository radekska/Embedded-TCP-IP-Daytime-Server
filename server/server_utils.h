#ifndef __SERVER_UTILS_H__
#define __SERVER_UTILS_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_hal.h>

#include "server.h"

extern int16_t getSocketStatus(Socket_t socket);
extern int numberToString(uint32_t number, char *str);
extern char* getChildTaskName(int socketNumber);
extern int hasReceivedDataFromSocket(int32_t bytesFromSocket);
extern int hasSentBackDataToSocket(int32_t bytesSentToSocket);
extern char *constructCurrentTime(void);

#endif /* __SERVER_UTILS_H__ */
