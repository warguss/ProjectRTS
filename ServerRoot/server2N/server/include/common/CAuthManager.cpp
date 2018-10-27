#include "CAuthManager.h"

CAuthManager::CAuthManager()
{
	key = "TEST_SERVICE_AUTH_WINDOW";
	authByte = AUTH_BYTE;
}

CAuthManager::~CAuthManager()
{
	key.clear();
}

bool CAuthManager::authAgent(const char* psKey)
{
	if ( !psKey || strncasecmp(key.c_str(), psKey, key.size()) )
	{
		return false;
	}

	LOG_DEBUG("Auth Key(%s) psKey(%s)", key.c_str(), psKey);
	return true;
}

bool CAuthManager::agentReturnBuffer(char* buffer, bool isAgentSucc)
{
	if ( !buffer )
	{
		LOG_ERROR("Agent Buffer Error(System Error)");
		return false;
	} 

	std::string retAgent = (isAgentSucc) ? "Success" : "fail agent";
	memcpy(buffer, retAgent.c_str(), retAgent.length());
	LOG_DEBUG("Copy buffer (%s)", buffer);
	return true;
} 

