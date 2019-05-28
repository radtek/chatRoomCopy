#include "public.h"
#include "client_proc.h"

/* 处理分析添加好友的用户请求报文 */
void doProcAddFriendRequest(MSG_DATA_S *pstClientMsg, char *pcName)
{
	assert(pstClientMsg != NULL);
	
	char sendName[MAX_NAME_LEN + 1];
	char ch;
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstClientMsg;
	strcpy(sendName, pstHead->srcName);	

	/* 发送者srcName是自己, 说明该报文是个回复报文,可能是服务端回复的,也可能是对方接受处理后回复的
	 为什么对方回复的报文srcName会是自己,不应该是对方用户名吗?这样做是为了区分收到的报文: 是别人发来的请求(srcName是别人的用户名) 还是自己发去的请求被别人回复的内容(srcName是自己) */
	if(strcmp(sendName, pcName) == 0)
	{
		printf("%s\n", pstClientMsg->pData);
		return;
	}
	/* 别人发来的请求报文 */
	printf("%s想添加您为好友, 是否同意?(y/n):");
	scanf("%s", &ch);
	if(ch == 'y')
	{
		insertFriend2Mysql();
	}
	else
	{
	
	}
}
