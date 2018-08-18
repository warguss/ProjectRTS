#include "CProtoManager.h"
#include "CUserPool.h"

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
	 * DisConnect
	 ************************************/
	//_disConnectCase = new server2N::UserConnection;
}

void CProtoManager::close()
{
}

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
    LOG_DEBUG("Header Size:%d", headerSize);
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

    LOG_INFO("bodyLength [%d]", bodyLength);
    return true;
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
    if ( (*protoPacket)->_proto->has_event() )
    {
        server2N::GameEvent tEvent = (*protoPacket)->_proto->event();
        LOG_DEBUG("Has Event (%s)", tEvent.DebugString().c_str());
		(*protoPacket)->_type = tEvent.acttype();

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
        return false;
    }

    return true;
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
	if ( type == (int32_t)server2N::GameEvent_action_EventShoot )
	{
		LOG_DEBUG("Type EventShoot");
	}
	else if ( type == (int32_t)server2N::GameEvent_action_EventHit )
	{
		LOG_DEBUG("Type EventHit");
	}
	else if ( type == (int32_t)server2N::GameEvent_action_EventDeath )
	{
		/******************************
		 * Trigger Id 확인
		 * Kill Info 갱신 및 노티 필요
		 ******************************/
		LOG_DEBUG("Type EventDeath");
	}
	else if ( type == (int32_t)server2N::GameEvent_action_EventUserSync || type == (int32_t)server2N::GameEvent_action_EventMove || type == (int32_t)server2N::GameEvent_action_EventStop || type == (int32_t)server2N::GameEvent_action_EventJump || type == (int32_t)server2N::GameEvent_action_EventSpawn )
	{
		LOG_DEBUG("Check Move(%d), event(%d)", recvUser->_fd, eventUser->_fd);
		if ( isSelfEvent )
		{
			recvUser->_x = tEvent.eventpositionx();
			recvUser->_y = tEvent.eventpositiony();
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
	}

	tEvent.set_sectorno(eventUser->_sector);
	if ( !isSelfEvent )
	{
		(*packet)->_protoEvent->CopyFrom(tEvent);
		(*packet)->_fd = recvUser->_fd;
		(*packet)->_proto->set_senderid(recvUser->_fd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_GameEvent);
		(*packet)->_type = (int32_t)type;
		(*packet)->_proto->set_allocated_event((*packet)->_protoEvent);
	}
	else
	{
		delete (*packet);
		(*packet) = NULL;
	} 
	
	LOG_INFO("Success Event Part");
	return true;
}

bool CProtoManager::setNotiType(int type, CUser* recvUser, CProtoPacket* eventUser, list<CUser*> allUser, CProtoPacket** packet)
{
	if ( type < 0 || !eventUser || eventUser->_fd <= 0 || recvUser <= 0 )
	{
		LOG_ERROR("Not Exist User , User ProtoPacket");
		return false;
	}

	(*packet) = new CProtoPacket();
	(*packet)->_protoNoti = new server2N::GlobalNotice();
	if ( type == (int32_t)server2N::GameEvent_action_EventDeath )
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
			return false;
		}
		(*packet)->_protoNoti->add_victim((int32_t)eventFd);
		victimUser->_deathInfo++;
		int triggerFd = tEvent.deathevent().triggerid();
		CUser* triggerUser = g_userPool.findUserInPool(triggerFd);
		if ( !triggerUser )
		{
			LOG_ERROR("Not Exist TriggerUser(%d)\n", triggerUser->_fd);
			return false;
		}
		(*packet)->_protoNoti->set_performer((int32_t)triggerFd);
		//LOG_DEBUG("Set Kill Trigger(%d)\n", triggerUser->_fd);
		triggerUser->_killInfo++;
	}

	(*packet)->_type = type;
	(*packet)->_fd = recvUser->_fd;
	(*packet)->_protoNoti->set_notitype((int32_t)type);
	(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_GlobalNotice);
	(*packet)->_proto->set_allocated_notice((*packet)->_protoNoti);

	return true;
}

