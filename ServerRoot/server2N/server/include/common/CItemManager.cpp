#include "CItemManager.h"
//#include "CThreadLock.h"

CItemManager::CItemManager()
{
	_mutex = PTHREAD_MUTEX_INITIALIZER;
	_cond = PTHREAD_COND_INITIALIZER;
}

CItemManager::~CItemManager()
{
}

bool CItemManager::spawnItem(std::string itemId, Item* item)
{	
	/****************************
	 * Validation Check Item Id
	 ****************************/
	if ( itemId.size() <= 0 || !item )
	{
		LOG_DEBUG("item not exist(%s)", itemId.c_str());
		return false;
	}

	/****************************
	 * Item Spawn
	 * And Map Add
	 ****************************/
	//const char* idKey = strdup(itemId);
	if ( !_addItem(itemId, item) )
	{
		LOG_ERROR("addItem Error(%s)", itemId.c_str());
		return false;
	}

	return true;
}

bool CItemManager::userGetItem(std::string itemId)
{
	/*****************************
	 * Lock GetItem
	 *****************************/
	CThreadLockManager manager(&_mutex, &_cond, true);
	LOG_DEBUG("Check Get Item(%s)", itemId.c_str());
	map<string, Item*>::iterator it = _itemInfo.find(itemId);
	if ( it == _itemInfo.end() )
	{
		LOG_ERROR("Not Exist Iterator in Item(%s), mapSize(%d)", itemId.c_str(), _itemInfo.size());
		return false;
	} 

	//const char* key = it->first;
	Item* item = it->second;
	if ( !item )
	{
		LOG_ERROR("Item Not Exist(%s)", itemId.c_str());
		return false;
	}
	else
	{
		delete item;
		item = NULL;
		_itemInfo.erase(itemId);

		//free(key);
		//key = NULL;
	}

	LOG_INFO("Item Id Get(%s)", itemId.c_str());
	return true;
}

Item* CItemManager::itemReturn(string itemId)
{

	map<string, Item*>::iterator it = _itemInfo.find(itemId);
	if ( it == _itemInfo.end() )
	{
		return false;
	} 

	Item* item = it->second;
	if ( !item )
	{
		return false;
	}

	return item;
}

#if 0 
Item* CItemManager::lastSpawnItemReturn()
{
	if ( _itemQueue.size() <= 0 )
	{
		return NULL;
	}

	//Item* item = _itemQueue.pop();
	return item;
}
#endif

bool CItemManager::_addItem(string key, Item* item)
{
	/********************************
	 * Lock
	 ********************************/
	CThreadLockManager manager(&_mutex, &_cond, true);
	if ( !item )
	{
		LOG_ERROR("Not Exist Item (%s)", key.c_str());
		return false;
	}

	_itemInfo.insert(std::pair<string, Item*>(key, item));
	//_itemQueue.push(item);

	LOG_ERROR("insert Item(%s)", key.c_str());
	return true;
}

bool CItemManager::_delItem(string itemId)
{
	/********************************
	 * Lock
	 ********************************/
	bool isItem = false;
	map<string, Item*>::iterator it = _itemInfo.find(itemId);
	if ( it == _itemInfo.end() )
	{
		return false;
	} 

	Item* item = it->second;
	if ( item )
	{
		delete item;
		item = NULL;
		_itemInfo.erase(itemId);
		isItem = true;
	}
	else
	{
		LOG_ERROR("Not Exist Id(%s)", itemId.c_str());
	}
		
	return isItem;
}
