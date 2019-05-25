#include "public.h"
#include "server_mysql.h"

/* 将用户名和用户对应套接字加入usersfd表 */
void insertFd2Mysql(char *pUsername, int fd)
{
	MYSQL 		*pConnect = NULL;
	char tmpSql[128];
	int res;
	printf("aaa\n");
	pConnect = mysql_init(NULL);
	printf("bbb\n");
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	printf("ccc\n");
	snprintf(tmpSql, sizeof(tmpSql), "insert into usersfd values('%s','%d') on duplicate key update userfd='%d'", pUsername, fd, fd);

	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		perror("insertFd2Mysql:mysql_query");
		mysql_close(pConnect);
		return;
	}

	mysql_close(pConnect);
	printf("ddd\n");
}

/* 获取当前的系统时间 */
static char *getMsgTime(char *pRetTime, int maxTimeLen)
{
	time_t tmpTime;
	/* 获取当前系统时间 */
	time(&tmpTime);
	struct tm *pNowTime;
	/* 将获取的时间结构化为本地时间 */
	pNowTime = localtime(&tmpTime);
	snprintf(pRetTime, maxTimeLen, "%d:%d:%d", pNowTime->tm_hour, pNowTime->tm_min, pNowTime->tm_sec);
}

/* 将用户离线时收到的消息暂存在数据库中 */
void insertUserMsg2Mysql(MSG_DATA_S *pstData, char *pcDesName)
{
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstData;
	MYSQL 		*pConnect = NULL;
	char tmpSql[128];
	char msg[256];
	char msgTime[32];
	char srcName[MAX_NAME_LEN + 1];
	int res;
	printf("aaa\n");
	pConnect = mysql_init(NULL);
	printf("bbb\n");
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	printf("ccc\n");
	strcpy(srcName, pstHead->srcName);
	memcpy(msg, pstData->pData, MAX_WORD_LEN);
	getMsgTime(msgTime, sizeof(msgTime));
	snprintf(tmpSql, sizeof(tmpSql), "insert into message(srcName,desName,words,time) values('%s','%s','%s','%s')", srcName, pcDesName, msg, msgTime);

	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		perror("insertFd2Mysql:mysql_query");
		mysql_close(pConnect);
		return;
	}

	mysql_close(pConnect);
	printf("ddd\n");
}	
