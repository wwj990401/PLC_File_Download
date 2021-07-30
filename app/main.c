#include "main.h"

int main(void)
{
        Stm32_Clock_Init(423,12,2,9);
        LED_Init();
        
        Cache_Enable();                 //打开L1-Cache
        HAL_Init();			//初始化HAL库
        delay_init(216);                //延时初始化
	uart_init(115200);		//串口初始化
        SDRAM_Init();                   //初始化SDRAM
	usmart_dev.init(108); 		//初始化USMART	
        my_mem_init(SRAMIN);		//初始化内部内存池
        my_mem_init(SRAMEX);		//初始化外部内存池
        my_mem_init(SRAMDTCM);		//初始化DTCM内存池
    
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
        OSTaskCreate(ledB_task,(void*)0,(OS_STK*)&LEDB_TASK_STK[LEDB_STK_SIZE-1],LEDB_TASK_PRIO);//创建LEDA任务
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

//LEDB任务
void ledB_task(void *pdata)
{
	while(1)
	{
              LED1(1);
              OSTimeDlyHMSM(0,0,0,50);
              LED1(0);
              OSTimeDlyHMSM(0,0,0,50);
	}
}