#include "CThreadLockManager.h"

CThreadLockManager::CThreadLockManager(pthread_mutex_t* mutex, pthread_cond_t* cond)
{
	_mutex = mutex;
	_cond = cond;
	if ( _mutex && _cond )
	{
		pthread_mutex_lock(_mutex);
		while(!(cond))
		{
			pthread_cond_wait(_cond, _mutex);
		}
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

