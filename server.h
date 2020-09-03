#ifndef __SERVER_H__
#define __SERVER_H__

/** TODO: in FreeRTOSConfig.h set widow size (MSS) ipconfigUSE_TCP_WIN to 1 
 * in order to have proper buffer length and scaale */

/******** DEFINE CONSTANTS ********/
/* Socket created successfully */
#define SOCKET_SUCCESS 1
/* There was insufficient FreeRTOS heap memory available for the socket to be created. */
#define SOCKET_FAILED -1
/* Value indicating unused parameter */
#define UNUSED_PARAM ((void *) 0)
/* 7 is well-known port for echo protocol on unix-like os */
#define ECHO_PORT 7
/* The maximum time to wait for a closing socket to close. */
#define SOCKET_TIMEOUT	( pdMS_TO_TICKS( 5000 ) )

#define RX_BUFF_SIZE 2048
#define TX_BUFF_SIZE 2048

typedef struct __sockaddr
{
    uint8_t ip_addr[4];
    uint16_t port;
} sockaddr_t;

typedef struct __socket_t {
    uint8_t sockNumber;
    sockaddr_t sockaddr;
} socket_t;

void createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority);


#endif /* __SERVER_H__ */
