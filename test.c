#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

void main01()
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

void main02()
{
	char str[5];
	char a[5] = "hehe";
	//snprintf对于超出sizeof(str)的部分会截断,返回值为用户想要写入的长度,而不是实际写入的长度
	//sprintf不会截断,所以snprintf更安全
	int n = snprintf(str, sizeof(str), "%s:hello,youqu", a);
	printf("n = %d\n", n);  //16
	printf("%s\n", str);    //hehe
}

void main03()
{
	
}

int main()
{
	main03();
}



