#include "CProtoLogicFactory.h"
#include "CProtoPacket.h"
#include "CItemManager.h"
#include "CUserPool.h"

using namespace std;
extern CUserPool g_userPool;
std::map<int32_t, CLS_CALLBACK> *g_commandMap;
template <class T> CProtoLogicBase* createProtoLogic(bool isPartSend)
{
	CProtoLogicBase* logic = new T();
	logic->_isPartSend = isPartSend;
	return logic;
}

CLS_CALLBACK afxCreateClass(int32_t type)
{
	std::map<int32_t, CLS_CALLBACK>::iterator iter = g_commandMap->find(type);
	if ( iter == g_commandMap->end() )
	{
		return NULL;
	}
		
	return iter->second;
}

CProtoLogicBase::CProtoLogicBase(bool isPartSend)
{
	LOG_DEBUG("CProgoLogicBase1");
	//_isPartSend = isPartSend;
	_userList.clear();
	_packetOutList.clear();
}

CProtoLogicBase::CProtoLogicBase()
{
	LOG_DEBUG("CProgoLogicBase2");
	_isPartSend = false;
	_userList.clear();
	_packetOutList.clear();
}

CProtoLogicBase::~CProtoLogicBase()
{
	LOG_DEBUG("~CProgoLogicBase3");
	_userList.clear();
	_packetOutList.clear();
}

/***********************************
 * Pre 단계 User List Setup
 ***********************************/ 
bool CProtoLogicBase::onPreProcess(int eventSector)
{
	LOG_DEBUG("Common onPreProcess (%s)", _isPartSend ? "Part Send True" : "All Send True");
	if ( _isPartSend  ) 
	{
		g_userPool.getPartUserList(_userList, eventSector);
	}
	else
	{
		g_userPool.getAllUserList(_userList);
	}

	return true;
}

bool CProtoLogicBase::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("BaseLogic, Virtual Function")
	return true;
}

/***********************************
 * Post 단계 User Packet Send 
 ***********************************/ 
bool CProtoLogicBase::onPostProcess(CSessionManager& session)
{
	LOG_DEBUG("Common onPostProces");
	list<CProtoPacket*>::iterator it = _packetOutList.begin();
	for ( ; it != _packetOutList.end() ; ++it )
	{
		CProtoPacket* packet = (CProtoPacket*)*it;
		if ( !packet )
		{
			continue ;
		}
	
		session.m_writeQ_Manager.unLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Event", packet->_fd);
	}

	return true;
}

/***************************************
 * 1. A Client -> Server -> A Client 
 * (TryConnect -> Connect 바꿔서 던짐) 
 * 
 * 2. Server -> All Client 
 * (All Connect 전송 유저 접속 판단 위함)
 ***************************************/
PROTO_REGISTER((int32_t)server2N::UserConnection_ConnectionType_TryConnect, false, CProtoConnection);
CProtoConnection::CProtoConnection()
{
	LOG_DEBUG("CProgoLogicBase")
}


CProtoConnection::~CProtoConnection()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoConnection::onProcess(CSessionManager& manager, CProtoPacket* eventPacket)
{
	int32_t type = eventPacket->_type;
	CUser* eventUser = g_userPool.findUserInPool(eventPacket->_fd);
	if ( !eventUser ) 
	{
		LOG_ERROR("Try Conenct Not Exist EventUser");
		return false;
	}
	
	CProtoPacket *connectPacket = NULL;
	if ( !g_packetManager.setConnectType(type, eventUser, eventPacket->_fd, _userList, &connectPacket) || !connectPacket )
	{
		LOG_ERROR("Error Connect Packet Type");
		return false;
	}
	_packetOutList.push_back(connectPacket);

	/******************************************************
	 * Convet TryConnect -> Connect
	 ******************************************************/
	type = server2N::UserConnection_ConnectionType_Connect;
	int userConnectSize = _userList.size(); 
	list<CUser*>::iterator it = _userList.begin();
	for ( ; it != _userList.end(); ++it )
	{
		CUser* user = *it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setConnectType(type, eventUser, user->_fd, _userList, &packet) ||  !packet )
		{
			LOG_ERROR("Error Connector Type");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		_packetOutList.push_back(packet);
		LOG_INFO("User(%d) All Send Connection Event", packet->_fd);
	}

	return true;
}

#if 0 
CProtoTryConnection::CProtoTryConnection()
{
	LOG_DEBUG("CProgoLogicBase")
}

CProtoTryConnection::~CProtoTryConnection()
{
	LOG_DEBUG("~CProgoLogicBase")
}


bool CProtoTryConnection::onPreProcess()
{
	LOG_DEBUG("onPreProcess")
}

bool CProtoTryConnection::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{





}

bool CProtoTryConnection::onPostProcess()
{
	LOG_DEBUG("onProcess")
}
#endif

CProtoDisConnection::CProtoDisConnection()
{
	LOG_DEBUG("CProgoLogicBase")
}

CProtoDisConnection::~CProtoDisConnection()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoDisConnection::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess")
}

CProtoItemEvent::CProtoItemEvent()
{
	LOG_DEBUG("CProgoLogicBase")
}


CProtoItemEvent::~CProtoItemEvent()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoItemEvent::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess")
}

CProtoGameEventMoveAll::CProtoGameEventMoveAll()
{
	LOG_DEBUG("CProgoLogicBase")
}


CProtoGameEventMoveAll::~CProtoGameEventMoveAll()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoGameEventMoveAll::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess")
}

CProtoGameEventSpawn::CProtoGameEventSpawn()
{
	LOG_DEBUG("CProgoLogicBase")
}


CProtoGameEventSpawn::~CProtoGameEventSpawn()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoGameEventSpawn::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess")
}

CProtoNotiKillInfo::CProtoNotiKillInfo()
{
	LOG_DEBUG("CProgoLogicBase")
}


CProtoNotiKillInfo::~CProtoNotiKillInfo()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoNotiKillInfo::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess")
}

CProtoNotiSystem::CProtoNotiSystem()
{
	LOG_DEBUG("CProgoLogicBase")
}


CProtoNotiSystem::~CProtoNotiSystem()
{
	LOG_DEBUG("~CProgoLogicBase")
}

bool CProtoNotiSystem::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess")
}

#if 0 
void PROTO_MAP_REGISTER(int32_t type, CLS_CALLBACK fnc)
{
	if ( !fnc )
	{
		LOG_DEBUG("Create Error CLS_CALLBACK");
		return ;
	}
	g_proto_module_map.insert(std::pair<int32_t, CLS_CALLBACK>(type, fnc));
}
#endif




