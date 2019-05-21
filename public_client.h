#ifndef PUBLIC_CLIENT_H
#define PUBLIC_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <assert.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <termios.h>

#define handle_error(msg) \
do{ perror(msg); exit(1); }while(0)

#define ulong unsigned long

#define INVALID_FD -1
#define SERVER_IP    "127.0.0.1"
#define SERVER_PORT  6666 

#define ERROR_SUCCESS 0
#define ERROR_FAILED  1

#define MAX_NAME_LEN  31
#define MAX_WORD_LEN  511

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
	MSG_TYPE_E enMsgType;           	//发送的请求类型
	char srcName[MAX_NAME_LEN + 1];		//发送者
	char desName[MAX_NAME_LEN + 1];		//接收者
}MSG_HEAD_S;

/* 报文内容 */
typedef struct requestMsg
{
	MSG_HEAD_S stMsgHead;           //报文头结构体
	void *pData;                    //有效数据
}MSG_DATA_S;

typedef ulong  (*CLIENT_OPT_DISPATCH_PF)(MSG_DATA_S *pstClientMsg, int socket);

/* 根据消息类型进行转发的函数跳转表 */
typedef struct clientOperator
{
		MSG_TYPE_E enMsgType;
		CLIENT_OPT_DISPATCH_PF pfClientMsgOption;
}CLIENT_OPTION_S;

#endif
