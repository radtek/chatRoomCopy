#include "public.h"
#include "server_proc.h"
#include "linklist.h"

/* 对其他文件中全局变量的引用声明 */
extern Linklist *g_pList;

/* 接收用户显示用户列表的请求 */
ulong	procShowuserMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	printf("showuser");
}

/* 接收用户一对一通信的请求 */
ulong 	procMsg2One(MSG_DATA_S 	*pstData, char *pcDesName, int srcFd)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	char *tmpWord = (char *)malloc(MAX_WORD_LEN);
	char *reply = (char *)malloc(64);
	if(NULL == reply)
	{
		perror("procMsg2One");
		return ERROR_FAILED;
	}
	strcpy(reply, "the receiver is not online");
	Linklist *pDestNode = searchName(g_pList, pcDesName);
	if(NULL == pDestNode)
	{
		printf("not found the receiver..\n");
		/* 对方不在线, 暂且将信息原路返回 */
		memcpy((char *)pstData + sizeof(MSG_HEAD_S), reply, strlen(reply));
		if(reply != NULL)
		{
			free(reply);
			reply = NULL;
		}
		write(srcFd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
	}
	else
	{
		printf("found the receiver..\n");
		//memcpy(tmpWord, pstData+sizeof(MSG_HEAD_S),MAX_WORD_LEN);
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
	Linklist 	*pSearchNode = g_pList->next;
	while(pSearchNode != NULL && strcmp(pSearchNode->name, pcDesName) != 0)
		pSearchNode = pSearchNode->next;
	/* 没找到接收者 */
	if(pSearchNode == NULL)
	{
		write(srcFd, "recevier is not online", strlen("recevier is not online"));
	}
	else
	{
		pstMsgHead->desFd = pSearchNode->sfd;
		//将文件信息发送给接收者，让其准备接受文件
		write(pSearchNode->sfd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
		//将接收者的通信套接字信息转发给文件发送者
		write(srcFd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
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
	if(pstData->pData != NULL)
	{
		free(pstData->pData);
		pstData->pData = NULL;
	}
	if(pstData != NULL)
	{
		free(pstData);
		pstData = NULL;
	}

	return ulErrCode;
}








