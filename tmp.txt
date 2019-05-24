#include <stdio.h>
#include <stdlib.h>

int main()
{
	char command[64];
	gets(command);
	if(strcmp(command, "exit") == 0)
	{
		goto end;
	}
	printf("command = %s\n", command);

end:
	printf("hehe\n");
	return 0;
}
