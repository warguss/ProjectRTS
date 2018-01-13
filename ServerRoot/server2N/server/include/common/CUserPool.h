#ifndef _MODULE_USERPOOL_H
#define _MODULE_UESRPOOL_H

#include <map>
#include "CUser.h"

using namespace std;
class CUserPool
{
    private:
        map<int , CUser* > userInfo;

        pthread_mutex_t pool_mutex;
        pthread_cond_t pool_cond;

    public:
        CUserPool();
        ~CUserPool();

        bool addUserInPool(int fd, int32_t x, int32_t y);
        bool delUserInPool(int fd);
        CUser* findUserInPool(int fd);

        int32_t userCount();
        bool sendUser(int fd);
        static bool allSendEvent();
};
extern CUserPool g_userPool;

#endif

