#include "CThreadLockManager.h"


CThreadLockManager::CThreadLockManager(int type)
{
	pthread_mutex_t* mutex = (type == READ_TYPE) ? &_rMutex : &_wMutex;
	pthread_cond_t* cond = (type == READ_TYPE) ? &_rCond : &_wCond;

	pthread_mutex_lock(mutex);
	while(!(*cond))
	{
		LOG("condition Wait\n");
		pthread_cond_wait(cond, &mutex);
	} 

}

~CThreadLockManager::CThreadLockManager(int type)
{
	pthread_mutex_t* mutex = (type == READ_TYPE) ? &_rMutex : &_wMutex;
	pthread_cond_t* cond = (type == READ_TYPE) ? &_rCond : &_wCond;

	pthread_mutex_unlock(mutex);
	pthread_cond_signal(cond);	
}
