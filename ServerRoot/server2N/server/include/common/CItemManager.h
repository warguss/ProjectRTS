#ifndef _CITEMMANAGER_H_
#define _CITEMMANAGER_H_
#include <map>
#include <queue>
#include "MsgString.h"
#include "CThreadLockManager.h"
using namespace std;

class Item
{
	public: 
		int64_t _excellX;
		int64_t _excellY;
		int64_t _posX;
		int64_t _posY;
		int32_t _weaponId;
		int32_t _itemType;
		int32_t _amount;

		//server2N::InfoItem* item;
	public:
		Item(int32_t type, int32_t wId, int64_t posX, int64_t posY, int32_t amount)
		{
			_posX = posX;
			_posY = posY;
			_weaponId = wId;
			_itemType = type;
			_amount = amount;

			//item = new server2N::InfoItem();
		}

		Item()
		{
		}
		~Item()
		{
		}
};

class CItemManager
{
	public:
		CItemManager();
		~CItemManager();

	private:
		//int32_t _spawnItemId;
		//int32_t _curSpawnItemId;

		//queue<Item*> _itemQueue;

		pthread_mutex_t _mutex;
		pthread_cond_t _cond;
	public:
		bool spawnItem(string itemId, Item* item);
		bool userGetItem(string itemId);
		Item* itemReturn(string itemId);
		Item* lastSpawnItemReturn();

	private:
		bool _addItem(string key, Item* item);
		bool _delItem(string itemId);
	
	public:
		map<std::string , Item*> _itemInfo;
};
static CItemManager g_itemManager;
#endif
