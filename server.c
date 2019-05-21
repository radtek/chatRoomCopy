#include "public.h"
#include "threadPool.h"

#define MAX_EPOLL_EVENT 100

/* 通信套接字的回调函数 */
ulong 	addRequest2ThreadPool(void 	*arg)
{
	assert(arg != NULL);
	ulong 	ulErrCode = ERROR_FAILED;
	
	/* 将通信套接字加入线程池的任务队列中 */	
	ulErrCode = addRequest2JobQueue(pThreadPool, procClientRequest, (struct epoll_event *)arg);

	return ulErrCode;
}

/* 将套接字加入到epoll中 */
ulong addFd2Epoll(int iEpollFd, int iListenFd, EPOLL_CALLBACK_PF pfEpollCallback)
{
	ulong ulErrCode = ERROR_SUCCESS;
	struct epoll_event evt, events[MAX_EPOLL_EVENT];
	struct myepoll_event tmpEvent;

	tmpEvent.fd = iListenFd;
	tmpEvent.pfEpollCallback = pfEpollCallback;
	evt.events = EPOLLIN | EPOLLERR | EPOLLHUP;
	evt.data.ptr = &tmpEvent;

	ulErrCode = epoll_ctl(iEpollFd, EPOLL_CTL_ADD, iListenFd, &evt);
	
	return ulErrCode;
}

/* 监听套接字对应的回调函数 */
ulong acceptClientConnect(void 	*arg)
{
	ulong ulErrCode = ERROR_SUCCESS;

	struct epoll_event *pFdEvent = (struct epoll_event *)arg;
	struct myepoll_event *pMyEpollEvent;
	int listenFd = INVALID_FD;
	int cFd = INVALID_FD;
	pMyEpollEvent = pFdEvent->data.ptr;
	struct sockaddr_in cAddr;
	memset(&cAddr, 0, sizeof(cAddr));
	int cLen = sizeof(cAddr);
	listenFd = pMyEpollEvent->fd;

	if(pFdEvent->events & EPOLLHUP)
	{
		//epoll_ctl();
	}
	else
	{
		/* 与客户端通信的套接字 */
		cFd = accept(listenFd, (struct sockaddr *)&cAddr, &cLen);
		/* 套接字>=0,都是正常的 */
		if(cFd >= 0)
		{
			ulErrCode = addFd2Epoll(g_iEpollFd, cFd, addRequest2ThreadPool);
		}
		else
		{
			perror("acceptClientConnect");
			ulErrCode = ERROR_FAILED;
		}
	}

	return ulErrCode;
}

/* 处理epoll返回的事件 */
ulong procEpollResponse(struct epoll_event *pEvents, int iEventNum)
{
	ulong	ulErrCode = ERROR_SUCCESS; 
	int 	iLoop = -1;
	EPOLL_CALLBACK_PF 	pfEpollCallback = NULL;
	struct epoll_event 	fdEvent;
	struct myepoll_event *pMyEpollEvent;

	for(iLoop=0; iLoop<iEventNum; iLoop++)
	{
		fdEvent = pEvents[iLoop];
		pMyEpollEvent = fdEvent.data.ptr;
		pfEpollCallback = pMyEpollEvent->pfEpollCallback;
		ulErrCode += pfEpollCallback((void *)&fdEvent);	
		if(ulErrCode != ERROR_SUCCESS)
		{
			perror("procEpollResponse");
			return ulErrCode;
		}
	}

	return ulErrCode;
}

/* 创建epoll */
ulong createEpoll(int iListenFd)
{
	ulong ulErrCode = ERROR_SUCCESS;
	g_iEpollFd = epoll_create(MAX_EPOLL_EVENT);
	if(INVALID_FD == g_iEpollFd)
	{
		ulErrCode = ERROR_FAILED;
	}
	if(ERROR_SUCCESS == ulErrCode)
	{
		addFd2Epoll(g_iEpollFd, iListenFd, acceptClientConnect);
	}

	return ulErrCode;
}

/* 创建监听套接字  */
ulong createListenSocket(int *piSockFd)
{
	ulong ulErrCode = ERROR_SUCCESS;

	struct sockaddr_in sAddr,cAddr;
	int iServerLen = -1;
	int opt = 1;
	int iServerFd = INVALID_FD;
	memset(&sAddr, 0, sizeof(sAddr));
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	sAddr.sin_port = 8888;
	iServerFd = socket(AF_INET, SOCK_STREAM, 0);
	if(iServerFd < 0)
	{
		ulErrCode = ERROR_FAILED;
	}
	//端口复用,opt值为非0时,代表生效
	if(ERROR_SUCCESS == ulErrCode)
	{
	    ulErrCode =	setsockopt(iServerFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	}
	if(ERROR_SUCCESS == ulErrCode)
	{
		ulErrCode = bind(iServerFd, (struct sockaddr *)&sAddr, sizeof(sAddr));
	}
	if(ERROR_SUCCESS == ulErrCode)
	{
		ulErrCode = listen(iServerFd, 128);
	}	
	*piSockFd = iServerFd;

	return ulErrCode;
}

/* server_daemon主函数 */
int main()
{
	ulong ulErrCode = ERROR_FAILED;
	int listenFd = INVALID_FD;
	int nready = -1;
	struct epoll_event events[MAX_EPOLL_EVENT];

	ulErrCode = initThreadPools(MAX_THREADS, MIN_THREADS, START_THREADS);
	if(ulErrCode != ERROR_SUCCESS)
	{
		perror("initThreadPools");
		goto failed;
	}
	ulErrCode = createListenSocket(&listenFd);
	if(ulErrCode != ERROR_SUCCESS)
	{
		perror("createListenSocket");
		goto failed;
	}
	ulErrCode = createEpoll(listenFd);
	if(ulErrCode != ERROR_SUCCESS)
	{
		perror("createEpoll");
		goto failed;
	}
	while(1)
	{
		nready = epoll_wait(g_iEpollFd, events, MAX_EPOLL_EVENT, 0);
		if(nready > 0)
		{
			procEpollResponse(events, nready);
		}
	}

failed:
	if(listenFd > 0)
	{
		close(listenFd);
	}
	if(g_iEpollFd > 0)
	{
		close(g_iEpollFd);
	}

	return ulErrCode;
}





