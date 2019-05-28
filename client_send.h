#ifndef CLIENT_SEND_H
#define CLIENT_SEND_H

extern ulong sendShowAllMsg(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendShowFriendMsg(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendAddFriendMsg(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendDelFriendMsg(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendMsg2One(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendMsg2All(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendFile2One(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong banOne(MSG_DATA_S *pClientMsg, int iSocket);

extern ulong sendLogoutMsg(MSG_DATA_S *pClientMsg, int iSocket);

#endif
