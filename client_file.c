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

/* *******发送文件***************
 * 参数：
 * 		发送的报文;
 * 		发送的文件路径;
 * 		和服务端通信的套接字;
 *******************************/
void doSendFile(MSG_DATA_S  *pstClientMsg, char *pcFilePath, int iSocket)
{
	assert(pstClientMsg != NULL);
	assert(pcFilePath != NULL);

	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstClientMsg;
	int filesize = 0;
	int nread = 0;
	struct stat fileinfo;
	char fileBuf[MAX_WORD_LEN + 1];
	FILE *fp = NULL;
	fp = fopen(pcFilePath, "rb+");
	if(NULL == fp)
	{
		perror("doSendFile:fopen");
		return;
	}
	if(-1 == stat(pcFilePath, &fileinfo))
	{
		perror("doSendFile:fstat");
		fclose(fp);
		return;
	}
	filesize = fileinfo.st_size;
	pstHead->filesize = filesize;
	//printf("SendFile start\n");
	while(!feof(fp))
	{
		memset(fileBuf, 0, sizeof(fileBuf));
		nread = fread(fileBuf, 1, sizeof(fileBuf) - 1, fp);
		//printf("doSendFile  nread = %d\n", nread);
		if(nread <= 0)			/* 读取文件失败 */
		{
			perror("doSendFile:fread");
			goto end;
		}
		else
		{
			//printf("%s",fileBuf);
			memset((char *)pstClientMsg + sizeof(MSG_HEAD_S), 0, sizeof(fileBuf));
			/* 封装成完整报文体之后发给服务器 */
			memcpy((char *)pstClientMsg + sizeof(MSG_HEAD_S), fileBuf, MAX_WORD_LEN);
			write(iSocket, pstClientMsg, sizeof(MSG_DATA_S));
		}
	}
	//printf("SendFile success\n");

end:
	if(fp != NULL)
	{
		fclose(fp);		// 关闭文件描述符 
	}

	return;
}

/* 接收文件 */
void doRecvFile(MSG_DATA_S  *pstClientMsg, char *pcName, int *pFilePacketNum)
{
	assert(pstClientMsg != NULL);
	assert(pcName != NULL);

	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstClientMsg;

	/* 这个判断流程只走一次，即第一个分片报文过来的时候 */
	if(*pFilePacketNum == 0)
	{
		char *pRecvMsg = (char *)malloc(MAX_WORD_LEN + 1);
		if(NULL == pRecvMsg)
		{
			perror("doRecvFile:malloc");
			return;
		}
		/* 报文原路返回，即接收者不在线 */
		if(strcmp(pstHead->srcName,pcName) == 0)
		{
			//memcpy(pRecvMsg, (char *)pstClientMsg + sizeof(MSG_HEAD_S), MAX_WORD_LEN);
			memcpy(pRecvMsg, pstClientMsg->pData, MAX_WORD_LEN);
			printf("%s\n", pRecvMsg);
			if(pRecvMsg != NULL)
			{
				free(pRecvMsg);
				pRecvMsg = NULL;
			}
			return;
		}
	}
	
	/* 别人发来的文件 */
	int filesize = pstHead->filesize;
	int nread = 0;
	char *pcFileBuf = (char *)malloc(MAX_WORD_LEN + 1);
	if(NULL == pcFileBuf)
	{
		perror("doRecvFile:malloc");
		return;
	}
	int fileFd = open("tmp.txt", O_CREAT | O_APPEND | O_RDWR, 0644);
	if(-1 == fileFd)
	{
		perror("doRecvFile:open");
		return;
	}
	//printf("待接收的文件大小 : %d\n", filesize);
	//while(filesize > 0)
	//{
		memset(pcFileBuf, 0, MAX_WORD_LEN + 1);
		/* 拷贝报文体中的数据到缓冲区 */
		memcpy(pcFileBuf, (char *)pstClientMsg + sizeof(MSG_HEAD_S), MAX_WORD_LEN);
		//printf("\n\n%s\n\n", pcFileBuf);
		/* 将缓冲区的内容写入文件 */
		int nwrite = write(fileFd, pcFileBuf, strlen(pcFileBuf));
		if(nwrite == -1)
		{
			perror("doRecvFile:write");
			return;
		}
		//printf("recv pFileBuf:%s\n", pFileBuf);
		//filesize -= strlen(pcFileBuf);
	//}
	//printf("file packet num:%d\n", *pFilePacketNum);
	if(filesize <= ((*pFilePacketNum + 1)*MAX_WORD_LEN))
	{
		printf("您收到一份文件,接收时间:\n");
		*pFilePacketNum = 0;
	}
	else
	{
		*pFilePacketNum++;
	}

end:
	if(fileFd > 0)
	{
		close(fileFd);		/* 关闭文件描述符 */
	}
	
	if(pcFileBuf != NULL)
	{
		free(pcFileBuf);
		pcFileBuf = NULL;
	}

	return;
}


#if 1
#else

/* 发送抑或接收文件 */
void	doSendAndRecvFile(char *pstClientMsg, char *pcName, int iSocket)
{
	MSG_HEAD_S 	*pstHead = (MSG_HEAD_S *)pstClientMsg;
	/* 发送者是自己, 说明服务端发来的消息是有关接收者套接字信息的,即接下来应该执行发送文件的操作 */
	if(strcmp(pstHead->srcName, pcName) == 0)
	{
		doSendFile(pstClientMsg);
	}
	else	
	{
		doRecvFile(pstClientMsg, iSocket);
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
