#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#define MSG_WORD_LEN 511			
#define MAX_QUEUE 100			//任务队列的最大长度
#define START_THREADS 20		//开始时创建的线程数量
#define MIN_THREADS 20			//最小创建的线程数量
#define MAX_THREADS 100			//最大创建的线程数量

#define ulong unsigned long
#define handle_error(msg) \
	do{ perror(msg); exit(1); }while(0)

#define false 0
#define true 1

//job队列中每个job对应的回调处理函数
typedef 	void 	*(*THREAD_CALLBACK_PF)(void *arg);

//任务队列中的job对应的信息结构体
typedef struct  pool 
{
		THREAD_CALLBACK_PF 	pfThreadCallback; 
		void 	*arg;					
		struct pool 	*next;
}ThreadJob;

typedef struct 
{
		pthread_mutex_t 	mutex;					//结构体全局锁
		pthread_mutex_t 	queue_mutex;			//队列任务数互斥锁

		pthread_t 			*pThreads;					//创建的线程数组
		int 				maxThreadNum;						//最大线程数
		int 				minThreadNum;						//最小线程数
		int 				curThreadNum;						//当前线程数
		int 				busyThreadNum;						//忙线程数

		ThreadJob 			*queue_head;					//任务队列的队头
		ThreadJob 			*queue_tail;					//任务队列的队尾
		int 				queue_num;							//任务队列当前的任务数量
		pthread_cond_t 		queue_not_empty;      	//队列不为空
		pthread_cond_t 		queue_not_full;			//队列不为满

		int 				shutdown;							//线程池是否关闭的标志
}ThreadPools;

extern void 	*procClientRequest(void 	*arg);

extern void 	*manageThreadFunc(void 	*arg);

extern ulong 	createThreads(ThreadPools 	*pThreadPool, int 	startThreadNum);

extern ulong 	initThreadPools(int 	maxThreadNum, int 	minThreadNum, int 	startThreadNum);

extern ulong 	addRequest2JobQueue(THREAD_CALLBACK_PF 	pfCallback, void 	*arg);

extern void 	destroyThreadPools(ThreadPools 	*pThreadPool);

#endif
