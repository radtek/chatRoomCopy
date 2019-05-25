#ifndef  PUBLIC_H
#define  PUBLIC_H

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <termios.h>
#include <time.h>
#include <strings.h>
#include <sys/time.h>
#include "threadPool.h"
//#include "circleList.h"
#include "linklist.h"
//#include "server_mysql.h"

#define 	ulong 	unsigned long

#define 	handle_error(msg)	\
	do{ perror(msg); exit(1); }while(0)

#define 	SERVER_IP		"127.0.0.1"
#define 	SERVER_PORT		6666
#define 	false 			0
#define 	true 			1
#define 	INVALID_FD 		-1
#define 	ERROR_SUCCESS 	0
#define 	ERROR_FAILED 	1
#define 	MAX_NAME_LEN 	31
#define 	MAX_TIME_LEN    15
#define 	MAX_WORD_LEN    255 
/*
typedef struct client
{
	DTQ_Node node;
	char clientName[MAX_NAME_LEN + 1];
	int fd;
}CLIENT_S;*/

/* 定义一个全局链表, 存储连接过来的客户端的信息 */
//static CircleList 	*g_pList = NULL;
//static Linklist 	*g_pList = NULL;

/* static关键字解决multiple definition的错误 */
static int     g_iEpollFd = -1;

//static Linklist 	*g_pList = NULL;

//static ThreadPools *pThreadPool = NULL;

typedef     ulong   (*EPOLL_CALLBACK_PF)(void *);

/* 消息类型 */
typedef enum msgType
{
		MSG_TYPE_SHOWUSER = 0,
		MSG_TYPE_SENDONE,
		MSG_TYPE_SENDALL,
		MSG_TYPE_SENDFILE,
		MSG_TYPE_BANONE,
		MSG_TYPE_LOGOUT
}MSG_TYPE_E;

/* 报文头包含的内容 */
typedef struct requestMsgHead
{
		MSG_TYPE_E enMsgType;               //发送的请求类型
		char srcName[MAX_NAME_LEN + 1];     //发送者
		char desName[MAX_NAME_LEN + 1];     //接收者
		int  filesize;						//传输的文件大小
}MSG_HEAD_S;

/* 报文内容 */
typedef struct requestMsg
{
		MSG_HEAD_S stMsgHead;           //报文头结构体
		char pData[MAX_WORD_LEN + 1];                    //有效数据
}MSG_DATA_S;





/* epoll字段data.ptr指向的结构体 */
typedef struct myepoll_event
{
		int fd; 								/* 通信套接字 */
		EPOLL_CALLBACK_PF   pfEpollCallback;    /* Epoll callback function */      
}EPOLL_EVENT_ST;





/* 客户端对应的跳转函数回调 */
typedef ulong  (*CLIENT_OPT_DISPATCH_PF)(MSG_DATA_S *pstClientMsg, int socket);

/* 根据消息类型进行转发的函数跳转表 */
typedef struct clientOperator
{
		MSG_TYPE_E enMsgType;
		CLIENT_OPT_DISPATCH_PF pfClientMsgOption;
}CLIENT_OPTION_S;





/* 服务端对应的跳转函数回调 */
typedef ulong  (*SERVER_OPT_DISPATCH_PF)(MSG_DATA_S *pstClientMsg, char *pcDesName, int cFd);

/* 根据消息类型进行转发的函数跳转表 */
typedef struct serverOperator
{
		MSG_TYPE_E enMsgType;
		SERVER_OPT_DISPATCH_PF pfClientMsgOption;
}SERVER_OPTION_S;





/* 用户名和用户对应的通信套接字 */
typedef struct
{
	char name[MAX_NAME_LEN + 1];
	int fd;
}USER_INFO_S;


#endif


