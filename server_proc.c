#include "public.h"
#include "server_proc.h"
#include "linklist.h"

/* 对其他文件中全局变量的引用声明 */
extern Linklist *g_pList;

/* 接收管理员用户显示所有用户的请求 */
ulong	procShowAllMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	printf("users exec showAllUser operation\n");
}

/* 接收用户显示好友列表的请求 */
ulong	procShowFriendMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	printf("users exec showfriend operation\n");
}

/* 处理用户添加好友的请求 */
ulong	procAddFriendMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong ulErrCode = ERROR_SUCCESS;

	char retMsg[64];
	ulErrCode = checkUserIsExist(pcDesName);
	/* 发送者要添加的用户不存在 */
	if(ERROR_FAILED == ulErrCode)
	{
		strcpy(retMsg, "添加失败, 您要添加的用户不存在");
		memcpy(pstData->pData, retMsg, strlen(retMsg));
		write(srcFd, pstData, sizeof(MSG_DATA_S));
		return ulErrCode;
	}
	Linklist *pDestNode = searchName(g_pList, pcDesName);
	if(NULL == pDestNode)
	{
		/* 将用户添加好友的请求暂存在数据库中,待接收方上线之后发送 */
		//insertAddFriendRequest2Mysql(pstData, pcDesName);	
	}
	else
	{
		//printf("destination name:%s\n",pDestNode->name);
		write(pDestNode->sfd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
	}

	return ulErrCode;
}

/* 处理用户删除好友的请求 */
ulong	procDelFriendMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong ulErrCode = ERROR_SUCCESS;

	char retMsg[64];
	ulErrCode = checkReciverIsFriend(pstData, pcDesName, srcFd);
	// 发送者要添加的用户不存在 
	if(ERROR_FAILED == ulErrCode)
	{
		strcpy(retMsg, "删除失败,好友列表无此用户");
		memcpy(pstData->pData, retMsg, strlen(retMsg));
		write(srcFd, pstData, sizeof(MSG_DATA_S));
		return ulErrCode;
	}
	deleteFriendFromMysql(pstData, pcDesName);
	memset(pstData->pData, 0, MAX_WORD_LEN + 1);
	strcpy(pstData->pData, "删除成功");
	write(srcFd, pstData, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
}

/* 接收用户一对一通信的请求 */
ulong 	procSendMsg2One(MSG_DATA_S 	*pstData, char *pcDesName, int srcFd)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	char retMsg[64] = "您和对方还不是好友关系,无法通信";
	ulErrCode = checkReciverIsFriend(pstData, pcDesName, srcFd);
	/* 如果和对方还不是好友关系,则返回相应提示信息给发送者 */
	if(ERROR_FAILED == ulErrCode)
	{
		memcpy(pstData->pData, retMsg, strlen(retMsg));
		write(srcFd, pstData, sizeof(MSG_DATA_S));
		return ulErrCode;
	}
	Linklist *pDestNode = searchName(g_pList, pcDesName);
	if(NULL == pDestNode)
	{
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
ulong 	procSendMsg2All(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
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
ulong 	procSendFile2One(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
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
		//printf("\n\n%s\n\n", pstData->pData);
		/* 将文件报文转发给接收者 */
		write(pSearchNode->sfd, (char *)pstData, sizeof(MSG_DATA_S));
		sleep(1);
	}

	return ulErrCode;
}

/* 处理客户端发送的禁言某用户的请求 */
/* 该功能由客户端直接和数据库进行交互, 暂时保留该接口, 可以根据需求扩展 */
ulong 	procBanOne(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
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
ulong 	procSendLogoutMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
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








