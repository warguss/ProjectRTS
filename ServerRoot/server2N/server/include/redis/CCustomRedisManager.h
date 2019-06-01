#ifndef __REDIS_MANAGER_H_
#define __REDIS_MANAGER_H_
#include "hiredis.h"
#include "common/MsgString.h"


class CCustomRedisManager
{
	private:
		redisContext *_connect;

	public: 
		CCustomRedisManager();
		~CCustomRedisManager();

		bool initialize(std::string ip, int32_t port, uint32_t timeout);
		void destroyer();

		bool setRedis(std::string key, std::string value);
		bool getRedis(std::string key, std::string &getValue);
};


#endif
