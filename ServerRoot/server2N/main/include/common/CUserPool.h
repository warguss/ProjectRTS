#ifndef _MODULE_USERPOOL_H
#define _MODULE_UESRPOOL_H

#include "CUser.h"
#include <map>
#define MAX_POOL 300


class CUserPool
{
	private:
		map<int , CUser*> userInfo;

		pthread_mutex_t pool_mutex;
		pthread_cond_t pool_cond;

	public:
		CUserPool();
		~CUserPool();

		bool addUserInPool(int fd, int32_t x, int32_t y);
		bool delUserInPool(int fd);

		bool allSendEvent();
		bool sendUser(int fd);
		int32_t userCount();
};


#endif
