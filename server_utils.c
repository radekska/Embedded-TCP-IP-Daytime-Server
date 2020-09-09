#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stm32f4xx_hal.h>

#include "server_utils.h"
#include "server.h"
#include "socket.h"
#include "rtc.h"

static int checkBytesCount(int32_t bytes);

int16_t getSocketStatus(Socket_t socket)
{
    return (int16_t) getSn_SR(socket.sockNumber);
}

int numberToString(uint32_t number, char *str)
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

char* getChildTaskName(int socketNumber)
{
    char str[32] = "RedirectionTask";
    char *ptr = malloc(10);
    numberToString(socketNumber, ptr);
    char *result = strcat(str, ptr);
    return result;
}


int hasReceivedDataFromSocket(int32_t bytesFromSocket)
{
    return checkBytesCount(bytesFromSocket);
}

int hasSentBackDataToSocket(int32_t bytesSentToSocket)
{
    return checkBytesCount(bytesSentToSocket);
}

static int checkBytesCount(int32_t bytes)
{
    return bytes > 0 ? SOCKET_SUCCESS : SOCKET_FAILED;
}

char *constructCurrentTime()
{
    struct date_struct currentDate;
    char *str = malloc(20);
    char tmp[4];

    rtc_read_date(&currentDate);

    numberToString(currentDate.hour, tmp);
    strcat(str, tmp);
    strcat(str, ":");
    numberToString(currentDate.min, tmp);
    strcat(str, tmp);
    strcat(str, ":");
    numberToString(currentDate.sec, tmp);
    strcat(str, tmp);
    strcat(str, "\r\n");

    return str;
}
