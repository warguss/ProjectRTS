#ifndef _MODULE_PROTOLOGIC_H_
#define _MODULE_PROTOLOGIC_H_
#include "CSessionManager.h"
#include "CProtoManager.h"
#include "redis/CCustomRedisManager.h"
using namespace std;

class CProtoLogicBase
{
	public:
		bool _isPartSend;
		list<CUser*> _userList;
		list<CProtoPacket*> _packetOutList;

	public:
		CProtoLogicBase();
		CProtoLogicBase(bool isPartSend);
		virtual ~CProtoLogicBase();

		virtual bool onPreProcess(int32_t eventSector, bool isPartSend);
		virtual bool onPreProcess(int32_t eventSector);
		virtual bool onProcess(CSessionManager& session, CProtoPacket* eventPacket) = 0;
		virtual bool onPostProcess(CSessionManager& session);
};

class CProtoConnection : public CProtoLogicBase
{
	public:
		CProtoConnection();
		~CProtoConnection();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

class CProtoDisConnection : public CProtoLogicBase
{
	public:
		CProtoDisConnection();
		~CProtoDisConnection();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

class CProtoGameEventLogicBase : public CProtoLogicBase
{
	public:
		CProtoGameEventLogicBase();
		~CProtoGameEventLogicBase();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

#if 0 
class CProtoItemEvent : public CProtoLogicBase
{
	public:
		CProtoItemEvent();
		~CProtoItemEvent();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};
#endif

// Nothing , Stop, Jump, Move
class CProtoGameEventMoveAll : public CProtoLogicBase
{
	public:
		CProtoGameEventMoveAll();
		~CProtoGameEventMoveAll();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

// Shoot, Hit, Death, Spawn
class CProtoGameEventRule : public CProtoLogicBase
{
	private:
		/* KillInfo Noti위한 후처리 */
		list<CUser*> _allUserListForKillInfo;
		CProtoPacket* _eventPacket;

	public:
		CProtoGameEventRule();
		~CProtoGameEventRule();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
		bool onPostProcess(CSessionManager& session);
};

// Global Notice 에서만 필요할듯함
class CProtoNotiSystem : public CProtoLogicBase 
{
	public:
		CProtoNotiSystem();
		~CProtoNotiSystem();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

class CProtoScoreBoardNoti : public CProtoLogicBase
{
	public:
		CProtoScoreBoardNoti();
		~CProtoScoreBoardNoti();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

class CProtoRequestUserInfo : public CProtoLogicBase 
{
	public:
		CProtoRequestUserInfo();
		~CProtoRequestUserInfo();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

class CProtoSystemActionEvent : public CProtoLogicBase 
{
	private:
		list<string> _scoreList;
	public:
		CProtoSystemActionEvent();
		~CProtoSystemActionEvent();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

typedef CProtoLogicBase* (*CLS_CALLBACK)(bool isPartSend);
CLS_CALLBACK afxCreateClass(int32_t type);
template <class T> CProtoLogicBase* createProtoLogic(bool isPartSend);

void PROTO_MAP_REGISTER(int32_t type, CLS_CALLBACK fnc);
void SEND_PACKET_EVENT(CSessionManager& session, list<CProtoPacket*> packetList);
bool REDIS_SCORE_BOARD_UPDATE(std::string command, std::string key, std::string& value);

#if 0 
{
	if ( !fnc )
	{
		LOG_DEBUG("Create Error CLS_CALLBACK");
		return ;
	}
	g_proto_module_map.insert(std::pair<int32_t, CLS_CALLBACK>(type, fnc));
}
#endif

extern std::map<int32_t, bool> g_partSendMap;
extern std::map<int32_t, CLS_CALLBACK> *g_commandMap;
class CProtoRegister
{
	public:
		CProtoRegister(int32_t type, CLS_CALLBACK fnc, bool isPartSend)
		{
			if ( !g_commandMap )
			{
				g_commandMap = new std::map<int32_t, CLS_CALLBACK>;
			}

			//fnc(isPartSend);
			g_partSendMap.insert(std::pair<int32_t, bool>(type, isPartSend));
			g_commandMap->insert(std::pair<int32_t, CLS_CALLBACK>(type, fnc));
		}
};

bool scoreSort(CUser* first, CUser* second);


#define PROTO_REGISTER(type, isPartSend, fnc) \
	CProtoRegister g_##fnc(type, createProtoLogic<fnc>, isPartSend);
//#define PROTO_REGISTER(type, isPartSend, fnc) PROTO_MAP_REGISTER(type, createProtoLogic<fnc>(isPartSend));

#define PROTO_REGISTER_IDX(type, isPartSend, fnc, index) \
	CProtoRegister g_##fnc##index(type, createProtoLogic<fnc>, isPartSend);
#endif

