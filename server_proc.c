#include "public.h"
#include "server_proc.h"
#include "linklist.h"

/* 对其他文件中全局变量的引用声明 */
extern Linklist *g_pList;

/* 接收用户显示用户列表的请求 */
ulong	procShowuserMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	printf("users exec showuser operation\n");
}

/* 接收用户一对一通信的请求 */
ulong 	procMsg2One(MSG_DATA_S 	*pstData, char *pcDesName, int srcFd)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	char *tmpWord = (char *)malloc(MAX_WORD_LEN);
	/*char *reply = (char *)malloc(64);
	if(NULL == reply)
	{
		perror("procMsg2One");
		return ERROR_FAILED;
	}
	strcpy(reply, "the receiver is not online");*/
	Linklist *pDestNode = searchName(g_pList, pcDesName);
	if(NULL == pDestNode)
	{
		/* 对方不在线, 暂且将信息原路返回 */
		/*memcpy((char *)pstData + sizeof(MSG_HEAD_S), reply, strlen(reply));
		if(reply != NULL)
		{
			free(reply);
			reply = NULL;
		}
		write(srcFd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);*/
		/* 将用户离线时收到的消息暂存在数据库中 */
		insertUserMsg2Mysql(pstData, pcDesName);	
	}
	else
	{
		//printf("destination name:%s\n",pDestNode->name);
		write(pDestNode->sfd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
	}

	return ulErrCode;
}

/* 服务端处理转发群发消息的请求 */
ulong 	procMsg2All(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	//char *tmpWord = (char *)malloc(MAX_WORD_LEN);
	Linklist 	*pSearchNode = g_pList->next;
	while(pSearchNode != NULL)
	{
		write(pSearchNode->sfd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
		pSearchNode = pSearchNode->next;
	}
	
	return ulErrCode;
}

/* pstData是报文数据, pcDesName是接受者的用户名, srcFd是和发送者通信的套接字 */
ulong 	sendFile2One(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	MSG_HEAD_S *pstMsgHead = (MSG_HEAD_S *)pstData;
	if(NULL == pstMsgHead)
	{
		perror("sendFile2One");
		return ERROR_FAILED;
	}
	char reply[32] = "receiver is not online\n";
	Linklist 	*pSearchNode = g_pList->next;
	while(pSearchNode != NULL && strcmp(pSearchNode->name, pcDesName) != 0)
		pSearchNode = pSearchNode->next;
	/* 没找到接收者 */
	if(pSearchNode == NULL)
	{
		/* 清空报文体，原路返回 */
		memset(pstData->pData, 0, MAX_WORD_LEN + 1);
		memcpy(pstData->pData, reply, strlen(reply));
		write(srcFd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN); 
	}
	else
	{
		/* 将文件报文转发给接收者 */
		write(pSearchNode->sfd, (char *)pstData, sizeof(MSG_DATA_S));
	}

	return ulErrCode;
}

/* 处理客户端发送的禁言某用户的请求 */
/* 该功能由客户端直接和数据库进行交互, 暂时保留该接口, 可以根据需求扩展 */
ulong 	banOne(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong ulErrCode = ERROR_SUCCESS;

	if(NULL == pstData || pcDesName == NULL)
	{
		return ERROR_FAILED;
	}
	write(srcFd, "操作成功\n", strlen("操作成功\n"));

	return ERROR_SUCCESS;
}

/* 处理用户退出登录的请求 */
ulong 	sendLogoutMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong ulErrCode = ERROR_SUCCESS;

	if(NULL == pstData || pcDesName == NULL)
	{
		return ERROR_FAILED;
	}
	MSG_HEAD_S *pstMsgHead = NULL;
	pstMsgHead = (MSG_HEAD_S *)pstData;
	printf("user %s loged out\n", pstMsgHead->srcName);
	/*
	if(pstData->pData != NULL)
	{
		free(pstData->pData);
		pstData->pData = NULL;
	}*/
	if(pstData != NULL)
	{
		free(pstData);
		pstData = NULL;
	}

	return ulErrCode;
}








