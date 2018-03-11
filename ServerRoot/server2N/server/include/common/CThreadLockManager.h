#ifndef _MODULE_CTHREAD_LOCK_
#define _MODULE_CTHREAD_LOCK_

#include <stdio.h>
#include <pthread.h>

#include "MsgString.h"

class CThreadLockManager
{
private:
	int _type;
	int _sectorNo;
public:
	CThreadLockManager(int type, int idx = -1);
	~CThreadLockManager();
};

#endif
