#include "public.h"
#include "client_proc.h"

/* 处理分析添加好友的用户请求报文 */
void doProcAddFriendRequest(MSG_DATA_S *pstClientMsg, char *pcName, int iSocket)
{
	assert(pstClientMsg != NULL);
	
	char sendName[MAX_NAME_LEN + 1];
	int choice;
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstClientMsg;
	strcpy(sendName, pstHead->srcName);	

	/* 发送者srcName是自己, 说明该报文是个回复报文,可能是服务端回复的,也可能是对方接受处理后回复的
	 *如果srcName是别人，说明是别人发来的请求，处理内容之后，把报文头的desName改为对方，然后交由服务端转发
	 */
	if(strcmp(sendName, pcName) == 0)
	{
		printf("%s\n", pstClientMsg->pData);
		return;
	}
	memset(pstHead->desName, 0, MAX_NAME_LEN + 1);
	memset(pstClientMsg->pData, 0, MAX_WORD_LEN + 1);
	strcpy(pstHead->desName, sendName);
	/* 别人发来的请求报文 */
	printf("%s请求添加您为好友,是否同意(输入两次0:同意  输入两次1:拒绝):\n", sendName);
	scanf("%d", &choice);
	//printf("choice = %d\n",choice);
	if(choice == 0)
	{
		printf("您已同意添加对方为好友,添加成功\n");
		/* 将信息更新至数据库存储，并给予对方添加成功的提示 */
		insertFriend2Mysql(sendName, pcName);
		strcpy(pstClientMsg->pData, "对方已同意您的好友请求");
	}
	else
	{
		strcpy(pstClientMsg->pData, "对方拒绝了您的好友请求");
	}
	write(iSocket, pstClientMsg, sizeof(MSG_DATA_S));
}
