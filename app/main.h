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


//START 任务
//设置任务优先级
#define START_TASK_PRIO			12  ///开始任务的优先级为最低
//设置任务堆栈大小
#define START_STK_SIZE			128
//任务任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

//LEDA任务
//设置任务优先级
#define LEDA_TASK_PRIO			3
//设置任务堆栈大小
#define LEDA_STK_SIZE			128
//任务堆栈
OS_STK LEDA_TASK_STK[LEDA_STK_SIZE];
//任务函数
void ledA_task(void *pdata);

//LEDB任务
//设置任务优先级
#define LEDB_TASK_PRIO			4
//设置任务堆栈大小
#define LEDB_STK_SIZE			128
//任务堆栈
OS_STK LEDB_TASK_STK[LEDB_STK_SIZE];
//任务函数
void ledB_task(void *pdata);

#endif
