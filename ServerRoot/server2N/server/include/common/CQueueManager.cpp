#include "CQueueManager.h"

CQueueManager::CQueueManager()
{
    _queue.clear();
    _queueSize = 0;
}


CQueueManager::~CQueueManager()
{
    list<CUser*>::iterator it;
    for ( it = _queue.begin(); it != _queue.end(); it++ )
    {
        CUser *User = *it;
        if ( User )
        {
            delete User;
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

bool CQueueManager::enqueue(CUser* user)
{
	if ( !user )
	{
		LOG("User Invalid\n");
		return false; 
	}

    /* Auto Lock */
	CThreadLockManager lock(_type);
    _queue.push_back(user);
    _queueSize++;

    return true;
}

CUser* CQueueManager::dequeue()
{
    CUser* user = NULL;
	if ( !isQueueDataExist() ) 
	{
		return user;
	}

	LOG("Dequeue Start\n");
	/* Auto Lock */
	CThreadLockManager lock(_type);
	user = _queue.front();
	_queue.pop_front();
	_queueSize--;

    return user;
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
