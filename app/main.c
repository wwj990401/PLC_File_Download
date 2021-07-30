#include "main.h"

int main(void)
{
//        Stm32_Clock_Init(423,12,2,9);
//        LED_Init();
//        
//        Cache_Enable();                 //��L1-Cache
//        HAL_Init();			//��ʼ��HAL��
//        delay_init(216);                //��ʱ��ʼ��
//	uart_init(115200);		//���ڳ�ʼ��
//        SDRAM_Init();                   //��ʼ��SDRAM
//	usmart_dev.init(108); 		//��ʼ��USMART	
//        my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
//        my_mem_init(SRAMEX);		//��ʼ���ⲿ�ڴ��
//        my_mem_init(SRAMDTCM);		//��ʼ��DTCM�ڴ��
//        
//
//        while(1)
//	{
//              LED0(0);
//              delay_ms(500);
//              LED0(1);
//              delay_ms(500);
//	}
    
//	OSInit();  //UCOS��ʼ��
//	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //������ʼ����
//	OSStart(); //��ʼ����
        
            u16 i=0;
    
    Write_Through();                //����ǿ��͸д��
    MPU_Memory_Protection();        //������ش洢����
    Cache_Enable();                 //��L1-Cache
    
    HAL_Init();				        //��ʼ��HAL��
    Stm32_Clock_Init(432,12,2,9);   //����ʱ��,216Mhz 
    delay_init(216);                //��ʱ��ʼ��
	uart_init(115200);		        //���ڳ�ʼ��
	usmart_dev.init(108); 		    //��ʼ��USMART
    LED_Init();                     //��ʼ��LED
    SDRAM_Init();                   //��ʼ��SDRAM
    my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);		    //��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMDTCM);		    //��ʼ��DTCM�ڴ��
    TIM3_Init(1000-1,1080-1);       //��ʱ��3��ʼ������ʱ��ʱ��Ϊ108M����Ƶϵ��Ϊ1080-1��
                                    //���Զ�ʱ��3��Ƶ��Ϊ108M/1080=100K���Զ���װ��Ϊ1000-1����ô��ʱ�����ھ���10ms
	while(lwip_comm_init())         //lwip��ʼ��
	{
        LED0(0);
        delay_ms(500);
        LED0(1);
        delay_ms(500);
	}
        LED1(0);
#if LWIP_DHCP   //ʹ��DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{  
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
	}
#endif
	while(1)
	{
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
        delay_us(200);
        i++;
        if(i==2000)
        {
            i=0;
            LED0_Toggle;
        }
	}		
}

////��ʼ����
//void start_task(void *pdata)
//{
//	OS_CPU_SR cpu_sr=0;
//	pdata=pdata;
//	OSStatInit();  //����ͳ������
//		
//	
//	OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
//	OSTaskCreate(ledA_task,(void*)0,(OS_STK*)&LEDA_TASK_STK[LEDA_STK_SIZE-1],LEDA_TASK_PRIO);//����LEDA����
//        OSTaskCreate(ledB_task,(void*)0,(OS_STK*)&LEDB_TASK_STK[LEDB_STK_SIZE-1],LEDB_TASK_PRIO);//����LEDA����
//	OSTaskSuspend(START_TASK_PRIO);//����ʼ����
//	OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
//}
// 
////LEDA����
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
////LEDB����
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