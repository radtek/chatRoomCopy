#ifndef LINKLIST_H
#define LINKLIST_H

typedef struct node
{
	char name[64];
	int sfd;
	struct node *next;
}Linklist;

/* 创建链表 */
extern Linklist *createList(Linklist *head);

/* 头插法, 插入新节点 */
extern void  insertNode(Linklist *head,char *name,int sfd);

/* 根据用户名删除相应的客户节点 */
extern void  delNode(Linklist *head,char *name);

/* 销毁链表 */
extern void destoryList(Linklist *head);

/* 查询链表中的用户信息 */
extern Linklist *searchName(Linklist *head,char *name);

/* 显示当前所有在线用户 */
extern void showList(Linklist *head);

#endif



