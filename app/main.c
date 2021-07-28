#include "main.h"

int main(void)
{
        Stm32_Clock_Init(423,12,2,9);
        LED_Init();
	OSInit();  //UCOS初始化
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //创建开始任务
	OSStart(); //开始任务
}

//开始任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //开启统计任务
		
	
	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(ledA_task,(void*)0,(OS_STK*)&LEDA_TASK_STK[LEDA_STK_SIZE-1],LEDA_TASK_PRIO);//创建LEDA任务
	OSTaskSuspend(START_TASK_PRIO);//挂起开始任务
	OS_EXIT_CRITICAL();  //退出临界区(开中断)
}
 
//LEDA任务
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