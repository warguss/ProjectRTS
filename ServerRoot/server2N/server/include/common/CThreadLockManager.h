#ifndef _MODULE_CTHREAD_LOCK_
#define _MODULE_CTHREAD_LOCK_

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "MsgString.h"

class CThreadLockManager
{
private:
	int _type;
	int _sectorNo;

	pthread_mutex_t* _mutex;
	pthread_cond_t* _cond;

public:
	CThreadLockManager();
	CThreadLockManager(int type = -1, int idx = -1);
	CThreadLockManager(pthread_mutex_t* mutex, pthread_cond_t* cond, bool isAuto = true);
	~CThreadLockManager();

	void lock();
	void release();
	void setValue(pthread_mutex_t* mutex, pthread_cond_t* cond);
};

#endif
