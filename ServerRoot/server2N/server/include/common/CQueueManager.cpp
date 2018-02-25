#include "CQueueManager.h"

CQueueManager::CQueueManager()
{
    _queue.clear();
    _queueSize = 0;
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

#if 0 
bool CQueueManager::enqueue(int fd, char* buf, int length)
{
    CUser* user = new CUser;
    user->setData(fd, _type);

    /* Auto Lock */
	CThreadLockManager lock(_type);
    _queue.push_back(user);
    _queueSize++;

    return true;
}
#endif

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
#if 0 
CUser* CQueueManager::dequeue()
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
#endif

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
	/* Auto Lock */
	CThreadLockManager lock(_type);
    if ( _queueSize > 0 )
    {
        return true;
    }
    return false;
}
