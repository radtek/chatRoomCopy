#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

int main()
{
	char name[24] = "a.png";
	if(strstr(name, "png") != NULL)
	{
		printf("yes\n");
	}
	else
	{
		printf("no\n");
	}

	return 0;
}
