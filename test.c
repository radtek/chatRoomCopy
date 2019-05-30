#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

int main()
{
	char ch;
	/*
	char *pNowTime = (char *)malloc(32);
	getLocalTime(pNowTime, 32);
	printf("%s\n",pNowTime);
	printf("this is a test case\n");
	printf("hello world\n");
	*/
	printf("y/n ?:");
	ch = getchar();
	printf("ch = %c\n",ch);
	if(ch == 'y')
		printf("yes\n");
	else
		printf("no\n");
}


