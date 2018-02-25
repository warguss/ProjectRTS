#include "CProtoManager.h"

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
	//_connectPacket.clear();
	//_tryConnectPacket.clear();
	//_disConnectPacket.clear();

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

	//resetProtoPacket(protoPacket*);

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
    else
    {
        LOG("Not Exist ProtoBuffer\n");
        return false;
    }



    return true;
}

#if 0 
int32_t CProtoManager::typeReturn(server2N::PacketBody* protoPacket)
{
	if ( !protoPacket )
	{
		return INVALID_USER;
	}

	int32_t type = -1;
    if ( protoPacket->_proto->has_event() )
    {
        server2N::GameEvent tEvent = protoPacket->_proto->event();
		server2N::GameEvent_action actType = tEvent.act();
		if ( actType == server2N::GameEvent_action_Nothing )
		{
			type = NOTHING;
		}
		else if ( actType == server2N::GameEvent_action_Move )
		{
			type = MOVE;
		} 
		else if ( actType == server2N::GameEvent_action_Stop )
		{
			type = STOP;
		}
		else if ( actType == server2N::GameEvent_action_Jump )
		{
			type = JUMP;
		}
		else if ( actType == server2N::GameEvent_action_Shoot )
		{
			type = SHOOT;
		}
		else if ( actType == server2N::GameEvent_action_GetHit )
		{
			type = SHOOT;
		}
		else if ( actType == server2N::GameEvent_action_Spawn )
		{
			type = SPAWN;
		}
        LOG("Has Event\n");
    }
    else if ( protoPacket->_proto->has_connect() )
    {
        server2N::UserConnection tConnect = protoPacket->connect();
		server2N::UserConnection_ConnectionType conType = tConnect.contype();
		if ( conType == server2N::UserConnection_ConnectionType_TryConnect )
		{
			type = (int32_t)TRYCONNECT;
		}
		else if ( conType == server2N::UserConnection_ConnectionType_DisConnect )
		{
			type = (int32_t)DISCONNECT;
		}
		else if ( conType == server2N::UserConnection_ConnectionType_Connect )
		{
			type = (int32_t)CONNECT;
		}
		
        LOG("Has Connect\n");
    }
    else
    {
        LOG("Not Exist ProtoBuffer\n");
    }

	return type;
}
#endif

bool CProtoManager::setConnectType(int32_t type, int32_t senderFd, int32_t eventFd, list<int32_t> allUser, CProtoPacket** packet)
{
	if ( type < 0 || eventFd <= 0 || senderFd <= 0 )
	{
		LOG("Not Exist User , User ProtoPacket\n");
		return false;
	}

	(*packet) = new CProtoPacket();
	(*packet)->_protoConnect = new server2N::UserConnection; 
	if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect )
	{
		LOG("USERID(%d) TRYCONNECT Change To AccepConnect\n", senderFd);
		
		(*packet)->_fd = senderFd;
		(*packet)->_proto->set_senderid(senderFd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_UserConnection);
		(*packet)->_type = server2N::PacketBody_messageType_UserConnection;

		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_AcceptConnect);
		(*packet)->_protoConnect->add_connectorid(eventFd);
		
		(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
		cout << "TRYCONNECT PACKET " << (*packet)->_proto->DebugString() << endl;
	}
	else if ( type == (int32_t)server2N::UserConnection_ConnectionType_Connect )
	{
		(*packet)->_type = server2N::PacketBody_messageType_UserConnection;
		(*packet)->_fd = senderFd;
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_UserConnection);
		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_Connect);
		LOG("USERID(%d) TRYCONNECT Change To CONNECT\n", senderFd);

		/**************************************
		 * id에 대해 list조회 전달
		 * (신규 유저일 경우 조회전달)
		 **************************************/
		if ( senderFd == eventFd ) 
		{
			list<int32_t>::iterator it = allUser.begin(); 
			/* Add 시점 생각해야할듯, 아니며 여기서 자기자신 예외처리필요함 */
			for ( ; it != allUser.end(); it++ )
			{
				if ( (int32_t)*it == senderFd )
				{
					LOG("Same User\n");
					continue ; 
				}
				(*packet)->_protoConnect->add_connectorid((int32_t)*it);
			}
			(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
			LOG("Set Connect\n");
		}
		else
		{
			/**************************************
			 * 기존 유저일 경우 eventFd만 전달
			 **************************************/
			(*packet)->_protoConnect->add_connectorid((int32_t)eventFd);
			(*packet)->_proto->set_allocated_connect((*packet)->_protoConnect);
		}

		cout << "CONNECT PACKET " << (*packet)->_proto->DebugString() << endl;
	}
	else if ( type == (int32_t)server2N::UserConnection_ConnectionType_DisConnect )
	{
		/**************************************
		 * 기존 유저일 경우 eventFd만 전달
		 **************************************/
		LOG("USERID(%d) DisConnect\n", senderFd);
		
		(*packet)->_fd = senderFd;
		(*packet)->_proto->set_senderid(senderFd);
		(*packet)->_proto->set_msgtype(server2N::PacketBody_messageType_UserConnection);
		(*packet)->_type = (int32_t)server2N::UserConnection_ConnectionType_DisConnect;
		(*packet)->_protoConnect->set_contype((int32_t)server2N::UserConnection_ConnectionType_DisConnect);
		(*packet)->_protoConnect->add_connectorid(eventFd);
		
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


bool CProtoManager::setActionType(server2N::PacketBody* protoPacket, int type)
{

	return true;
}

#if 0 
#include <sstream>
std::string hexStr(unsigned char*data, int len)
{
    std::stringstream ss;
    ss<<std::hex;
    for(int i(0);i<len;++i)
        ss<<(int)data[i];
    return ss.str();
}
#endif
