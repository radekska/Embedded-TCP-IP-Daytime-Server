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

static void listeningForConnectionTask(void *params);
static int openTCPServerSocket(socket_t socket);
static int16_t getSocketStatus(socket_t socket);
static int acceptTCPServerSocket(socket_t socket, sockaddr_t clientSocketInfo);
static void createEchoServerInstance(void *params);
static void receiveAndEchoBack(socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize);
static void clearBuffer(uint8_t *buffer, uint16_t bufferSize);
static int hasReceivedDataFromSocket(int32_t bytesFromSocket);
static int hasSentBackDataToSocket(int32_t bytesSentToSocket);
static void cleanUpResources(socket_t socketToClose, uint8_t *buffer, uint16_t bufferSize);
static int hasReceiveOnSocketReturnedError(socket_t socketToCheck, uint8_t *buffer, uint16_t bufferSize);



void createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority)
{
    xTaskCreate(listeningForConnectionTask, "EchoServerListener", stackSize, NULL, taskPriority + 1, NULL);
    usedStackSize = stackSize;
}


static void listeningForConnectionTask(void *params)
{
    uint8_t serverSocketNumber = 0;
    sockaddr_t clientSocket;

    sockaddr_t bindAddress = {
        .ip_addr = netInfo.ip,
        .port = (uint16_t) ECHO_PORT
        };

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
            while(1)
            {
                if (acceptTCPServerSocket(serverSocket, &clientSocket) == SOCK_OK)
                {
                    xTaskCreate(createEchoServerInstance,
                        “EchoServerInstance”,
                        usedStackSize,
                        (void *) serverSocket,
                        tskIDLE_PRIORITY,
                        NULL);
                } else
                {
                    /* Handle Socket Established Timeout Error */
                    __nop();
                }
            }
        } else
        {
            /* Handle Socket listening fail */
            __nop();
        }
    } else
    {
        /* Handle Socket opening fail */
        __nop();
    }
}

//TODO: Think about some queque for free sockets(socket pool) 
static int openTCPServerSocket(socket_t socket)
{
    int8_t retVal = SOCKERR_SOCKNUM;

    if (socket.sockNumber < _WIZCHIP_SOCK_NUM_)
    {
        retVal = socket(socket.sockNumber, Sn_MR_TCP, socket.sockaddr.port, 0);
    }
    
    return retVal == socket.sockNumber ? SOCKET_SUCCESS : retVal;
}

static int16_t getSocketStatus(socket_t socket)
{
    return (int16_t) getSn_SR(socket.sockNumber);
}

static int acceptTCPServerSocket(socket_t socket, sockaddr_t clientSocketInfo)
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
    socket_t connectedSocket = (socket_t) params;


    uint16_t bufferSize = (uint16_t) RX_BUFF_SIZE;
    uint8_t *rxBuffer = (uint8_t *) pvPortMalloc(bufferSize);

    if (rxBuffer != NULL)
    {
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
