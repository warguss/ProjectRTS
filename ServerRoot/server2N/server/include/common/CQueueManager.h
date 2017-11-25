#ifndef _MODULE_CQUEUE_MANAGER_H_
#define _MODULE_CQUEUE_MANAGER_H_

#include <stdio.h>
#include <pthread.h>
#include <list>
#include "CUser.h"

using namespace std;
class CQueueManager
{
private:
    list<CUser*> _queue;
    int _queueSize;
	int _type;

    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
public:
    CQueueManager();
    ~CQueueManager();

    bool enqueue(int fd, char* buf, int length, int type);
    bool enqueue(CUser* user);
    CUser* dequeue(int type);

    bool isQueueDataExist(int curSize);
    void setType(int type);
};

#endif

