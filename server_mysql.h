#ifndef SERVER_MYSQL_H
#define SERVER_MYSQL_H

//#include <mysql.h>
#include "/usr/include/mysql/mysql.h"

#define MYSQL_HOST "118.25.154.28"
#define MYSQL_USER  "root"
#define MYSQL_PWD   "du@h3c.COM"

extern 	void 	insertFd2Mysql(char *pUsername, int fd);

#endif
