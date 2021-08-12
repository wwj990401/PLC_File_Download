#ifndef __LWIP_TCP_CLIENT_SOCKET__
#define __LWIP_TCP_CLIENT_SOCKET__
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

#define DATA_MAX_LENGTH         1405

void Init_Client(void);
void Init_Client_task(void *pdata);
void Connect_Server_task(void *pdata);
void Rev_Server_File_task(void *pdata);

void Send_File_Name_task(void *pdata);
void Send_File_Name(char* fileName);

void Pause_Download(void);
void Continue_Download(void);
void Stop_Download(void);
void Stop_Download_task(void *pdata);

#endif
