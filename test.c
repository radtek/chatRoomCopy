#include <stdio.h>
#include <string.h>

int main()
{
	char desStr[10];
	char srcStr[5] = "abc";
	strncpy(desStr,srcStr, strlen(srcStr));
	desStr[strlen(srcStr)] = '\0';
	printf("%s\n",desStr);
}
