#include "lwip_tcp_server_socket.h"

//�׽�������
//�������ȼ�
#define SOCKET_TASK_PRIO	10
//�����ջ��С
#define SOCKET_STK_SIZE		128
//�����ջ
OS_STK	SOCKET_TASK_STK[SOCKET_STK_SIZE];

//���ӿͻ�������
//�������ȼ�
#define CONNECT_TASK_PRIO	9
//�����ջ��С
#define CONNECT_STK_SIZE	128
//�����ջ
OS_STK	CONNECT_TASK_STK[CONNECT_STK_SIZE];

//������������
//�������ȼ�
#define REV_TASK_PRIO	        8
//�����ջ��С
#define REV_STK_SIZE		1600
//�����ջ
OS_STK	REV_TASK_STK[REV_STK_SIZE];

////�����������
////�������ȼ�
//#define CHECK_TASK_PRIO	        11
////�����ջ��С
//#define CHECK_STK_SIZE		1280
////�����ջ
//OS_STK	CHECK_TASK_STK[CHECK_STK_SIZE];

int sock_fd;		//�������׽���
int sock_conn;	        //�ͻ����׽���

//�����׽��ֳ�ʼ������
void Socket_init(void)
{
        OS_CPU_SR cpu_sr=0;
        OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
	OSTaskCreate(Socket_task,(void*)0,(OS_STK*)&SOCKET_TASK_STK[SOCKET_STK_SIZE-1],SOCKET_TASK_PRIO);
        OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
}

//�׽��ֳ�ʼ��
void Socket_task(void *pdata)
{
        OS_CPU_SR cpu_sr=0;
        struct sockaddr_in server_addr;         //��������ַ
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      //�����׽���
	if (sock_fd == -1) 
        {
		printf("�׽��ִ���ʧ��!\n");
		OSTaskDel(OS_PRIO_SELF);
	}
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);
	if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)    //��
        {
		printf("��ʧ��\n");
		OSTaskDel(OS_PRIO_SELF);
	}
	
	if (listen(sock_fd, 1) < 0)             //����
        {
                printf("����ʧ��\n");
		OSTaskDel(OS_PRIO_SELF);
	} 
        
        OS_ENTER_CRITICAL();    //�����ٽ���(�ر��ж�)
	OSTaskCreate(Connect_client_task,(void*)0,(OS_STK*)&CONNECT_TASK_STK[CONNECT_STK_SIZE-1],CONNECT_TASK_PRIO);
        OS_EXIT_CRITICAL();     //�˳��ٽ���(���ж�)
        OSTaskDel(OS_PRIO_SELF);
}

//���ӿͻ�������
void Connect_client_task(void *pdata)
{
        OS_CPU_SR cpu_sr=0;
	struct sockaddr_in conn_addr;           //�ͻ��˵�ַ
	socklen_t addr_len;                     //�׽��ֳ���
        printf("������\r\n");        
	sock_conn = accept(sock_fd, (struct sockaddr *)&conn_addr, &addr_len);  //�ȴ����ӿͻ���  
	printf("���ӳɹ�\r\n");
        
        //���÷��ͳ�ʱ
        int nNetSendTimeout = 10000;
        setsockopt(sock_conn, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetSendTimeout, sizeof(int));
        //���ý��ճ�ʱ
        int nNetReceiveTimeout = 10000;
        setsockopt(sock_conn, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetReceiveTimeout, sizeof(int));
        
        OS_ENTER_CRITICAL();    //�����ٽ���(�ر��ж�)
//        OSTaskCreate(Check_connect_task,(void*)&sock_conn,(OS_STK*)&CHECK_TASK_STK[CHECK_STK_SIZE-1],CHECK_TASK_PRIO);
	OSTaskCreate(Rev_file_task,(void*)&sock_conn,(OS_STK*)&REV_TASK_STK[REV_STK_SIZE-1],REV_TASK_PRIO);
        OS_EXIT_CRITICAL();     //�˳��ٽ���(���ж�)
        OSTaskDel(OS_PRIO_SELF);
}

////�������
//void Check_connect_task(void* psock_conn)
//{
//        int sock_conn=*((int*)psock_conn);
//        char buf[1000]={0};
//        int i;
//        while(1)
//        {
//                printf("���ڷ���!\n");
//                i=send(sock_conn,buf, 1000, 0);     //������Ϣ���ͻ��� 
//                printf("%d\n",i);
//                delay_ms(1000);
//        }
//}

//�����ļ�����
void Rev_file_task(void* psock_conn)
{
        OS_CPU_SR cpu_sr=0;
        
  	u16 newLength = 0;
        u16 lastLength = 0;
	u8 receiveData[RECV_BUF_SIZE];          //�������ݳ���
        u8 flag=0;                            //�쳣��־
        u32 writeAddress;                       //���ص�ַ
        int sock_conn=*((int*)psock_conn);
        char buf[1];
        while (1) 
        {
                memset(receiveData, 0, RECV_BUF_SIZE);
                newLength = recv(sock_conn, receiveData, RECV_BUF_SIZE, 0);      //���տͻ�����Ϣ
                if(newLength <= 0 || newLength > RECV_BUF_SIZE)
                        break;
                
                lastLength=newLength;
                while(newLength < 5)                                            //ȷ�����ձ�ͷ����
                {
                        newLength=lastLength+recv(sock_conn, receiveData+lastLength, RECV_BUF_SIZE-lastLength, 0);
                        if(newLength == lastLength || newLength > RECV_BUF_SIZE)
                        {
                                flag=1;
                                break;
                        }
                        lastLength=newLength;
                }
                while(newLength != (receiveData[3]*256+receiveData[4]+5))       //ȷ��������һ֡����
                {
                        newLength=lastLength+recv(sock_conn, receiveData+lastLength, RECV_BUF_SIZE-lastLength, 0);
                        if(newLength == lastLength || newLength > RECV_BUF_SIZE)
                        {
                                flag=1;
                                break;
                        }
                        lastLength=newLength;
                }
                if(flag==1)             //�жϽ����Ƿ�����                  
                        break;
                
                printf("%d\n",newLength);
                
                if(receiveData[0]!=PLC_NUMBER)
                {
                        buf[0]=0x03;                    //PLC�������
                        send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                }
                else if((receiveData[1]*256+receiveData[2])==0)
                {
                        //���÷��ͳ�ʱ
                        int nNetSendTimeout = 3000;
                        setsockopt(sock_conn, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetSendTimeout, sizeof(int));
                        //���ý��ճ�ʱ
                        int nNetReceiveTimeout = 3000;
                        setsockopt(sock_conn, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetReceiveTimeout, sizeof(int));
                        
                        writeAddress=ADDR_FLASH_SECTOR_5;
                        STMFLASH_EraseSector(writeAddress);
                        buf[0]=0x00;                    //PLC�ɿ�ʼ�����ļ�
                        send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                }
                else if((receiveData[1]*256+receiveData[2])==newLength-5)
                {
                        buf[0]=0x04;                    //����֡���ȴ���
                        send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                }
                else if((receiveData[3]*256+receiveData[4])==188&&((newLength-5)>344))
                {
                        buf[0]=0x05;                    //�������洢�ڴ�
                        send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                }
                else
                {
                        writeAddress=ADDR_FLASH_SECTOR_5+((receiveData[1]*256+receiveData[2])-1)*1400;
                        STMFLASH_WriteDate(writeAddress,receiveData+5,newLength-5);
                        printf("%s\n",receiveData+5);
                        buf[0]=0x01;                    //�ļ����ճɹ�
                        send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                }
        }
        close(sock_conn);         //һ��ֻ����һ������ 
        printf("���ӶϿ�\n");
        OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
	OSTaskCreate(Connect_client_task,(void*)0,(OS_STK*)&CONNECT_TASK_STK[CONNECT_STK_SIZE-1],CONNECT_TASK_PRIO);
        OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
        OSTaskDel(OS_PRIO_SELF);
}