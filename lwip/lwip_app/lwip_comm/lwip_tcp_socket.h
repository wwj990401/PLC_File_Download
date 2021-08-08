#ifndef __LWIP_TCP_SOCKET__
#define __LWIP_TCP_SOCKET__
#include "stm32f7xx_hal.h"
#include "lwip/opt.h"
#include <string.h>
#include "lan8720.h"
#include "delay.h"
#include "lwip/sockets.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "malloc.h"
#include "stmflash.h"

#define PORT 8000

#define RECV_BUF_SIZE 1405

#define PLC_NUMBER 1

void Socket_init(void);

void Socket_task(void *pdata);

void Connect_client_task(void *pdata);

void Rev_file_task(void *sock_conn);


#endif
