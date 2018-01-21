#include "CUserPool.h"

CUserPool::CUserPool()
{
    userInfo.clear();
    pool_mutex = PTHREAD_MUTEX_INITIALIZER;
    pool_cond = PTHREAD_COND_INITIALIZER;
}

CUserPool::~CUserPool()
{
	/****************************
	 * Clear Logic 필요
	 ****************************/
    userInfo.clear();
}

bool CUserPool::addUserInPool(CUser* user)
{
	/***********************************
	 * 추가 부는 Lock이 필요없다.
	 ***********************************/
    if ( userInfo.size() >=  POOL_SIZE || !user )
    {
        return false;
    }

    userInfo.insert(std::pair<int, CUser*>(user->_fd, user));
    return true;
}

#if 0
bool CUserPool::addUserInPool(CUser* user)
{
	/***********************************
	 * 추가 부는 Lock이 필요없다.
	 ***********************************/
    if ( userInfo.size() >=  POOL_SIZE  )
    {
        return false;
    }

    userInfo.insert(std::pair<int, CUser*>(fd, user));
    return true;
}
#endif

bool CUserPool::delUserInPool(int fd)
{
    /* find */
	it = userInfo.find(fd);
	if ( it == userInfo.end() ) 
	{
		return NULL;
	}
	
	CUser *user = (CUser*)it->second;
	if ( user )
	{
		LOG("Delete User(%d)\n", user->_fd);
		delete user;
	}
	user = NULL;
    /* memory delete */


    /* Lock */


    /* Map delete */
	userInfo.erase(fd);

    /* UnLock */

    return true;
}

CUser* CUserPool::findUserInPool(int fd)
{
    it = userInfo.find(fd);
	if ( it == userInfo.end() ) 
	{
		return NULL;
	}
    return (CUser*)it->second;
}

#if 0 
static bool CUserPool::allSendEvent();
static bool CUserPool::allSendEvent()
{
	return true;
}
#endif
