#include "CQueueManager.h"

CQueueManager::CQueueManager()
{
    _queue.clear();
    _queueSize = 0;

	pthread_mutex_init(&_queue_mutex, (const pthread_mutexattr_t*)NULL);
	pthread_cond_init(&_queue_cond, (const pthread_condattr_t*)NULL);
}

CQueueManager::~CQueueManager()
{
    list<CProtoPacket*>::iterator it;
    for ( it = _queue.begin(); it != _queue.end(); it++ )
    {
        CProtoPacket *packet = *it;
        if ( packet )
        {
            delete packet;
        }
    }
}

void CQueueManager::setType(int type)
{
	/**********************
	 * Lock에 대해
	 * Send Q와
	 * Recv Q가 다르게 동작
	 **********************/
	_type = type;
}

bool CQueueManager::enqueue(CProtoPacket* packet)
{
	if ( !packet )
	{
		LOG("User Invalid\n");
		return false; 
	}

    /* Auto Lock */
	CThreadLockManager lock(_type);
    _queue.push_back(packet);
    _queueSize++;

    return true;
}

CProtoPacket* CQueueManager::dequeue()
{
    CProtoPacket* packet = NULL;
	if ( !isQueueDataExist() ) 
	{
		return packet;
	}

	LOG("Dequeue Start\n");
	/* Auto Lock */
	CThreadLockManager lock(_type);
	packet = _queue.front();
	_queue.pop_front();
	_queueSize--;

    return packet;
}

bool CQueueManager::isQueueDataExist()
{
    if ( _queueSize > 0 )
    {
        return true;
    }

    return false;
}

bool CQueueManager::unLock()
{
	if ( _queueSize > 0 )
	{
		return false;
	} 
	pthread_mutex_unlock(&_queue_mutex);
	pthread_cond_signal(&_queue_cond);
	
	return true;
}

bool CQueueManager::lock()
{
	if ( _queueSize == 0 )
	{
		LOG("Thread Lock\n");
		pthread_mutex_lock(&_queue_mutex);
		return true;
	}
	return false;
}

int CQueueManager::queueSize()
{
	return _queueSize;
}

