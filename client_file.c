#include "public.h"
#include "client_file.h"

/* 分割解析用户输入的命令 */
int mySplit(char *str, char delim, char **retStr)
{
	/* retStr的内存要由调用该函数者负责申请 */
	char *pre = str;
	char *cur = str;
	int i = 0;
	if((cur = strchr(cur,delim)))
	{
		//printf("%s\n",cur);
		strncpy(retStr[i], pre, cur-pre);
		retStr[i][cur-pre] = '\0';
		cur += 1;
		i++;
		pre = cur;
	}	
	if(cur != NULL)
		strcpy(retStr[i], cur);

	return;
}

void doCommandLs()
{
	system("ls");	
}

/* 发送文件信息至服务端, 获取接收者的通信套接字 */
void SendFileRequest2Server(MSG_DATA_S *pstMsgData, char *filePath, int fd)
{
	char *pFileInfo = (char *)malloc(32);
	if(NULL == pFileInfo)
	{
		return;
	}
	memset(pFileInfo, 0, 32);
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstMsgData;
	if(NULL == pstHead)
	{
		perror("doSendFile");
		return;
	}
	struct stat buf;
   	int fsize;	
	if(-1 == stat(filePath, &buf))
	{
		perror("fstat");
		return;
	}
	fsize = buf.st_size;
	//printf("file size = %d\n",fsize);
	snprintf(pFileInfo, 32, "%s##%d##",filePath, fsize);
	/* 注意pstMsgData要转成char *，否则指针步长会造成解析文件信息时SF */
	memcpy((char *)pstMsgData + sizeof(MSG_HEAD_S), pFileInfo, strlen(pFileInfo));
	/* 及时释放申请的内存资源 */
	if(pFileInfo != NULL)
	{
		free(pFileInfo);
		pFileInfo = NULL;
	}
	//printf("res = %d\n",res);
	//printf("文件信息 : %s\n", pFileInfo);
	write(fd, pstMsgData, sizeof(MSG_HEAD_S) + MSG_WORD_LEN);
}

/* 发送文件 */
void doSendFile(char *pcMsgBuf)
{
	printf("doSendFile function\n");
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pcMsgBuf;
	assert(pstHead != NULL);
	printf("接收者fd = %d\n",pstHead->desFd);
	int filesize = 0;
	int nread = 0;
	char filePath[32];
	char *pFileInfo = (char *)malloc(64);
	char *pFileBuf = (char *)malloc(512);
	char *pPre = NULL;
	char *pCur = NULL;
	int desFd = pstHead->desFd;
	if(NULL == pFileInfo)
	{
		perror("doSendFile:malloc");
		return;
	}
	//首先将文件信息拷贝出来
	memset(pFileInfo, 0, 64);
	memcpy(pFileInfo, (char *)pcMsgBuf + sizeof(MSG_HEAD_S), MAX_WORD_LEN);
	//然后根据分隔符解析该文件信息(文件名称，文件大小)
	pPre = pFileInfo;
	pCur = strchr(pFileInfo, '#');
	if(NULL == pCur)
	{
		perror("doSendFile:pCur");
		return;
	}
	strncpy(filePath, pPre, pCur-pPre);		//获取文件名称
	filePath[pCur-pPre] = '\0';
	pCur += 2;
	filesize = *pCur;						//获取文件大小
	/* 及时释放申请的资源 */
	if(pFileInfo != NULL)
	{
		free(pFileInfo);
		pFileInfo = NULL;
	}
	int fileFd = open(filePath, O_CREAT | O_RDONLY);
	if(-1 == fileFd)
	{
		perror("doSendFile:open");
		return;
	}
	while(filesize > 0)
	{
		memset(pFileBuf, 0, 512);
		nread = read(fileFd, pFileBuf, 511);
		if(nread < 0)			/* 读取文件失败 */
		{
			perror("doSendFile:read");
			if(pFileBuf != NULL)
			{
				free(pFileBuf);
				pFileBuf = NULL;
			}
			return;
		}
		else if(0 == nread)		/* 文件内容已经读完 */
		{
			break;
		}
		else
		{
			write(desFd, pFileBuf, nread);
			filesize -= nread;
		}
	}
	printf("文件%s:大小:%d 已成功发送\n", filePath, filesize);
	if(fileFd > 0)
	{
		close(fileFd);		/* 关闭文件描述符 */
	}
	if(pFileBuf != NULL)
	{
		free(pFileBuf);
		pFileBuf = NULL;
	}

	return;
}

/* 接收文件 */
void doRecvFile(char *pcMsgBuf, int iSocket)
{
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pcMsgBuf;
	assert(pstHead != NULL);

	int filesize = 0;
	int nread = 0;
	char filePath[32];
	char tmpFileSize[6];
	char *pFileInfo = (char *)malloc(64);
	char *pFileBuf = (char *)malloc(512);
	char *pPre = NULL;
	char *pCur = NULL;
	int desFd = pstHead->desFd;
	if(NULL == pFileInfo)
	{
		perror("doSendFile:malloc");
		return;
	}
	//首先将文件信息拷贝出来
	memset(pFileInfo, 0, 64);
	memcpy(pFileInfo, (char *)pcMsgBuf + sizeof(MSG_HEAD_S), MAX_WORD_LEN);
	//然后根据分隔符解析该文件信息(文件名称，文件大小)
	pPre = pFileInfo;
	pCur = strchr(pFileInfo, '#');
	strncpy(filePath, pPre, pCur-pPre);		//获取文件名称
	filePath[pCur-pPre] = '\0';
	pCur += 2;
	pPre = pCur;
	pCur = strchr(pCur, '#');
	strncpy(tmpFileSize, pPre, pCur-pPre);
	tmpFileSize[pCur-pPre] = '\0';

	filesize = atoi(tmpFileSize);						//获取文件大小
	/* 及时释放申请的资源 */
	if(pFileInfo != NULL)
	{
		free(pFileInfo);
		pFileInfo = NULL;
	}
	int fileFd = open("tmp.txt", O_CREAT | O_APPEND, 0644);
	if(-1 == fileFd)
	{
		perror("doSendFile:open");
		return;
	}
	printf("filePath : %s, filesize : %d\n",filePath, filesize);
	while(filesize > 0)
	{
		sleep(1);
		memset(pFileBuf, 0, 512);
		nread = read(iSocket, pFileBuf, 511);
		printf("nread = %d\n", nread);
		if(nread < 0)			/* 读取文件失败 */
		{
			perror("doRecvFile:read");
			if(pFileBuf != NULL)
			{
				free(pFileBuf);
				pFileBuf = NULL;
			}
			return;
		}
		else if(0 == nread)		/* 文件内容已经读完 */
		{
			break;
		}
		else
		{
			write(fileFd, pFileBuf, nread);
			filesize -= nread;
		}
	}
	printf("文件%s:大小:%d 已成功发送\n", filePath, filesize);
	if(fileFd > 0)
	{
		close(fileFd);		/* 关闭文件描述符 */
	}
	if(pFileBuf != NULL)
	{
		free(pFileBuf);
		pFileBuf = NULL;
	}

	return;
	
}

/* 发送抑或接收文件 */
void	doSendAndRecvFile(char *pcMsgBuf, char *pName, int iSocket)
{
	MSG_HEAD_S 	*pstHead = (MSG_HEAD_S *)pcMsgBuf;
	/* 发送者是自己, 说明服务端发来的消息是有关接收者套接字信息的,即接下来应该执行发送文件的操作 */
	if(strcmp(pstHead->srcName, pName) == 0)
	{
		doSendFile(pcMsgBuf);
	}
	else	
	{
		doRecvFile(pcMsgBuf, iSocket);
	}
}	

#if 1
#else
int main()
{

	doCommandLs();
	int i = 0;
	char **resStr = (char **)malloc(sizeof(char *)*5);
	for(i=0; i<5; i++)
	{
		resStr[i] = (char *)malloc(20);
	}
	char str[20] = "put a.txt";
	mySplit(str, ' ', resStr);
	printf("%s\n",resStr[1]);

	MSG_DATA_S *pstMsgData = (MSG_DATA_S *)malloc(sizeof(MSG_DATA_S));
	SendFileRequest2Server(pstMsgData, "test.c", 2);
}
#endif
