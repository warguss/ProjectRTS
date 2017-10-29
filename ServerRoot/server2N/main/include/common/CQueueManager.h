#ifndef _CQUEUE_H_
#define _CQUEUE_H_

#include <stdio.h>
#define BUFFER_SIZE 8196
class CQueueManager
{
private:
	list<char*> _bufferQueue;  	
	int front;
	int rear;

	pthread_mutex_t queue_mutex;
	pthread_cond_t queue_cond;
public:
	CQueueManager();
	~CQueueManager();

	bool enqueue();
	char* dequeue();
};

#endif
