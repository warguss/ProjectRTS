#ifdef _MODULE_CTHREAD_LOCK_
#define _MODULE_CTHREAD_LOCK_

#include <stdio.h>
#include <pthread.h>


pthread_mutex_t _rMutex = PTHREA_MUTEX_INITIALIZER;
pthread_cond_t _rCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t _wMutex = PTHREA_MUTEX_INITIALIZER;
pthread_cond_t _wCond = PTHREAD_COND_INITIALIZER;


class CThreadLockManager
{
public:
	CThreadLock();
	~CThreadLock();
};
#endif
