#include "lwip_tcp_server_socket.h"

//套接字任务
//任务优先级
#define SOCKET_TASK_PRIO	10
//任务堆栈大小
#define SOCKET_STK_SIZE		128
//任务堆栈
OS_STK	SOCKET_TASK_STK[SOCKET_STK_SIZE];

//连接客户端任务
//任务优先级
#define CONNECT_TASK_PRIO	9
//任务堆栈大小
#define CONNECT_STK_SIZE	128
//任务堆栈
OS_STK	CONNECT_TASK_STK[CONNECT_STK_SIZE];

//接收数据任务
//任务优先级
#define REV_TASK_PRIO	        8
//任务堆栈大小
#define REV_STK_SIZE		1600
//任务堆栈
OS_STK	REV_TASK_STK[REV_STK_SIZE];

////检查网络任务
////任务优先级
//#define CHECK_TASK_PRIO	        11
////任务堆栈大小
//#define CHECK_STK_SIZE		1280
////任务堆栈
//OS_STK	CHECK_TASK_STK[CHECK_STK_SIZE];

int sock_fd;		//服务器套接字
int sock_conn;	        //客户端套接字

//建立套接字初始化任务
void Socket_init(void)
{
        OS_CPU_SR cpu_sr=0;
        OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(Socket_task,(void*)0,(OS_STK*)&SOCKET_TASK_STK[SOCKET_STK_SIZE-1],SOCKET_TASK_PRIO);
        OS_EXIT_CRITICAL();  //退出临界区(开中断)
}

//套接字初始化
void Socket_task(void *pdata)
{
        OS_CPU_SR cpu_sr=0;
        struct sockaddr_in server_addr;         //服务器地址
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);      //创建套接字
	if (sock_fd == -1) 
        {
		printf("套接字创建失败!\n");
		OSTaskDel(OS_PRIO_SELF);
	}
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_addr.sin_port = htons(SERVER_PORT);
	if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)    //绑定
        {
		printf("绑定失败\n");
		OSTaskDel(OS_PRIO_SELF);
	}
	
	if (listen(sock_fd, 1) < 0)             //监听
        {
                printf("监听失败\n");
		OSTaskDel(OS_PRIO_SELF);
	} 
        
        OS_ENTER_CRITICAL();    //进入临界区(关闭中断)
	OSTaskCreate(Connect_client_task,(void*)0,(OS_STK*)&CONNECT_TASK_STK[CONNECT_STK_SIZE-1],CONNECT_TASK_PRIO);
        OS_EXIT_CRITICAL();     //退出临界区(开中断)
        OSTaskDel(OS_PRIO_SELF);
}

//连接客户端任务
void Connect_client_task(void *pdata)
{
        OS_CPU_SR cpu_sr=0;
	struct sockaddr_in conn_addr;           //客户端地址
	socklen_t addr_len;                     //套接字长度
        printf("监听中\r\n");        
	sock_conn = accept(sock_fd, (struct sockaddr *)&conn_addr, &addr_len);  //等待连接客户端  
	printf("连接成功\r\n");
        
        //设置发送超时
        int nNetSendTimeout = 10000;
        setsockopt(sock_conn, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetSendTimeout, sizeof(int));
        //设置接收超时
        int nNetReceiveTimeout = 10000;
        setsockopt(sock_conn, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetReceiveTimeout, sizeof(int));
        
        OS_ENTER_CRITICAL();    //进入临界区(关闭中断)
//        OSTaskCreate(Check_connect_task,(void*)&sock_conn,(OS_STK*)&CHECK_TASK_STK[CHECK_STK_SIZE-1],CHECK_TASK_PRIO);
	OSTaskCreate(Rev_file_task,(void*)&sock_conn,(OS_STK*)&REV_TASK_STK[REV_STK_SIZE-1],REV_TASK_PRIO);
        OS_EXIT_CRITICAL();     //退出临界区(开中断)
        OSTaskDel(OS_PRIO_SELF);
}

////心跳检测
//void Check_connect_task(void* psock_conn)
//{
//        int sock_conn=*((int*)psock_conn);
//        char buf[1000]={0};
//        int i;
//        while(1)
//        {
//                printf("正在发送!\n");
//                i=send(sock_conn,buf, 1000, 0);     //发送信息给客户端 
//                printf("%d\n",i);
//                delay_ms(1000);
//        }
//}

//接收文件任务
void Rev_file_task(void* psock_conn)
{
        OS_CPU_SR cpu_sr=0;
        
  	u16 newLength = 0;
        u16 lastLength = 0;
	u8 receiveData[RECV_BUF_SIZE];          //接收数据长度
        u8 flag=0;                            //异常标志
        u32 writeAddress;                       //下载地址
        int sock_conn=*((int*)psock_conn);
        char buf[1];
        while (1) 
        {
                memset(receiveData, 0, RECV_BUF_SIZE);
                newLength = recv(sock_conn, receiveData, RECV_BUF_SIZE, 0);      //接收客户端信息
                if(newLength <= 0 || newLength > RECV_BUF_SIZE)
                        break;
                
                lastLength=newLength;
                while(newLength < 5)                                            //确保接收报头完整
                {
                        newLength=lastLength+recv(sock_conn, receiveData+lastLength, RECV_BUF_SIZE-lastLength, 0);
                        if(newLength == lastLength || newLength > RECV_BUF_SIZE)
                        {
                                flag=1;
                                break;
                        }
                        lastLength=newLength;
                }
                while(newLength != (receiveData[3]*256+receiveData[4]+5))       //确保接收了一帧数据
                {
                        newLength=lastLength+recv(sock_conn, receiveData+lastLength, RECV_BUF_SIZE-lastLength, 0);
                        if(newLength == lastLength || newLength > RECV_BUF_SIZE)
                        {
                                flag=1;
                                break;
                        }
                        lastLength=newLength;
                }
                if(flag==1)             //判断接收是否正常                  
                        break;
                
                printf("%d\n",newLength);
                
                if(receiveData[0]!=PLC_NUMBER)
                {
                        buf[0]=0x03;                    //PLC号码错误
                        send(sock_conn,buf, 1, 0);      //发送信息给客户端
                }
                else if((receiveData[1]*256+receiveData[2])==0)
                {
                        //设置发送超时
                        int nNetSendTimeout = 3000;
                        setsockopt(sock_conn, SOL_SOCKET, SO_SNDTIMEO, (char *)&nNetSendTimeout, sizeof(int));
                        //设置接收超时
                        int nNetReceiveTimeout = 3000;
                        setsockopt(sock_conn, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetReceiveTimeout, sizeof(int));
                        
                        writeAddress=ADDR_FLASH_SECTOR_5;
                        STMFLASH_EraseSector(writeAddress);
                        buf[0]=0x00;                    //PLC可开始接收文件
                        send(sock_conn,buf, 1, 0);      //发送信息给客户端
                }
                else if((receiveData[1]*256+receiveData[2])==newLength-5)
                {
                        buf[0]=0x04;                    //数据帧长度错误
                        send(sock_conn,buf, 1, 0);      //发送信息给客户端
                }
                else if((receiveData[3]*256+receiveData[4])==188&&((newLength-5)>344))
                {
                        buf[0]=0x05;                    //超过最大存储内存
                        send(sock_conn,buf, 1, 0);      //发送信息给客户端
                }
                else
                {
                        writeAddress=ADDR_FLASH_SECTOR_5+((receiveData[1]*256+receiveData[2])-1)*1400;
                        STMFLASH_WriteDate(writeAddress,receiveData+5,newLength-5);
                        printf("%s\n",receiveData+5);
                        buf[0]=0x01;                    //文件接收成功
                        send(sock_conn,buf, 1, 0);      //发送信息给客户端
                }
        }
        close(sock_conn);         //一次只接收一个连接 
        printf("连接断开\n");
        OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(Connect_client_task,(void*)0,(OS_STK*)&CONNECT_TASK_STK[CONNECT_STK_SIZE-1],CONNECT_TASK_PRIO);
        OS_EXIT_CRITICAL();  //退出临界区(开中断)
        OSTaskDel(OS_PRIO_SELF);
}