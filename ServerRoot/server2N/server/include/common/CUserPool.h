#ifndef _MODULE_USERPOOL_H_
#define _MODULE_UESRPOOL_H_

#include <map>
#include <list>
#include "CUser.h"
#include "CThreadLockManager.h"

using namespace std;
class CUserPool
{
    private:
        pthread_mutex_t pool_mutex;
        pthread_cond_t pool_cond;
		
		int _xUnit;
		int _yUnit;
    public:
		/***********************************
		 * first Map Key	= Sector
		 * second Map Key	= fd
		 ***********************************/
		int totalUser;
        map<int , map<int, CUser*>* > userInfo;
		map<int , map<int, CUser*>* >::iterator itVal;
		map<int, CUser*>::iterator it;

        CUserPool();
        ~CUserPool();

        bool addUserInPool(CUser* user);
        bool delUserInPool(int fd, int sector = -1);
        bool changeUserInPool(CUser* user, int preSector, int curSector);
        CUser* findUserInPool(int fd, int sector = -1);

        int32_t userCount();
		void getAllUserList(list<CUser*>& userConnection);
		void getPartUserList(list<CUser*>& userConnection, int sector);
		int getSectionNo(CUser* user);

		void initialize();
		void clear();

};

#endif
