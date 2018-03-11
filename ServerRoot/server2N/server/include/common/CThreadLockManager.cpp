#include "CThreadLockManager.h"
#include "CUserPool.h"

pthread_mutex_t _rMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _rCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t _wMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t _wCond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t *_listMutex = NULL;
pthread_cond_t *_listCond = NULL;

extern int32_t g_sectorIdx;

CThreadLockManager::CThreadLockManager(int type, int sectorNo)
{
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
	if ( sectorNo < 0 )
	{
		_type = type;
		_sectorNo = sectorNo;
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
		}

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
	pthread_mutex_t* mutex;
	pthread_cond_t* cond;
	if ( _sectorNo < 0 )
	{
		mutex = (_type == READ_TYPE) ? &_rMutex : &_wMutex;
		cond = (_type == READ_TYPE) ? &_rCond : &_wCond;

		pthread_mutex_unlock(mutex);
		pthread_cond_signal(cond);	
	}
	else
	{
		mutex = &_listMutex[_sectorNo];
		cond = &_listCond[_sectorNo];

		pthread_mutex_unlock(mutex);
		pthread_cond_signal(cond);	
	}
}
