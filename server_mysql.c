#include "public.h"
//#include "server_mysql.h"

/* 将用户名和用户对应套接字加入usersfd表 */
void insertFd2Mysql(char *pUsername, int fd)
{
	MYSQL 		*pConnect = NULL;
	char tmpSql[128];
	int res;
	pConnect = mysql_init(NULL);
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	snprintf(tmpSql, sizeof(tmpSql), "insert into usersfd values('%s','%d') on duplicate key update userfd='%d'", pUsername, fd, fd);

	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		perror("insertFd2Mysql:mysql_query");
		return;
	}

	mysql_close(pConnect);
}


