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


//START ����
//�����������ȼ�
#define START_TASK_PRIO			12  ///��ʼ��������ȼ�Ϊ���
//���������ջ��С
#define START_STK_SIZE			128
//���������ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//LEDA����
//�����������ȼ�
#define LEDA_TASK_PRIO			3
//���������ջ��С
#define LEDA_STK_SIZE			128
//�����ջ
OS_STK LEDA_TASK_STK[LEDA_STK_SIZE];
//������
void ledA_task(void *pdata);

//LEDB����
//�����������ȼ�
#define LEDB_TASK_PRIO			4
//���������ջ��С
#define LEDB_STK_SIZE			128
//�����ջ
OS_STK LEDB_TASK_STK[LEDB_STK_SIZE];
//������
void ledB_task(void *pdata);

#endif
