#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_hal.h>

#include "server.h"
#include "chip_init.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "socket.h"
#include "w5100.h"
#include "socket_queue.h"
#include "retarget.h"
#include "rtc.h"
#include "server_utils.h"
#include "queue.h"
#include "logs.h"

/* Requested stack size when server listening task creates connection */
static uint16_t usedStackSize = 0;
extern wiz_NetInfo netInfoConfig;
extern QueueHandle_t socketQueue;		

static void listeningForConnectionTask(void *params);
static int openTCPServerSocket(Socket_t socket);
static int acceptTCPServerSocket(Socket_t socket, Sockaddr_t *clientAddrInfo);
static int sendCompleteMessage(Socket_t connectedSocket, char *bytesToSend);
static void redirectConnectionToChildSocket(void *params);
static int sendCurrentDate(Socket_t *clientSocket);
static void sendTimeoutDisconnectionMessage(Socket_t connectedSocket);

int8_t createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority)
{
    if (createSocketQueue() == 0) {
        if (pdPASS != xTaskCreate(listeningForConnectionTask, "DaytimeServerListener", stackSize, NULL, taskPriority, NULL)) {
            return SOCKET_FAILED;
    	}
        usedStackSize = stackSize;
    }
    return SOCKET_SUCCESS;
}

static void listeningForConnectionTask(void *params)
{
    uint8_t serverSocketNumber = 0;
    Sockaddr_t clientAddr;
    Socket_t childSocket;
    char *taskName;
	
    volatile Sockaddr_t bindAddress = {
        .port = (uint16_t) DAYTIME_PORT
        };
		memcpy(bindAddress.ip_addr, netInfoConfig.ip, 4);

    Socket_t serverSocket = {
        .sockNumber = serverSocketNumber,
        .sockaddr = bindAddress
    };

    while(1)
    {
        if (openTCPServerSocket(serverSocket) == SOCKET_SUCCESS)
        {
            setsockopt(serverSocket.sockNumber, SO_TTL, (uint8_t) 3);
            if (listen(serverSocket.sockNumber) == SOCK_OK)
            {
                while(getSocketStatus(serverSocket) == SOCK_LISTEN)
                {
                    vTaskDelay((TickType_t) 100);
                }
								
                if (acceptTCPServerSocket(serverSocket, &clientAddr) == SOCK_OK)
                {
										if(logsAddLog(MODULE_SERVER, 0) != 0) // log: server connected
                    {
                        printf("add log failed\n");
                    }
                    
                    if (xQueueReceive(socketQueue, &childSocket, 10000) == pdTRUE)
                    {						
                        char portAsString[4];
                        numberToString(childSocket.sockaddr.port, portAsString);
                        sendCompleteMessage(serverSocket, portAsString);

                        taskName = getChildTaskName(childSocket.sockNumber);
                        int childTaskPriority = 3;

                        if (pdPASS != xTaskCreate(redirectConnectionToChildSocket, taskName, usedStackSize, (void *)&childSocket, childTaskPriority, NULL))
                        {
                            if(logsAddLog(MODULE_SERVER, 2) != 0) // log: child not created
														{
																printf("add log failed\n");
														}
                        }
                    } else
                    {
												if(logsAddLog(MODULE_SERVER, 5) != 0) // log: no available sockets
												{
														printf("add log failed\n");
												}
											
                        sendTimeoutDisconnectionMessage(serverSocket);
                    }
										
                    if(logsAddLog(MODULE_SERVER, 3) != 0)
                    {
                        printf("add log failed\n");
                    }
                    
                    if(logsSave(MODULE_SERVER) != 0)
                    {
                        printf("save log failed\n");
                    }
                    free(taskName);
                    close(serverSocket.sockNumber);											
										
                } else
                {
                    /* Handle Socket Established Timeout Error */
                    __NOP();
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
}

static int openTCPServerSocket(Socket_t socketHandler)
{
    int8_t retVal = SOCKERR_SOCKNUM;

    if (socketHandler.sockNumber < _WIZCHIP_SOCK_NUM_)
    {
        retVal = socket(socketHandler.sockNumber, Sn_MR_TCP, socketHandler.sockaddr.port, 0);
    }
    
    return retVal == socketHandler.sockNumber ? SOCKET_SUCCESS : retVal;
}

static int acceptTCPServerSocket(Socket_t socket, Sockaddr_t *clientAddrInfo)
{
    TickType_t timeToOpen = xTaskGetTickCount();

    do
    {
        if (getSocketStatus(socket) == SOCK_ESTABLISHED) {
            getsockopt(socket.sockNumber, SO_DESTIP, clientAddrInfo->ip_addr);
            getsockopt(socket.sockNumber, SO_DESTPORT, (uint16_t *) clientAddrInfo->port);
            return SOCK_OK;
        }
    } while ( (xTaskGetTickCount() - timeToOpen) < SOCKET_TIMEOUT);

    return SOCKET_FAILED;
}

static int sendCompleteMessage(Socket_t connectedSocket, char *bufferToSend)
{
    int32_t bytesToSend;
    int32_t bytesSentBySocket;
    int32_t bytesTotalSentBack;

    bytesToSend = strlen(bufferToSend);

    if (hasReceivedDataFromSocket(bytesToSend))
    {
        bytesSentBySocket = 0;
        bytesTotalSentBack = bytesSentBySocket;

        while (hasSentBackDataToSocket(bytesSentBySocket) && (bytesTotalSentBack < bytesToSend))
        {
            bytesSentBySocket = send(connectedSocket.sockNumber, bufferToSend, bytesToSend - bytesTotalSentBack);
            if(bytesSentBySocket < 0) // error ocured
            {
                if(logsAddLog(MODULE_SERVER, 3) != 0) // log: date not sent
								{
										printf("add log failed\n");
								}
														
                return SOCKET_FAILED;
            }
            if (hasSentBackDataToSocket(bytesSentBySocket))
            {
                bytesTotalSentBack += bytesSentBySocket;
            }
            else
            {
                //Log it or something
                return SOCKET_FAILED;
            }
        }
    }
    else
    {
        return SOCKET_FAILED;
    }
    return SOCKET_SUCCESS;
}

static void redirectConnectionToChildSocket(void *params)
{
    Socket_t workerSocket = *((Socket_t *) params);
    Sockaddr_t clientAddr;
    memcpy(workerSocket.sockaddr.ip_addr, netInfoConfig.ip, 4);

    while(1)
    {
        if (openTCPServerSocket(workerSocket) == SOCKET_SUCCESS)
        {
            setsockopt(workerSocket.sockNumber, SO_TTL, (uint8_t) 3);
            if (listen(workerSocket.sockNumber) == SOCK_OK) {
                while(getSocketStatus(workerSocket) == SOCK_LISTEN)
                {
                    vTaskDelay((TickType_t)100);
                }

                if (acceptTCPServerSocket(workerSocket, &clientAddr) == SOCK_OK)
                {
									
									printf("connected!\n");
									
										if(logsAddLog(MODULE_SERVER, 1) != 0) // log: child connected
										{
												printf("add log failed\n");
										}
									
                    sendCompleteMessage(workerSocket, "Current time: \n");
                    while(1)
                    {
                        if(sendCurrentDate(&workerSocket) == SOCKET_FAILED)
                        {
                            close(workerSocket.sockNumber);
                            if (xQueueSend(socketQueue, &workerSocket, 100) == pdTRUE)
                            {
															printf("disconnected!\n");
															
															vTaskDelete(NULL);
                            }
                        }
                        vTaskDelay(1000);
                    }
                } else
                {
                    /* Handle Socket Established Timeout Error */
                    __NOP();
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
}

static void sendTimeoutDisconnectionMessage(Socket_t connectedSocket)
{
    sendCompleteMessage(connectedSocket, "-1");
}

static int sendCurrentDate(Socket_t *clientSocket)
{
    char *currentTime = constructCurrentTime();

	__disable_irq();
	int response = sendCompleteMessage(*clientSocket, currentTime);
	__enable_irq();
	
	free(currentTime);
	
	return response;
}
