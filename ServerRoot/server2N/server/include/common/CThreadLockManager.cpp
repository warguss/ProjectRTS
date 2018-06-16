#include "CThreadLockManager.h"

pthread_mutex_t _rMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _rCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t _wMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _wCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t *_listMutex = NULL;
pthread_cond_t *_listCond = NULL;

extern int32_t g_sectorIdx;


CThreadLockManager::CThreadLockManager(pthread_mutex_t* mutex, pthread_cond_t* cond)
{
	printf("Log Lock");
	_sectorNo = -1;
	_type = -1;

	_mutex = mutex;
	_cond = cond;
	pthread_mutex_lock(_mutex);
	while(!(cond))
	{
		pthread_cond_wait(_cond, _mutex);
	}
}


CThreadLockManager::CThreadLockManager(int type, int sectorNo)
{
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
	_sectorNo = sectorNo;
	if ( sectorNo < 0 )
	{
		_type = type;
		mutex = (_type == READ_TYPE) ? &_rMutex : &_wMutex;
		cond = (_type == READ_TYPE) ? &_rCond : &_wCond;

		pthread_mutex_lock(mutex);
		while(!(cond))
		{
			pthread_cond_wait(cond, mutex);
		}
	}
	else
	{
		if ( !_listMutex && !_listCond )
		{
			_listMutex = new pthread_mutex_t[g_sectorIdx];
			_listCond = new pthread_cond_t[g_sectorIdx];
		
			for ( int idx = 0; idx < g_sectorIdx; idx++ )
			{
				pthread_mutex_init(&_listMutex[idx], (const pthread_mutexattr_t*)NULL);
				pthread_cond_init(&_listCond[idx], (const pthread_condattr_t*)NULL);
			}
			//_sectorNo = sectorNo;
		}
		//LOG_DEBUG("Sector Lock(%d)", sectorNo);
		mutex = &_listMutex[sectorNo];
		cond = &_listCond[sectorNo];
		pthread_mutex_lock(mutex);
		while (!(cond))
		{
			pthread_cond_wait(cond, mutex);
		} 	
	} 
	 

}

CThreadLockManager::~CThreadLockManager()
{
	release();
}

void CThreadLockManager::release()
{
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
	if ( _sectorNo < 0 && _type > 0 )
	{
		printf("Test1\n");
		mutex = (_type == READ_TYPE) ? &_rMutex : &_wMutex;
		cond = (_type == READ_TYPE) ? &_rCond : &_wCond;

		pthread_mutex_unlock(mutex);
		pthread_cond_signal(cond);	
	}
	else if ( _sectorNo > 0 )
	{
		printf("Test2 (%d)\n", _sectorNo);
		mutex = &_listMutex[_sectorNo];
		cond = &_listCond[_sectorNo];

		pthread_mutex_unlock(mutex);
		pthread_cond_signal(cond);	
	}
	else
	{
		printf("Test3\n");
		pthread_mutex_unlock(_mutex);
		pthread_cond_signal(_cond);	
	} 
} 


