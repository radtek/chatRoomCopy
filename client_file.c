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
#if 0
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
#endif

/* 发送文件 
 * 参数：
 * 发送的报文，发送的文件路径，和服务端通信的套接字*/
void doSendFile(char *pcMsgBuf, char *pFilePath, int iSocket)
{
	printf("doSendFile function\n");
	assert(pcMsgBuf != NULL);

	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pcMsgBuf;
	int filesize = 0;
	int nread = 0;
	struct stat buf;
	char *pFileBuf = (char *)malloc(MAX_WORD_LEN + 1);
	if(NULL == pFileBuf)
	{
		perror("doSendFile:malloc");
		return;
	}
	int fileFd = open(pFilePath, O_CREAT | O_RDONLY);
	if(-1 == fileFd)
	{
		perror("doSendFile:open");
		return;
	}
	if(-1 == fstat(fileFd, &buf))
	{
		perror("doSendFile:fstat");
		close(fileFd);
		return;
	}
	filesize = buf.st_size;
	pstHead->filesize = filesize;
	while(filesize > 0)
	{
		memset((char *)pcMsgBuf + sizeof(MSG_HEAD_S), 0, MAX_WORD_LEN);
		memset(pFileBuf, 0, MAX_WORD_LEN + 1);
		nread = read(fileFd, pFileBuf, MAX_WORD_LEN);
		printf("doSendFile  nread = %d\n", nread);
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
			/* 封装成完整报文体之后发给服务器 */
			memcpy((char *)pcMsgBuf + sizeof(MSG_HEAD_S), pFileBuf, MAX_WORD_LEN);
			write(iSocket, pcMsgBuf, sizeof(MSG_HEAD_S) + MAX_WORD_LEN);
			printf("send pFileBuf:%s\n", pFileBuf);
			filesize -= nread;
		}
	}
	printf("文件%s已成功发送\n", pFilePath);
	if(fileFd > 0)
	{
		close(fileFd);		/* 关闭文件描述符 */
	}
	if(pFileBuf != NULL)	/* 释放内存缓冲区 */
	{
		free(pFileBuf);
		pFileBuf = NULL;
	}

	return;
}

/* 接收文件 */
void doRecvFile(char *pcMsgBuf)
{
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pcMsgBuf;
	assert(pstHead != NULL);

	int filesize = pstHead->filesize;
	int nread = 0;
	char *pFileBuf = (char *)malloc(MAX_WORD_LEN + 1);
	if(NULL == pFileBuf)
	{
		perror("doRecvFile:malloc");
		return;
	}
	int fileFd = open("tmp.txt", O_CREAT | O_RDWR, 0644);
	if(-1 == fileFd)
	{
		perror("doRecvFile:open");
		return;
	}
	printf("待接收的文件大小 : %d\n", filesize);
	while(filesize > 0)
	{
		memset(pFileBuf, 0, MAX_WORD_LEN + 1);
		/* 拷贝报文体中的数据到缓冲区 */
		memcpy(pFileBuf, (char *)pcMsgBuf + sizeof(MSG_HEAD_S), MAX_WORD_LEN);
		printf("doRecvFile start read.");
		/* 将缓冲区的内容写入文件 */
		int nwrite = write(fileFd, pFileBuf, strlen(pFileBuf));
		if(nwrite == -1)
		{
			perror("doRecvFile:write");
			return;
		}
		else
		{
			printf("doRecvFile nwrite = %d\n", nwrite);
		}
		printf("recv pFileBuf:%s\n", pFileBuf);
		filesize -= strlen(pFileBuf);
	}
	printf("成功接收一份文件\n");
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


#if 1
#else

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
