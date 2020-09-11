#ifndef __SERVER_H__
#define __SERVER_H__

#include "FreeRTOS.h"
#include "task.h"

/******** DEFINE CONSTANTS ********/
#define SOCKET_SUCCESS 1
#define SOCKET_FAILED -1
/* 13 is well-known port for daytime protocol on unix-like os */
#define DAYTIME_PORT 13
#define SOCKET_TIMEOUT	( pdMS_TO_TICKS( 5000 ) ) // 5s timeout

#define RX_BUFF_SIZE 2048
#define TX_BUFF_SIZE 2048

typedef struct __sockaddr
{
    uint8_t ip_addr[4];
    uint16_t port;
} Sockaddr_t;

typedef struct __socket
{
    uint8_t sockNumber;
    Sockaddr_t sockaddr;
} Socket_t;

extern int8_t createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority);

#endif /* __SERVER_H__ */
