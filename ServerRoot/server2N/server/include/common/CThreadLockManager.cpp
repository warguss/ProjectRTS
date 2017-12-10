#include "CThreadLockManager.h"

pthread_mutex_t _rMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _rCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t _wMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _wCond = PTHREAD_COND_INITIALIZER;

CThreadLockManager::CThreadLockManager(int type)
{
	_type = type;
	pthread_mutex_t* mutex = (_type == READ_TYPE) ? &_rMutex : &_wMutex;
	pthread_cond_t* cond = (_type == READ_TYPE) ? &_rCond : &_wCond;

	pthread_mutex_lock(mutex);
	while(!(cond))
	{
		pthread_cond_wait(cond, mutex);
	} 

}

CThreadLockManager::~CThreadLockManager()
{
	pthread_mutex_t* mutex = (_type == READ_TYPE) ? &_rMutex : &_wMutex;
	pthread_cond_t* cond = (_type == READ_TYPE) ? &_rCond : &_wCond;

	pthread_mutex_unlock(mutex);
	pthread_cond_signal(cond);	
}
