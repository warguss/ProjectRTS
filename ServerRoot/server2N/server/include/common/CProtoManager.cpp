#include "CProtoManager.h"
#include "CUserPool.h"
#include "CItemManager.h"

extern CUserPool g_userPool;

CProtoManager::CProtoManager()
{
	initialize();
}

CProtoManager::~CProtoManager()
{
	close();
}

void CProtoManager::initialize()
{
	/************************************
	 * Connect
	 ************************************/ 
	_tryConnectCase = new server2N::UserConnection; 	
	_tryConnectCase->set_contype(server2N::UserConnection_ConnectionType_AcceptConnect);
	_tryConnectPacket.set_allocated_connect(_tryConnectCase);

	_connectCase = new server2N::UserConnection;
	_connectCase->set_contype(server2N::UserConnection_ConnectionType_Connect);
	_connectPacket.set_allocated_connect(_connectCase);


	_disConnectCase = new server2N::UserConnection;
	_disConnectCase->set_contype(server2N::UserConnection_ConnectionType_DisConnect);

	/************************************
	 * Loging Setup
	 * Connection의 int To String Map 저장
	 * Connection Int To String
	 ************************************/
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserConnection_ConnectionType_Nothing, (const char*)strdup("Connection_Nothing")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserConnection_ConnectionType_Connect, (const char*)strdup("Connection_Connect")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserConnection_ConnectionType_TryConnect, (const char*)strdup("Connection_TryConnect")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserConnection_ConnectionType_AcceptConnect, (const char*)strdup("Connection_AcceptConnect")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserConnection_ConnectionType_DisConnect, (const char*)strdup("Connection_DisConnect")) );

	/************************************
	 * Loging Setup
	 * Action의 int To String Map 저장
	 * UserEvent Int To String
	 ************************************/
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_Nothing, (const char*)strdup("UserEvent_Nothing")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventMove, (const char*)strdup("UserEvent_EventMove")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventStop, (const char*)strdup("UserEvent_EventStop")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventJump, (const char*)strdup("UserEvent_EventJump")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventShoot, (const char*)strdup("UserEvent_EventShoot")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventHit, (const char*)strdup("UserEvent_EventHit")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventSpawn, (const char*)strdup("UserEvent_EventSpawn")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventUserSync, (const char*)strdup("UserEvent_EventUserSync")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventDeath, (const char*)strdup("UserEvent_EventDeath")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventBullet, (const char*)strdup("UserEvent_EventBullet")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::UserEvent_action_EventChangeWeapon, (const char*)strdup("UserEvent_EventChangeWeapon")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>((int32_t)server2N::UserEvent_action_EventRoll, (const char*)strdup("UserEvent_EventRoll")) );


	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::SystemEvent_action_EventItemSpawn, (const char*)strdup("SystemEvent_EventSpawn")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::SystemEvent_action_EventItemGet, (const char*)strdup("SystemEvent_EventGet")) );

	/************************************
	 * Loging Setup
	 * Noti의 int To String Map 저장
	 * Noti Int To String
	 ************************************/
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::GlobalNotice_NoticeInfo_Nothing, (const char*)strdup("GlobalNotice_Nothing")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::GlobalNotice_NoticeInfo_KillInfo, (const char*)strdup("GlobalNotice_KillInfo")) );
	_userEventMsgMap.insert( std::pair<int32_t, const char*>(server2N::GlobalNotice_NoticeInfo_Notice, (const char*)strdup("GlobalNotice_Notice")) );

}

void CProtoManager::close()
{
	map<int, const char*>::iterator it = _userEventMsgMap.begin();
	for ( ; it != _userEventMsgMap.end(); it++ )
	{
		char* psMsg = (char*)it->second;
		if ( !psMsg )
		{
			continue ;
		}
		else 
		{	
			free(psMsg);
			psMsg = NULL;
		}
	} 
}

#if 0 
bool CProtoManager::createProto(int type, CUser* senderUser, CProtoPacket** packet)
{
	int type;
	(*packet) = new CProtoPacket();

	return true;
}
#endif

bool CProtoManager::encodingHeader(unsigned char* outputBuf, server2N::PacketBody* protoPacket, uint32_t& bodyLength)
{
    /*************************************
     * unsigned Char의 단위 계산을 위해
     * 256 나머지와 나누기를 병행
     *************************************/
	if ( !protoPacket ) 
	{
		LOG_ERROR("Invalid ProtoPacket");
		return false;
	} 

    int bitDigit = 256;
    uint32_t headerSize = (uint32_t)protoPacket->ByteSizeLong();
    bodyLength = headerSize;
    for ( int idx = 0; idx < HEADER_SIZE ; idx++, outputBuf++, headerSize /= bitDigit )
    {
        if ( !outputBuf )
        {
            LOG_ERROR("ERROR Output Buf");
            return false;
        }
        *outputBuf = (unsigned char*)(headerSize % bitDigit);
    }

	return true;
}


bool CProtoManager::encodingBody(unsigned char* buffer, server2N::PacketBody* protoPacket, uint32_t bodyLength)
{
    if ( !protoPacket->SerializeToArray(buffer, bodyLength) )
    {
        LOG_ERROR("Packet SerializeToArray Error");
        return false;
    }

	return true;
}


bool CProtoManager::decodingHeader(unsigned char* buffer, uint32_t bufLength, uint32_t& bodyLength)
{
    if ( bufLength < HEADER_SIZE )
    {
        LOG_ERROR("Packet Size Not Exist Header");
        return false;
    }

    /*******************************************
     * Message구분을 위한, Proto Buffer Ptr이동
     * [0 - 3] Body Length
     * [4 - length] ProtoBuffer
     *******************************************/
	bodyLength = 0;
    int bitDigit = 0;
    for ( int idx = 0 ; idx < HEADER_SIZE ; buffer++, idx++, bitDigit += 8 )
    {
        if ( !buffer )
        {
            LOG_ERROR("Ptr is Null, Packet is Invalid");
            return false;
        }
        bodyLength += (*buffer) << bitDigit;
    }

    return true;
}

const char* CProtoManager::getLogValue(int32_t type, const char* psDefault)
{
	map<int, const char*>::iterator it = _userEventMsgMap.find(type);
	if ( it == _userEventMsgMap.end() )
	{
		return psDefault;
	}

	const char* psMsg = (const char*)it->second;
	if ( !psMsg )
	{
		return psDefault;
	}

	return (const char*)psMsg;
} 

bool CProtoManager::decodingBody(unsigned char* buffer, uint32_t bufLength, uint32_t bodyLength, CProtoPacket** protoPacket)
{
    if ( bufLength != bodyLength || !buffer )
    {
        LOG_ERROR("Error length or Buffer Not Exist");
        return false;
    }

	(*protoPacket) = new CProtoPacket();
    google::protobuf::io::CodedInputStream is(buffer, (int)bodyLength);
    if ( !(*protoPacket)->_proto->MergeFromCodedStream(&is) )
    {
        LOG_ERROR("Error CodedStream");
        return false;
    }

    // 디버깅 용도, 있는지 없는 지만 체크하도 리턴해도 충분하다.
	bool isSuccess = true;
    if ( (*protoPacket)->_proto->has_event() )
    {
        server2N::GameEvent tEvent = (*protoPacket)->_proto->event();
        LOG_DEBUG("Has Event (%s)", tEvent.DebugString().c_str());
		int32_t type;
		if ( tEvent.evttype() == server2N::GameEvent_eventType_UserEvent )
		{
			type = tEvent.userevent().acttype();
		}
		else if ( tEvent.evttype() == server2N::GameEvent_eventType_SystemEvent )
		{
			type = tEvent.systemevent().acttype();
		}
		(*protoPacket)->_type = type;
    }
    else if ( (*protoPacket)->_proto->has_connect() )
    {
        server2N::UserConnection tConnect = (*protoPacket)->_proto->connect();
        LOG_DEBUG("Has Connect (%s)", tConnect.DebugString().c_str());
		(*protoPacket)->_type = tConnect.contype();
    }

	else if ( (*protoPacket)->_proto->has_notice() )
    {
        server2N::GlobalNotice tNoti = (*protoPacket)->_proto->notice();
        LOG_DEBUG("Has Notice (%s)", tNoti.DebugString().c_str());
		(*protoPacket)->_type = tNoti.notitype();
    }
    else
    {
        LOG_ERROR("Not Exist ProtoBuffer");
		isSuccess = false;
    }

    return isSuccess;
}

bool CProtoManager::setConnectType(int32_t type, CUser* eventUser, int32_t fd, list<CUser*> allUser, CProtoPacket** packet)
{
	if ( type < 0 || fd <= 0 || !eventUser || eventUser->_fd <= 0 )
	{
		LOG_ERROR("Not Exist User , User ProtoPacket fd(%d) type(%d) isExist Event(%s)", fd, type, (eventUser) ? "Exist" : "Not Exist");
		return false;
	}

	(*packet) = new CProtoPacket();
	(*packet)->_protoConnect = new server2N::UserConnection; 
	(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_UserConnection);
	(*packet)->_nickName = eventUser->_nickName;
	(*packet)->_act = getLogValue(type, "Invalid Connection");
	if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect )
	{
		LOG_INFO("USERID(%d) TRYCONNECT Change To AccepConnect", eventUser->_fd);
		(*packet)->_fd = eventUser->_fd;
		(*packet)->_proto->set_senderid(eventUser->_fd);
		(*packet)->_type = server2N::PacketBody_messageType_UserConnection;

		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_AcceptConnect);
		(*packet)->_protoConnect->add_connectorid(eventUser->_fd);
		(*packet)->_protoConnect->add_killinfo(eventUser->_killInfo);
		(*packet)->_protoConnect->add_deathinfo(eventUser->_deathInfo);
		(*packet)->_protoConnect->add_nickname(eventUser->_nickName.c_str());
		
		(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
	}
	else if ( type == (int32_t)server2N::UserConnection_ConnectionType_Connect )
	{
		LOG_INFO("USERID(%d) TRYCONNECT Change To CONNECT", eventUser->_fd);
		(*packet)->_type = server2N::PacketBody_messageType_UserConnection;
		(*packet)->_fd = fd;
		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_Connect);

		/**************************************
		 * id에 대해 list조회 전달
		 * (신규 유저일 경우 조회전달)
		 **************************************/
		if ( eventUser->_fd == fd ) 
		{
			list<CUser*>::iterator it = allUser.begin(); 
			if ( allUser.end() == it )
			{
				return true;
			} 

			/* Add 시점 생각해야할듯, 아니며 여기서 자기자신 예외처리필요함 */
			for ( ; it != allUser.end(); it++ )
			{
				CUser* user = (CUser*)*it;
				if ( user->_fd == fd )
				{
					continue ; 
				} 
				(*packet)->_protoConnect->add_connectorid((int32_t)user->_fd);
				(*packet)->_protoConnect->add_killinfo((int64_t)user->_killInfo);
				(*packet)->_protoConnect->add_deathinfo((int64_t)user->_deathInfo);
				(*packet)->_protoConnect->add_nickname(user->_nickName.c_str());
			}
			/* Add Item Id */
			map<std::string, Item*>::iterator iter = g_itemManager._itemInfo.begin();	
			if ( iter != g_itemManager._itemInfo.end() )
			{
				for ( ; iter != g_itemManager._itemInfo.end(); iter++ )
				{
					Item* item = iter->second;
					if ( !item )
					{
						continue ;
					}	

					server2N::InfoItem *protoItem = (*packet)->_protoConnect->add_item();
					if ( protoItem )
					{
						LOG_ERROR("Not Exist protoITem");
						std::string itemID = iter->first;
						protoItem->set_itemid(itemID.c_str());
						protoItem->set_amount(item->_amount);
						protoItem->set_itemtype(item->_itemType);
						protoItem->set_weaponid(item->_weaponId);
						protoItem->set_itempositionx(item->_posX);
						protoItem->set_itempositiony(item->_posY);
					}
					else
					{
						LOG_ERROR("Not Exist protoITem");
					}
				} 
			}
			(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
		}
		else
		{
			/**************************************
			 * 기존 유저일 경우 eventFd만 전달
			 **************************************/
			(*packet)->_protoConnect->add_connectorid((int32_t)eventUser->_fd);
			(*packet)->_protoConnect->add_killinfo((int64_t)eventUser->_killInfo);
			(*packet)->_protoConnect->add_deathinfo((int64_t)eventUser->_deathInfo);
			(*packet)->_protoConnect->add_nickname(eventUser->_nickName.c_str());
			(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
		}
	}
	else if ( type == (int32_t)server2N::UserConnection_ConnectionType_DisConnect )
	{
		/**************************************
		 * 기존 유저일 경우 eventFd만 전달
		 **************************************/
		LOG_DEBUG("USERID(%d) DisConnect", eventUser->_fd);
		(*packet)->_fd = fd;
		(*packet)->_proto->set_senderid(fd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_UserConnection);
		(*packet)->_type = (int32_t)server2N::UserConnection_ConnectionType_DisConnect;
		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_DisConnect);
		(*packet)->_protoConnect->add_connectorid(eventUser->_fd);
		
		(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
	}
	else
	{
		LOG_ERROR("Error Not Exist Type");
		if ( *packet ) 
		{
	
			delete (*packet)->_protoConnect;
			(*packet)->_protoConnect = NULL;

			delete *packet;
			*packet = NULL;
		}
		return false;
	}

	LOG_DEBUG("Send Packet Set(%s)", (*packet)->_proto->DebugString().c_str());
	return true;
}

void CProtoManager::resetProtoPacket(CProtoPacket* protoPacket)
{
	if ( protoPacket )
	{
		LOG_ERROR("Reset ProtoPacket Delete");
		delete protoPacket;
	}
	protoPacket = NULL;
}

bool CProtoManager::setActionType(int32_t type, CUser* recvUser, CProtoPacket* eventUser, list<CUser*> userList, CProtoPacket** packet)
{
	if ( type < 0 || !eventUser || eventUser->_fd <= 0 || !recvUser || recvUser->_fd <= 0 )
	{
		LOG_ERROR("Not Exist User , User ProtoPacket");
		return false;
	}

	bool isSelfEvent = (recvUser->_fd == eventUser->_fd) ? true : false;
	
	(*packet) = new CProtoPacket();
	(*packet)->_protoEvent = new server2N::GameEvent();
	server2N::GameEvent tEvent = eventUser->_proto->event(); 
	if ( type == (int32_t)server2N::UserEvent_action_EventShoot )
	{
		LOG_DEBUG("Type EventShoot");
	}
	else if ( type == (int32_t)server2N::UserEvent_action_EventHit )
	{
		LOG_DEBUG("Type EventHit");
	}
	else if ( type == (int32_t)server2N::UserEvent_action_EventDeath )
	{
		/******************************
		 * Trigger Id 확인
		 * Kill Info 갱신 및 노티 필요
		 ******************************/
		LOG_DEBUG("Type EventDeath");
	}
	else if ( type == (int32_t)server2N::UserEvent_action_EventUserSync || type == (int32_t)server2N::UserEvent_action_EventMove || type == (int32_t)server2N::UserEvent_action_EventStop || type == (int32_t)server2N::UserEvent_action_EventJump || type == (int32_t)server2N::UserEvent_action_EventSpawn || type == (int32_t)server2N::UserEvent_action_EventRoll )
	{
		LOG_DEBUG("Check Move(%d), event(%d)", recvUser->_fd, eventUser->_fd);
		if ( isSelfEvent )
		{
			recvUser->_x = (float)tEvent.eventpositionx();
			recvUser->_y = (float)tEvent.eventpositiony();
			recvUser->_accelX = tEvent.velocityx();
			recvUser->_accelY = tEvent.velocityy();
			bool isSuccess = true;
			do
			{
				int preSector = recvUser->_sector;
				int sector = g_userPool.getSectionNo(recvUser);
				LOG_DEBUG("change Sector preSector(%d) sector(%d)", preSector, sector);
				if ( preSector != sector )
				{
					if ( !g_userPool.changeUserInPool(recvUser, preSector, sector) )
					{
						LOG_ERROR("Change User In Pool Error");
						isSuccess = false;
						break;
					} 
				}
			}
			while(false);
		}

		tEvent.set_isinterested(recvUser->_isInterested);
	}
	else if ( type == (int32_t)server2N::SystemEvent_action_EventItemSpawn )
	{
		const char* itemKey = tEvent.systemevent().itemspawnevent().item().itemid().c_str();
		Item* item = new Item();
		item->_excellX = tEvent.velocityx();
		item->_excellY = tEvent.velocityy();
		item->_posX = tEvent.eventpositionx();
		item->_posY = tEvent.eventpositiony();
		item->_weaponId = tEvent.systemevent().itemspawnevent().item().weaponid();
		item->_itemType = tEvent.systemevent().itemspawnevent().item().itemtype()  ;
		item->_amount = tEvent.systemevent().itemspawnevent().item().amount();

		if ( !g_itemManager.spawnItem(itemKey, item) )
		{
			LOG_ERROR("Spawn add Item Error");
		} 
		LOG_DEBUG("Event Item Spawn (%s)", itemKey);
	} 
	else if ( type == (int32_t)server2N::SystemEvent_action_EventItemGet )
	{
		/**************************************
		 * 먼저 먹은 사람이 있을 경우, 
		 * 해당 로직에서 걸러준다.
		 **************************************/
		if ( isSelfEvent )
		{
			const char* itemKey = tEvent.systemevent().itemgetevent().item().itemid().c_str();
			LOG_DEBUG("Event Item Get (%s)", itemKey);
			if ( !itemKey || !g_itemManager.userGetItem(itemKey) )
			{
				LOG_DEBUG("Already Event Item Get (%s)", itemKey);
				return false;
			}
		}	
	}
	else
	{
		return false;
	}

	if ( !isSelfEvent || type == (int32_t)server2N::SystemEvent_action_EventItemGet || type == (int32_t)server2N::SystemEvent_action_EventItemSpawn)
	{
		(*packet)->_protoEvent->CopyFrom(tEvent);
		(*packet)->_fd = recvUser->_fd;
		(*packet)->_proto->set_senderid(recvUser->_fd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_GameEvent);
		(*packet)->_type = (int32_t)type;
		(*packet)->_proto->set_allocated_event((*packet)->_protoEvent);
		(*packet)->_act = getLogValue(type, "Invalid Action");
		(*packet)->_nickName = eventUser->_nickName;
		LOG_DEBUG("Send Proto(%d) Sending debugString(%s)", (*packet)->_fd, tEvent.DebugString().c_str());
	}
	else
	{
		LOG_DEBUG("is Self Check? recvFd(%d), EventFd(%d)", recvUser->_fd, eventUser->_fd);
		delete (*packet)->_protoEvent;
		(*packet)->_protoEvent = NULL;

		delete (*packet);
		(*packet) = NULL;
	} 
	
	return true;
}

bool CProtoManager::setNotiType(int type, CUser* recvUser, CProtoPacket* eventUser, CProtoPacket** packet)
{
	if ( type < 0 || !eventUser || eventUser->_fd <= 0 || recvUser <= 0 )
	{
		LOG_ERROR("Not Exist User , User ProtoPacket");
		return false;
	}

	bool isSuccess = true;
	(*packet) = new CProtoPacket();
	(*packet)->_protoNoti = new server2N::GlobalNotice();
	do 
	{
		if ( type == (int32_t)server2N::UserEvent_action_EventDeath )
		{
			type = (int32_t)server2N::GlobalNotice_NoticeInfo_KillInfo;
			server2N::GameEvent tEvent = eventUser->_proto->event(); 
			/********************************
			 * Kill 
			 * eventUser_fd -> victim
			 * A User KillInfo Add
			 * B User DeathInfo Add
			 ********************************/
			int32_t eventFd = eventUser->_fd;
			CUser* victimUser = g_userPool.findUserInPool(eventFd);
			if ( !victimUser ) 
			{
				/*********************************
				 * 전체 조회이기 때문에, 
				 * 찾지 못했다면 close(fd)로 종료
				 *********************************/
				LOG_ERROR("Not Exist VictimUser(%d)\n", victimUser->_fd);
				isSuccess = false;
				break;
			}
			(*packet)->_protoNoti->add_victim((int32_t)eventFd);
			victimUser->_deathInfo++;
			int triggerFd = tEvent.userevent().deathevent().triggerid();
			CUser* triggerUser = g_userPool.findUserInPool(triggerFd);
			if ( !triggerUser )
			{
				LOG_ERROR("Not Exist TriggerUser(%d)\n", triggerUser->_fd);
				isSuccess = false;
				break;
			}
			(*packet)->_protoNoti->set_performer((int32_t)triggerFd);
			triggerUser->_killInfo++;
		}
	}
	while(false);

	if ( isSuccess )
	{
		(*packet)->_type = type;
		(*packet)->_fd = recvUser->_fd;
		(*packet)->_protoNoti->set_notitype((int32_t)type);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_GlobalNotice);
		(*packet)->_proto->set_allocated_notice((*packet)->_protoNoti);
		(*packet)->_act = getLogValue(type, "Invalid Noti");
		(*packet)->_nickName = eventUser->_nickName;
	}
	else
	{
		delete (*packet)->_protoNoti;
		(*packet)->_protoNoti = NULL;

		delete (*packet);
		(*packet) = NULL;
	}

	return isSuccess;
}

