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

bool CQueueManager::enqueue(int fd, char* buf, int type)
{
    CUser* user = new CUser;
    user->setData(fd, buf, type);

    /* 1thread...? Queue Lock */
    _queue.push_back(user);
    _queueSize++;
    /* Queue UnLock */

    return true;
}

CUser* CQueueManager::dequeue()
{
    CUser* user = NULL;

    /* Queue Lock */
    user = _queue.front();
    _queue.pop_front();
    _queueSize--;
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

