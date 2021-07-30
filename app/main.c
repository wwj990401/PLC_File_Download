#include "main.h"

int main(void)
{
//        Stm32_Clock_Init(423,12,2,9);
//        LED_Init();
//        
//        Cache_Enable();                 //打开L1-Cache
//        HAL_Init();			//初始化HAL库
//        delay_init(216);                //延时初始化
//	uart_init(115200);		//串口初始化
//        SDRAM_Init();                   //初始化SDRAM
//	usmart_dev.init(108); 		//初始化USMART	
//        my_mem_init(SRAMIN);		//初始化内部内存池
//        my_mem_init(SRAMEX);		//初始化外部内存池
//        my_mem_init(SRAMDTCM);		//初始化DTCM内存池
//        
//
//        while(1)
//	{
//              LED0(0);
//              delay_ms(500);
//              LED0(1);
//              delay_ms(500);
//	}
    
//	OSInit();  //UCOS初始化
//	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //创建开始任务
//	OSStart(); //开始任务
        
            u16 i=0;
    
    Write_Through();                //开启强制透写！
    MPU_Memory_Protection();        //保护相关存储区域
    Cache_Enable();                 //打开L1-Cache
    
    HAL_Init();				        //初始化HAL库
    Stm32_Clock_Init(432,12,2,9);   //设置时钟,216Mhz 
    delay_init(216);                //延时初始化
	uart_init(115200);		        //串口初始化
	usmart_dev.init(108); 		    //初始化USMART
    LED_Init();                     //初始化LED
    SDRAM_Init();                   //初始化SDRAM
    my_mem_init(SRAMIN);		    //初始化内部内存池
	my_mem_init(SRAMEX);		    //初始化外部内存池
	my_mem_init(SRAMDTCM);		    //初始化DTCM内存池
    TIM3_Init(1000-1,1080-1);       //定时器3初始化，定时器时钟为108M，分频系数为1080-1，
                                    //所以定时器3的频率为108M/1080=100K，自动重装载为1000-1，那么定时器周期就是10ms
	while(lwip_comm_init())         //lwip初始化
	{
        LED0(0);
        delay_ms(500);
        LED0(1);
        delay_ms(500);
	}
        LED1(0);
#if LWIP_DHCP   //使用DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//等待DHCP获取成功/超时溢出
	{  
		lwip_periodic_handle();	//LWIP内核需要定时处理的函数
	}
#endif
	while(1)
	{
		lwip_periodic_handle();	//LWIP内核需要定时处理的函数
        delay_us(200);
        i++;
        if(i==2000)
        {
            i=0;
            LED0_Toggle;
        }
	}		
}

////开始任务
//void start_task(void *pdata)
//{
//	OS_CPU_SR cpu_sr=0;
//	pdata=pdata;
//	OSStatInit();  //开启统计任务
//		
//	
//	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
//	OSTaskCreate(ledA_task,(void*)0,(OS_STK*)&LEDA_TASK_STK[LEDA_STK_SIZE-1],LEDA_TASK_PRIO);//创建LEDA任务
//        OSTaskCreate(ledB_task,(void*)0,(OS_STK*)&LEDB_TASK_STK[LEDB_STK_SIZE-1],LEDB_TASK_PRIO);//创建LEDA任务
//	OSTaskSuspend(START_TASK_PRIO);//挂起开始任务
//	OS_EXIT_CRITICAL();  //退出临界区(开中断)
//}
// 
////LEDA任务
//void ledA_task(void *pdata)
//{
//	while(1)
//	{
//              LED0(0);
//              OSTimeDlyHMSM(0,0,0,500);
//              LED0(1);
//              OSTimeDlyHMSM(0,0,0,500);
//	}
//}
//
////LEDB任务
//void ledB_task(void *pdata)
//{
//	while(1)
//	{
//              LED1(1);
//              OSTimeDlyHMSM(0,0,0,50);
//              LED1(0);
//              OSTimeDlyHMSM(0,0,0,50);
//	}
//}