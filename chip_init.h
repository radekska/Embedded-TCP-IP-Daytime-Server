#ifndef __CHIP_INIT_H__
#define __CHIP_INIT_H__

#include "ioLibrary_Driver/Ethernet/socket.h"
#include "wizchip_conf.h"


/* socket sizes in kB */
#define SOCK_1_BUFSIZE 2
#define SOCK_2_BUFSIZE 2
#define SOCK_3_BUFSIZE 2
#define SOCK_4_BUFSIZE 2

wiz_NetInfo netInfo = { 
                        .mac 	= {0x00, 0x08, 0xdc, 0xab, 0xcd, 0xef},	// Mac address
                        .ip 	= {10, 0, 1, 200},					    // IP address
                        .sn 	= {255, 255, 255, 0},					// Subnet mask
                        .gw 	= {10, 0, 1, 1}                         // Gateway address
                    };


#endif /* __CHIP_INIT_H__ */