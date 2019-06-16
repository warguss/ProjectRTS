#include "CCustomRedisManager.h"

// Init을 한번만 하기위해 다음과 같이 사용
CCustomRedisManager g_redisManager;

CCustomRedisManager::CCustomRedisManager()
{
	LOG_DEBUG("CCustomRedisManager()");
	_connect = NULL;
}

CCustomRedisManager::~CCustomRedisManager()
{
	LOG_DEBUG("~CCustomRedisManager()");
}

bool CCustomRedisManager::initialize(std::string ip, int32_t port, uint32_t timeout)
{
	LOG_DEBUG("Initialize()");
	struct timeval timeoutVal;
	timeoutVal.tv_sec = timeout;
	timeoutVal.tv_usec = 0;

	_connect = redisConnectWithTimeout(ip.c_str(), port, timeoutVal);

	if ( !_connect || _connect->err )
	{
		LOG_DEBUG("Redis Initialize ERR DES(%s)", _connect->err);
		redisFree(_connect);
		return false;
	}

	/********************************************
	 * 특정 로그 파일 존재 시,
	 * Redis에 다시 Setting한다
	 ********************************************/

	return true;
}

void CCustomRedisManager::destroyer()
{
	if ( _connect )
	{
		redisFree(_connect);
		_connect = NULL;
	}
}

bool CCustomRedisManager::setRedis(std::string key, std::string value)
{
	redisReply *reply = NULL;
	reply = redisCommand(_connect, "SET %s %s", key.c_str(), value.c_str());
	if ( !reply || reply->type == REDIS_REPLY_ERROR )
	{
		LOG_ERROR("Redis Set Error Key[%s] Value[%s] DES[%s]", key.c_str(), value.c_str(), reply->str);
		return false;
	}

	if ( reply )
	{
		freeReplyObject(reply);
	}
	LOG_INFO("Redis Set Key[%s] Value[%s]", key.c_str(), value.c_str());

	return true;
}

bool CCustomRedisManager::getRedis(std::string key, std::string &getValue)
{
	redisReply *reply = NULL;
	reply = redisCommand(_connect, "GET %s", key.c_str());
	if ( !reply || reply->type == REDIS_REPLY_ERROR )
	{
		LOG_ERROR("Redis Set Error Key[%s] DES[%s]", key.c_str(),  reply->str);
		return false;
	}

	getValue = reply->str;
	if ( reply )
	{
		freeReplyObject(reply);
	}

	LOG_INFO("Redis Get Key[%s] Value[%s]", key.c_str(), getValue.c_str());
	return true;
}




