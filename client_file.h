#ifndef CLIENT_FILE_H
#define CLIENT_FILE_H

/* 分割解析用户输入的命令 */
extern int mySplit(char *str, char delim, char **retStr);

extern void doCommandLs();

/* 向服务端发送请求, 获取接受者的信息 */
extern void SendFileRequest2Server(MSG_DATA_S *pstMsgData, char *filePath, int fd);

/* 发送或接收文件的相关函数 */
extern void doSendAndRecvFile(char *pcMsgBuf, char *pUsername, int iSocket);

extern void doSendFile(char *pcMsgBuf, char *pFilePath, int iSocket);

extern void doRecvFile(char *pcMsgBuf);

#endif
