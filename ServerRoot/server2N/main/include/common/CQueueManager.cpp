#include "CQueueManager.h"

CQueueManager::CQueueManager()
{
	_queue.clear();
	_queueSize = 0;
	queue_mutext = PTHREAD_MUTEX_INITIALIZER;
	queue_cond	 = PTHREAD_COND_INITIALIZER;
}


CQueueManager::~CQueueManager()
{
	list<CData*>::iterator it;
	for ( it = _queue.begin(); it != _queue.end(); it++ )
	{
		CData *data = *it;
		if ( data )
		{
			delete *data;
		}
	} 
}

void CQueueManager::enqueue(int fd, char* buf, int type)
{
	CData* data = new CData;
	data.setData(fd, buf, type);

	/* 1thread...? Queue Lock */
	_queue.push_back(data);
	_queueSize++;
	/* Queue UnLock */
} 


CData* CQueueManager::dequeue()
{
	CData* data = NULL;

	/* Queue Lock */ 
	data = _queue.front();
	_queue.pop_front();
	_queueSize--;
	/* UnLock */
	

	return data;
}

bool CQueueManager::isQueueDataExist(int curSize)
{
	if ( _queueSize > 0 )
	{
		return true;
	}
	return false;
} 
