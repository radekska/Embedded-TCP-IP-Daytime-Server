#include <stdint.h>
#include <stdio.h>
#include "server.h"
#include "ioLibrary_Driver/Ethernet/socket.h"
#include "chip_init.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/* Requested stack size when server listening task creates connection */
static uint16_t usedStackSize = 0;

static void createEchoServerInstance(void *params);
static void listeningForConnectionTask(void *params);
static void clearBuffer(uint8_t *buffer, uint16_t bufferSize);
static void receiveAndEchoBack(Socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize);
static int openTCPServerSocket(Socket_t* socketToOpen);
static int acceptTCPServerSocket(Socket_t* connectedSocketHandle, Socket_t* listeningSocket, struct freertos_sockaddr *connectedSocketInfo, socklen_t addressLen);
static int hasReceivedDataFromSocket(int32_t bytesFromSocket);
static int hasSentBackDataToSocket(int32_t bytesSentToSocket);


void createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(listeningForConnectionTask, "EchoServerListener", stackSize, NULL, taskPriority + 1, NULL);
    usedStackSize = stackSize;
}


static void listeningForConnectionTask(void *params)
{
    static const uint8_t rxBuffSize[RX_BUFF_SIZE];
    static const uint8_t txBuffSize[TX_BUFF_SIZE];

    sockaddr_t clientAddress;
    sockaddr_t bindAddress = {  .ip_addr = netInfo.ip,
                                .port = (uint16_t) ECHO_PORT };

    //TODO:
    uint8_t serverSocketNumber = 0;
    Socket_t connectedSocket;
    socklen_t clientAddrSize = sizeof(clientAddress);
    static const TickType_t receiveTimeout = portMAX_DELAY;
    const uint8_t backlog = 5;
    //TODO


    if (openTCPServerSocket(serverSocketNumber, bindAddress.port) == SOCKET_SUCCESS) {

        FreeRTOS_setsockopt(listeningSocket, UNUSED_PARAM, FREERTOS_SO_RCVBUF, &bufferSize, UNUSED_PARAM);
        FreeRTOS_setsockopt(listeningSocket, UNUSED_PARAM, FREERTOS_SO_SNDBUF, &bufferSize, UNUSED_PARAM);
        FreeRTOS_setsockopt(listeningSocket, UNUSED_PARAM, FREERTOS_SO_RCVTIMEO, &receiveTimeout, UNUSED_PARAM);
        FreeRTOS_setsockopt(listeningSocket, UNUSED_PARAM, FREERTOS_SO_WIN_PROPERTIES, (void *) &xWinProps, UNUSED_PARAM);

        bindAddress.port = htons(bindAddress.port);
        FreeRTOS_bind(listeningSocket, &bindAddress, sizeof(bindAddress.sin_port));
        FreeRTOS_listen(listeningSocket, backlog);

        while(1)
        {
            if (acceptTCPServerSocket(&connectedSocket, &listeningSocket, &clientAddress, &clientAddrSize) == SOCKET_SUCCESS)
            {
                xTaskCreate(createEchoServerInstance,
                    “EchoServerInstance”,
                    usedStackSize,
                    (void *) connectedSocket,
                    tskIDLE_PRIORITY,
                    NULL);
            }
        }
    }    
}


//TODO: Think about some queque for free sockets(socket pool) 
static int openTCPServerSocket(uint8_t socketNumberToOpen, uint16_t port)
{
    int8_t retVal = SOCKERR_SOCKNUM;

    if (socketNumberToOpen < _WIZCHIP_SOCK_NUM_)
    {
        retVal = socket(socketNumberToOpen, Sn_MR_TCP, port, 0);
    }
    
    return retVal == socketNumberToOpen ? SOCKET_SUCCESS : SOCKET_FAILED;
}

static int acceptTCPServerSocket(Socket_t* connectedSocketHandle, Socket_t* listeningSocket, struct freertos_sockaddr *connectedSocketInfo, socklen_t addressLen)
{
    *connectedSocketHandle = FreeRTOS_accept(*listeningSocket, connectedSocketInfo, addressLen);
    return (connectedSocketHandle == FREERTOS_INVALID_SOCKET || connectedSocketHandle == NULL) ? 
        SOCKET_FAILED : SOCKET_SUCCESS;
}

static void createEchoServerInstance(void *params)
{
    static const TickType_t timeout = pdMS_TO_TICKS( 5000 );
    static const TickType_t receiveTimeout = timeout;
    static const TickType_t sendTimeout = timeout;
    uint16_t bufferSize = ipconfigTCP_MSS;

    Socket_t connectedSocket = (Socket_t) params;
    
    uint8_t *receiveBuffer = (uint8_t *) pvPortMalloc(bufferSize);

    if (receiveBuffer != NULL)
    {
        FreeRTOS_setsockopt(connectedSocket, UNUSED_PARAM, FREERTOS_SO_RCVTIMEO, &receiveTimeout, UNUSED_PARAM);
		FreeRTOS_setsockopt(connectedSocket, UNUSED_PARAM, FREERTOS_SO_SNDTIMEO, &sendTimeout, UNUSED_PARAM);

        receiveAndEchoBack(connectedSocket, receiveBuffer, bufferSize);
    }

    cleanUpResources(connectedSocket, receiveBuffer, bufferSize);
}

static void receiveAndEchoBack(Socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize)
{
    int32_t bytesReceivedOnSocket;
    int32_t bytesSentBackBySocket;
    int32_t bytesTotalSentBySocket;

    while (1)
    {
        clearBuffer(receiveBuffer, bufferSize);

        bytesReceivedOnSocket = FreeRTOS_recv(connectedSocket, receiveBuffer, bufferSize, UNUSED_PARAM);
        
        if (hasReceivedDataFromSocket(bytesReceivedOnSocket))
        {
            bytesSentBackBySocket = 0;
            bytesTotalSentBySocket = bytesSentBackBySocket;

            while (hasSentBackDataToSocket(bytesSentBackBySocket)) && 
                    (bytesTotalSentBySocket < bytesReceivedOnSocket))
            {
                bytesSentBackBySocket = FreeRTOS_send(connectedSocket, receiveBuffer, bytesReceivedOnSocket - bytesTotalSentBySocket, UNUSED_PARAM);

                if (hasSentBackDataToSocket(bytesSentBackBySocket))
                    bytesTotalSentBySocket += bytesSentBackBySocket;
                else
                    return;
            }
        }
        else
            return;
    }
}

static void clearBuffer(uint8_t *buffer, uint16_t bufferSize)
{
    memset(buffer, 0, bufferSize);
}

static int hasReceivedDataFromSocket(int32_t bytesFromSocket)
{
    return bytesFromSocket >= 0 ? SOCKET_SUCCESS : SOCKET_FAILED;
}

static int hasSentBackDataToSocket(int32_t bytesSentToSocket)
{
    return bytesSentToSocket >= 0 ? SOCKET_SUCCESS : SOCKET_FAILED;
}

static void cleanUpResources(Socket_t socketToClose, uint8_t *buffer, uint16_t bufferSize)
{
    TickType_t timeToShut = xTaskGetTickCount();

    FreeRTOS_shutdown(socketToClose, FREERTOS_SHUT_RDWR);

    do
    {
        if (hasReceiveOnSocketReturnedError(socketToClose, buffer, bufferSize))
            break;
    } while ( (xTaskGetTickCount() - timeToShut) < SOCKET_SHUTDOWN_MAX_TIME);

    vPortFree(buffer);
	FreeRTOS_closesocket(socketToClose);
	vTaskDelete(NULL);
}

static int hasReceiveOnSocketReturnedError(Socket_t socketToCheck, uint8_t *buffer, uint16_t bufferSize)
{
    return FreeRTOS_recv(socketToCheck, buffer, bufferSize, UNUSED_PARAM ) < 0;
}
