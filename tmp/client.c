#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define handle_error(msg) \
{perror(msg); exit(EXIT_FAILURE); }

typedef struct
{
	char srcname[64];
	char desname[64];
	//char action[64];
	char filename[64];
	int filelength;
	int action;
	char word[512];
	char filebuf[BUFSIZ];
}Message;

void readFileFromServer(Message *mes)
{
	int fd;
	fd=open("clientnewFile.txt",O_CREAT|O_APPEND);
	if(fd == -1)
		handle_error("open");
	fprintf(stdout,"%s\n",mes->srcname,mes->filebuf);	
	write(fd,mes->filebuf,strlen(mes->filebuf));
}

void readBanFromServer(Message *mes)
{
	fprintf(stdout,"%s:%s\n",mes->srcname,mes->word);	
	return;
}

void *func(void *arg)
{
	int sfd=(int)(long)arg;
	Message mes;
	while(1)
	{
		memset(&mes,0,sizeof(Message));
		int n=read(sfd,&mes,sizeof(mes));
		if(n == -1)
		{
			handle_error("read error");
		}
		else if(n == 0)
		{
			return;
		}
		else
		{
			switch(mes.action)
			{
				case 1:
					//这个\n就这么重要?
					fprintf(stdout,"%s:%s\n",mes.srcname,mes.word);	
					break;
				case 2:
					fprintf(stdout,"%s:%s\n",mes.srcname,mes.word);	
					break;
				case 3:
					readFileFromServer(&mes);
					break;
				case 4:
					readBanFromServer(&mes);
			}
		}
	}
}

void chatOne(Message *mes,int sfd)
{
	system("clear");
	mes->action=1;
	printf("请输入你要聊天的对象名称:");
	scanf("%s",mes->desname);
	if(strcmp(mes->desname,"exit") == 0)
		return;
    while(1)
	{
		scanf("%s",mes->word);
		if(strcmp(mes->word,"exit") == 0)
			return;
		write(sfd,mes,sizeof(Message));
	}
}

void chatAll(Message *mes,int sfd)
{
	system("clear");
	mes->action=2;
    printf("请输入你要群发的消息(exit退出)\n");
	while(1)
	{
		scanf("%s",mes->word);
		if(strcmp(mes->word,"exit") == 0)
			return;
		write(sfd,mes,sizeof(Message));
	}
}

void sendFile(Message *mes,int sfd)
{
	system("clear");
	mes->action=3;
	int fd, res;
	struct stat file;
	printf("请输入接收者姓名:");
	scanf("%s",mes->desname);
	while(1)
	{
		printf("请输入你要发送的文件绝对路径名(exit退出)\n");
		scanf("%s",mes->filename);
		if(strcmp(mes->filename,"exit") == 0)
			return;
		fd=open(mes->filename,O_RDWR);
		if(fd == -1)
			handle_error("open file error");
		res=fstat(fd,&file);
		if(res == -1)
			handle_error("open file error");
		//printf("%d\n",file.st_size);
		mes->filelength=file.st_size;
		int total=0;
		char buf[BUFSIZ];
		do
		{
			int n=read(fd,buf,sizeof(buf));
			strncpy(mes->filebuf,buf,n);
			//printf("%s\n\n\n\n",mes->filebuf);
			write(sfd,&mes,sizeof(Message));
			total+=n;
		}while(total < mes->filelength);
	}
}
//client是管理员, 可以禁言某人
void banOne(Message *mes,int sfd)
{
	system("clear");
	mes->action=4;
	printf("请输入你要禁言的用户名(exit退出)\n");
	while(1)
	{
		scanf("%s",mes->desname);
		if(strcmp(mes->desname,"exit") == 0)
			return;
		write(sfd,mes,sizeof(Message));
	}
}

int main()
{
	//printf("%d\n",sizeof(Message));  704
	Message mes, rec;
	mes.action=0;
	strcpy(mes.srcname,"client");
	pthread_t tid;

	int sfd, s_len;
	struct sockaddr_in s_addr;
	s_addr.sin_family=AF_INET;
	s_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	s_addr.sin_port = htons(9999);
	s_len=sizeof(s_addr);
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd == -1)
		handle_error("socket");
	printf("clientFd : %d\n",sfd);
	if(connect(sfd,(struct sockaddr *)&s_addr,s_len) == -1)
		handle_error("connect error");
	strcpy(mes.word,"client connect success");
	write(sfd,&mes,sizeof(mes));

	pthread_create(&tid,0,func,(void *)(long)sfd);
	pthread_detach(tid);

	memset(&mes,0,sizeof(mes));
	strcpy(mes.srcname,"client");
	char buf[1024]={0};
	int x;
	do
	{
		printf("请输入你要进行的操作\n");
		printf("1.与某人闲聊\n");
		printf("2.和所有人闲聊\n");
		printf("3.给某人发送文件\n");
		printf("4.禁言某人\n");
		printf("5.下线\n");
		scanf("%d",&x);
		switch(x)
		{
			case 1:
				chatOne(&mes,sfd);
				break;
			case 2:
				chatAll(&mes,sfd);
				break;
			case 3:
				sendFile(&mes,sfd);
				break;
			case 4:
				banOne(&mes,sfd);
				break;
			case 5:
				return;
			default:
				printf("输入有误\n");
				break;
		}
	}while(x != 0);
	return 0;
}
