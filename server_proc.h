#ifndef SERVER_PROC_H
#define SERVER_PROC_H

ulong	procShowAllMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong	procShowFriendMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong	procAddFriendMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong	procDelFriendMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	procSendMsg2One(MSG_DATA_S 	*pstData, char *pcDesName, int srcFd);

ulong 	procSendMsg2All(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	procSendFile2One(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	procBanOne(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

ulong 	procSendLogoutMsg(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

#endif






