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
	LOG_DEBUG("BaseLogic, Virtual Function");
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
	LOG_DEBUG("CProgoLogicBase");
}


CProtoConnection::~CProtoConnection()
{
	LOG_DEBUG("~CProgoLogicBase");
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
			continue;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		_packetOutList.push_back(packet);
		LOG_INFO("User(%d) All Send Connection Event", packet->_fd);
	}

	return true;
}

PROTO_REGISTER((int32_t)server2N::UserConnection_ConnectionType_DisConnect, false, CProtoDisConnection);
CProtoDisConnection::CProtoDisConnection()
{
	LOG_DEBUG("CProgoLogicBase");
}

CProtoDisConnection::~CProtoDisConnection()
{
	LOG_DEBUG("~CProgoLogicBase");
}

bool CProtoDisConnection::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess");
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

	/******************************************************
	 * Send Disconnect Packet
	 ******************************************************/
	int userConnectSize = _userList.size(); 
	list<CUser*>::iterator it = _userList.begin();
	for ( ; it != _userList.end(); ++it )
	{
		CUser* user = *it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setConnectType(type, eventUser, user->_fd, _userList, &packet) ||  !packet )
		{
			LOG_ERROR("Error Connector Type");
			continue;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		_packetOutList.push_back(packet);
		LOG_INFO("User(%d) All Send Connection Event", packet->_fd);
	}
	return true;
}

CProtoItemEvent::CProtoItemEvent()
{
	LOG_DEBUG("CProgoLogicBase");
}


CProtoItemEvent::~CProtoItemEvent()
{
	LOG_DEBUG("~CProgoLogicBase");
}

bool CProtoItemEvent::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess");
}

/***************************************
 * MoveAll
 * Move Action Class
 ***************************************/
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_Nothing, true, CProtoGameEventMoveAll, 0);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventMove, true, CProtoGameEventMoveAll, 1);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventStop, true, CProtoGameEventMoveAll, 2);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventJump, true, CProtoGameEventMoveAll, 3);
CProtoGameEventMoveAll::CProtoGameEventMoveAll()
{
	LOG_DEBUG("CProgoLogicBase");
}


CProtoGameEventMoveAll::~CProtoGameEventMoveAll()
{
	LOG_DEBUG("~CProgoLogicBase");
}

bool CProtoGameEventMoveAll::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess");
	int32_t type = eventPacket->_type;
	list<CUser*>::iterator it = _userList.begin();
	for ( ; it != _userList.end(); it++ )
	{
		CUser* recvUser = (CUser*)*it;
		CProtoPacket* packet = NULL;
		if ( !g_packetManager.setActionType(type, recvUser, eventPacket, _userList, &packet) || !packet )
		{
			LOG_ERROR("Error Move Type");
			continue ;
		}
		
		/********************************
		 * Enqueue
		 ********************************/
		_packetOutList.push_back(packet);
		LOG_INFO("User(%d) Part Send Move Event", packet->_fd);
	}

	return true;
}

/***************************************
 * Kill Info Noti 
 ***************************************/
CProtoNotiSystem::CProtoNotiSystem()
{
	LOG_DEBUG("CProgoLogicBase");
}

CProtoNotiSystem::~CProtoNotiSystem()
{
	LOG_DEBUG("~CProgoLogicBase");
}

bool CProtoNotiSystem::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess");
}

/***************************************
 * For Cam Request
 * (Response Position)
 * 1:1 Request - Response
 ***************************************/
PROTO_REGISTER((int32_t)server2N::SystemEvent_action_RequestUserInfo, false, CProtoRequestUserInfo);
CProtoRequestUserInfo::CProtoRequestUserInfo()
{
	LOG_DEBUG("CProtoRequestUserInfo");
}

CProtoRequestUserInfo::~CProtoRequestUserInfo()
{
	LOG_DEBUG("~CProtoRequestUserInfo");
}

bool CProtoRequestUserInfo::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoRequestUserInfo::onProcess()");
	if ( !eventPacket || !eventPacket->_proto )
	{
		LOG_ERROR("Invalid Data Foramt");
		return false;
	}

	int getCamUserFd = eventPacket->_proto->event().systemevent().requestuserinfo().targetid();
	CUser* userInfo = g_userPool.findUserInPool(getCamUserFd);
	if ( !userInfo )
	{
		LOG_ERROR("Not Exist In Map");
		return false;
	}
	
	LOG_DEBUG("float Set X (%lld) Set y (%lld)", userInfo->_x, userInfo->_y);

	eventPacket->_proto->event().systemevent().requestuserinfo().set_eventpositionx(userInfo->_x);
	eventPacket->_proto->event().systemevent().requestuserinfo().set_eventpositiony(userInfo->_y);

	_packetOutList.push_back(eventPacket);
	LOG_DEBUG("Send Proto(%d) Sending debugString(%s)", eventPacket->_fd, eventPacket->_proto->event().DebugString().c_str());
	return true;
}
	
// Shoot | Hit | Death | Spawn
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventShoot, true, CProtoGameEventRule, 0);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventHit, true, CProtoGameEventRule, 1);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventDeath, true, CProtoGameEventRule, 2);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventSpawn, true, CProtoGameEventRule, 3);
CProtoGameEventRule::CProtoGameEventRule()
{
	LOG_DEBUG("CProtoGameEventRule");
}

CProtoGameEventRule::~CProtoGameEventRule()
{
	LOG_DEBUG("CProtoGameEventRule");
}

bool CProtoGameEventRule::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("onProcess");
	int32_t type = eventPacket->_type;
	list<CUser*>::iterator it = _userList.begin();
	for ( ; it != _userList.end(); it++ )
	{
		CUser* recvUser = (CUser*)*it;
		CProtoPacket* packet = NULL;
		if ( !g_packetManager.setActionType(type, recvUser, eventPacket, _userList, &packet) || !packet )
		{
			LOG_ERROR("Error Move Type");
			continue ;
		}
		
		/********************************
		 * Enqueue
		 ********************************/
		_packetOutList.push_back(packet);

		/******************************************
		 * Noti의 경우에는 추가로 보낸다
		 * Death일때, type을 바꿔서 KillInfo 보냄
		 ******************************************/
		if ( _type == (int32_t)server2N::UserEvent_action_EventDeath )
		{
			CProtoPacket *notiPacket = NULL;
			if ( !g_packetManager.setNotiType(type, user, eventUser, connectList, &notiPacket) || !notiPacket )
			{
				LOG_ERROR("Error Noti All Type");
				continue ;
			}
			
			_packetOutList.push_back(notiPacket);
		}
		LOG_INFO("User(%d) Part Send Move Event", packet->_fd);
	}

	return true;
}

// Sync


// Item



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
