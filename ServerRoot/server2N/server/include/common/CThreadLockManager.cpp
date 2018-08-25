#include "CThreadLockManager.h"



CThreadLockManager::CThreadLockManager()
{
	printf("Test");
}

CThreadLockManager::CThreadLockManager(pthread_mutex_t* mutex, pthread_cond_t* cond, bool isAutoLock)
	: _mutex(mutex), _cond(cond)
{
	if ( isAutoLock )
	{
		lock();
	}
}

CThreadLockManager::~CThreadLockManager()
{
	release();
}

void CThreadLockManager::release()
{
	if ( _mutex && _cond )
	{
		pthread_mutex_unlock(_mutex);
		pthread_cond_signal(_cond);	
	}
}

void CThreadLockManager::lock()
{
	if ( _mutex && _cond )
	{
		pthread_mutex_lock(_mutex);
		while(!(_cond))
		{
			pthread_cond_wait(_cond, _mutex);
		}
	}
	else
	{
		printf("Not Exist thread");
	}
}

void CThreadLockManager::setValue(pthread_mutex_t* mutex, pthread_cond_t* cond)
{
	_mutex = mutex;
	_cond = cond;
}
