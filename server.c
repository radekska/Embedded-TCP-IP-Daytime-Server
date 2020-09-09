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
#include "queue.h"
#include "retarget.h"
#include "rtc.h"

extern UART_HandleTypeDef huart2;
static QueueHandle_t socketQueue;

static int createSocketQueue(void)
{
	uint8_t numberOfAvailableSockets = 3;
	
	socketQueue = xQueueCreate(numberOfAvailableSockets, sizeof(Socket_t));
	if(socketQueue) {
        // starts with socket number 1, because 0 is reserved for main task
        for (uint8_t sockNumber = 1; sockNumber <= numberOfAvailableSockets; sockNumber++)
        {
            Socket_t socket = {
                    .sockNumber = sockNumber,
                    .sockaddr.port = sockNumber * 1111
            };
            memset(socket.sockaddr.ip_addr, 0, 4);

            if (xQueueSendToBack(socketQueue, (void *) &socket, (TickType_t) 10) != pdPASS) {
                // log to eeprom
                return -1;
            }
        }
    } else {
	    // log to eeprom that Queue cannot been created
        return -1;
	}
    return 0;
}

/* Requested stack size when server listening task creates connection */
static uint16_t usedStackSize = 0;
										
static void listeningForConnectionTask(void *params);
static int openTCPServerSocket(Socket_t socket);
static int16_t getSocketStatus(Socket_t socket);
static int acceptTCPServerSocket(Socket_t socket, Sockaddr_t *clientAddrInfo);
static int receiveAndEchoBack(Socket_t connectedSocket, char *bytesToSend);
static void clearBuffer(uint8_t *buffer, uint16_t bufferSize);
static int hasReceivedDataFromSocket(int32_t bytesFromSocket);
static int hasSentBackDataToSocket(int32_t bytesSentToSocket);
static void cleanUpResources(Socket_t *socketToClose, uint8_t *buffer, uint16_t bufferSize);
static void redirectConnectionToChildSocket(void *params);
static int hasReceiveOnSocketReturnedError(Socket_t *socketToCheck, uint8_t *buffer, uint16_t bufferSize);
static int number_to_string(uint32_t number, char *str);
static char* getChildTaskName(int socketNumber);
static void createEchoServerInstance(Socket_t *echoSocket);
static int sendCurrentDate(Socket_t *clientSocket);
	
void createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority)
{
	HAL_UART_Transmit(&huart2, "createTCPServerSocket\n", strlen("createTCPServerSocket\n"), 100);

    if (create_socketQueue() == 0) {
        if (pdPASS != xTaskCreate(listeningForConnectionTask, "EchoServerListener", stackSize, NULL, taskPriority + 1, NULL)) {
	    	HAL_UART_Transmit(&huart2, "createTCPServerSocket error\n", strlen("createTCPServerSocket error\n"), 100);
    	}

        usedStackSize = stackSize;
    } else
    {
        HAL_UART_Transmit(&huart2, "createTCPServerSocket error\n", strlen("createTCPServerSocket error\n"), 100);
    }
}

static void listeningForConnectionTask(void *params)
{
    uint8_t serverSocketNumber = 0;
    Sockaddr_t clientAddr;
	
	HAL_UART_Transmit(&huart2, "listeningForConnectionTask\n", strlen("listeningForConnectionTask\n"), 100);

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
            if (listen(serverSocket.sockNumber) == SOCK_OK) {
                // wait for remote conn
                while(getSocketStatus(serverSocket) == SOCK_LISTEN)
                {
                    vTaskDelay((TickType_t)100);
                }

                if (acceptTCPServerSocket(serverSocket, &clientAddr) == SOCK_OK)
                {
                    Socket_t child_socket;
                    
                    // handle error later
                    if (xQueueReceive(socketQueue, &child_socket, 10000) == pdTRUE)
                    {
                        RedirectionParams_t params = {
                            .clientAddr = clientAddr,
                            .childSocket = child_socket,
                        };
                        char portAsString[4];
                        number_to_string(child_socket.sockaddr.port, portAsString);
                        send(serverSocket.sockNumber, portAsString, 4);
												
												char *taskName = getChildTaskName(child_socket.sockNumber);
												printf(taskName);
                        
                        if (pdPASS != xTaskCreate(redirectConnectionToChildSocket, taskName, usedStackSize, &params, 2, NULL)) {
                            HAL_UART_Transmit(&huart2, "redirectConnectionToChildSocket error\n", strlen("redirectConnectionToChildSocket error\n"), 100);
                        }
												
                        // redirectConnectionToChildSocket(clientAddr, socketQueue);
                        // createEchoServerInstance((void *)&serverSocket);
                    } else
                    {
                        receiveAndEchoBack(serverSocket, "-1");
                    }
										
										printf("main socket close\n");

//										disconnect(serverSocket.sockNumber);
										close(serverSocket.sockNumber);

										// vTaskDelay(100); //wait after close // TODO time_wait
										
										
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

static int number_to_string(uint32_t number, char *str)
{
	char tmp[10] = {0};	
	uint32_t iter = 0;

	do 
	{
		tmp[iter] = (char)(number % 10 + '0');
		number -= number % 10;
		iter++;
		
	} while(number /= 10);

	iter = 0;
	
	for(int i = strlen(tmp)-1; i >= 0; i--)
	{
		str[iter] = tmp[i];
		iter++;
	}	

	str[iter] = '\0';
	
	return 0;
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

static int16_t getSocketStatus(Socket_t socket)
{
    return (int16_t) getSn_SR(socket.sockNumber);
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

static void redirectConnectionToChildSocket(void *params)
{
    Sockaddr_t clientAddr = ((RedirectionParams_t *) params)->clientAddr;
    Socket_t workerSocket = ((RedirectionParams_t *) params)->childSocket;
    memcpy(workerSocket.sockaddr.ip_addr, netInfoConfig.ip, 4);

		printf("task: %s\n", pcTaskGetName(NULL));

    while(1)
    {
        if (openTCPServerSocket(workerSocket) == SOCKET_SUCCESS)
        {
            setsockopt(workerSocket.sockNumber, SO_TTL, (uint8_t) 3);
            if (listen(workerSocket.sockNumber) == SOCK_OK) {
                // wait for remote conn
                while(getSocketStatus(workerSocket) == SOCK_LISTEN)
                {
                    vTaskDelay((TickType_t)100);
                }

                if (acceptTCPServerSocket(workerSocket, &clientAddr) == SOCK_OK)
                {
										printf("Echo server created\n");							
									
                    //createEchoServerInstance(&workerSocket);
									
										receiveAndEchoBack(workerSocket, "\n");
									
										while(1)
										{
											if(sendCurrentDate(&workerSocket) == SOCKET_FAILED)
											{
												printf("client disconnect\n");

//												disconnect(workerSocket.sockNumber);
												close(workerSocket.sockNumber);

												if (xQueueSend(socketQueue, &workerSocket, 100) == pdTRUE)
												{
													// break;
												} 
												else
												{
													/* some err */
												}
												
												printf("child delete\n");

												vTaskDelete(NULL);

												printf("task delete failed\n");
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
	
    // Delete the task in case the provided argument is NULL
    vTaskDelete(NULL);
}

static void createEchoServerInstance(Socket_t *echoSocket)
{
    //Socket_t connectedSocket = *((Socket_t *) params);
	
	printf("createEchoServerInstance task created\n");


    uint16_t bufferSize = (uint16_t) RX_BUFF_SIZE;
    uint8_t *rxBuffer = (uint8_t *) pvPortMalloc(bufferSize);

    if (rxBuffer != NULL)
    {
				printf("rxBuffer not empty\n");
			
        receiveAndEchoBack(*echoSocket, rxBuffer);
    }

    cleanUpResources(echoSocket, rxBuffer, bufferSize);
}

static int receiveAndEchoBack(Socket_t connectedSocket, char *bufferToSend)
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
					
						//printf("byte sent: %d\n", bytesSentBySocket);
					
						if(bytesSentBySocket < 0) // error ocured
						{
								//printf("socket error\n");
							
                //Log it or something
                return SOCKET_FAILED;
						}

            if (hasSentBackDataToSocket(bytesSentBySocket))
                bytesTotalSentBack += bytesSentBySocket;
            else
            {
							
							//printf("SOCKET_FAILED\n");
							
                //Log it or something
                return SOCKET_FAILED;
            }
        }
    }
    else
    {
				//printf("SOCKET_FAILED\n");
        //Log it or something
        return SOCKET_FAILED;
    }
		
		//printf("SOCKET_SUCCESS\n");		
		return SOCKET_SUCCESS;

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

static void cleanUpResources(Socket_t *socketToClose, uint8_t *buffer, uint16_t bufferSize)
{
    TickType_t timeToShut = xTaskGetTickCount();

    disconnect(socketToClose->sockNumber);

    do
    {
        if (hasReceiveOnSocketReturnedError(socketToClose, buffer, bufferSize))
            break;
    } while ( (xTaskGetTickCount() - timeToShut) < SOCKET_TIMEOUT);

    vPortFree(buffer);
    close(socketToClose->sockNumber);
	// vTaskDelete(NULL);
}

static int hasReceiveOnSocketReturnedError(Socket_t *socketToCheck, uint8_t *buffer, uint16_t bufferSize)
{
    return recv(socketToCheck->sockNumber, buffer, bufferSize) < 0;
}

static char* getChildTaskName(int socketNumber)
{
    char str[19] = "RedirectionTask";
    char *ptr = malloc(10);
    number_to_string(socketNumber, ptr);
    char *result = strcat(str, ptr);
    return result;
}

static int sendCurrentDate(Socket_t *clientSocket)
{
	struct date_struct currentDate;
	
	__disable_irq();
	rtc_read_date(&currentDate);
	__enable_irq();	
	
	char *str = malloc(20), tmp[4]; // temporary
	
	number_to_string(currentDate.hour, tmp);
	strcat(str, tmp);
	strcat(str, ":");
	number_to_string(currentDate.min, tmp);
	strcat(str, tmp);
	strcat(str, ":");
	number_to_string(currentDate.sec, tmp);
	strcat(str, tmp);
	strcat(str, "\r\n");	
	
	__disable_irq();
	//volatile uint32_t response = send(clientSocket->sockNumber, str, 20);
	volatile int response = receiveAndEchoBack(*clientSocket, str);
	__enable_irq();
	
	free(str);
	
	//printf("end of rtc\n");
	
	return response;
}
