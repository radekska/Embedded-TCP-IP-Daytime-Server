#ifndef __SOCKET_QUEUE_H__
#define __SOCKET_QUEUE_H__

#include "queue.h"

extern QueueHandle_t socketQueue;

extern int createSocketQueue(void);

#endif /* __SOCKET_QUEUE_H__ */
