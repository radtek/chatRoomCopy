/************************************************************
 * 线程池实现函数
 *
 * 管理线程:		 
 * 		负责创建或销毁线程, 以达到实时响应客户端请求
 * 
 * 普通线程处理函数:  
 * 		处理任务队列中的job, 该任务可能是客户端的读事件,
 *  	也有可能是服务端这边的写事件, 
 * 
 * 每一个job对应的组成包括:
 *  	job对应的回调处理函数,
 *  	与客户端通信的套接字(回调函数的参数)
 *
 * *********************************************************/
#include "public.h"
#include "threadPool.h"
#include "server_proc.h"
#include "linklist.h"

/* static修饰的变量具有全局生命周期, 不过只能在本文件内被使用 */
static ThreadPools *pThreadPool = NULL;

extern Linklist *g_pList;

static SERVER_OPTION_S 		astProcClientOptFunc[] =
{
		{	MSG_TYPE_SHOWUSER,	procShowuserMsg },
		{	MSG_TYPE_SENDONE,	procMsg2One		},
		{	MSG_TYPE_SENDALL,	procMsg2All		},
		{	MSG_TYPE_SENDFILE,	sendFile2One	},
		{	MSG_TYPE_BANONE,	banOne 			},
		{	MSG_TYPE_LOGOUT,	sendLogoutMsg	}
};

/***********************************************
    Date         :   2019/4/24
	FuncName     : 	 procClientRequest
	Description  :   处理任务队列中的客户端的请求
    Input        :   void 	*arg   客户端对应的相应struct epoll_event结构体
	Output       :   none
    Return       :   void
************************************************/
void 	*procClientRequest(void 	*arg)
{
		if(arg == NULL)
		{
			perror("procClientRequest");
			return NULL;
		}
		printf("线程回调处理函数\n");
		char 	*pcMsgBuf = (char *)malloc(sizeof(MSG_DATA_S));
		if(NULL == pcMsgBuf)
		{
			perror("procClientRequest:calloc");
			return;
		}
		char 	*pcDesName = (char *)malloc(MAX_NAME_LEN + 1);
		if(NULL == pcDesName)
		{
			perror("procClientRequest:malloc");
			if(pcMsgBuf != NULL)
			{
				free(pcMsgBuf);
				pcMsgBuf = NULL;
			}
			return NULL;
		}
		int 	nread = -1;
		int 	i;
		SERVER_OPTION_S 	*pstClientOpt = astProcClientOptFunc;
		MSG_HEAD_S 	*pstMsgHead = NULL;
		MSG_TYPE_E 	enMsgType;
		char *pRecvMsg = (char *)malloc(MAX_WORD_LEN + 1);
		if(NULL == malloc)
		{
			perror("procClientRequest:malloc");
			return;
		}
		memset(pRecvMsg, 0, MAX_NAME_LEN + 1);
		char 	clientName[MAX_NAME_LEN + 1];
		//struct epoll_event 	*pFdEvent = (struct epoll_event *)arg;
		//struct myepoll_event *pMyEpollEvent;
	    //pMyEpollEvent = pFdEvent->data.ptr;
		//int cFd = pMyEpollEvent->fd;
		int cFd = (int)(long)arg;
  		/* 服务端读取客户端发来的报文 */	
		do
		{
			memset(pcMsgBuf, 0, sizeof(MSG_DATA_S));
			//printf("我阻塞在read这了, cFd = %d, ..\n");
			nread = read(cFd, pcMsgBuf, sizeof(MSG_DATA_S));
			memcpy(pRecvMsg, (char *)pcMsgBuf + sizeof(MSG_HEAD_S), MAX_WORD_LEN);
			printf("服务端读取客户端的数据\n");
			/* 如果客户端exit停止发送消息, 线程结束此次任务 */
			if(strcmp(pRecvMsg, "exit") == 0)
			{
				//printf("pRecvMsg = %s\n", pRecvMsg);
				//continue不能break, break则该套接字数据就无法被继续读取
				continue;
			}
			/* read failed */
			if(nread < 0)
			{
				handle_error("procClientRequest read");
			}
			else if(0 == nread)   /* 对端关闭连接 */
			{
				if(cFd > 0)
				{
					close(cFd);
					cFd = -1;
				}
				//epoll_ctl(g_iEpollFd, EPOLL_CTL_DEL, cFd, pFdEvent);
				delNode(g_pList, clientName);
				/* 该线程处理任务结束, 跳出该次循环 */
				printf("user : %s has loged out\n",clientName);
				break;
			}
			else
			{
				pstMsgHead = (MSG_HEAD_S *)pcMsgBuf;
				strcpy(clientName, pstMsgHead->srcName);
				enMsgType = pstMsgHead->enMsgType;
				strcpy(pcDesName, pstMsgHead->desName);
				//printf("the username as followings.....\n");
				//printf("%d %s %s\n", enMsgType, pstMsgHead->srcName, pstMsgHead->desName);
				/* 服务端根据客户端发送请求消息的不同, 调用相对应的回调处理函数 */
				pstClientOpt[enMsgType].pfClientMsgOption((MSG_DATA_S *)pcMsgBuf, pcDesName, cFd);
			}
		}while(1);
		if(pRecvMsg != NULL)
		{
			free(pRecvMsg);
			pRecvMsg = NULL;
		}
		if(pcMsgBuf != NULL)
		{
			free(pcMsgBuf);
			pcMsgBuf = NULL;
		}

		return;
}

/***********************************************
    Date         :   2019/4/24
	FuncName     : 	 threadFunc
	Description  :   线程工作函数
    Input        :   void *arg    线程工作函数参数
	Output       :   none
    Return       :   void
************************************************/
void 	*threadFunc(void 	*arg)
{
		/* 加锁操作需在while循环之前 */
		pthread_mutex_lock(&pThreadPool->mutex);

		int queue_num = 0;
		ThreadPools *pThreadPool = (ThreadPools *)arg;
		ThreadJob *pThreadJob;

		/* 判断队列是否为空, 如果是则等待任务队列非空条件 */
		while(pThreadPool->queue_num == 0 && !pThreadPool->shutdown)
			pthread_cond_wait(&pThreadPool->queue_not_empty, &pThreadPool->mutex);
		pThreadJob = pThreadPool->queue_head;						/* 取出头任务 */
		pThreadPool->queue_head = pThreadPool->queue_head->next;
		//pthread_mutex_unlock(&pThreadPool->mutex);
		//pthread_mutex_lock(&pThreadPool->queue_mutex);
		queue_num = pThreadPool->queue_num--;				/* 任务数减一,注意锁的粒度尽可能小 */
		printf("这是服务端的工作线程\n");
		pthread_mutex_unlock(&pThreadPool->queue_mutex);

		if(queue_num == MAX_QUEUE)
		{
			pthread_cond_signal(&pThreadPool->queue_not_full);
		}

		pThreadJob->pfThreadCallback(pThreadJob->arg);

		return;
}

/***********************************************
    Date         :   2019/4/24
	FuncName     : 	 threadFunc
	Description  :   管理线程工作函数
    Input        :   void *arg    线程工作函数参数
	Output       :   none
    Return       :   void
************************************************/
void 	*manageThreadFunc(void 	*arg)
{
	
}

/***********************************************
    Date        :    2019/4/24
	FuncName    : 	 createThreads
	Description :    创建线程池中的工作线程
	Input       :    startThreadNum   刚开始创建的线程个数
	Output      :    none
    Return      :    ERROR_SUCCESS  成功
					 ERROR_FAILED   失败
***********************************************/
ulong 	createThreads(ThreadPools *pThreadPool,  int startThreadNum)
{
		ulong 	ulErrCode = ERROR_SUCCESS;
		int i;
		pThreadPool->pThreads = (pthread_t *)calloc(startThreadNum + 1, sizeof(pthread_t));
		if(NULL == pThreadPool->pThreads)
		{
			perror("createThreads");
			return ERROR_FAILED;
		}
		for(i=0; i<startThreadNum; i++)
		{
			ulErrCode += pthread_create(&pThreadPool->pThreads[i], NULL, threadFunc, (void *)pThreadPool); 
		}	
		ulErrCode += pthread_create(&pThreadPool->pThreads[i], NULL, manageThreadFunc, (void *)pThreadPool);
		if(ulErrCode != ERROR_SUCCESS)
		{
			perror("createThreads");
		}

		return ulErrCode;
}

/***********************************************
    Date          :    2019/4/24
	FuncName      :    initThreadPools
	Description   :    初始化线程资源
    Input  		  :    maxThreadNum     最大线程数
					   startThreadNum   初始创建线程数
	Output        :    none
    Return        :    ERROR_SUCCESS    成功
					   ERROR_FAILED     失败
*************************************************/
ulong 	initThreadPools(int maxThreadNum, int minThreadNum, int startThreadNum)
{
		ulong 	ulErrCode = ERROR_SUCCESS;

		pThreadPool = (ThreadPools *)calloc(1, sizeof(ThreadPools));
		if(NULL == pThreadPool)
		{
			perror("initThreadPools");
			return ERROR_FAILED;
		}
		printf("initThreadPools exec\n");
		pThreadPool->queue_head = NULL;
		pThreadPool->queue_tail = NULL;
		pThreadPool->maxThreadNum = maxThreadNum;
		pThreadPool->minThreadNum = minThreadNum;
		pThreadPool->curThreadNum = startThreadNum;
		pThreadPool->busyThreadNum = 0;
		pThreadPool->queue_num = 0;
		pThreadPool->shutdown = false;
		
		ulErrCode += pthread_mutex_init(&pThreadPool->mutex, NULL);
		ulErrCode += pthread_cond_init(&pThreadPool->queue_not_empty, NULL); 
		ulErrCode += pthread_cond_init(&pThreadPool->queue_not_full, NULL);
		if(ulErrCode != ERROR_SUCCESS)
		{
			perror("pthread_cond_init");
			if(pThreadPool != NULL)
			{
				free(pThreadPool);
				pThreadPool = NULL;
			}
		}
		
		return createThreads(pThreadPool, startThreadNum);
}

/***********************************************
    Date         :   2019/4/24
	FuncName     : 	 addRequest2JobQueue
	Description  :   将服务端与客户端的通信套接字放入job队列
	Input        :   pThreadPools    线程池变量
					 pClientFd       通信套接字地址
	Output       :   none
    Return       :   ERROR_SUCCESS   成功
					 ERROR_FAILED    失败
************************************************/
ulong 	addRequest2JobQueue(THREAD_CALLBACK_PF 	pfThreadCallback, void 	*arg)
{
		pthread_mutex_lock(&pThreadPool->mutex);
		ulong 	ulErrCode = ERROR_FAILED;

		int queue_num = 0;
		ThreadJob 	*pJob = calloc(1, sizeof(ThreadJob));
		if(NULL == pJob)
		{
			perror("calloc");
			return ulErrCode;
		}
		/* 初始化线程对应的任务 */
		pJob->pfThreadCallback = pfThreadCallback;
		pJob->arg = arg;
		queue_num = pThreadPool->queue_num++;
		while(queue_num == MAX_QUEUE && !pThreadPool->shutdown)
			pthread_cond_wait(&pThreadPool->queue_not_full, &pThreadPool->mutex);
		
		/* 第一个任务, 则向工作线程发送队列非空的信号 */
		if(queue_num == 0)
		{
			pThreadPool->queue_head = pThreadPool->queue_tail = pJob;
			pthread_cond_signal(&pThreadPool->queue_not_empty);
		}
		else
		{
			pThreadPool->queue_tail->next = pJob;
			pThreadPool->queue_tail = pJob;
		}
		pThreadPool->queue_tail->next = NULL;

		pthread_mutex_unlock(&pThreadPool->mutex);

		return ERROR_SUCCESS;
}

/***********************************************
    Date         :   2019/5/1
	FuncName     : 	 destroyThreadPools
	Description  :   销毁线程池资源
	Input        :   pThreadPools    线程池变量
	Output       :   none
    Return       :   void
************************************************/
void 	destroyThreadPools(ThreadPools 	*pThreadPool)
{
		int i;
		int res;
		for(i=0; i<pThreadPool->curThreadNum; i++)
		{
			res = pthread_join(pThreadPool->pThreads[i], NULL);
			if(res)
			{
				goto failed;
			}
		}
		res = pthread_mutex_destroy(&pThreadPool->mutex);
		//res += pthread_cond_destory(&pThreadPool->queue_not_full);
		//res += pthread_cond_destory(&pThreadPool->queue_not_empty);
		if(res)
			goto failed;
		pThreadPool->queue_head = pThreadPool->queue_tail = NULL;
		if(pThreadPool != NULL)
		{
			/* 释放的是指针指向的内存资源, 指针自身所占的资源有进程结束后操作系统回收 */
			if(pThreadPool->pThreads != NULL)
			{
				free(pThreadPool->pThreads);
			}
			free(pThreadPool);
			pThreadPool = NULL;		
		}
	failed:
		handle_error("destroy Resource");
}

