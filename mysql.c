#include "public_client.h"
#include "mysql.h"

/* 判断用户是否被禁言 */
int IsBan(char *pUsername)
{
	MYSQL 		*pConnect = NULL;
	MYSQL_RES 	*pResult = NULL;		//用于接收sql语句返回的结果
	MYSQL_ROW 	*pSqlRow = NULL;		//指向返回结果对应的行		
	char tmpSql[64];
	int flag;
	int res;
	char isBan;
	pConnect = mysql_init(NULL);
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	snprintf(tmpSql, sizeof(tmpSql), "select username,ban from users where username = '%s'", pUsername);

	res = mysql_query(pConnect, tmpSql);
	if(res != 0)
	{
		handle_error("IsBan:mysql_query");
	}
	else
	{
		pResult = mysql_store_result(pConnect);
		if(pResult != NULL)
		{
			/* 逐行取出数据 */
			while((pSqlRow = (MYSQL_ROW *)mysql_fetch_row(pResult)))
			{
				/* 用户名和密码均相等则登录成功 */
				if(strcmp(pUsername, (char *)pSqlRow[0]) == 0)	
				{	
					isBan = *(char *)pSqlRow[1];
					if(atoi(&isBan))
					{
						flag = 1;
					}
					else
					{
						flag = 0;
					}
					break;
				}
			}	
			/* 记得释放结果指针指向的资源 */
			mysql_free_result(pResult);
		}
	}
	mysql_close(pConnect);
	
	return flag;
}

/* 获取用户的输入,并使其不回显到标准输出 */
int getmychar()
{   
	struct termios oldt, newt;
	int ch;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt; 
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
} 

void getPassword(char *pwd)
{
	int i;
	for(i=0; i<20; i++)
	{
		char ch = getmychar();
		if(ch == '\r' || ch == '\n')
			break;
		pwd[i] = ch;
		printf("*");
	}
	pwd[i] = '\0';			//防止乱码

	return;
}

/* 登录功能函数 */
ulong 	doLogin(MYSQL 	*pConnect, char *pSrcName)
{
	ulong 	ulErrCode = ERROR_SUCCESS;
	
	MYSQL_RES 	*pResult = NULL;		//用于接收sql语句返回的结果
	MYSQL_ROW 	*pSqlRow = NULL;		//指向返回结果对应的行		
	MYSQL_FIELD *pField  = NULL;		//指向行中的每一列
   	int colNum = 0;
	int i;
	int res;
	char isBan;
	int flag = 0;
	char username[MAX_NAME_LEN];
	char pwd[MAX_NAME_LEN];
	printf("username:");
	scanf("%s",username);
	getchar();					//用于接收回车符

	printf("password:");
	getPassword(pwd);
	//printf("\nusername : %s\n",username);
	//printf("password : %s\n", pwd);
	res = mysql_query(pConnect, "select username,password from users");
	if(res != 0)
	{
		perror("mysql_query");
		ulErrCode = ERROR_FAILED;
	}
	else
	{
		//printf("yes\n");
		pResult = mysql_store_result(pConnect);
		if(pResult != NULL)
		{
			//colNum = mysql_num_fields(pResult);
			/* 逐行取出数据 */
			while((pSqlRow = (MYSQL_ROW *)mysql_fetch_row(pResult)))
			{
				/* 用户名和密码均相等则登录成功 */
				if((strcmp(username, (char *)pSqlRow[0]) == 0) && 
						(strcmp(pwd, (char *)pSqlRow[1]) == 0))
				{	
					printf("\nlogin success.\n");
					strncpy(pSrcName, username, strlen(username));
					pSrcName[strlen(username)] = '\0';
					flag = 1;
					break;
				}
			}	
			if(0 == flag)
			{
				printf("\nusername or password invalid.\n");
				ulErrCode = ERROR_FAILED;
			}
			/* 记得释放结果指针指向的资源 */
			mysql_free_result(pResult);
		}
	}
	
	return ulErrCode;
}

/* 注册功能函数 */
ulong 	doReg(MYSQL 	*pConnect, char *pSrcName)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	char username[32];
	char pwd[32];
	char repwd[32];
	int  res;
	int iBan = 0;
	char sqlStr[512];
	do
	{
		memset(sqlStr, 0, sizeof(sqlStr));
		memset(username, 0, sizeof(username));
		memset(pwd, 0, sizeof(pwd));
		memset(repwd, 0, sizeof(repwd));
		printf("请输入注册用户名:");
		scanf("%s", username);
		getchar();					//接收回车符
		printf("请输入注册密码(长度>=6):");
		getPassword(pwd);
		printf("\n请确认注册密码:");
		getPassword(repwd);
		/* 密码长度小于6位, 或者两次输入的密码不一致 */
		if((strlen(pwd) < 6) || (strcmp(pwd,repwd) != 0))
		{
			printf("\n用户名或密码不符合要求, 请确认输入.\n");
			continue;
		}
		else
		{
			break;
		}
	}while(1);
	//printf("\n%s %s\n",username,pwd);
	/* 先把sql语句给格式化, 然后再去执行相应操作 */
	snprintf(sqlStr, sizeof(sqlStr), "insert into users(username,password,ban) values('%s','%s','%d')", username, pwd,iBan);
	res = mysql_query(pConnect, sqlStr); 
	if(res != 0)
	{
		printf("\n注册失败, 用户名已存在\n");
		fprintf(stderr, "mysql_errno: %d\n", mysql_errno(pConnect));
		ulErrCode = ERROR_FAILED;
	}
	else
	{
		printf("注册成功,正在跳转至登录界面..\n");
		ulErrCode = doLogin(pConnect, pSrcName);
	}

	return ulErrCode;
}

/* 检查用户的信息是否合法 */
ulong 	checkUserInfo(char 	*pSrcName)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	//char ch;
	char x;
	MYSQL *pConnect;
	pConnect = mysql_init(NULL);
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(pConnect == NULL)
	{
		handle_error("mysql_connect_error");
	}

	do	
	{
		printf("/*********************/\n");
		printf("****1. 注册  2.登录****\n");
		printf("*********0.退出********\n");
		/*  会导致段错误
		scanf("%s", &ch);
		if((!(ch >= '0' && ch <= '9')) || strlen(&ch) > 1)
		{
			printf("输入不合法, 请重新输入\n");
			continue;
		}
		x = atoi(&ch);
		*/
		scanf("%c",&x);
		/* 输入的字符非数字字符, 则直接退出 */
		if(!isdigit(x))
		{
			perror("输入不合法,已退出");
			exit(1);
		}
		if(x == '0')
		{
			ulErrCode = ERROR_FAILED;
			break;
		}
		if('1' == x)
		{
			ulErrCode = doReg(pConnect, pSrcName);
			break;
		}
		else if('2' == x)
		{
			ulErrCode = doLogin(pConnect, pSrcName);
			break;
		}
		else
		{
			printf("无此选项, 请重新输入\n");
		}
	}while(x != '0');

	/* 关闭打开的数据库资源, 防止内存泄漏 */
	mysql_close(pConnect);
	return ulErrCode;
}

/* 获取所有用户列表 */
void 	getUserList()
{
	int res = 0;
	char msg[32];
	MYSQL 	*pConnect = NULL;
	MYSQL_RES *pResult = NULL;
	MYSQL_ROW 	*pRow = NULL;
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
	res = mysql_query(pConnect, "select username,password from users");
    if(res != 0)
	{
		perror("getUserList:mysql_query");
	}
	else
	{
		//printf("yes\n");
		pResult = mysql_store_result(pConnect);
		if(pResult != NULL)
		{
			printf("所有用户列表(exit退出):\n");
			while((pRow = (MYSQL_ROW *)mysql_fetch_row(pResult)))
			{
				snprintf(msg, sizeof(msg), "用户:%s\t\t", pRow[0]);
				printf("%s\n",msg);
			}
			mysql_free_result(pResult);
		}
	}
	mysql_close(pConnect);
	do
	{
		scanf("%s",msg);
	}while(strcmp(msg,"exit") != 0);

	return;
}

/* 获取当前用户在离线时收到的消息 */
void getUserMsgFromMysql(char *pSrcName)
{
	ulong 	ulErrCode = ERROR_SUCCESS;

	int res = 0;
	char msg[64];
	char tmpSql[128];
	MYSQL 	*pConnect = NULL;
	MYSQL_RES *pResult = NULL;
	MYSQL_ROW 	*pRow = NULL;
	pConnect = mysql_init(NULL);
	if(pConnect == NULL)
	{
		perror("getUserMsgFromMysql:mysql_init");
		return;
	}
	pConnect = mysql_real_connect(pConnect, MYSQL_HOST, MYSQL_USER, MYSQL_PWD, "mysql", 0, NULL, 0);
	if(NULL == pConnect)
	{
		handle_error("mysql_real_connect");
	}
	/* 获取用户收到的离线消息并展示给用户 */
	snprintf(tmpSql, sizeof(tmpSql), "select srcName, words, time  from message where desName ='%s'", pSrcName);
	res = mysql_query(pConnect, tmpSql); 
    if(res != 0)
	{
		perror("getUserMsgFromMysql:mysql_query");
		mysql_close(pConnect);
	}
	else
	{
		//printf("yes\n");
		pResult = mysql_store_result(pConnect);
		if(pResult != NULL)
		{
			printf("您收到的离线消息:\n");
			while((pRow = (MYSQL_ROW *)mysql_fetch_row(pResult)))
			{
				bzero(msg, sizeof(msg));
				snprintf(msg, sizeof(msg), "%s:%s\t%s", pRow[0],pRow[1],pRow[2]);
				printf("%s\n",msg);
			}
			mysql_free_result(pResult);
		}
	}
	/* 删除离线消息记录 */
	bzero(tmpSql, sizeof(tmpSql));
	snprintf(tmpSql, sizeof(tmpSql), "delete from message where desName ='%s'", pSrcName);
	res = mysql_query(pConnect, tmpSql); 
    if(res != 0)
	{
		perror("getUserMsgFromMysql:mysql_query");
		mysql_close(pConnect);
	}

	mysql_close(pConnect);
}

/* 获取好友列表 */
void 	getFriendList(char *pSrcName)
{
	int res = 0;
	char msg[32];
	char tmpSql[64];
	MYSQL 	*pConnect = NULL;
	MYSQL_RES *pResult = NULL;
	MYSQL_ROW 	*pRow = NULL;
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
	snprintf(tmpSql, sizeof(tmpSql), "select desName from friends where srcName = '%s'", pSrcName);
	res = mysql_query(pConnect, tmpSql); 
    if(res != 0)
	{
		perror("getFriendList:mysql_query");
	}
	else
	{
		//printf("yes\n");
		pResult = mysql_store_result(pConnect);
		if(pResult != NULL)
		{
			printf("好友列表(exit退出):\n");
			/* 遍历返回的所有行 */
			while((pRow = (MYSQL_ROW *)mysql_fetch_row(pResult)))
			{
				snprintf(msg, sizeof(msg), "用户:%s\t\t", pRow[0]);
				printf("%s\n",msg);
			}
			mysql_free_result(pResult);
		}
	}
	mysql_close(pConnect);
	do
	{
		scanf("%s",msg);
	}while(strcmp(msg,"exit") != 0);

	return;
}

/* 添加好友操作 */
void insertFriend2Mysql(char *pSendName, char *pRecvName)
{
	//printf("pSendName = %s, pRecvName = %s\n",pSendName, pRecvName);
	int res = 0;
	char msg[32];
	char insertSql1[64];
	char insertSql2[64];
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
	snprintf(insertSql1, sizeof(insertSql1), "insert into friends(srcName,desName) values('%s','%s')", pSendName, pRecvName);
	snprintf(insertSql2, sizeof(insertSql2), "insert into friends(srcName,desName) values('%s','%s')", pRecvName, pSendName);
	res = mysql_query(pConnect, insertSql1); 
	res += mysql_query(pConnect, insertSql2);
    if(res != 0)
	{
		perror("insertFriend2Mysql:mysql_query");
	}

	mysql_close(pConnect);

	return;
}


#if 0
int main()
{
	return checkUserInfo();
}

#endif
