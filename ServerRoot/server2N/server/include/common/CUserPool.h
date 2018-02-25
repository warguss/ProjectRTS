#ifndef _MODULE_USERPOOL_H
#define _MODULE_UESRPOOL_H

#include <map>
#include <list>
#include "CUser.h"

using namespace std;
class CUserPool
{
    private:
        pthread_mutex_t pool_mutex;
        pthread_cond_t pool_cond;
		
    public:
        map<int , CUser* >::iterator it;
        map<int , CUser* > userInfo;
        CUserPool();
        ~CUserPool();

        bool addUserInPool(CUser* user);
        bool delUserInPool(int fd);
        CUser* findUserInPool(int fd);

        int32_t userCount();
		void getUserList(list<int32_t>& userConnection);
};

extern CUserPool g_userPool;

#endif
