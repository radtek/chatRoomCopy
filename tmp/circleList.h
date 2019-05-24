#pragma once
//#include "public.h"

typedef struct list 
{
	struct list *next;
}DTQ_Node;

/* 链表结构体 */
typedef struct
{
	DTQ_Node head;			//链表头结点
	DTQ_Node *pTail;		//链表尾指针
	int count;				//链表节点数量
}CircleList;
/* 创建链表 */
extern CircleList *createList();
/* 获取链表长度 */
extern int getListLength(CircleList *list);
/* 向链表中插入节点 */
extern CircleList *insertList(CircleList *list, DTQ_Node *pNode);
/* 根据关键字删除指定节点 */
extern void  delNodeByIndex(CircleList *list, int index);
/* 根据关键字获取指定节点信息 */
extern DTQ_Node *getNodeByIndex(CircleList *list, int index);
/* 清空链表中存储的节点 */
extern void clearList(CircleList *list); 
/* 销毁链表 */
extern void destroyList(CircleList *list);







