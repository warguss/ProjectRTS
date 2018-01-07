#ifndef _MODULE_CQUEUE_MANAGER_H_
#define _MODULE_CQUEUE_MANAGER_H_

#include <stdio.h>
#include <pthread.h>
#include <list>
#include "CUser.h"
#include "CThreadLockManager.h"

using namespace std;
class CQueueManager
{
private:
	int _type;
    int _queueSize;
    list<CUser*> _queue;

    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
public:
    CQueueManager();
    ~CQueueManager();

    bool enqueue(int fd, char* buf, int length);
    bool enqueue(CUser* user);
    CUser* dequeue();

    bool isQueueDataExist();
    void setType(int type);
};

#endif

