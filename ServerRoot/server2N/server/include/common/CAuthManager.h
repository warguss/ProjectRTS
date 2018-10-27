#ifndef _CAUTHMANAGER_H_
#define _CAUTHMANAGER_H_
#include <stdio.h>
#include <stdlib.h>
#include "MsgString.h"

using namespace std;
class CAuthManager
{
	private:
		string key;	
	
	public:
		int32_t authByte;

	public:
		CAuthManager();
		~CAuthManager();

		bool authAgent(const char* psKey);
		bool agentReturnBuffer(char* buffer, bool isAgentSucc);

};

#endif
