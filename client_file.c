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
	strcpy(retStr[i], cur);

	return;
}

void doCommandLs()
{
	system("ls");	
}

/* 发送请求至服务端, 获取接收者的信息 */
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
	int fileFd = open(filePath, O_CREAT | O_RDONLY);
	if(-1 == fileFd)
	{
		perror("open file failed");
		return;
	}
	if(-1 == fstat(fileFd, &buf))
	{
		perror("fstat");
		return;
	}
	fsize = buf.st_size;
	//printf("file size = %d\n",fsize);
	snprintf(pFileInfo, 32, "%s##%d",filePath, fsize);
	memcpy(pstMsgData + sizeof(MSG_HEAD_S), pFileInfo, strlen(pFileInfo));
	//printf("res = %d\n",res);
	//printf("文件信息 : %s\n", pFileInfo);
	write(fd, pstMsgData, sizeof(MSG_HEAD_S) + MSG_WORD_LEN);
}

/* 发送文件 */
void doSendFile(char *pcMsgBuf)
{
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pcMsgBuf;
	assert(pstHead != NULL);
	printf("接收者fd = %d\n",pstHead->desFd);
}

/* 接收文件 */
void doRecvFile(char *pcMsgBuf)
{

}

/* 发送抑或接收文件 */
void	doSendAndRecvFile(char *pcMsgBuf, char *pName)
{
	MSG_HEAD_S 	*pstHead = (MSG_HEAD_S *)pcMsgBuf;
	/* 发送者是自己, 说明服务端发来的消息是有关接收者套接字信息的,即接下来应该执行发送文件的操作 */
	if(strcmp(pstHead->srcName, pName) == 0)
	{
		doSendFile(pcMsgBuf);
	}
	else	
	{
		doRecvFile(pcMsgBuf);
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
