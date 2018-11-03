#include "CUserPool.h"


int32_t g_sectorIdx;
CUserPool g_userPool;

CUserPool::CUserPool()
{
	_xUnit = X_GAME_MAX/X_SECTOR_MAX;
	_yUnit = Y_GAME_MAX/Y_SECTOR_MAX;
	_diagnolCount = 0;
	_pool_mutex = NULL;
	_pool_cond = NULL;
}

CUserPool::~CUserPool()
{
	/****************************
	 * Clear Logic 필요
	 ****************************/
    userInfo.clear();
}

void CUserPool::initialize()
{
	int idx = 0;
	totalUser = 0;
	userInfo.clear();

	_diagnolCount = (g_sectorIdx / 2) / _xUnit;
	LOG_INFO("total Sector(%d) diagnolCount(%d)", g_sectorIdx, _diagnolCount);
	while ( idx < g_sectorIdx )
	{
		map<int, CUser*>* tmp = new map<int, CUser*>;
		userInfo.insert(std::pair<int, map<int, CUser*>* >(idx, tmp));
		idx++;
	}

	if ( !_pool_mutex && !_pool_cond )
	{
		_pool_mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t) * g_sectorIdx);
		_pool_cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * g_sectorIdx);

		for ( int idx = 0; idx < g_sectorIdx; idx++ )
		{
			pthread_mutex_init(&_pool_mutex[idx], (const pthread_mutexattr_t*)NULL);
			pthread_cond_init(&_pool_cond[idx], (const pthread_condattr_t*)NULL);
		} 
	} 
} 

int CUserPool::_getUpLeftDiagnolSector(int sector)
{
	int returnSector = 0;
	int sectorLine = sector / _yUnit;
	sector--;
	if ( sector < 0 || (sectorLine != (sector / _yUnit)) )
	{
		return -1;
	}

	returnSector = sector + _yUnit;
	return returnSector;
}

int CUserPool::_getRightSector(int sector)
{
	int returnSector = sector;
	int sectorLine = sector / _yUnit;
	sector++;
	if ( sector  > g_sectorIdx || (sectorLine != (sector / _yUnit)) )
	{
		return -1;
	}

	returnSector = sector;
	return returnSector;
}

int CUserPool::_getLeftSector(int sector)
{
	int returnSector = 0;
	int sectorLine = sector / _yUnit;
	sector--;
	if ( sector < 0 || (sectorLine != (sector / _yUnit)) )
	{
		return -1;
	}

	returnSector = sector;
	return returnSector;
}

int CUserPool::_getUpSector(int sector)
{
	int returnSector = 0;
	int sectorLine = sector / _yUnit;
	sector = sector + _yUnit;
	if ( sector > g_sectorIdx || (sectorLine >= (sector / _yUnit)) )
	{
		return -1;
	}

	returnSector = sector;
	return returnSector;
}

int CUserPool::_getXSectorPosition(int sector)
{
	int returnSector = 0;
	int sectorLine = sector / _yUnit;

	int xPosition = sector % _xUnit;
	return xPosition;
}

int CUserPool::_getDownSector(int sector)
{
	int returnSector = 0;
	int sectorLine = sector / _yUnit;
	sector = sector - _yUnit;
	if ( sector < 0 || (sectorLine <= (sector / _yUnit)) )
	{
		return -1;
	}

	returnSector = sector;
	return returnSector;
}

int CUserPool::getSectionNo(CUser* user)
{
	int idx = 0;
	int yIdx = user->_y / Y_SECTOR_MAX;
	yIdx = yIdx * _yUnit;

	int xIdx = user->_x / X_SECTOR_MAX;
	int xPosition = _getXSectorPosition(xIdx);
	if ( xPosition > _xUnit )
	{
		xIdx = (_xUnit - 1);
	} 
	
	int line = yIdx / _yUnit;
	if ( line > _yUnit )
	{
		line = (_yUnit - 1);
		yIdx = (line * _yUnit);
	}

	idx = yIdx + xIdx;
	LOG_DEBUG("User Sector xIdx(%d) + yIdx(%d) = idx(%d)", xIdx, yIdx, idx);

	return idx;
}


int32_t CUserPool::addUserInPool(CUser* user)
{
	/***********************************
	 * 추가 부는 Lock이 필요없다.
	 ***********************************/
	int idx = getSectionNo(user);
	itVal = userInfo.find(idx);
    if ( totalUser >= POOL_SIZE || itVal == userInfo.end() )
    {
		LOG_ERROR("Error User In Pool");
        return -1;
    }

	map<int, CUser*>* tMap = (map<int, CUser*>*)itVal->second;
	CThreadLockManager lock(&_pool_mutex[idx], &_pool_cond[idx]);
	tMap->insert(std::pair<int, CUser*>(user->_fd, user));
    return idx;
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

	CThreadLockManager lock(&_pool_mutex[idx], &_pool_cond[idx]);
	if ( it_user != tMap->end() )
	{
		user = (CUser*)it_user->second;
		tMap->erase(fd);
		if ( user )
		{
			LOG_DEBUG("Delete User(%d)", user->_fd);
			delete user;
		}
	} 
	user = NULL;
	isDelete = true;


    return isDelete;
}

bool CUserPool::changeUserInPool(CUser* user, int preSector, int curSector)
{
	if ( !user || preSector < 0 || curSector < 0 )
	{
		LOG_ERROR("Invalid Sector Info");
		return false;
	}

	if ( preSector == curSector )
	{
		LOG_DEBUG("do not change Sector, Same(%d-%d)", preSector, curSector);
		return true;
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
		if ( it_preSectorMap == userInfo.end() || it_postSectorMap == userInfo.end() )
		{
			LOG_ERROR("Invalid Sector No pre(%d) post(%d)", preSector, curSector);
			break;
		}

		preMap = (map<int, CUser*>*)it_preSectorMap->second;
		postMap = (map<int, CUser*>*)it_postSectorMap->second;

		CThreadLockManager preLock(&_pool_mutex[preSector], &_pool_cond[preSector]);
		CThreadLockManager postLock(&_pool_mutex[curSector], &_pool_cond[curSector]);
		if ( preMap && postMap )
		{
			preMap->erase(user->_fd);
			user->_sector = curSector;
			postMap->insert(std::pair<int, CUser*>(user->_fd, user));
			isSuccess = true;
			LOG_INFO("Delete User(%d) sector Change(%d->%d)", user->_fd, preSector, user->_sector);
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
				continue;
			}

			isFind = true;
			break;
		}
	}	
	else
	{
		isFind = true;
		do
		{
			itVal = userInfo.find(sector);
			if ( totalUser >= POOL_SIZE || itVal == userInfo.end() || !itVal->second )
			{
				LOG_ERROR("User(%d) Find Error", user->_fd);
				isFind = false;
				break;
			}

			map<int, CUser*>* tMap = (map<int, CUser*>*)itVal->second;
			it = tMap->find(fd);	
			if ( it == tMap->end() )
			{
				LOG_ERROR("Not Exist Sector(%d) In Map", sector);
				isFind = false;
				break;
			}
		}
		while(false);
	} 
	
	if ( isFind ) 
	{
		user = (CUser*)it->second;
		if ( !user )
		{
			LOG_ERROR("User Not Exist in Map User(%d)", fd);
		}
		LOG_DEBUG("Find User(%d)", fd);
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
	int32_t attentionSector = ATTENTION_SECTOR_RANGE;
	int32_t notAttentionSector = NOT_ATTENTION_SECTOR_RANGE * NOT_ATTENTION_SECTOR_RANGE;

	int countLeftUpDiagnol = _diagnolCount;
	int32_t diagnolSector = sector;
	int yBaseSector = NOT_ATTENTION_SECTOR_RANGE;
	int xBaseSector = NOT_ATTENTION_SECTOR_RANGE;

	for ( int idx = 0; idx < countLeftUpDiagnol; idx++ )
	{
		int getSector = _getLeftSector(diagnolSector);
		if ( getSector < 0 )
		{
			diagnolSector = diagnolSector;
			xBaseSector--;
		} 
		else
		{
			diagnolSector = getSector;
		}

		getSector = _getUpSector(diagnolSector);
		if ( getSector < 0 )
		{
			diagnolSector = diagnolSector;
			yBaseSector--;
		} 
		else
		{
			diagnolSector = getSector;
		}
	} 

	int firstSector = diagnolSector;
	for ( int yIdx = 0; yIdx < yBaseSector; yIdx++ )
	{
		int connectionSector = firstSector;
		for ( int xIdx = 0; xIdx < xBaseSector; xIdx++ )
		{
			_addConnectionListBySector(userConnection, connectionSector);
			connectionSector = _getRightSector(connectionSector);
			if ( connectionSector < 0 )
			{
				break;
			} 
		}
		firstSector = _getDownSector(firstSector);
		if ( firstSector < 0 )
		{
			break;
		} 
	}	
}

bool CUserPool::_addConnectionListBySector(list<CUser*>& userConnection, int sector)
{
    map<int, CUser*>* tMap = NULL;
    map<int, map<int, CUser*>* >::iterator it_sectorMap;
    map<int, CUser*>::iterator it_user;

    it_sectorMap = userInfo.find(sector);
    if ( it_sectorMap == userInfo.end() )
    {
        LOG_ERROR("Not Exist Sector");
        return false;
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
    return true;
}


