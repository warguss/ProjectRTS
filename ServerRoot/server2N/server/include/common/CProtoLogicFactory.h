#ifndef _MODULE_PROTOLOGIC_H_
#define _MODULE_PROTOLOGIC_H_
#include "CSessionManager.h"
#include "CProtoManager.h"
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


// ItemSpawn , ItemGet
class CProtoItemEvent : public CProtoLogicBase
{
	public:
		CProtoItemEvent();
		~CProtoItemEvent();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

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
	public:
		CProtoGameEventRule();
		~CProtoGameEventRule();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
		//bool onPostProcess(CSessionManager& session);
};

// Global Notice 에서만 필요할듯함
class CProtoNotiSystem : public CProtoLogicBase 
{
	public:
		CProtoNotiSystem();
		~CProtoNotiSystem();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

class CProtoRequestUserInfo : public CProtoLogicBase 
{
	public:
		CProtoRequestUserInfo();
		~CProtoRequestUserInfo();

		bool onProcess(CSessionManager& session, CProtoPacket* eventPacket);
};

typedef CProtoLogicBase* (*CLS_CALLBACK)(bool isPartSend);
CLS_CALLBACK afxCreateClass(int32_t type);
template <class T> CProtoLogicBase* createProtoLogic(bool isPartSend);

void PROTO_MAP_REGISTER(int32_t type, CLS_CALLBACK fnc);
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

extern std::map<int32_t, CLS_CALLBACK> *g_commandMap;
class CProtoRegister
{
	public:
		CProtoRegister(int32_t type, CLS_CALLBACK fnc)
		{
			if ( !g_commandMap )
			{
				g_commandMap = new std::map<int32_t, CLS_CALLBACK>;
			}

			g_commandMap->insert(std::pair<int32_t, CLS_CALLBACK>(type, fnc));
		}
};


#define PROTO_REGISTER(type, isPartSend, fnc) \
	CProtoRegister g_##fnc(type, createProtoLogic<fnc>);
//#define PROTO_REGISTER(type, isPartSend, fnc) PROTO_MAP_REGISTER(type, createProtoLogic<fnc>(isPartSend));

#define PROTO_REGISTER_IDX(type, isPartSend, fnc, index) \
	CProtoRegister g_##fnc##index(type, createProtoLogic<fnc>);




#endif











