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
std::map<int32_t, bool> g_partSendMap;
template <class T> CProtoLogicBase* createProtoLogic(bool isPartSend)
{
	CProtoLogicBase* logic = new T();
	LOG_DEBUG("isPartSend ADD(%s)", isPartSend ? "PartSend Create" : "AllSend Create");
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
	_isPartSend = isPartSend;
	_userList.clear();
	_packetOutList.clear();
}

CProtoLogicBase::CProtoLogicBase()
{
	LOG_DEBUG("CProgoLogicBase2");
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
bool CProtoLogicBase::onPreProcess(int32_t eventSector, bool isPartSend)
{
	_isPartSend = isPartSend;
	LOG_DEBUG("Common onPreProcess (%s)", _isPartSend ? "Part Send True" : "All Send True");
	if ( _isPartSend ) 
	{
		g_userPool.getPartUserList(_userList, eventSector);
	}
	else
	{
		g_userPool.getAllUserList(_userList);
	}

	return true;
}
bool CProtoLogicBase::onPreProcess(int32_t eventSector)
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
	if ( _packetOutList.size() <= 0 )
	{
		LOG_ERROR("Not Exist PacketList");
		return false;
	}

	SEND_PACKET_EVENT(session, _packetOutList);

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

bool CProtoConnection::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
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

	CProtoScoreBoardNoti noti;
	noti.onPreProcess(0, allSend);
	noti.onProcess(session, NULL);
	noti.onPostProcess(session);

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
	_eventPacket = eventPacket;

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
	if ( _packetOutList.size() <= 0 )
	{
		LOG_ERROR("Not Exist PacketOutList");
		return false;
	}

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
		for ( int idx = 0; it != _allUserListForKillInfo.end(); it++, idx++ )
		{
			CUser* recvUser = (CUser*)*it;
			CProtoPacket* packet = NULL;
			if ( !g_packetManager.setNotiType(type, recvUser, _eventPacket, &packet, idx) || !packet )
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

		if ( _packetOutList.size() <= 0 )
		{
			LOG_ERROR("Not Exist PacketOutList");
			return false;
		}

		SEND_PACKET_EVENT(session, _packetOutList);
	
		/***************************************
		 * Redis에서 가져와 수치를 비교한다
		 * Key
		 * = score1~4 , String
		 *
		 * Value
		 * = nickname_kill_death , String
		 ***************************************/
		list<CUser*> forRankUpd;
		g_userPool.getAllUserList(forRankUpd);

		forRankUpd.sort(scoreSort);
		list<CUser*>::iterator rankIt = forRankUpd.begin();
		for ( int32_t idx = 1; rankIt != forRankUpd.end(); rankIt++, idx++ )
		{
			if ( idx == 5 )
			{
				break;
			}

			/***********************************
			 * User별 데이터 저장
			 ***********************************/
			CUser* user = (CUser*)*rankIt;
			char psKey[128] = {'\0',};
			sprintf(psKey, "score_%d", idx);
			std::string key = psKey;

			char psValue[256] = {'\0',};
			sprintf(psValue, "%s_%d_%d", user->_nickName.c_str(), user->_killInfo, user->_deathInfo);
			std::string value = psValue;

			REDIS_SCORE_BOARD_UPDATE("SET",  key, value);
		}

		CProtoScoreBoardNoti noti;
		noti.onPreProcess(0, allSend);
		noti.onProcess(session, NULL);
		noti.onPostProcess(session);
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

/***************************************
 * ScoreBoard Logic
 ***************************************/
PROTO_REGISTER((int32_t)server2N::GlobalNotice_NoticeInfo_ScoreBoard, allSend, CProtoScoreBoardNoti);
CProtoScoreBoardNoti::CProtoScoreBoardNoti()
{
	LOG_DEBUG("CProtoScoreBoardNoti");
}

CProtoScoreBoardNoti::~CProtoScoreBoardNoti()
{
	LOG_DEBUG("~CProtoScoreBoardNoti");
}

bool CProtoScoreBoardNoti::onProcess(CSessionManager& session, CProtoPacket* eventPacket)
{
	LOG_DEBUG("CProtoScoreBoardNoti::onProcess()");
	int32_t rankMax = 5;

	/***************************************
	 * Redis Format
	 * Key : score_$idx
	 * value : $nick_$kill_$death
	 ***************************************/
	list<std::string> scoreList;
	for ( int idx = 1; idx < rankMax; idx++ )
	{
		char psKey[128] = {'\0',};
		sprintf(psKey, "score_%d", idx);
		std::string key = psKey;

		std::string value;
		if ( !REDIS_SCORE_BOARD_UPDATE("GET", key, value) )
		{
			LOG_ERROR("Get Redis Error");
			continue ;	
		}

		LOG_INFO("getValue(%s)", value.c_str());
		scoreList.push_back(value.c_str());
	}

	/***********************************************
	 * Proto Set
	 ***********************************************/
	int32_t type = server2N::GlobalNotice_NoticeInfo_ScoreBoard;
	list<CUser*>::iterator it = _userList.begin();
	LOG_DEBUG("UserList Set(%d)", _userList.size());
	for ( ; it != _userList.end(); ++it )
	{
		CUser* user = *it;
		if ( !user )
		{
			continue ;
		}

		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setNotiType(type, user, &packet, scoreList) || !packet )
		{
			LOG_ERROR("Error Connector Type");
			continue;
		} 

		LOG_INFO("User(%d) PushNoti (%s)", packet->_fd, packet->_proto->notice().DebugString().c_str());
		/*********************************
		 * Enqueue
		 *********************************/
		_packetOutList.push_back(packet);
	}
	return true;
}

void SEND_PACKET_EVENT(CSessionManager& session, list<CProtoPacket*> packetList)
{
	if ( packetList.size() <= 0 )
	{
		LOG_ERROR("Not Exist PacketList");
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
	
		LOG_DEBUG("SendPacketEvent (%d)", packet->_fd);
		session.m_writeQ_Manager.unLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Event", packet->_fd);
	}
}

bool REDIS_SCORE_BOARD_UPDATE(std::string command, std::string key, std::string& value)
{
	/***************************************
	 * UserPool에서 User 정보를 꺼내온다
	 ***************************************/
	bool isSuccess = true;
	if ( strcasecmp(command.c_str(), "SET") == 0 )
	{
		if ( !(isSuccess = g_redisManager.setRedis(key, value)) )
		{
			LOG_ERROR("Redis Set Value Error KEY(%s) VALUE(%s)"
					, key.c_str(), value.c_str());
		}
	}
	else if ( strcasecmp(command.c_str(), "GET") == 0 )
	{
		value.clear();
		if ( !(isSuccess = g_redisManager.getRedis(key, value)) )
		{
			LOG_ERROR("Redis Get Value Error KEY(%s)"
					, key.c_str());
		}
	}
	else
	{
		LOG_DEBUG("Not Exist Command(%s) key(%s) value(%s)"
				, command.c_str(), key.c_str(), value.c_str());
		isSuccess = false;
	}

	if ( isSuccess )
	{
		LOG_INFO("Redis Success Command(%s) Key(%s) Value(%s)"
				, command.c_str(), key.c_str(), value.c_str());
	}

	return isSuccess;
}

bool scoreSort(CUser* first, CUser* second)
{
	if ( first->_score < second->_score )
	{
		return false;
	}
	else
	{
		return true;
	}
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
