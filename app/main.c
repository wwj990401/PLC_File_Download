#include "main.h"

int main(void)
{
        Stm32_Clock_Init(423,12,2,9);
        LED_Init();
	OSInit();  //UCOS��ʼ��
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //������ʼ����
	OSStart(); //��ʼ����
}

//��ʼ����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //����ͳ������
		
	
	OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
	OSTaskCreate(ledA_task,(void*)0,(OS_STK*)&LEDA_TASK_STK[LEDA_STK_SIZE-1],LEDA_TASK_PRIO);//����LEDA����
	OSTaskSuspend(START_TASK_PRIO);//����ʼ����
	OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
}
 
//LEDA����
void ledA_task(void *pdata)
{
	while(1)
	{
              LED0(0);
              OSTimeDlyHMSM(0,0,0,500);
              LED0(1);
              OSTimeDlyHMSM(0,0,0,500);
	}
}