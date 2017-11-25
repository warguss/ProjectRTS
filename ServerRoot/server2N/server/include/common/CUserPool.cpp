#include "CUserPool.h"
static bool CUserPool::allSendEvent();

CUserPool::CUserPool()
{
    userInfo.clear();
    pool_mutex = PTHREAD_MUTEX_INITIALIZER;
    pool_cond = PTHREAD_COND_INITIALIZER;
}

CUserPool::~CUserPool()
{
    userInfo.clear();
}

bool CUserPool::addUserInPool(int fd, int32_t x, int32_t y)
{
    /* Lock  */
    if ( userInfo.size() >=  POOL_SIZE  )
    {
        return false;
    }

    CUser *user = new CUser(fd, x, y);
    userInfo.insert(std::pair<int, CUser*>(fd, user));
    /* UnLock  */
    return true;
}

bool CUserPool::delUserInPool(int fd)
{
    /* find */

    /* memory delete */


    /* Lock */


    /* Map delete */


    /* UnLock */

    return true;
}

CUser* CUserPool::findUserInPool(int fd)
{
    /* 이건 Lock 이 필요없을듯 */
    CUser* user = NULL;
    /* Lock */
    /* find */

    /* UnLock */
    /* return */
    return user;
}

static bool CUserPool::allSendEvent()
{
	return true;
}
