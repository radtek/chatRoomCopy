#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

char *getLocalTime(char *pRetTime, int len)
{
	time_t tmpTime;
	/* 获取当前系统时间 */
	time(&tmpTime);
	struct tm *pNowTime;
	/* 将获取的时间结构化为本地时间 */
	pNowTime = localtime(&tmpTime);
	snprintf(pRetTime, len, "%d:%d:%d", pNowTime->tm_hour, pNowTime->tm_min, pNowTime->tm_sec);
}

int main()
{
	char *pNowTime = (char *)malloc(32);
	getLocalTime(pNowTime, 32);
	printf("%s\n",pNowTime);
}
