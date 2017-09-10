#ifndef _MODULE_USERPOOL_H
#define _MODULE_UESRPOOL_H

#include "CUser.h"
#define MAX_POOL 300


class CUserPool
{
	private:
		List<CUser> userPool;

	public:
		CUserPool();
		~CUserPool();

		bool addUserInPool(int fd, int32_t x, int32_t y);
		bool delUserInPool(CUser* user);

		bool allSendEvent();
};


#endif
