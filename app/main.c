#include "main.h"

int main(void)
{  
        Write_Through();                //开启强制透写！
        MPU_Memory_Protection();        //保护相关存储区域
        Cache_Enable();                 //打开L1-Cache
    
        HAL_Init();				        //初始化HAL库
        Stm32_Clock_Init(432,12,2,9);   //设置时钟,216Mhz 
        delay_init(216);                //延时初始化
	uart_init(115200);		        //串口初始化
	usmart_dev.init(216); 		    //初始化USMART
        LED_Init();                     //初始化LED
        SDRAM_Init();                   //初始化SDRAM
        my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMDTCM);		    //初始化DTCM内存池

	OSInit();  //UCOS初始化
        while(lwip_comm_init())         //lwip初始化
	{
        LED0(0);
        delay_ms(500);
        LED0(1);
        delay_ms(500);
	}
	OSTaskCreate(Start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //创建开始任务
	OSStart(); //开始任务		
}

//开始任务
void Start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //开启统计任务
			
	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(LedA_task,(void*)0,(OS_STK*)&LEDA_TASK_STK[LEDA_STK_SIZE-1],LEDA_TASK_PRIO);//创建LEDA任务
	OSTaskSuspend(START_TASK_PRIO);//挂起开始任务
	OS_EXIT_CRITICAL();  //退出临界区(开中断)
}
 
//LEDA任务
void LedA_task(void *pdata)
{
        Socket_init();
	while(1)
	{
            OSTimeDlyHMSM(0,0,0,50);
            LED0_Toggle;
	}
}