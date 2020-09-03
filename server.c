#include <stdint.h>
#include <stdio.h>
#include "server.h"
#include "chip_init.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "socket.h"
#include "w5100.h"
#include <string.h>

#include <stm32f4xx_hal.h>
extern UART_HandleTypeDef  huart2;

//temporary
static wiz_NetInfo netInfo = { 
                        .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},	// Mac address
                        .ip 	= {10, 0, 1, 200},					    // IP address
                        .sn 	= {255, 255, 255, 0},					// Subnet mask
                        .gw 	= {10, 0, 1, 1}                         // Gateway address
                    };


/* Requested stack size when server listening task creates connection */
static uint16_t usedStackSize = 0;

static void listeningForConnectionTask(void *params);
static int openTCPServerSocket(socket_t socket);
static int16_t getSocketStatus(socket_t socket);
static int acceptTCPServerSocket(socket_t socket, sockaddr_t *clientSocketInfo);
static void createEchoServerInstance(void *params);
static void receiveAndEchoBack(socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize);
static void clearBuffer(uint8_t *buffer, uint16_t bufferSize);
static int hasReceivedDataFromSocket(int32_t bytesFromSocket);
static int hasSentBackDataToSocket(int32_t bytesSentToSocket);
static void cleanUpResources(socket_t socketToClose, uint8_t *buffer, uint16_t bufferSize);
static int hasReceiveOnSocketReturnedError(socket_t socketToCheck, uint8_t *buffer, uint16_t bufferSize);

void createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority)
{
	HAL_UART_Transmit(&huart2, "createTCPServerSocket\n", strlen("createTCPServerSocket\n"), 100);
	
	if (pdPASS != xTaskCreate(listeningForConnectionTask, "EchoServerListener", stackSize, NULL, taskPriority + 1, NULL)) {
		HAL_UART_Transmit(&huart2, "createTCPServerSocket error\n", strlen("createTCPServerSocket error\n"), 100);
	}

	usedStackSize = stackSize;
}


static void listeningForConnectionTask(void *params)
{
    uint8_t serverSocketNumber = 0;
    sockaddr_t clientSocket;
	
	HAL_UART_Transmit(&huart2, "listeningForConnectionTask\n", strlen("listeningForConnectionTask\n"), 100);

    volatile sockaddr_t bindAddress = {
        .port = (uint16_t) ECHO_PORT
        };
		memcpy(bindAddress.ip_addr, netInfo.ip, 4);

    socket_t serverSocket = {
        .sockNumber = serverSocketNumber,
        .sockaddr = bindAddress
    };
    //bindAddress.port = htons(bindAddress.port);

    if (openTCPServerSocket(serverSocket) == SOCKET_SUCCESS) {
        setsockopt(serverSocket.sockNumber, SO_TTL, (uint8_t) 3);
        if (listen(serverSocket.sockNumber) == SOCK_OK) {
            // wait for remote conn
            while(getSocketStatus(serverSocket) == SOCK_LISTEN);
            //while(1)
            {

                if (acceptTCPServerSocket(serverSocket, &clientSocket) == SOCK_OK)
                {
										//HAL_UART_Transmit(&huart2, "socket connected\n", strlen("socket connected\n"), 100);
									
									createEchoServerInstance((void *)&serverSocket);
									
									/*
                    xTaskCreate(createEchoServerInstance,
                        "EchoServerInstance",
                        usedStackSize,
                        (void *) &serverSocket,
                        tskIDLE_PRIORITY,
                        NULL);*/
                } else
                {
                    /* Handle Socket Established Timeout Error */
                    __NOP();
                }
            }
        } else
        {
            /* Handle Socket listening fail */
            __NOP();
        }
    } else
    {
        /* Handle Socket opening fail */
        __NOP();
    }
}

//TODO: Think about some queque for free sockets(socket pool) 
static int openTCPServerSocket(socket_t socketHandler)
{
    int8_t retVal = SOCKERR_SOCKNUM;

    if (socketHandler.sockNumber < _WIZCHIP_SOCK_NUM_)
    {
        retVal = socket(socketHandler.sockNumber, Sn_MR_TCP, socketHandler.sockaddr.port, 0);
    }
    
    return retVal == socketHandler.sockNumber ? SOCKET_SUCCESS : retVal;
}

static int16_t getSocketStatus(socket_t socket)
{
    return (int16_t) getSn_SR(socket.sockNumber);
}

static int acceptTCPServerSocket(socket_t socket, sockaddr_t *clientSocketInfo)
{
    TickType_t timeToOpen = xTaskGetTickCount();

    do
    {
        if (getSocketStatus(socket) == SOCK_ESTABLISHED) {
            getsockopt(socket.sockNumber, SO_DESTIP, clientSocketInfo->ip_addr);
            getsockopt(socket.sockNumber, SO_DESTPORT, (uint16_t *) clientSocketInfo->port);
            return SOCK_OK;
        }
    } while ( (xTaskGetTickCount() - timeToOpen) < SOCKET_TIMEOUT);

    return SOCKET_FAILED;
}

static void createEchoServerInstance(void *params)
{
    static const TickType_t timeout = pdMS_TO_TICKS( 5000 );
    static const TickType_t receiveTimeout = timeout; 
    static const TickType_t sendTimeout = timeout;
    socket_t connectedSocket = *((socket_t *) params);
	
	HAL_UART_Transmit(&huart2, "createEchoServerInstance task created\n", strlen("createEchoServerInstance task created\n"), 100);


    uint16_t bufferSize = (uint16_t) RX_BUFF_SIZE;
    uint8_t *rxBuffer = (uint8_t *) pvPortMalloc(bufferSize);

    if (rxBuffer != NULL)
    {
			HAL_UART_Transmit(&huart2, "rxBuffer not empty\n", strlen("rxBuffer not empty\n"), 100);
			
        receiveAndEchoBack(connectedSocket, rxBuffer, bufferSize);
    }

    cleanUpResources(connectedSocket, rxBuffer, bufferSize);
}

static void receiveAndEchoBack(socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize)
{
    int32_t bytesReceivedOnSocket;
    int32_t bytesSentBySocket;
    int32_t bytesTotalSentBack;

    while (1)
    {
        clearBuffer(receiveBuffer, bufferSize);

        bytesReceivedOnSocket = recv(connectedSocket.sockNumber, receiveBuffer, bufferSize);
        
        if (hasReceivedDataFromSocket(bytesReceivedOnSocket))
        {
            bytesSentBySocket = 0;
            bytesTotalSentBack = bytesSentBySocket;

            while (hasSentBackDataToSocket(bytesSentBySocket) && (bytesTotalSentBack < bytesReceivedOnSocket))
            {
                bytesSentBySocket = send(connectedSocket.sockNumber, receiveBuffer, bytesReceivedOnSocket - bytesTotalSentBack);

                if (hasSentBackDataToSocket(bytesSentBySocket))
                    bytesTotalSentBack += bytesSentBySocket;
                else
                {
                    //Log it or something
                    return;
                }
            }
        }
        else
        {
            //Log it or something
            return;
        }
    }
}

static void clearBuffer(uint8_t *buffer, uint16_t bufferSize)
{
    memset(buffer, 0, bufferSize);
}

static int hasReceivedDataFromSocket(int32_t bytesFromSocket)
{
    return bytesFromSocket > 0 ? SOCKET_SUCCESS : SOCKET_FAILED;
}

static int hasSentBackDataToSocket(int32_t bytesSentToSocket)
{
    return bytesSentToSocket > 0 ? SOCKET_SUCCESS : SOCKET_FAILED;
}

static void cleanUpResources(socket_t socketToClose, uint8_t *buffer, uint16_t bufferSize)
{
    TickType_t timeToShut = xTaskGetTickCount();

    disconnect(socketToClose.sockNumber);

    do
    {
        if (hasReceiveOnSocketReturnedError(socketToClose, buffer, bufferSize))
            break;
    } while ( (xTaskGetTickCount() - timeToShut) < SOCKET_TIMEOUT);

    vPortFree(buffer);
    close(socketToClose.sockNumber);
	vTaskDelete(NULL);
}

static int hasReceiveOnSocketReturnedError(socket_t socketToCheck, uint8_t *buffer, uint16_t bufferSize)
{
    return recv(socketToCheck.sockNumber, buffer, bufferSize) < 0;
}
