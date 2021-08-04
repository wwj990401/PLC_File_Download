#include "lwip_tcp_socket.h"

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

//�����������
//�������ȼ�
#define CHECK_TASK_PRIO	        11
//�����ջ��С
#define CHECK_STK_SIZE		128
//�����ջ
OS_STK	CHECK_TASK_STK[CHECK_STK_SIZE];

int sock_fd;		//�������׽���
int sock_conn;	        //�ͻ����׽���

uint16_t Generate_crc16_check_code(uint8_t *msg, uint16_t msgLen);

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
        server_addr.sin_addr.s_addr = inet_addr("192.168.1.30");
	server_addr.sin_port = htons(PORT);
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
	OSTaskCreate(Rev_file_task,(void*)&sock_conn,(OS_STK*)&REV_TASK_STK[REV_STK_SIZE-1],REV_TASK_PRIO);
        OS_EXIT_CRITICAL();     //�˳��ٽ���(���ж�)
        OSTaskDel(OS_PRIO_SELF);
}

//�����ļ�����
void Rev_file_task(void* psock_conn)
{
        OS_CPU_SR cpu_sr=0;
        
  	u16 length = 0;
	u8 receiveData[RECV_BUF_SIZE];        //�������ݳ���
        u8 CRCLo;
        u8 CRCHi;
        //u16 frameCount=0;                       //����֡����
        u32 writeAddress;                       //���ص�ַ
        int sock_conn=*((int*)psock_conn);
        while (1) 
        {
                memset(receiveData, 0, RECV_BUF_SIZE);
                length = recv(sock_conn, receiveData, RECV_BUF_SIZE, 0);      //���տͻ�����Ϣ
                if(length<=0||length>RECV_BUF_SIZE)
                        break;
                CRCLo=receiveData[length-2];
                CRCHi=receiveData[length-1];
                Generate_crc16_check_code(receiveData,length-2);
                char buf[1]={0};
                if(CRCLo!=receiveData[length-2]||CRCHi!=receiveData[length-1])
                {
                        buf[0]=0x02;                    //CRCУ��ʧ��
                        send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                }
                else
                {
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
                        else if(receiveData[1]==0xFF&&receiveData[2]==0xFF)
                        {
                                break;
                        }
                        else if((receiveData[3]*256+receiveData[4])==188&&((length-7)>344))
                        {
                                buf[0]=0x05;                    //�������洢�ڴ�
                                send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                        }
                        else if((receiveData[1]*256+receiveData[2])==length-7)
                        {
                                buf[0]=0x04;                    //����֡���ȴ���
                                send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                        }
                        else
                        {
                                writeAddress=ADDR_FLASH_SECTOR_5+((receiveData[1]*256+receiveData[2])-1)*1400;
                                STMFLASH_WriteDate(writeAddress,receiveData+5,length-7);
                                printf("%s\n",receiveData+5);
                                buf[0]=0x01;                    //�ļ����ճɹ�
                                send(sock_conn,buf, 1, 0);      //������Ϣ���ͻ���
                        }
                }
        }
        close(sock_conn);         //һ��ֻ����һ������ 
        printf("���ӶϿ�\n");
        OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
	OSTaskCreate(Connect_client_task,(void*)0,(OS_STK*)&CONNECT_TASK_STK[CONNECT_STK_SIZE-1],CONNECT_TASK_PRIO);
        OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
        OSTaskDel(OS_PRIO_SELF);
}

//CRC��λ�ֽ�ֵ��
//CRC_16��8λ������
const uint8_t auchCRCHi[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40 };
//CRC_16��8λ������
const uint8_t auchCRCLo[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40 };

//CRCУ��������
uint16_t Generate_crc16_check_code(uint8_t *msg, uint16_t msgLen)
{
	uint16_t length = msgLen;
	uint8_t uchCRCHi = 0xFF; //��CRC�ֽڳ�ʼ��
	uint8_t uchCRCLo = 0xFF; //��CRC �ֽڳ�ʼ��
	uint32_t uIndex;         //CRCѭ���е�����
	//������Ϣ������
	while (msgLen--)
	{
		//����CRC
		uIndex = uchCRCLo ^ *msg++;
		uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
		uchCRCHi = auchCRCLo[uIndex];
	}
	//���ؽ������λ��ǰ
	*msg++ = uchCRCLo;
	*msg++ = uchCRCHi;
	return length + 2;
}


