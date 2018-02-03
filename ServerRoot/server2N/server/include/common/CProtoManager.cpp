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
	//_connectCase = new server2N::UserConnection;
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


bool printTest(unsigned char* buffer, int size);
bool CProtoManager::decodingBody(unsigned char* buffer, uint32_t bufLength, uint32_t bodyLength, server2N::PacketBody** protoPacket)
{
    if ( bufLength != bodyLength || !buffer )
    {
        LOG("Error length or Buffer Not Exist\n");
        return false;
    }

	resetProtoPacket(protoPacket);

	*protoPacket = new server2N::PacketBody;
    google::protobuf::io::CodedInputStream is(buffer, (int)bodyLength);
    if ( !(*protoPacket)->MergeFromCodedStream(&is) )
    {
        LOG("Error CodedStream\n");
        return false;
    }

    // 디버깅 용도, 있는지 없는 지만 체크하도 리턴해도 충분하다.
    if ( (*protoPacket)->has_event() )
    {
        server2N::GameEvent tEvent = (*protoPacket)->event();
        cout << "Debug String" << tEvent.DebugString() << endl;
        LOG("Has Event\n");
    }
    else if ( (*protoPacket)->has_connect() )
    {
        server2N::UserConnection tConnect = (*protoPacket)->connect();
        cout << "Debug String" << tConnect.DebugString() << endl;
        LOG("Has Connect\n");
    }
    else
    {
        LOG("Not Exist ProtoBuffer\n");
        return false;
    }

    return true;
}

server2N::PacketBody* CProtoManager::getBroadCastProtoPacket(int type)
{
	server2N::PacketBody* packet = NULL;
#if 0
	if ( type == CONNECT )
	{
		packet = _connectCase;
	}
	else if ( type == DISCONNECT )
	{
		packet = _disConnectCase;
	}
#endif

	return packet;
}


int32_t CProtoManager::typeReturn(server2N::PacketBody* protoPacket)
{
	if ( !protoPacket )
	{
		return INVALID_USER;
	}

	int32_t type = -1;
    if ( protoPacket->has_event() )
    {
        server2N::GameEvent tEvent = protoPacket->event();
        LOG("Has Event\n");
    }
    else if ( protoPacket->has_connect() )
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
#if 0 
int32_t CProtoManager::typeReturn(server2N::UserConnection_ConnectionType conType)
{
	int32_t type = 0;
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

	return type;
}
#endif

bool CProtoManager::setConnType(server2N::PacketBody* protoPacket, int32_t type, int32_t senderFd, int32_t eventFd)
{
	if ( !(protoPacket) )
	{
		LOG("Not Exist User , User ProtoPacket\n");
		return false;
	}

	if ( !(protoPacket)->has_connect() ) 
	{
		LOG("Invalid ProtoPacket And Type\n");
		return false;
	}

	if ( type == (int32_t)TRYCONNECT )
	{
		LOG("USERID(%d) TRYCONNECT Change To AccepConnect\n", senderFd);
		/**************************************
		 * set_contype 안먹음
		 * 0은 표시안함
		 **************************************/
		_tryConnectCase->set_connectorid(eventFd);

		_tryConnectPacket.clear_senderid();
		_tryConnectPacket.set_senderid(senderFd);
		_tryConnectPacket.set_msgtype(server2N::PacketBody_messageType_UserConnection);
		cout << "TRYCONNECT PACKET " << _tryConnectPacket.DebugString() << endl;
		protoPacket->CopyFrom(_tryConnectPacket);
		//protoPacket = &_tryConnectPacket;

	}
	else if ( type == (int32_t)CONNECT )
	{
		LOG("USERID(%d) TRYCONNECT Change To CONNECT\n", senderFd);
		/**************************************
		 * set_contype 안먹음
		 * 0은 표시안함
		 **************************************/
		_connectCase->set_connectorid(eventFd);

		_connectPacket.clear_senderid();
		_connectPacket.set_senderid(senderFd);
		_connectPacket.set_msgtype(server2N::PacketBody_messageType_UserConnection);

		cout << "CONNECT PACKET " << _connectPacket.DebugString() << endl;
		protoPacket->CopyFrom(_connectPacket);
	}

	return true;
}

void CProtoManager::resetProtoPacket(server2N::PacketBody** protoPacket)
{
	if ( *protoPacket )
	{
		LOG("Reset ProtoPacket Delete");
		delete (*protoPacket);
	}

	*protoPacket = NULL;
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
