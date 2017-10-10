#ifndef _CQUEUE_MANAGER_H_
#define _CQUEUE_MANAGER_H_

#include <stdio.h>
class CQueueManager
{
private:
	list<CData*> _queue;  	
	int _queueSize;

	pthread_mutex_t queue_mutex;
	pthread_cond_t queue_cond;
public:
	CQueueManager();
	~CQueueManager();

	bool enqueue(int fd, char* buf, int type);
	CData dequeue();
};

#endif
