#include "public_client.h"
#include "circleList.h"

/* creat circle pList */
CircleList *createList()
{
	CircleList *pList = (CircleList *)malloc(sizeof(CircleList));
	if(NULL == pList)
	{
		return NULL;
	}
	memset(pList, 0, sizeof(CircleList));
	pList->count = 0;
	pList->head.next = NULL;
	pList->pTail = &(pList->head);

	return pList;
}

/* get the length of pList */
int getListLength(CircleList *pList)
{
	assert(pList != NULL);

	return pList->count;
}

/* insert node into circlepList */
CircleList *insertList(CircleList *pList, DTQ_Node *pInsertNode)
{
	if(NULL == pList || NULL == pInsertNode)
	{
		return NULL;
	}

	pInsertNode->next  = pList->pTail->next;
	pList->pTail->next = pInsertNode;
	pList->pTail 	   = pInsertNode;
	pList->count++;

	return pList;
}

/* get node from circleList by index */
DTQ_Node *getNodeByIndex(CircleList *pList, int pos)
{
	if(NULL == pList || pos <= 0)
	{
		return NULL;
	}

	int i = 0;
	DTQ_Node 	*pStartNode = &(pList->head);
	/* 链表节点最小下标为1 */
	while(i++ < pos && pStartNode != NULL)
	{
		pStartNode = pStartNode->next;
	}

	/* 找到则返回该节点, 找不到则返回NULL */
	return pStartNode;
}

/* delete node from circleList by index */
void  delNodeByIndex(CircleList *pList, int index)
{
	if(NULL == pList || index <= 0)
	{
		return;
	}

	int i = 0;
	DTQ_Node *pPreNode = &(pList->head);
	DTQ_Node *pCurNode = &(pList->head);
	while(pCurNode != NULL && i++ < index)
	{
		pPreNode = pCurNode;
		pCurNode = pCurNode->next;
	}

	/* 摘链, 释放节点 */
	pPreNode->next = pCurNode->next;
	/*
	 * if(pCurNode != NULL)
	{
		free(pCurNode);
		pCurNode = NULL;
	}*/
	pList->count--;
	
	return;
}

/* clear all node in circlepList */
void clearList(CircleList *pList)
{
	if(NULL == pList)
	{
		return;
	}
	
	DTQ_Node *pTmpNode = NULL;
	DTQ_Node *pStartNode = &(pList->head);
	while(pStartNode != NULL)
	{
		pTmpNode = pStartNode;
		pStartNode = pStartNode->next;
		if(pTmpNode != NULL)
		{
			free(pTmpNode);
			pTmpNode = NULL;
		}
	}
	pList->head.next = NULL;
	pList->pTail = &(pList->head);
	pList->count = 0;

	return;
}

/* destroy the circlepList */
void destroyList(CircleList *pList)
{
	if(pList != NULL)
	{
		free(pList);
		pList = NULL;
	}
}

#if 0
int main()
{
	int i;
	Stu s[3];
	char *name[3] = {"xiaoai", "xiaomei", "xiaoli"};
    for(i=0; i<3; i++)
	{
		strcpy(s[i].name, name[i]);
		s[i].sno = 101 + i;
	}	

	CircleList *pList = NULL;
	pList = createList();
	if(NULL == pList)
	{
		perror("createList");
		exit(1);
	}	
	for(i=0; i<3; i++)
	{
		pList = insertList(pList, (DTQ_Node *)&s[i]);
	}
	printf("count = %d\n", pList->count);
	for(i=1; i<=pList->count; i++)
	{
		Stu *s = (Stu *)getNodeByIndex(pList, i);
		if(NULL == s)
		{
			perror("getNodeByIndex");
			exit(EXIT_FAILURE);
		}
		printf("第%d个学生 姓名: %s, 学号:%d\n", i, s->name, s->sno);
	}

	delNodeByIndex(pList, 2);
	for(i=1; i<=pList->count; i++)
	{
		Stu *s = (Stu *)getNodeByIndex(pList, i);
		if(NULL == s)
		{
			perror("getNodeByIndex");
			exit(EXIT_FAILURE);
		}
		printf("第%d个学生 姓名: %s, 学号:%d\n", i, s->name, s->sno);
	}

	exit(0);
}
#endif




