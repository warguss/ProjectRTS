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
#if 0 
	if ( _connectCase )
	{
		//delete _connectCase;
	}
	
	if ( _disConnectCase )
	{
		//delete _disConnectCase;
	}
#endif
}

bool CProtoManager::encodingHeader(unsigned char* outputBuf, server2N::PacketBody* protoPacket, uint32_t& bodyLength)
{
    /*************************************
     * unsigned Char의 단위 계산을 위해
     * 256 나머지와 나누기를 병행
     *************************************/
	if ( !protoPacket ) 
	{
		LOG("Invalid ProtoPacket");
		return false;
	} 

    int bitDigit = 256;
    uint32_t headerSize = (uint32_t)protoPacket->ByteSizeLong();
    bodyLength = headerSize;
    LOG("Header Size:%d\n", headerSize);
    for ( int idx = 0; idx < HEADER_SIZE ; idx++, outputBuf++, headerSize /= bitDigit )
    {
        if ( !outputBuf )
        {
            LOG("ERROR Output Buf\n");
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
        LOG("Packet SerializeToArray Error\n");
        return false;
    }

	return true;
}


bool CProtoManager::decodingHeader(unsigned char* buffer, uint32_t bufLength, uint32_t& bodyLength)
{
    if ( bufLength < HEADER_SIZE )
    {
        LOG("Packet Size Not Exist Header\n");
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
            LOG("Ptr is Null, Packet is Invalid\n");
            return false;
        }
        bodyLength += (*buffer) << bitDigit;
    }
    LOG("bodyLength [%d]\n", bodyLength);
    return true;
}


bool CProtoManager::decodingBody(unsigned char* buffer, uint32_t bufLength, uint32_t bodyLength, CProtoPacket** protoPacket)
{
    if ( bufLength != bodyLength || !buffer )
    {
        LOG("Error length or Buffer Not Exist\n");
        return false;
    }

	(*protoPacket) = new CProtoPacket();
    google::protobuf::io::CodedInputStream is(buffer, (int)bodyLength);
    if ( !(*protoPacket)->_proto->MergeFromCodedStream(&is) )
    {
        LOG("Error CodedStream\n");
        return false;
    }

    // 디버깅 용도, 있는지 없는 지만 체크하도 리턴해도 충분하다.
    if ( (*protoPacket)->_proto->has_event() )
    {
        server2N::GameEvent tEvent = (*protoPacket)->_proto->event();
        cout << "Debug String" << tEvent.DebugString() << endl;
        LOG("Has Event\n");
		(*protoPacket)->_type = tEvent.act();

    }
    else if ( (*protoPacket)->_proto->has_connect() )
    {
        server2N::UserConnection tConnect = (*protoPacket)->_proto->connect();
        cout << "Debug String" << tConnect.DebugString() << endl;
        LOG("Has Connect\n");
		(*protoPacket)->_type = tConnect.contype();
    }
	else if ( (*protoPacket)->_proto->has_notice() )
    {
        server2N::GlobalNotice tNoti = (*protoPacket)->_proto->notice();
        cout << "Debug String" << tNoti.DebugString() << endl;
        LOG("Has Connect\n");
		(*protoPacket)->_type = tNoti.notitype();
    }
    else
    {
        LOG("Not Exist ProtoBuffer\n");
        return false;
    }

    return true;
}

bool CProtoManager::setConnectType(int32_t type, CUser* eventUser, int32_t fd, list<CUser*> allUser, CProtoPacket** packet)
{
	if ( type < 0 || fd <= 0 || !eventUser || eventUser->_fd <= 0 )
	{
		LOG("Not Exist User , User ProtoPacket\n");
		return false;
	}

	(*packet) = new CProtoPacket();
	(*packet)->_protoConnect = new server2N::UserConnection; 
	(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_UserConnection);
	if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect )
	{
		LOG("USERID(%d) TRYCONNECT Change To AccepConnect\n", eventUser->_fd);
		
		(*packet)->_fd = eventUser->_fd;
		(*packet)->_proto->set_senderid(eventUser->_fd);
		(*packet)->_type = server2N::PacketBody_messageType_UserConnection;

		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_AcceptConnect);
		(*packet)->_protoConnect->add_connectorid(eventUser->_fd);
		(*packet)->_protoConnect->add_killinfo(eventUser->_killInfo);
		(*packet)->_protoConnect->add_deathinfo(eventUser->_deathInfo);
		(*packet)->_protoConnect->add_nickname(eventUser->_nickName.c_str());
		
		(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
		cout << "TRYCONNECT PACKET " << (*packet)->_proto->DebugString() << endl;
	}
	else if ( type == (int32_t)server2N::UserConnection_ConnectionType_Connect )
	{
		LOG("USERID(%d) TRYCONNECT Change To CONNECT\n", eventUser->_fd);
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
			LOG("----------------Set Connect\n");
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
			LOG("---------------------Set 기존 유저\n");
			(*packet)->_protoConnect->add_connectorid((int32_t)eventUser->_fd);
			(*packet)->_protoConnect->add_killinfo((int64_t)eventUser->_killInfo);
			(*packet)->_protoConnect->add_deathinfo((int64_t)eventUser->_deathInfo);
			(*packet)->_protoConnect->add_nickname(eventUser->_nickName.c_str());
			(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
		}

		cout << "CONNECT PACKET " << (*packet)->_proto->DebugString() << endl;
	}
	else if ( type == (int32_t)server2N::UserConnection_ConnectionType_DisConnect )
	{
		/**************************************
		 * 기존 유저일 경우 eventFd만 전달
		 **************************************/
		LOG("USERID(%d) DisConnect\n", eventUser->_fd);
		
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
		LOG("Error Not Exist Type\n");
		if ( *packet ) 
		{
			delete *packet;
			*packet = NULL;
		}
		return false;
	}

	return true;
}

void CProtoManager::resetProtoPacket(CProtoPacket* protoPacket)
{
	if ( protoPacket )
	{
		LOG("Reset ProtoPacket Delete");
		delete protoPacket;
	}
	protoPacket = NULL;
}

bool CProtoManager::setActionType(int32_t type, CUser* senderUser, CProtoPacket* eventUser, list<CUser*> userList, CProtoPacket** packet)
{
	if ( type < 0 || !eventUser || eventUser->_fd <= 0 || senderUser <= 0 )
	{
		LOG("Not Exist User , User ProtoPacket\n");
		return false;
	}

	(*packet) = new CProtoPacket();
	(*packet)->_protoEvent = new server2N::GameEvent();
	server2N::GameEvent tEvent = eventUser->_proto->event(); 
	if ( type == (int32_t)server2N::GameEvent_action_Move || type == server2N::GameEvent_action_Stop || type == (int32_t)server2N::GameEvent_action_Jump || type == (int32_t)server2N::GameEvent_action_UserSync )
	{
		/* Copy From이 나을지, getset하는게 나을지 모르겠음 */
		(*packet)->_protoEvent->CopyFrom(tEvent);
		(*packet)->_fd = senderUser->_fd;
		(*packet)->_proto->set_senderid(senderUser->_fd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_GameEvent);
		(*packet)->_type = (int32_t)type;
		(*packet)->_proto->set_allocated_event((*packet)->_protoEvent);
	}
	else if ( type == (int32_t)server2N::GameEvent_action_UserSync )
	{
		if ( senderUser->_fd == eventUser->_fd )
		{
			/******************************
			 * Sector 조회
			 * 현재 Sector랑 변경없으면
			 * 해당 유지, 변화있으면
			 * 삭제 후 Add
			 ******************************/
			senderUser->_x = tEvent.eventpositionx();
			senderUser->_y = tEvent.eventpositiony();
			senderUser->_accelX = tEvent.velocityx();
			senderUser->_accelY = tEvent.velocityy();
			bool isSuccess = true;
			do
			{
				int preSector = senderUser->_sector;
				int sector = g_userPool.getSectionNo(senderUser);
				if ( preSector != sector )
				{
					if ( !g_userPool.changeUserInPool(senderUser, preSector, sector) )
					{
						isSuccess = false;
						break;
					} 
				}
			}
			while(false);

			delete (*packet)->_protoEvent;
			delete *packet;
			return isSuccess;
		} 
		(*packet)->_protoEvent->CopyFrom(tEvent);
		(*packet)->_fd = senderUser->_fd;
		(*packet)->_proto->set_senderid(senderUser->_fd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_GameEvent);
		(*packet)->_type = (int32_t)type;
		(*packet)->_proto->set_allocated_event((*packet)->_protoEvent);
	}

	return true;
}


//bool CProtoManager::setNotiType(int32_t type, CUser* senderUser, CProtoPacket* eventUser, list<CUser*> userList, CProtoPacket** packet)
bool CProtoManager::setNotiType(int type, CUser* senderUser, CProtoPacket* eventUser, list<CUser*> allUser, CProtoPacket** packet)
{
	if ( type < 0 || !eventUser || eventUser->_fd <= 0 || senderUser <= 0 )
	{
		LOG("Not Exist User , User ProtoPacket\n");
		return false;
	}

	(*packet) = new CProtoPacket();
	(*packet)->_protoNoti = new server2N::GlobalNotice();
	server2N::GlobalNotice tNoti = eventUser->_proto->notice(); 
	if ( type == (int32_t)server2N::GlobalNotice_NoticeInfo_KillInfo  )
	{
		/********************************
		 * Kill 
		 * eventUser_fd -> victim
		 * A User KillInfo Add
		 * B User DeathInfo Add
		 ********************************/
		int32_t eventFd = eventUser->_fd;
		CUser* triggerUser = g_userPool.findUserInPool(eventFd);
		if ( !triggerUser ) 
		{
			/*********************************
			 * 전체 조회이기 때문에, 
			 * 찾지 못했다면 close(fd)로 종료
			 *********************************/
			LOG("Not Exist TriggerUser(%d)\n", triggerUser->_fd);
			return false;
		}
		triggerUser->_killInfo++;
		int victimSize = tNoti.victim_size();
		for ( int idx = 0;  idx < victimSize; idx++ )
		{
			int victimFd = tNoti.victim(idx);
			CUser* victimUser = g_userPool.findUserInPool(victimFd);
			if ( !victimUser )
			{
				LOG("Not Exist VictimUser(%d)\n", victimUser->_fd);
			}
			victimUser->_deathInfo++;
		} 
	}

	(*packet)->_protoNoti->CopyFrom(tNoti);
	(*packet)->_proto->set_allocated_notice((*packet)->_protoNoti);

	return true;
}

