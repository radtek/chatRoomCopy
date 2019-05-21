#ifndef SERVER_PROC_H
#define SERVER_PROC_H

ulong	procShowuserMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	procMsg2One(MSG_DATA_S 	*pstData, char *pcDesName, int srcFd);

ulong 	procMsg2All(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	sendFile2One(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	banOne(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	sendLogoutMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

#endif






