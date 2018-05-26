#include "CUserPool.h"


int32_t g_sectorIdx;
CUserPool g_userPool;

CUserPool::CUserPool()
{
}

void CUserPool::initialize()
{
	int idx = 0;
	totalUser = 0;
	userInfo.clear();
	LOG_INFO("total Sector(%d)", g_sectorIdx);
	while ( idx < g_sectorIdx )
	{
		map<int, CUser*>* tmp = new map<int, CUser*>;
		userInfo.insert(std::pair<int, map<int, CUser*>* >(idx, tmp));
		idx++;
	}
} 


CUserPool::~CUserPool()
{
	/****************************
	 * Clear Logic 필요
	 ****************************/
    userInfo.clear();
}


int CUserPool::getSectionNo(CUser* user)
{
	int idx = 0;
	int yIdx = user->_y / Y_SECTOR_MAX;
	yIdx = yIdx * Y_SECTOR_MAX;

	int xIdx = user->_x / X_SECTOR_MAX;
	idx = yIdx + xIdx;
	LOG_INFO("User Sector xIdx(%d) + yIdx(%d) = idx(%d)", xIdx, yIdx, idx);

	return idx;
}


bool CUserPool::addUserInPool(CUser* user)
{
	/***********************************
	 * 추가 부는 Lock이 필요없다.
	 ***********************************/
	int idx = getSectionNo(user);
	user->_sector = idx;
	itVal = userInfo.find(idx);
    if ( totalUser >= POOL_SIZE || itVal == userInfo.end() )
    {
		LOG_ERROR("Error User In Pool");
        return false;
    }
	map<int, CUser*>* tMap = (map<int, CUser*>*)itVal->second;
	CThreadLockManager lock(NOT_SET, idx);
	tMap->insert(std::pair<int, CUser*>(user->_fd, user));
    return true;
}

bool CUserPool::delUserInPool(int fd, int sector)
{
	/****************************
	 * 각각의 맵에 대해
	 * Lock 처리가 필요하다
	 ****************************/
	CUser* user = NULL;
	bool isDelete = false;
	map<int, map<int, CUser*>* >::iterator it_sectorMap;
	map<int, CUser*>::iterator it_user;
	map<int, CUser*>* tMap = NULL;
	int idx = 0;
	if ( sector < 0 ) 
	{
		for ( it_sectorMap = userInfo.begin(); it_sectorMap != userInfo.end(); it_sectorMap++ )
		{
			tMap = (map<int, CUser*>*)it_sectorMap->second;
			it_user = tMap->find(fd);
			if ( it_user == tMap->end() )
			{
				continue;
			}
			
			idx = it_sectorMap->first;
			break;
		}
	}	
	else
	{
		it_sectorMap = userInfo.find(sector);
		if ( it_sectorMap == userInfo.end() || !it_sectorMap->second )
		{
		    return isDelete;
		}

		idx = sector;
		tMap = (map<int, CUser*>*)it_sectorMap->second;
		it_user = tMap->find(fd);	
	}

	CThreadLockManager lock(NOT_SET, idx);
	user = (CUser*)it->second;
	if ( user && tMap )
	{
		LOG_DEBUG("Delete User(%d)", user->_fd);
		tMap->erase(fd);
		
		//(map<int, CUser*>)it->erase(fd);
		delete user;
	} 
	user = NULL;
	isDelete = true;


    return isDelete;
}

bool CUserPool::changeUserInPool(CUser* user, int preSector, int curSector)
{
	if ( !user || preSector <= 0 || curSector <= 0 )
	{
		LOG_ERROR("Invalid Sector Info");
		return false;
	}

	bool isSuccess = false;
	do
	{
		map<int, map<int, CUser*>* >::iterator it_preSectorMap;
		map<int, map<int, CUser*>* >::iterator it_postSectorMap;
		map<int, CUser*>* preMap = NULL;
		map<int, CUser*>* postMap = NULL;
		int idx = 0;
		it_preSectorMap = userInfo.find(preSector);
		it_postSectorMap = userInfo.find(curSector);
		if ( it_preSectorMap == userInfo.end() || !it_preSectorMap->second || it_postSectorMap == userInfo.end() || it_postSectorMap->second )
		{
			LOG_ERROR("Invalid Sector No pre(%d) post(%d)", preSector, curSector);
			break;
		}

		preMap = (map<int, CUser*>*)it_preSectorMap->second;
		postMap = (map<int, CUser*>*)it_postSectorMap->second;
		CThreadLockManager preLock(NOT_SET, preSector);
		CThreadLockManager postLock(NOT_SET, curSector);
		if ( preMap && postMap )
		{
			LOG_INFO("Delete User(%d)", user->_fd);
			preMap->erase(user->_fd);
			postMap->insert(std::pair<int, CUser*>(user->_fd, user));
			isSuccess = true;
		} 
	}while(false);

	return isSuccess;
}


CUser* CUserPool::findUserInPool(int fd, int sector)
{
	CUser* user = NULL;
	bool isFind = false;
	if ( sector < 0 ) 
	{
		for ( itVal = userInfo.begin(); itVal != userInfo.end(); itVal++ )
		{
			map<int, CUser*>* tMap = (map<int, CUser*>*)itVal->second;
			it = tMap->find(fd);
			if ( it == tMap->end() )
			{
				break;
			}

			isFind = true;
			break;
		}
	}	
	else
	{
		itVal = userInfo.find(sector);
		if ( totalUser >=  POOL_SIZE || itVal == userInfo.end() || !itVal->second )
		{
			LOG_ERROR("Find Error");
		    return (CUser*)NULL;
		}

		map<int, CUser*>* tMap = (map<int, CUser*>*)itVal->second;
		it = tMap->find(fd);	
		if ( it == tMap->end() )
		{
			isFind = true;
		}
	} 
	
	if ( isFind ) 
	{
		user = (CUser*)it->second;
		if ( !user )
		{
			LOG_ERROR("User Not Exist");
			return NULL;
		}
	}
	return user;
}


void CUserPool::getAllUserList(list<CUser*>& userConnection)
{
	/***********************************
	 * 전체 순회
	 ***********************************/ 
	for ( itVal = userInfo.begin(); itVal != userInfo.end(); itVal++ )
	{
		map<int, CUser*>* tMap = (map<int, CUser*>*)itVal->second;
		for ( it = tMap->begin() ; it != tMap->end() ; it++ )
		{
			CUser* user = (CUser*)it->second;
			if ( !user )
			{
				continue ; 
			}
			userConnection.push_back(user);
		}
	}
	LOG_INFO("size allUser Connect (%d)", userConnection.size());
} 

void CUserPool::getPartUserList(list<CUser*>& userConnection, int sector)
{
	/***********************************
	 * 구역 순회
	 ***********************************/ 
	map<int, CUser*>* tMap = NULL;
	map<int, map<int, CUser*>* >::iterator it_sectorMap;
	map<int, CUser*>::iterator it_user;
	it_sectorMap = userInfo.find(sector);
	if ( it_sectorMap == userInfo.end() )
	{
		LOG_ERROR("Not Exist Sector");
		return ;
	} 

	tMap = (map< int, CUser* >*)it_sectorMap->second;
	for ( it_user = tMap->begin(); it_user != tMap->end(); it_user++ )
	{
		CUser* user = (CUser*)it_user->second;
		if ( !user )
		{
			continue ; 
		}
		userConnection.push_back(user);
	}
}


