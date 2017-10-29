#include "CQueue.h"

CQueueManager::CQueueManager()
{
	front = -1;
	rear = -1;
	_bufferQueue.clear();
	queue_mutext = PTHREAD_MUTEX_INITIALIZER;
	queue_cond	 = PTHREAD_COND_INITIALIZER;
}


CQueueManager::~CQueueManager()
{

}

bool CQueueManager::enqueue()
{


	return true;
} 


bool CQueueManager::dequeue()
{


	return true;
}


