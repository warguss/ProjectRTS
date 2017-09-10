
#include "CUserPool.h"
#define MAX_POOL 300



CUserPool::CUserPool()
{
	userInfo.clear()
} 


CUserPool::~CUserPool()
{
	userInfo.clear()
} 
		
bool CUserPool::addUserInPool(int fd, int32_t x, int32_t y)
{
	/* Lock  */
	if ( userInfo.size() >=  MAX_POOL  )
	{
		return false;
	}

	CUser *user = new CUser(fd, x, y);
	userInfo.add(fd, user);
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
	CUser* user = NULL;
	/* find */

	/* return */
	return user;
} 

