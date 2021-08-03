#ifndef _MAIN_H
#define _MAIN_H

#include "includes.h"
#include "led.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "sdram.h"
#include "malloc.h"
#include "usmart.h"

#include "lan8720.h"
#include "mpu.h"
#include "timer.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "lwip_tcp_socket.h"


//START 任务
//设置任务优先级
#define START_TASK_PRIO			15  ///开始任务的优先级为最低
//设置任务堆栈大小
#define START_STK_SIZE			128
//任务任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void Start_task(void *pdata);

//LEDA任务
//设置任务优先级
#define LEDA_TASK_PRIO			14
//设置任务堆栈大小
#define LEDA_STK_SIZE			128
//任务堆栈
OS_STK LEDA_TASK_STK[LEDA_STK_SIZE];
//任务函数
void LedA_task(void *pdata);

#endif
