#include "main.h"

int main(void)
{  
        Write_Through();                //����ǿ��͸д��
        MPU_Memory_Protection();        //������ش洢����
        Cache_Enable();                 //��L1-Cache
    
        HAL_Init();				        //��ʼ��HAL��
        Stm32_Clock_Init(432,12,2,9);   //����ʱ��,216Mhz 
        delay_init(216);                //��ʱ��ʼ��
	uart_init(115200);		        //���ڳ�ʼ��
	usmart_dev.init(216); 		    //��ʼ��USMART
        LED_Init();                     //��ʼ��LED
        SDRAM_Init();                   //��ʼ��SDRAM
        my_mem_init(SRAMIN);		    //��ʼ���ڲ��ڴ��
	my_mem_init(SRAMDTCM);		    //��ʼ��DTCM�ڴ��
        //while(1);
	OSInit();  //UCOS��ʼ��
        while(lwip_comm_init())         //lwip��ʼ��
	{
        LED0(0);
        delay_ms(500);
        LED0(1);
        delay_ms(500);
	}
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
        OSTaskCreate(ledB_task,(void*)0,(OS_STK*)&LEDB_TASK_STK[LEDB_STK_SIZE-1],LEDB_TASK_PRIO);//����LEDB����
	OSTaskSuspend(START_TASK_PRIO);//����ʼ����
	OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
}
 
//LEDA����
void ledA_task(void *pdata)
{
#if LWIP_DHCP
	lwip_comm_dhcp_creat(); //����DHCP����
#endif
	while(1)
	{
            delay_ms(50);
            LED0_Toggle;
	}
}

//LEDB����
void ledB_task(void *pdata)
{
	while(1)
	{
              LED1(0);
              delay_ms(500);
              LED1(1);
              delay_ms(500);
	}
}