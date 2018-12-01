#include "CItemManager.h"
//#include "CThreadLock.h"

CItemManager::CItemManager()
{
	_spawnItemId = 0;
	_curSpawnItemId = 0;
}

CItemManager::~CItemManager()
{
}

bool CItemManager::spawnItem(const char* itemId, Item* item)
{	
	/****************************
	 * Validation Check Item Id
	 ****************************/
	if ( !itemId || !item )
	{
		return false;
	}

	LOG_DEBUG("item(%d)", _spawnItemId);
	/****************************
	 * Item Spawn
	 * And Map Add
	 ****************************/
	if ( !_addItem(itemId, item) )
	{
		return false;
	}

	return true;
}

bool CItemManager::userGetItem(const char* itemId)
{
	/*****************************
	 * Lock GetItem
	 *****************************/
	map<const char*, Item*>::iterator it = _itemInfo.find(itemId);
	if ( it == _itemInfo.end() )
	{
		return false;
	} 

	Item* item = it->second;
	if ( !item )
	{
		return false;
	}

	if ( _delItem(itemId) )
	{
		return false;
	} 

	LOG_INFO("Item Id Get(%d)", itemId);
	return true;
}

Item* CItemManager::itemReturn(const char* itemId)
{

	map<const char*, Item*>::iterator it = _itemInfo.find(itemId);
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

bool CItemManager::_addItem(const char* key, Item* item)
{
	/********************************
	 * Lock
	 ********************************/
	if ( !item )
	{
		LOG_ERROR("Not Exist Item");
		return false;
	}

	_itemInfo.insert(std::pair<const char*, Item*>(key, item));
	_itemQueue.push(item);

	return true;
}

bool CItemManager::_delItem(const char* itemId)
{
	/********************************
	 * Lock
	 ********************************/
	bool isItem = false;

	map<const char*, Item*>::iterator it = _itemInfo.find(itemId);
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
		LOG_ERROR("Not Exist Id(%d)", itemId);
	}
		
	return isItem;
}
