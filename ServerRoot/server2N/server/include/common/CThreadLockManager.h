#ifndef _MODULE_CTHREAD_LOCK_
#define _MODULE_CTHREAD_LOCK_

#include <stdio.h>
#include <pthread.h>

#include "MsgString.h"

#if 0 
pthread_mutex_t _rMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _rCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t _wMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _wCond = PTHREAD_COND_INITIALIZER;
#endif

class CThreadLockManager
{
private:
	int _type;

public:
	CThreadLockManager(int type);
	~CThreadLockManager();
};
#endif
