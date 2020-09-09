#include "socket_queue.h"
#include "server.h"

QueueHandle_t socketQueue;

int createSocketQueue(void)
{
	uint8_t no_sockets = 4; // number of sockets
	
	socketQueue = xQueueCreate(no_sockets, sizeof(Socket_t));
	if(socketQueue == NULL)
	{
		return -1; // error: queue not created
	}
	
	// fill socket_queue with available sockets
	for(uint8_t sock_number = 1; sock_number < no_sockets; sock_number++) // starts with soctet number 1, because 0 is reserved for main task
	{
		Socket_t socket = 
		{
			.sockNumber = sock_number,
			.sockaddr.port = sock_number * 1111,
		};
		memset(socket.sockaddr.ip_addr, 0, sizeof(*(socket.sockaddr.ip_addr) * 4));
		
		if(xQueueSendToBack(socketQueue, (void *)&socket, (TickType_t)10) != pdPASS)
		{
			return -1; // error: couldn't push socket to queue
		}
	}	
	
	return 0;
}



