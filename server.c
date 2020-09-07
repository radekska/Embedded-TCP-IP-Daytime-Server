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
#include "queue.h"


#include <stm32f4xx_hal.h>
extern UART_HandleTypeDef  huart2;
static QueueHandle_t socket_queue;

static int create_socket_queue(void)
{
	uint8_t no_sockets = 4; // number of sockets
	
	socket_queue = xQueueCreate(no_sockets, sizeof(Socket_t));
	if(socket_queue == NULL)
	{
		return -1; // error: queue not created
	}
	
	// fill socket_queue with available sockets
	for(uint8_t sock_number = 1; sock_number < no_sockets; sock_number++) // starts with soctet number 1, because 0 is reserved for main task
	{
		Socket_t socket = 
		{
			.sockNumber = sock_number,
			.sockaddr.port = 0x0000,
		};
		memset(socket.sockaddr.ip_addr, 0, 4);
		
		if(xQueueSendToBack(socket_queue, (void *)&socket, (TickType_t)10) != pdPASS)
		{
			return -1; // error: couldn't push socket to queue
		}
	}	
	
	return 0;
}

//temporary
static wiz_NetInfo netInfo = { 
                        .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},	// Mac address
                        .ip 	= {10, 0, 1, 200},					    // IP address
                        .sn 	= {255, 255, 255, 0},					// Subnet mask
                        .gw 	= {10, 0, 1, 1}                         // Gateway address
                    };


/* Requested stack size when server listening task creates connection */
static uint16_t usedStackSize = 0;
static uint16_t childPortCounter = 5000;

static void listeningForConnectionTask(void *params);
static int openTCPServerSocket(Socket_t socket);
static int16_t getSocketStatus(Socket_t socket);
static int acceptTCPServerSocket(Socket_t socket, Sockaddr_t *clientAddrInfo);
static void createEchoServerInstance(void *params);
static void receiveAndEchoBack(Socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize);
static void clearBuffer(uint8_t *buffer, uint16_t bufferSize);
static int hasReceivedDataFromSocket(int32_t bytesFromSocket);
static int hasSentBackDataToSocket(int32_t bytesSentToSocket);
static void cleanUpResources(Socket_t socketToClose, uint8_t *buffer, uint16_t bufferSize);
static int hasReceiveOnSocketReturnedError(Socket_t socketToCheck, uint8_t *buffer, uint16_t bufferSize);

static int number_to_string(uint32_t number, char *str);


void createTCPServerSocket(uint16_t stackSize, UBaseType_t taskPriority)
{
	HAL_UART_Transmit(&huart2, "createTCPServerSocket\n", strlen("createTCPServerSocket\n"), 100);

    if (create_socket_queue() == 0) {
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
        .port = (uint16_t) ECHO_PORT
        };
		memcpy(bindAddress.ip_addr, netInfo.ip, 4);

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
                    if (xQueueReceive(socket_queue, &child_socket, 1000) == pdTRUE)
                    {
                        child_socket.sockaddr.port = childPortCounter;
                        RedirectionParams_t params = {
                            .clientAddr = clientAddr,
                            .childSocket = child_socket,
                        };
                        char portAsString[4];
                        number_to_string(child_socket.sockaddr.port, portAsString);
                        send(serverSocket.sockNumber, portAsString, 4);
                        
                        if (pdPASS != xTaskCreate(redirectConnectionToChildSocket, getChildTaskName(child_socket.sockNumber), usedStackSize, &params, 2, NULL)) {
                            HAL_UART_Transmit(&huart2, "redirectConnectionToChildSocket error\n", strlen("redirectConnectionToChildSocket error\n"), 100);
                        }
                        childPortCounter++;
                        // redirectConnectionToChildSocket(clientAddr, socket_queue);
                        // createEchoServerInstance((void *)&serverSocket);
                    } else
                    {
                        char* msg = "Cannot obtain socket from queue\n";
                        HAL_UART_Transmit(&huart2, msg, strlen(msg), 100);
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
            disconnect(serverSocket.sockNumber);
            close(serverSocket.sockNumber);
        } else
        {
            /* Handle Socket opening fail */
            __NOP();
        }
    }
}

static int number_to_string(uint32_t number, char *str)
{	
	//char *tmp = malloc(10); //tmp size
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
	
	str[iter] = '\r';
	iter++;
	str[iter] = '\n';
	iter++;
	str[iter] = '\0';
	
	//free(tmp);
	
	return 0;
}

static char* getChildTaskName(int socketNumber)
{
    char *str;
    number_to_string(socketNumber, str);
    return strcat("RedirectionTask", str);
}

//TODO: Think about some queque for free sockets(socket pool) 
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
    } while ( (xTaskGetTickCount() - timeToOpen) < Socket_tIMEOUT);

    return SOCKET_FAILED;
}

static void redirectConnectionToChildSocket(void *params)
{
    Sockaddr_t clientAddr = ((RedirectionParams_t *) params)->clientAddr;
    Socket_t workerSocket = ((RedirectionParams_t *) params)->childSocket;
    memcpy(workerSocket.sockaddr.ip_addr, netInfo.ip, 4);

    printf(rcGet)

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
                    createEchoServerInstance(workerSocket);
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
        if (xQueueSend(socket_queue, &workerSocket, 1000) = pdTRUE)
        {
            break;
        } else
        {
            /* some err */
        }
    }

    // Delete the task in case the provided argument is NULL
    vTaskDelete(NULL);
}

static void createEchoServerInstance(socket_t echoSocket)
{
    //Socket_t connectedSocket = *((Socket_t *) params);
	
	HAL_UART_Transmit(&huart2, "createEchoServerInstance task created\n", strlen("createEchoServerInstance task created\n"), 100);


    uint16_t bufferSize = (uint16_t) RX_BUFF_SIZE;
    uint8_t *rxBuffer = (uint8_t *) pvPortMalloc(bufferSize);

    if (rxBuffer != NULL)
    {
			HAL_UART_Transmit(&huart2, "rxBuffer not empty\n", strlen("rxBuffer not empty\n"), 100);
			
        receiveAndEchoBack(echoSocket, rxBuffer, bufferSize);
    }

    cleanUpResources(echoSocket, rxBuffer, bufferSize);
}

static void receiveAndEchoBack(Socket_t connectedSocket, uint8_t *receiveBuffer, uint16_t bufferSize)
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

static void cleanUpResources(Socket_t socketToClose, uint8_t *buffer, uint16_t bufferSize)
{
    TickType_t timeToShut = xTaskGetTickCount();

    disconnect(socketToClose.sockNumber);

    do
    {
        if (hasReceiveOnSocketReturnedError(socketToClose, buffer, bufferSize))
            break;
    } while ( (xTaskGetTickCount() - timeToShut) < Socket_tIMEOUT);

    vPortFree(buffer);
    close(socketToClose.sockNumber);
	// vTaskDelete(NULL);
}

static int hasReceiveOnSocketReturnedError(Socket_t socketToCheck, uint8_t *buffer, uint16_t bufferSize)
{
    return recv(socketToCheck.sockNumber, buffer, bufferSize) < 0;
}
