#include "CProtoLogicFactory.h"
#include "CProtoPacket.h"
#include "CItemManager.h"
#include "CUserPool.h"

bool partSend = true;
bool allSend = false;

using namespace std;
extern CUserPool g_userPool;
extern CCustomRedisManager g_redisManager;
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
	SEND_PACKET_EVENT(session, _packetOutList);
	/*
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
	*/

	return true;
}

/***************************************
 * 1. A Client -> Server -> A Client 
 * (TryConnect -> Connect 바꿔서 던짐) 
 * 
 * 2. Server -> All Client 
 * (All Connect 전송 유저 접속 판단 위함)
 ***************************************/
PROTO_REGISTER((int32_t)server2N::UserConnection_ConnectionType_TryConnect, allSend, CProtoConnection);
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

PROTO_REGISTER((int32_t)server2N::UserConnection_ConnectionType_DisConnect, allSend, CProtoDisConnection);
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

#if 0
CProtoItemEvent::CProtoItemEvent()
{
	LOG_DEBUG("CProtoItemEvent");
}

CProtoItemEvent::~CProtoItemEvent()
{
	LOG_DEBUG("~CProtoItemEvent");
}

bool CProtoItemEvent::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoItemEvent::onProcess");
}
#endif

/***************************************
 * MoveAll
 * Move Action Class
 ***************************************/
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_Nothing, partSend, CProtoGameEventMoveAll, 0);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventMove, partSend, CProtoGameEventMoveAll, 1);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventStop, partSend, CProtoGameEventMoveAll, 2);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventJump, partSend, CProtoGameEventMoveAll, 3);
CProtoGameEventMoveAll::CProtoGameEventMoveAll()
{
	LOG_DEBUG("CProtoGameEventMoveAll");
}

CProtoGameEventMoveAll::~CProtoGameEventMoveAll()
{
	LOG_DEBUG("~CProtoGameEventMoveAll");
}

bool CProtoGameEventMoveAll::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoGameEventMoveAll::onProcess");
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
PROTO_REGISTER_IDX((int32_t)server2N::GlobalNotice_NoticeInfo_Notice, allSend, CProtoNotiSystem, 0);
CProtoNotiSystem::CProtoNotiSystem()
{
	LOG_DEBUG("CProtoNotiSystem");
}

CProtoNotiSystem::~CProtoNotiSystem()
{
	LOG_DEBUG("~CProtoNotiSystem");
}

bool CProtoNotiSystem::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoNotiSystem::onProcess()");
	int32_t type = eventPacket->_type;
	list<CUser*>::iterator it = _userList.begin();
	for ( ; it != _userList.end(); it++ )
	{
		CUser* recvUser = (CUser*)*it;
		CProtoPacket* packet = NULL;
		if ( !g_packetManager.setNotiType(type, recvUser, eventPacket, &packet) || !packet )
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
 * For Cam Request
 * (Response Position)
 * 1:1 Request - Response
 ***************************************/
PROTO_REGISTER((int32_t)server2N::SystemEvent_action_RequestUserInfo, allSend, CProtoRequestUserInfo);
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
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventShoot, partSend, CProtoGameEventRule, 0);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventHit, partSend, CProtoGameEventRule, 1);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventDeath, partSend, CProtoGameEventRule, 2);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventSpawn, partSend, CProtoGameEventRule, 3);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventUserSync, partSend, CProtoGameEventRule, 4);
PROTO_REGISTER_IDX((int32_t)server2N::UserEvent_action_EventRoll, partSend, CProtoGameEventRule, 5);
CProtoGameEventRule::CProtoGameEventRule()
{
	LOG_DEBUG("CProtoGameEventRule");
	_eventPacket = NULL;
	_allUserListForKillInfo.clear();
}

CProtoGameEventRule::~CProtoGameEventRule()
{
	LOG_DEBUG("CProtoGameEventRule");
}

bool CProtoGameEventRule::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoGameEventRule::onProcess");
	int32_t type = eventPacket->_type;
	if ( type == (int32_t)server2N::UserEvent_action_EventDeath )
	{
		_eventPacket = eventPacket;
	}

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

bool CProtoGameEventRule::onPostProcess(CSessionManager& session)
{
	LOG_DEBUG("GameEventRule PostProcess");
	int32_t type = _eventPacket->_type;
	SEND_PACKET_EVENT(session, _packetOutList);

	/******************************************
	 * Noti의 경우에는 추가로 보낸다
	 * Death일때, type을 바꿔서 KillInfo 보냄
	 * KillInfo는 All로 보내야한다.
	 ******************************************/
	if ( type == (int32_t)server2N::UserEvent_action_EventDeath )
	{
		_packetOutList.clear();
		list<CUser*> _allUserListForKillInfo;
		g_userPool.getAllUserList(_allUserListForKillInfo);

		list<CUser*>::iterator it = _allUserListForKillInfo.begin();
		for ( ; it != _allUserListForKillInfo.end(); it++ )
		{
			CUser* recvUser = (CUser*)*it;
			CProtoPacket* packet = NULL;
			if ( !g_packetManager.setNotiType(type, recvUser, _eventPacket, &packet) || !packet )
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

		SEND_PACKET_EVENT(session, _packetOutList);
		REDIS_SCORE_BOARD_UPDATE(0);
	}


	return true;
}

// Item
PROTO_REGISTER_IDX((int32_t)server2N::SystemEvent_action_EventItemSpawn, allSend, CProtoSystemActionEvent, 0);
PROTO_REGISTER_IDX((int32_t)server2N::SystemEvent_action_EventItemGet, allSend, CProtoSystemActionEvent, 1);
CProtoSystemActionEvent::CProtoSystemActionEvent()
{
	LOG_DEBUG("CProtoSystemActionEvent");
	//_eventPacket = NULL;
	//_allUserListForKillInfo.clear();
}

CProtoSystemActionEvent::~CProtoSystemActionEvent()
{
	LOG_DEBUG("CProtoSystemActionEvent");
}

bool CProtoSystemActionEvent::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoSystemActionEvent::onProcess");
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


void SEND_PACKET_EVENT(CSessionManager& session, list<CProtoPacket*> packetList)
{
	if ( packetList.size() <= 0 )
	{
		return ;
	}

	list<CProtoPacket*>::iterator it = packetList.begin();
	for ( ; it != packetList.end() ; ++it )
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
}

void REDIS_SCORE_BOARD_UPDATE(int32_t performerFd)
{
	/***************************************
	 * UserPool에서 User 정보를 꺼내온다
	 ***************************************/


	/***************************************
	 * User의 KillInfo 수치를 계산한다
	 * Kill = 2, Death = 1
	 ***************************************/


	/***************************************
	 * Redis에서 가져와 수치를 비교한다
	 * Key
	 * = score1~10 , String
	 *
	 * Value
	 * = nickname_kill_death , String
	 ***************************************/

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
