#ifndef MYSQL_H
#define MYSQL_H

//#include <mysql.h>
#include "/usr/include/mysql/mysql.h"

#define MYSQL_HOST "118.25.154.28"
#define MYSQL_USER  "root"
#define MYSQL_PWD   "du@h3c.COM"


/* 检查用户是否被禁言 */
extern int IsBan(char *pUsername);

/* 用户初始注册与登录函数 */
extern 	ulong 	checkUserInfo(char *pUsername);

/* 获取用户列表 */
extern 	void 	getUserList();

extern 	void 	insertFd2Mysql(char *pUsername, int fd);

#endif
