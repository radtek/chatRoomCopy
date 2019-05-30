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
	//printf("aaa\n");
	pConnect = mysql_init(NULL);
	//printf("bbb\n");
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
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
}	

/* 判断发送者和接收者是否是好友关系 */
ulong   checkReciverIsFriend(MSG_DATA_S *pstData, char *pcDesName, int srcFd)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstData;
	MYSQL 		*pConnect = NULL;
	MYSQL_RES 	*pResult  = NULL;
	char tmpSql[128];
	char srcName[MAX_NAME_LEN + 1];
	char desName[MAX_NAME_LEN + 1];
	int iRowCount = 0;
	int res;
	pConnect = mysql_init(NULL);
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	strcpy(srcName, pstHead->srcName);
	strcpy(desName, pstHead->desName);
	snprintf(tmpSql, sizeof(tmpSql), "select * from friends where srcName = '%s' and desName = '%s'", srcName, desName);

	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		mysql_close(pConnect);
		handle_error("checkReciverIsFriend:mysql_query");
	}
	pResult = mysql_store_result(pConnect);
	if(pResult != NULL)
	{
		/* 记录返回的结果集中受影响的行数 */
 		iRowCount = mysql_num_rows((pResult));
		if(0 == iRowCount)
		{
			ulErrCode = ERROR_FAILED;
		}
		mysql_free_result(pResult);
	}

	mysql_close(pConnect);

	return ulErrCode;
}

/* 添加好友操作时,判断被添加的用户名是否存在 */
ulong 	checkUserIsExist(char *pcDesName)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	MYSQL 		*pConnect = NULL;
	MYSQL_RES 	*pResult  = NULL;
	char tmpSql[64];
	int iRowCount = 0;
	int res;
	pConnect = mysql_init(NULL);
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	snprintf(tmpSql, sizeof(tmpSql), "select * from users where username = '%s'", pcDesName);

	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		mysql_close(pConnect);
		handle_error("checkReciverIsFriend:mysql_query");
	}
	pResult = mysql_store_result(pConnect);
	if(pResult != NULL)
	{
		/* 记录返回的结果集中受影响的行数 */
 		iRowCount = mysql_num_rows((pResult));
		if(0 == iRowCount)
		{
			ulErrCode = ERROR_FAILED;
		}
		mysql_free_result(pResult);
	}

	mysql_close(pConnect);

	return ulErrCode;
}

/* 删除好友操作 */
void     deleteFriendFromMysql(MSG_DATA_S *pstData, char *pcDesName)
{
	assert(pstData   != NULL);
	assert(pcDesName != NULL);

	int res = 0;
	char msg[32];
	char deleteSql1[128];
	char deleteSql2[128];
	MSG_HEAD_S *pstHead = (MSG_HEAD_S *)pstData;
	MYSQL 	*pConnect = NULL;
	pConnect = mysql_init(NULL);
	if(pConnect == NULL)
	{
		perror("mysql_init");
		return;
	}
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	printf("删除操作:srcName = %s, desName =%s\n", pstHead->srcName, pcDesName);
	snprintf(deleteSql1, sizeof(deleteSql1), "delete from friends where srcName = '%s' and desName = '%s'", pstHead->srcName, pcDesName);
	snprintf(deleteSql2, sizeof(deleteSql2), "delete from friends where srcName = '%s' and desName = '%s'", pcDesName, pstHead->srcName);
	res = mysql_query(pConnect, deleteSql1); 
	res += mysql_query(pConnect, deleteSql2);
    if(res != 0)
	{
		perror("deleteFriendFromMysql:mysql_query");
	}

	mysql_close(pConnect);
}





