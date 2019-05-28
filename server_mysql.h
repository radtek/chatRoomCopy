#ifndef SERVER_MYSQL_H
#define SERVER_MYSQL_H

//#include <mysql.h>
#include "/usr/include/mysql/mysql.h"

#define MYSQL_HOST "118.25.154.28"
#define MYSQL_USER  "root"
#define MYSQL_PWD   "du@h3c.COM"

extern 	void 	insertFd2Mysql(char *pUsername, int fd);

/* 将用户离线时收到的消息存入数据库 */
extern  void 	insertUserMsg2Mysql(MSG_DATA_S *pstData, char *pcDesName);

/* 判断发送者和接收者是否是好友关系 */
extern  ulong   checkReciverIsFriend(MSG_DATA_S *pstData, char *pcDesName, int srcFd);

/* 判断用户是否在系统中注册 */
extern  ulong 	checkUserIsExist(char *pcDesName);

#endif
