#ifndef __CHIP_INIT_H__
#define __CHIP_INIT_H__

#include "ioLibrary_Driver/Ethernet/socket.h"
#include "wizchip_conf.h"


/* socket sizes in kB */
#define SOCK_1_BUFSIZE 2
#define SOCK_2_BUFSIZE 2
#define SOCK_3_BUFSIZE 2
#define SOCK_4_BUFSIZE 2

int8_t init_wiz_chip();


#endif /* __CHIP_INIT_H__ */