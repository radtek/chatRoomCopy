#include <stdio.h>
#include "linklist.c"
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define handle_error(msg) \
{perror(msg); exit(EXIT_FAILURE); }

Linklist *head=NULL;
typedef struct
{
	char srcname[64];
	char desname[64];
	char filename[64];
	int filelength;
	int action;
	char word[512];
	char filebuf[BUFSIZ];
}Message;

void sendMessageOne(Message *mes,int sfd)
{
	Linklist *node=(Linklist *)malloc(sizeof(Linklist));
	node=searchName(head,mes->desname);
	if(node == NULL)
	{
		strcpy(mes->srcname,"error");
		strncpy(mes->word,"receiver is not online",strlen("receiver is not online"));
		write(sfd,mes,sizeof(Message));
	}
//	printf("%s\n",node->name);
//	showList(head);
//	printf("meslen:%d\n",strlen(mes));
	else
		write(node->sfd,mes,sizeof(Message));
}

void sendMessageAll(Message *mes)
{
	Linklist *p=head->next;
	while(p != NULL)
	{
		if(strcmp(p->name,mes->srcname) != 0)
		{
			//printf("%s\n",p->name);
			write(p->sfd,mes,sizeof(Message));
		}
		p=p->next;
	}
}

void sendFile(Message *mes,int sfd)
{	
	Linklist *node=(Linklist *)malloc(sizeof(Linklist));
	node=searchName(head,"client1");
	printf("%s\n",mes->desname);
	//printf("%s\n",node->name);
	//showList(head);
	if(node == NULL)
	{
		printf("node==NULL\n");
		strcpy(mes->srcname,"error");
		strncpy(mes->filebuf,"receiver is not online",strlen("receiver is not online"));
		write(sfd,mes,sizeof(Message));
	}
	else
	{	
		printf("haha\n");
		printf("%s\n\n",mes->filebuf);
		write(node->sfd,mes,sizeof(Message));
	}
}

void banOne(Message *mes,int sfd)
{
	Linklist *node=(Linklist *)malloc(sizeof(Linklist));
	node=searchName(head,mes->desname);
	if(node == NULL)
	{
		strcpy(mes->srcname,"error");
		strncpy(mes->word,"receiver is not online",strlen("receiver is not online"));
		write(sfd,mes,sizeof(Message));	
	}
	else
		write(node->sfd,mes,sizeof(Message));
}

void *func(void *arg)
{
	Message rec;
	int cfd=(int)arg;
	while(1)
	{
		memset(&rec,0,sizeof(rec));
		int n=read(cfd,&rec,sizeof(rec));
		if(n == -1)
		{
			perror("read error");
			return;
		}
		else if(n == 0)
			return;
		else
		{
			//服务器根据客户端发送来的不同请求, 执行相对应的要求
			switch(rec.action)
			{
				case 1:
					sendMessageOne(&rec,cfd);
					//printf("%s\n",rec.word);
					//printf("%s\n",rec.desname);
					break;
				case 2:
					sendMessageAll(&rec);
					break;
				case 3:
					printf("%d\n",rec.action);
					printf("%s\n",rec.desname);
					sendFile(&rec,cfd);
					break;
				case 4:
					banOne(&rec,cfd);
					break;
			}
		}
	}
}

int main()
{
	head=(Linklist *)malloc(sizeof(Linklist));
	head=createList(head);
	pthread_t tid;
	int sfd, cfd, c_len, s_len;
	int opt=1;
	struct sockaddr_in s_addr, c_addr;
	s_addr.sin_family=AF_INET;
	s_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	s_addr.sin_port=6666;
	s_len=sizeof(s_addr);
	sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd == -1)
		handle_error("socket");
	setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	if(bind(sfd,(struct sockaddr *)&s_addr,s_len))
		handle_error("bind error");
	if(listen(sfd,128) == -1)
		handle_error("listen error");
	c_len=sizeof(c_addr);
	
	Message m;
	while(1)
	{
		memset(&m,0,sizeof(Message));
		//c_addr传出参数, c_len传入传出参数
		cfd=accept(sfd,(struct sockaddr *)&c_addr,&c_len);
		printf("server:%d\n",cfd);
		if(cfd == -1)
			handle_error("accept errro");
		read(cfd,&m,sizeof(Message));
		fprintf(stdout,"%s\n",m.word);
		insertNode(head,m.srcname,cfd);

		pthread_create(&tid,0,func,(void *)cfd);
		pthread_detach(tid);
	}
	//destoryList(head);
	return 0;
}



