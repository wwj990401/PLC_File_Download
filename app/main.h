#ifndef _MAIN_H
#define _MAIN_H

#include "includes.h"
#include "led.h"


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
#define LEDA_TASK_PRIO			7
//���������ջ��С
#define LEDA_STK_SIZE			128
//�����ջ
OS_STK LEDA_TASK_STK[LEDA_STK_SIZE];
//������
void ledA_task(void *pdata);


#endif
