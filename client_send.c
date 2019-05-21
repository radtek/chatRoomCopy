#include "public.h"
#include "client_send.h"
#include "mysql.h"

extern int errno;
int nwrite = 0;

/* 查看好友列表 */
ulong sendShowuserMsg(MSG_DATA_S *pClientMsg, int iSocket)
{
	ulong ulErrCode = ERROR_SUCCESS;
	
	if(NULL == pClientMsg)
	{
		perror("sendMsg2One");
		return ERROR_FAILED;
	}
	do
	{
		/* 向和服务端通信的套接字中写数据 */
		nwrite = write(iSocket, pClientMsg, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
	}while(nwrite == -1 && errno == EINTR);
	getUserList();

	return ulErrCode;
}

/* 发送消息给单个好友 */
ulong sendMsg2One(MSG_DATA_S *pClientMsg, int iSocket)
{
	ulong ulErrCode = ERROR_SUCCESS;
	
	if(NULL == pClientMsg)
	{
		perror("sendMsg2One");
		return ERROR_FAILED;
	}
	char *pMsg = (char *)malloc(MAX_WORD_LEN + 1);
	if(NULL == pMsg)
	{
		perror("sendMsg2One");
		return ERROR_FAILED;
	}
	//printf("befor strlen(pMsg) : %d\n",strlen(pMsg));
	/* 首先填充报文头 */
	MSG_HEAD_S *pstMsgHead = (MSG_HEAD_S *)pClientMsg;
	printf("%s\n", pstMsgHead->srcName);
	system("clear");
	printf("请输入你要聊天的对象(exit退出):");
	scanf("%s",pstMsgHead->desName);
	do
	{
		memset(pMsg, 0, MAX_WORD_LEN + 1);
		scanf("%s",pMsg);
		memset((char *)pClientMsg + sizeof(MSG_HEAD_S), 0, strlen(pMsg) + 1);
		/* 填充报文体, 填充的长度是strlen(pMsg); 
		 * 注意指针步长问题 */
		memcpy((char *)pClientMsg + sizeof(MSG_HEAD_S), pMsg, strlen(pMsg));
		do
		{
			//strlen(pClientMsg) = 1;
			//printf("strlen(pClientMsg) = %d\n",strlen(pClientMsg));
			/* 向和服务端通信的套接字中写数据 */
			nwrite = write(iSocket, pClientMsg, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
			//printf("client write %d bytes\n", nwrite);
		}while(nwrite == -1 && errno == EINTR);

		/* 如果用户输入的是exit, 则将此退出请求发给服务端后, 结束此函数 */
		if(strcmp(pMsg, "exit") == 0)
		{
			if(pMsg != NULL)
			{
				free(pMsg);
				pMsg = NULL;
			}
			return ulErrCode;
		}
	}while(1);

	return ulErrCode;
}

/* 向群组人员发送消息 */
ulong sendMsg2All(MSG_DATA_S *pClientMsg, int iSocket)
{
	ulong ulErrCode = ERROR_SUCCESS;
	
	if(NULL == pClientMsg)
	{
		perror("sendMsg2All");
		return ERROR_FAILED;
	}
	char *pMsg = (char *)malloc(MAX_WORD_LEN + 1);
	if(NULL == pMsg)
	{
		perror("sendMsg2All");
		return ERROR_FAILED;
	}
	//printf("befor strlen(pMsg) : %d\n",strlen(pMsg));
	/* 首先填充报文头 */
	MSG_HEAD_S *pstMsgHead = (MSG_HEAD_S *)pClientMsg;
	printf("%s\n", pstMsgHead->srcName);
	system("clear");
	printf("请输入你要群发的消息(exit退出)\n");
	do
	{
		memset(pMsg, 0, MAX_WORD_LEN + 1);
		scanf("%s",pMsg);
		memset((char *)pClientMsg + sizeof(MSG_HEAD_S), 0, strlen(pMsg) + 1);
		if(strcmp(pMsg, "exit") == 0)
		{
			if(pMsg != NULL)
			{
				free(pMsg);
				pMsg = NULL;
			}
			return ulErrCode;
		}
		/* 填充报文体, 填充的长度是strlen(pMsg); 
		 * 注意指针步长问题 */
		memcpy((char *)pClientMsg + sizeof(MSG_HEAD_S), pMsg, strlen(pMsg));
		do
		{
			//strlen(pClientMsg) = 1;
			//printf("strlen(pClientMsg) = %d\n",strlen(pClientMsg));
			/* 向和服务端通信的套接字中写入请求 */
			nwrite = write(iSocket, pClientMsg, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
			//printf("send success\n");
		}while(nwrite == -1 && errno == EINTR);
	}while(1);

	return ulErrCode;
}

/* 向某人发送文件 */
ulong sendFile2One(MSG_DATA_S *pClientMsg, int iSocket)
{
	ulong   ulErrCode = ERROR_SUCCESS;

	if(NULL == pClientMsg)
	{
		perror("sendFile2One");
		return ERROR_FAILED;
	}
	int i;
	MSG_HEAD_S *pstMsgHead = NULL;
	char recvName[16];
	char command[32];
	pstMsgHead = (MSG_HEAD_S *)pClientMsg;
	char **pResult = (char **)malloc(sizeof(char *)*3);
	if(NULL == pResult)
	{
		perror("sendFile2One:malloc");
		return ERROR_FAILED;
	}
	for(i=0; i<3; i++)
	{
		pResult[i] = (char *)malloc(32);
		if(NULL == pResult[i])
		{
			perror("sendFile2One");
			return ERROR_FAILED;
		}
	}
	printf("请输入接收文件的用户名(exit退出)");
	scanf("%s",recvName);
	getchar();
	if(strcmp(recvName,"exit") == 0)
	{
		return ulErrCode;
	}
	strcpy(pstMsgHead->desName, recvName);
	do
	{
		printf("ftp>(ls:查看文件 put:上传文件 exit:退出)");
		memset(command, 0, sizeof(command));
		gets(command);
		if(strcmp(command, "exit") == 0)
		{
			goto end;
		}
		if(strcmp(command, "ls") == 0)
		{
			doCommandLs();
		}
		else
		{
			for(i=0; i<3; i++)
				memset(pResult[i], 0, 32);
			mySplit(command, ' ', pResult);
			if(pResult[0] != NULL && strcmp(pResult[0], "put") == 0)
			{
				/* 读取指定文件并发送 */
				doSendFile(pClientMsg, pResult[1], iSocket);
			}
			else
			{
				printf("输入无法识别,请重新输入\n");
			}
		}
	}while(1);
end:
	for(i=0; i<3; i++)
	{
		if(pResult[i] != NULL)
		{
			free(pResult[i]);
			pResult[i] = NULL;
		}
	}
	if(pResult != NULL)
	{
		free(pResult);
		pResult = NULL;
	}

	return ulErrCode;

}

/* 禁言某人 */
ulong banOne(MSG_DATA_S *pClientMsg, int iSocket)
{
	ulong ulErrCode = ERROR_SUCCESS;
	
	if(NULL == pClientMsg)
	{
		perror("banOne");
		return ERROR_FAILED;
	}
	MYSQL 		*pConnect = NULL;
	int i;
	int res;
	int flag = 0;
	char username[MAX_NAME_LEN];
	char tmpSql[64];
	char option[8];
	pConnect = mysql_init(NULL);
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		perror("mysql_connect_error");
		return ERROR_FAILED;
	}
	system("clear");
	printf("请输入你要进行的操作(0:解禁某人 1:禁言某人):");
	scanf("%d", &flag);
	if(1 == flag)
	{
		printf("请输入你要禁言的用户名称:");
	}
	else
	{
		printf("请输入你要解禁的用户名称:");
	}
	scanf("%s", username);
	getchar();					//用于接收回车符

	snprintf(tmpSql, sizeof(tmpSql), "update users set ban = '%d' where username = '%s'", flag, username);
	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		perror("mysql_query");
		ulErrCode = ERROR_FAILED;
	}
	else
	{
		printf("操作成功(exit退出)\n");
	}
	do
	{
		scanf("%s",option);
	}while(strcmp(option, "exit") != 0);
	/* 记得关闭数据库资源 */
	mysql_close(pConnect);
}

/* 退出登录 */
ulong sendLogoutMsg(MSG_DATA_S *pClientMsg, int iSocket)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	printf("logout success\n");
	return ulErrCode;
}

