#include "CQueueManager.h"

CQueueManager::CQueueManager()
{
    _queue.clear();
    _queueSize = 0;


	/************************************
	 * Quque가 만들어지고, lock을 건다
	 ************************************/
	_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
	_queue_cond = PTHREAD_COND_INITIALIZER;

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

void CQueueManager::setStartLock()
{
	if ( !isQueueDataExist() )
	{
		LOG_DEBUG("Thread Lock Setting %s", (_type == READ_TYPE) ? "Read Queue Lock" : "Write Queue Lock");
		pthread_mutex_lock(&_queue_mutex);
		while ( !(&_queue_cond) )
		{
			LOG_DEBUG("Wait?");
			pthread_cond_wait(&_queue_cond, &_queue_mutex);
		}
	}
}

void CQueueManager::releaseLock()
{
	LOG_DEBUG("Thread Lock Release %s", (_type == READ_TYPE) ? "Read Queue Lock" : "Write Queue Lock");
	pthread_mutex_unlock(&_queue_mutex);
	pthread_cond_signal(&_queue_cond);
}


bool CQueueManager::enqueue(CProtoPacket* packet)
{
	if ( !packet )
	{
		LOG_ERROR("User Invalid");
		return false; 
	}

    /***********************
	 * Auto Lock 
	 * 공유자원에 대한 Lock
	 ***********************/
	//CThreadLockManager lock(_type);
	CThreadLockManager lock(&_queue_mutex, &_queue_cond);
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

	/***********************
	 * Auto Lock 
	 * 공유자원에 대한 Lock
	 ***********************/
	//CThreadLockManager lock(_type);
	CThreadLockManager lock(&_queue_mutex, &_queue_cond);
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
		LOG_DEBUG("Thread Lock");
		pthread_mutex_lock(&_queue_mutex);
		return true;
	}
	return false;
}

int CQueueManager::queueSize()
{
	return _queueSize;
}

