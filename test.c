#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main()
{
	char buf[20];
	read(STDIN_FILENO, buf, sizeof(buf)-1);
	printf("%s",buf);
}
