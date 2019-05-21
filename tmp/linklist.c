#include "public.h"
#include "linklist.h"

/* 声明为全局变量, 其他文件如需引用加上extern引用声明即可 */
typedef struct node
{
	char name[64];
	int sfd;
	struct node *next;
}Linklist;

//Linklist *g_pList = NULL;

/* 创建链表 */
Linklist *createList(Linklist *head)
{
	head = (Linklist *)calloc(1, sizeof(Linklist));
	if(NULL == head)
	{
		return NULL;
	}
	head->next=NULL;
	return head;
}

/* 头插法, 插入新节点 */
void  insertNode(Linklist *head,char *name,int sfd)
{
	Linklist *newNode=(Linklist *)malloc(sizeof(Linklist));
	strcpy(newNode->name,name);
	newNode->sfd=sfd;

	newNode->next=head->next;
	head->next=newNode;
	printf("a user add to global list\n");

	return;
}

/* 根据用户名删除相应的客户节点 */
void  delNode(Linklist *head,char *name)
{
	Linklist *pre=head, *p=head->next;
	while(p != NULL)
	{
		if(strcmp(p->name,name) == 0)
		{
			break;
		}
		else
		{
			pre=p;
			p=p->next;
		}
	}
	/* 没有该节点, 直接返回 */
	if(p == NULL)
		return;
	pre->next=p->next;
	free(p);

	return;
}

/* 销毁链表 */
void destoryList(Linklist *head)
{
	Linklist *p=head->next;
	while(p != NULL)
	{
		head->next=p->next;
		free(p);
		p=head->next;
	}
	free(head);
	return;
}

/* 查询链表中的用户信息 */
Linklist *searchName(Linklist *head,char *name)
{
	Linklist *p=head->next;
	while(p != NULL)
	{
		if(strcmp(p->name,name) == 0)
		{
			printf("searchName : found the dest name\n");
			return p;
		}
		p=p->next;
	}
	return NULL;
}

/* 显示当前所有在线用户 */
void showList(Linklist *head)
{
	Linklist *p=head->next;
	while(p != NULL)
	{
		printf("%s     online\n",p->name);
		p=p->next;
	}
}





