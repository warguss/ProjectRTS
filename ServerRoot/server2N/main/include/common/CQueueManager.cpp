#include "CQueueManager.h"

CQueueManager::CQueueManager()
{
    _queue.clear();
    _queueSize = 0;
    queue_mutex = PTHREAD_MUTEX_INITIALIZER;
    queue_cond   = PTHREAD_COND_INITIALIZER;
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

bool CQueueManager::enqueue(int fd, char* buf, int length, int type)
{
	LOG("deque Test");
    CUser* user = new CUser;
    user->setData(fd, buf, length, type);

    /* 1thread...? Queue Lock */
    _queue.push_back(user);
    _queueSize++;
    /* Queue UnLock */

    return true;
}

bool CQueueManager::enqueue(CUser* user)
{
	LOG("Send deque Test");

    /* 1thread...? Queue Lock */
    _queue.push_back(user);
    _queueSize++;
    /* Queue UnLock */

    return true;
}

CUser* CQueueManager::dequeue(int type)
{
    CUser* user = NULL;

	if ( type == READ_TYPE ) 
	{
		/* Queue Lock */
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

bool CQueueManager::isQueueDataExist(int curSize)
{
    if ( _queueSize > 0 )
    {
        return true;
    }
    return false;
}

