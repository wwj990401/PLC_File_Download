#include "lwip_tcp_client_socket.h"

//初始化客户端任务
//任务优先级
#define INIT_CLIENT_TASK_PRIO	        15
//任务堆栈大小
#define INIT_CLIENT_STK_SIZE		128
//任务堆栈
OS_STK	INIT_CLIENT_TASK_STK[INIT_CLIENT_STK_SIZE];

//连接服务器任务
//任务优先级
#define CONNECT_SERVER_TASK_PRIO	14
//任务堆栈大小
#define CONNECT_SERVER_STK_SIZE	        128
//任务堆栈
OS_STK	CONNECT_SERVER_TASK_STK[CONNECT_SERVER_STK_SIZE];

//接收服务器信息任务
//任务优先级
#define REV_SERVER_TASK_PRIO	        13
//任务堆栈大小
#define REV_SERVER_STK_SIZE		1600
//任务堆栈
OS_STK	REV_SERVER_TASK_STK[REV_SERVER_STK_SIZE];

//发送文件路径及名字任务
//任务优先级
#define SEND_FILENAME_TASK_PRIO	        12
//任务堆栈大小
#define SEND_FILENAME_STK_SIZE		256
//任务堆栈
OS_STK	SEND_FILENAME_TASK_STK[SEND_FILENAME_STK_SIZE];

int sock_connect;	        //客户端套接字
char g_fileName[100];           //文件名字及路径

//创建初始化客户端任务
void Init_Client(void)
{
        OS_CPU_SR cpu_sr=0;
        OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(Init_Client_task,(void*)0,(OS_STK*)&INIT_CLIENT_TASK_STK[INIT_CLIENT_STK_SIZE-1],INIT_CLIENT_TASK_PRIO);
        OS_EXIT_CRITICAL();  //退出临界区(开中断)
}

//初始化客户端任务
void Init_Client_task(void *pdata)
{
        OS_CPU_SR cpu_sr=0;
	//创建客户端套接字
	sock_connect = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //客户端套接字
	if (sock_connect == -1)
	{
		printf("\n客户端套接字创建失败!\n");
		OSTaskDel(OS_PRIO_SELF);
	}
        printf("\n客户端套接字创建成功!\n");
        printf("%d\n",sock_connect);
	OS_ENTER_CRITICAL();    //进入临界区(关闭中断)
	OSTaskCreate(Connect_Server_task,(void*)0,(OS_STK*)&CONNECT_SERVER_TASK_STK[CONNECT_SERVER_STK_SIZE-1],CONNECT_SERVER_TASK_PRIO);
        OS_EXIT_CRITICAL();     //退出临界区(开中断)
        OSTaskDel(OS_PRIO_SELF);
}

//连接服务器任务
void Connect_Server_task(void *pdata)
{
        OS_CPU_SR cpu_sr=0;
        struct sockaddr_in server_addr;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("192.168.1.10");
	server_addr.sin_port = htons(8000);
        
	if(connect(sock_connect, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) //与指定IP地址和端口的服务端连接
	{
                close(sock_connect);         //一次只接收一个连接 
		printf("服务器连接失败!\n");
		OSTaskDel(OS_PRIO_SELF); 
	}
	printf("服务器连接成功!\n");
        
        int val = SO_KEEPALIVE;                 //开启心跳帧检测
        setsockopt(sock_connect, SOL_SOCKET, SO_KEEPALIVE, &val, 4); 
        int keepIdle = 3;                       //首次探测开始前的tcp无数据收发空闲时间
        int keepInterval = 1;                   //每次探测的间隔时间
        int keepCount = 2;                      //探测次数
        setsockopt(sock_connect, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle)); 
        setsockopt(sock_connect, IPPROTO_TCP,TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)); 
        setsockopt(sock_connect,IPPROTO_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));
        
        OS_ENTER_CRITICAL();    //进入临界区(关闭中断)
	OSTaskCreate(Rev_Server_File_task,(void*)0,(OS_STK*)&REV_SERVER_TASK_STK[REV_SERVER_STK_SIZE-1],REV_SERVER_TASK_PRIO);
        OS_EXIT_CRITICAL();     //退出临界区(开中断)
	OSTaskDel(OS_PRIO_SELF); 
}

//接收文件任务
void Rev_Server_File_task(void *pdata)
{
        
  	u16 newLength = 0;
        u16 lastLength = 0;
	u8 receiveData[DATA_MAX_LENGTH];          //接收数据长度
        u8 flag=0;                            //异常标志
        u32 writeAddress;                       //下载地址
        char buf[1];
        
        while (1) 
        {
                memset(receiveData, 0, DATA_MAX_LENGTH);
                newLength = recv(sock_connect, receiveData, DATA_MAX_LENGTH, 0);      //接收客户端信息
                if(newLength <= 0 || newLength > DATA_MAX_LENGTH)
                {
                        printf("%d\n",newLength);
                        break;
                }
                
                lastLength=newLength;
                while(newLength < 5)                                            //确保接收报头完整
                {
                        newLength=lastLength+recv(sock_connect, receiveData+lastLength, DATA_MAX_LENGTH-lastLength, 0);
                        if(newLength == lastLength || newLength > DATA_MAX_LENGTH)
                        {
                                flag=1;
                                break;
                        }
                        lastLength=newLength;
                }
                while(newLength != (receiveData[3]*256+receiveData[4]+5))       //确保接收了一帧数据
                {
                        newLength=lastLength+recv(sock_connect, receiveData+lastLength, DATA_MAX_LENGTH-lastLength, 0);
                        if(newLength == lastLength || newLength > DATA_MAX_LENGTH)
                        {
                                flag=1;
                                break;
                        }
                        lastLength=newLength;
                }
                if(flag==1)             //判断接收是否正常                  
                        break;
                
                printf("%d\n",newLength);
                
                if(receiveData[0]!=1)
                {
                        buf[0]=0x03;                    //PLC号码错误
                        send(sock_connect,buf, 1, 0);      //发送信息给客户端
                }
                else if((receiveData[1]*256+receiveData[2])==0)
                {                       
                        writeAddress=ADDR_FLASH_SECTOR_5;
                        STMFLASH_EraseSector(writeAddress);
                        buf[0]=0x00;                    //PLC可开始接收文件
                        send(sock_connect,buf, 1, 0);      //发送信息给客户端
                }
                else if((receiveData[1]*256+receiveData[2])==newLength-5)
                {
                        buf[0]=0x04;                    //数据帧长度错误
                        send(sock_connect,buf, 1, 0);      //发送信息给客户端
                }
                else if((receiveData[3]*256+receiveData[4])==188&&((newLength-5)>344))
                {
                        buf[0]=0x05;                    //超过最大存储内存
                        send(sock_connect,buf, 1, 0);      //发送信息给客户端
                }
                else
                {
                        writeAddress=ADDR_FLASH_SECTOR_5+((receiveData[1]*256+receiveData[2])-1)*1400;
                        STMFLASH_WriteDate(writeAddress,receiveData+5,newLength-5);
                        buf[0]=0x01;                    //文件接收成功
                        send(sock_connect,buf, 1, 0);      //发送信息给客户端
                }
        }
        close(sock_connect);         //一次只接收一个连接 
        printf("连接断开\n");
        OSTaskDel(OS_PRIO_SELF);
}

//发送文件路径及名字任务
void Send_File_Name_task(void *pdata)
{
        int length=strlen(g_fileName);
        printf("%d\n",length);
        if(length==send(sock_connect,g_fileName, length, 0))      //发送文件名字给客户端
        {
                printf("发送文件路径及名字成功！\n");
        }
        else
        {
                printf("发送文件路径及名字失败\n");
        }
        OSTaskDel(OS_PRIO_SELF);
}

//接收文件路径及名字并创建发送文件路径及名字任务
void Send_File_Name(char* fileName)
{
        OS_CPU_SR cpu_sr=0;
        int length=strlen(fileName);
        printf("%d\n",length);
        memset(g_fileName, 0, 100);     //清空文件名字缓存区
        strncpy(g_fileName,fileName,length);
        OS_ENTER_CRITICAL();            //进入临界区(关闭中断)
	OSTaskCreate(Send_File_Name_task,(void*)0,(OS_STK*)&SEND_FILENAME_TASK_STK[SEND_FILENAME_STK_SIZE-1],SEND_FILENAME_TASK_PRIO);
        OS_EXIT_CRITICAL();             //退出临界区(开中断)
}

//暂停下载
void Pause_Download(void)
{
        OSTaskSuspend(REV_SERVER_TASK_PRIO);
}

//继续下载
void Continue_Download(void)
{
        OSTaskResume(REV_SERVER_TASK_PRIO);
}
//停止下载
void Stop_Download(void)
{
        
}