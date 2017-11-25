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
	_type = type;
}


bool CQueueManager::enqueue(int fd, char* buf, int length, int type)
{
	LOG("deque Test");
    CUser* user = new CUser;
    user->setData(fd, buf, length, type);

    /* 1thread...? Queue Lock */
	CThreadLockManager lock(_type);
    _queue.push_back(user);
    _queueSize++;
    /* Queue UnLock */

    return true;
}

bool CQueueManager::enqueue(CUser* user,int type)
{
	LOG("Send deque Test");

    /* 1thread...? Queue Lock */
	CThreadLockManager lock(type);
	CThreadLock lock;
    _queue.push_back(user);
    _queueSize++;
    /* Queue UnLock */

    return true;
}

CUser* CQueueManager::dequeue(int type)
{
    CUser* user = NULL;
	/* Queue Lock */
	CThreadLockManager lock(type);
	if ( type == READ_TYPE ) 
	{
		if ( _queue.size () > 0 ) 
		{
			user = _queue.front();
			_queue.pop_front();
			_queueSize--;
			LOG("dequeu user[%d]", user->_fd);
		}
	}
	else 
	{
		if ( _queue.size () > 0 ) 
		{
			user = _queue.front();
			_queue.pop_front();
			_queueSize--;
			LOG("dequeu user[%d]", user->_fd);
		}
	}
    /* UnLock */

    return user;
}

bool CQueueManager::isQueueDataExist(int curSize,int type)
{
	/* Lock */
	CThreadLockManager lock(type);
    if ( _queueSize > 0 )
    {
        return true;
    }
	/* UnLock */

    return false;
}

