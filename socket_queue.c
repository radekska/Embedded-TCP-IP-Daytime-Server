#include "socket_queue.h"

QueueHandle_t socketQueue;

int createSocketQueue(void)
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

