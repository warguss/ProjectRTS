#ifndef _MODULE_CQUEUE_MANAGER_H_
#define _MODULE_CQUEUE_MANAGER_H_

#include <stdio.h>
#include <pthread.h>
#include <list>
#include "CUser.h"
#include "CProtoPacket.h"
#include "CThreadLockManager.h"

using namespace std;
class CQueueManager
{
private:
	int _type;
    int _queueSize;
    list<CProtoPacket*> _queue;

    pthread_mutex_t _queue_mutex;
    pthread_cond_t _queue_cond;
public:
    CQueueManager();
    ~CQueueManager();

    bool enqueue(int fd, char* buf, int length);
    bool enqueue(CProtoPacket* packet);
    CProtoPacket* dequeue();

    bool isQueueDataExist();
    void setType(int type);

	static void* autoQueueLock();

	bool unLock();
	bool lock();

	int queueSize();
	
};
#endif
