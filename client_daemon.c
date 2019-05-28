/* 被依赖的头文件要放在前面 */
#include "public.h"
#include "client_send.h"
#include "mysql.h"
extern int errno;

static CLIENT_OPTION_S 			astClientOptFunc[] = 
{
	{	MSG_TYPE_SHOWALL, 		sendShowAllMsg		},
	{	MSG_TYPE_SHOWFRIEND, 	sendShowFriendMsg	},
	{	MSG_TYPE_ADDFRIEND, 	sendAddFriendMsg	},
	{	MSG_TYPE_DELFRIEND, 	sendDelFriendMsg	},
	{	MSG_TYPE_SENDONE,		sendMsg2One			},
	{	MSG_TYPE_SENDALL,		sendMsg2All			},
	{	MSG_TYPE_SENDFILE,		sendFile2One		},
	{	MSG_TYPE_BANONE,		banOne				},
	{	MSG_TYPE_LOGOUT,		sendLogoutMsg		}
};

/* 获取当前的系统时间 */
char *getLocalTime(char *pRetTime, int maxTimeLen)
{
	time_t tmpTime;
	/* 获取当前系统时间 */
	time(&tmpTime);
	struct tm *pNowTime;
	/* 将获取的时间结构化为本地时间 */
	pNowTime = localtime(&tmpTime);
	snprintf(pRetTime, maxTimeLen, "%d:%d:%d", pNowTime->tm_hour, pNowTime->tm_min, pNowTime->tm_sec);
}

/* 客户端接收服务端消息的函数 */
void *clientProcRecvMsg(void *arg)
{	
	int iRec = -1;
	int nread = 0;
	int filePacketNum = 0;
	USER_INFO_S *pstUserInfo = (USER_INFO_S *)arg;
	int iSocket = pstUserInfo->fd;
	MSG_HEAD_S *pMsgHead = NULL;
	char *pcWord = (char *)malloc(MAX_WORD_LEN + 1);
	char *pcMsgBuf = (char *)malloc(sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
	char *pNowTime = (char *)malloc(MAX_TIME_LEN + 1);
	if(NULL == pcMsgBuf || NULL == pcWord || NULL == pNowTime)
	{
		perror("clientProcRecvMsg:malloc");
		return NULL;
	}
	do
	{
		//printf("client waiting for recv msg\n");
		memset(pcMsgBuf, 0, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
		memset(pcWord, 0, MAX_WORD_LEN + 1);
		nread = read(iSocket, pcMsgBuf, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
		//printf("nread = %d\n",nread);
		//printf("aaa%s\n",((MSG_DATA_S *)pcMsgBuf)->pData);
		if(nread < 0)
		{
			perror("client read");
			break;
		}
		else if(0 == nread)
		{
			printf("the server is shutdown abnormally\n");
			break;
		}
		else
		{
			//printf("client recve %d bytes\n", nread);
			pMsgHead = (MSG_HEAD_S *)pcMsgBuf;
			if(pMsgHead->enMsgType == MSG_TYPE_SENDFILE)
			{
				doRecvFile((MSG_DATA_S *)pcMsgBuf, pstUserInfo->name, &filePacketNum);
			}
			else
			{
				getLocalTime(pNowTime, MAX_TIME_LEN + 1);
				memcpy(pcWord, (char *)pcMsgBuf+sizeof(MSG_HEAD_S), MAX_WORD_LEN);
				printf("%s:%s\t\t%s\n",pMsgHead->srcName, pcWord, pNowTime);
				//printf("%s\n", pcMsgBuf);
			}
		}
	}while(1);

	/* 客户端退出, 则释放该客户端对应的资源信息 */
	if(pcMsgBuf != NULL)
	{
		free(pcMsgBuf);
		pcMsgBuf = NULL;
	}
	if(pcWord != NULL)
	{
		free(pcWord);
		pcWord = NULL;
	}
	if(pstUserInfo != NULL)
	{
		free(pstUserInfo);
		pstUserInfo = NULL;
	}
	if(pNowTime != NULL)
	{
		free(pNowTime);
		pNowTime = NULL;
	}
}

/* 将与服务端通信的套接字设置为非阻塞 */
void setnonblockFd(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	flag |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flag);

	return;
}

/* 创建与服务端通信的套接字 */
ulong createClientSocket(int *piSocket)
{
	ulong ulErrCode = ERROR_SUCCESS;

	struct sockaddr_in sAddr;
	memset(&sAddr, 0, sizeof(struct sockaddr_in));
	int sLen = -1;
	int iClientSocket;

	sAddr.sin_family = AF_INET;
	//sAddr.sin_addr.s_addr = inet_addr("118.25.154.28");
	sAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sAddr.sin_port = htons(6666);
	sLen = sizeof(sAddr);

	iClientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == iClientSocket)
	{
		perror("socket");
		return ERROR_FAILED;
	}
	if(-1 == connect(iClientSocket, (struct sockaddr *)&sAddr, sLen))
	{
		printf("%d\n",errno);
		printf("%d\n", iClientSocket);
		perror("connect");
		ulErrCode = ERROR_FAILED;
	}
	//setnonblockFd(iClientSocket);
	*piSocket = iClientSocket;

	return ulErrCode;
}

void dispatchMsg(MSG_DATA_S *pClientData, int iClientFd)
{ 
	assert(pClientData != NULL);
	
	int 	iOpt = pClientData->stMsgHead.enMsgType;
	//printf("clientFd =%d iOpt = %d\n",iClientFd, iOpt);
	CLIENT_OPTION_S 	*pstMsgOpt = astClientOptFunc;
	//根据客户端的不同请求调用不同的处理函数
	pstMsgOpt[iOpt].pfClientMsgOption(pClientData, iClientFd);
}

void dispatchClientRequest(MSG_DATA_S *pClientData, int iClientFd, char *pSrcName)
{
	assert(pClientData != NULL);
	assert(pSrcName != NULL);

	int iChoice;
	int iAddOrDel;
	char ch;
	int flag = 1;
	int hasCheck = 0;
	do
	{
		system("clear");
		if(flag == 2)
		{
			printf("您已被禁言,无法执行该项操作\n");
		}
		if(flag == 0)
		{
			printf("输入有误, 请重新输入\n");
		}
		if(flag == 4)
		{
			printf("您不是管理员，无法进行该项操作\n");
		}
		printf("***********请输入你要进行的操作	  	  **********\n");
		printf("***********1.查看所有用户 	    **********\n");
		printf("***********2.查看好友列表 	    **********\n");
		printf("***********3.添加/删除好友 	    **********\n");
		printf("***********4.和好友私聊			  **********\n");
		printf("***********5.和所有好友闲聊		  **********\n");
		printf("***********6.给好友发送文件		  **********\n");
		printf("***********7.禁言某人			  **********\n");
		printf("***********8.下线			    **********\n");
		printf("***********返回此界面快捷键exit	  	  **********\n");
		/* 访问数据库获取离线时收到的消息 */
		if(0 == hasCheck)
		{
			/* 获取数据库中记录的该用户相关的离线消息 
			 * 获取并显示给用户之后则清除该消息 */
			getUserMsgFromMysql(pSrcName);
			/* 为什么在这改变hascheck的值没作用？ */
			//hasCheck = 1;
		}
		//这里要用%s, ch也要取地址
		scanf("%s",&ch);
		if(!(ch >= '0' && ch <= '9'))
		{
			flag = 0;
			continue;
		}
		if(strlen(&ch) > 1)
		{
			flag = 0;
			continue;
		}
		flag = 1;
		hasCheck = 1;
		iChoice = atoi(&ch);
		//memset(pClientData, 0, sizeof(MSG_DATA_S));
		//printf("iChoice = %d\n",iChoice);
		//printf("flag = %d\n",flag);
		switch(iChoice)
		{
			case 1:
				pClientData->stMsgHead.enMsgType = MSG_TYPE_SHOWALL;
				break;
			case 2:
				pClientData->stMsgHead.enMsgType = MSG_TYPE_SHOWFRIEND;
				break;
			case 3:
				do
				{
					printf("**********0.添加好友 1.删除好友*********\n");
					scanf("%d", &iAddOrDel);
				}while(iAddOrDel != 0 && iAddOrDel != 1);
				if(0 == iAddOrDel)
					pClientData->stMsgHead.enMsgType = MSG_TYPE_ADDFRIEND;
				else
					pClientData->stMsgHead.enMsgType = MSG_TYPE_DELFRIEND;
				break;
			case 4:
				if(IsBan(pSrcName))
					flag = 2;
				pClientData->stMsgHead.enMsgType = MSG_TYPE_SENDONE;
				break;
			case 5:
				if(IsBan(pSrcName))
					flag = 2;
				pClientData->stMsgHead.enMsgType = MSG_TYPE_SENDALL;
				break;
			case 6:
				pClientData->stMsgHead.enMsgType = MSG_TYPE_SENDFILE;
				break;
			case 7:
				pClientData->stMsgHead.enMsgType = MSG_TYPE_BANONE;
				break;
			case 8:
				pClientData->stMsgHead.enMsgType = MSG_TYPE_LOGOUT;
				return;
			default:
				flag = 0;
				break;
		}
		if(2 == flag || 0 == flag)
			continue;

		/* 用户没有被禁言且操作选项无误 */
		if(1 == flag)
		{
			/* 普通用户想使用禁言功能，则提示没有权限 */
			if((iChoice == 7 || iChoice == 1) && 
					(strcmp(pSrcName, "admin") != 0))
			{
				flag = 4;
				continue;
			}
			/* 清空之前存储的报文体内容 */
			memset(pClientData->pData, 0, MAX_WORD_LEN + 1);
			dispatchMsg(pClientData,iClientFd);
		}

	}while(iChoice != 8);

	return;
}

/* 初始化客户端资源:           
 * 创建客户端套接字
 * 创建接收服务端消息的线程 */
ulong initClientResource(char *pSrcName)
{
	ulong ulErrCode = ERROR_SUCCESS;
	
	int iChoice = -1;
	int iClientFd;
	pthread_t pId;
	MSG_DATA_S 	*pClientData;
	char *pData = NULL;
	USER_INFO_S *pstUserInfo = (USER_INFO_S *)malloc(sizeof(USER_INFO_S));
	if(NULL == pstUserInfo)
	{
		perror("initClientResource:malloc");
		return ERROR_FAILED;
	}
	//char *pMsg = NULL;
	do
	{
		/*pMsg = (char *)malloc(1024);
		if(NULL == pMsg)
		{
			return ERROR_FAILED;
		}*/
		pClientData = (MSG_DATA_S *)malloc(sizeof(MSG_DATA_S));
		if(NULL == pClientData)
		{
			perror("initClientResource:malloc");
			return ERROR_FAILED;
		}
		/* 对这种复合型资源, 要先清零外层资源,然后才能申请内层资源 */
		memset(pClientData, 0, sizeof(MSG_DATA_S));
		/*
		pData = (char *)malloc(MAX_WORD_LEN + 1);
		if(NULL == pData)
		{
			free(pClientData);
			pClientData = NULL;
			perror("initClientResource:malloc");
			return ERROR_FAILED;
		}
		memset(pData, 0, MAX_WORD_LEN + 1);
		pClientData->pData = pData;
		*/
		/* 创建客户端套接字 */
		ulErrCode = createClientSocket(&iClientFd);
		if(ERROR_FAILED == ulErrCode)
		{
			perror("initClientResource:createClientSocket");
			break;
		}
		write(iClientFd, pSrcName, strlen(pSrcName));
		//客户端这边副线程负责处理服务端发来的数据,主线程负责向服务端发送数据
		strcpy(pstUserInfo->name, pSrcName);
		pstUserInfo->fd = iClientFd;
		ulErrCode = pthread_create(&pId, NULL, clientProcRecvMsg, (void *)pstUserInfo);
		if(ulErrCode != ERROR_SUCCESS)
		{
			perror("initClientResource:pthread_create");
			break;
		}
		strcpy(pClientData->stMsgHead.srcName, pSrcName);
		printf("当前用户 : %s\n", pClientData->stMsgHead.srcName);
		/* 设置线程状态为分离态,待运行结束由操作系统回收其资源 */
		pthread_detach(pId);
		dispatchClientRequest(pClientData, iClientFd, pSrcName);
		
		return ulErrCode;
	}while(0);	
	
	/* 以上执行失败会走到这, 统一释放资源 */	
	if(iClientFd > 0)
	{
		close(iClientFd);
	}
	/*
	if(pClientData->pData != NULL)
	{
		free(pClientData->pData);
		pClientData->pData = NULL;
	}*/
	if(pClientData != NULL)
	{
		free(pClientData);
		pClientData = NULL;
	}
	
	return ulErrCode;
}

int main()
{
	ulong 	ulErrCode = ERROR_SUCCESS;
	
	//char 	*pSrcName = (char *)malloc(MAX_NAME_LEN + 1);
	char pSrcName[MAX_NAME_LEN + 1];
	ulErrCode = checkUserInfo(pSrcName);
	if(ulErrCode != ERROR_SUCCESS)
	{
		perror("checkUserInfo");
		return ulErrCode;
	}

	ulErrCode = initClientResource(pSrcName);
	if(ulErrCode != ERROR_SUCCESS)
	{
		perror("initClientResource");
		return ulErrCode;
	}

	return ulErrCode;
}
