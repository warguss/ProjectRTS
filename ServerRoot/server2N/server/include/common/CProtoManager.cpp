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
	_connectCase = new server2N::PacketBody;


	/************************************
	 * DisConnect
	 ************************************/
	_disConnectCase = new server2N::PacketBody;
}

void CProtoManager::close()
{
	if ( _connectCase )
	{
		delete _connectCase;
	}
	
	if ( _disConnectCase )
	{
		delete _disConnectCase;
	}
}

bool CProtoManager::encodingHeader(unsigned char* outputBuf, server2N::PacketBody* protoPacket, uint32_t& bodyLength)
{
    /*************************************
     * unsigned Char의 단위 계산을 위해
     * 256 나머지와 나누기를 병행
     *************************************/
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
std::string hexStr(unsigned char*data, int len);
bool CProtoManager::decodingBody(unsigned char* buffer, uint32_t bufLength, uint32_t bodyLength, server2N::PacketBody* protoPacket)
{
    if ( bufLength != bodyLength || !buffer )
    {
        LOG("Error length or Buffer Not Exist\n");
        return false;
    }

	if ( protoPacket )
	{
		delete protoPacket;
	}

	printf("hex [%s]\n", hexStr(buffer, (int)bufLength).c_str());

	protoPacket = new server2N::PacketBody;
    google::protobuf::io::CodedInputStream is(buffer, (int)bodyLength);
    if ( !protoPacket->MergeFromCodedStream(&is) )
    {
        LOG("Error CodedStream\n");
        return false;
    }

    // 디버깅 용도, 있는지 없는 지만 체크하도 리턴해도 충분하다.
    if ( protoPacket->has_event() )
    {
        server2N::GameEvent tEvent = protoPacket->event();
        cout << "Debug String" << tEvent.DebugString() << endl;
        LOG("Has Event\n");
    }
    else if ( protoPacket->has_connect() )
    {
        server2N::UserConnection tConnect = protoPacket->connect();
        cout << "Debug String" << tConnect.DebugString() << endl;
        LOG("Has Connect\n");
    }
    else
    {
		cout << "Debug String" << protoPacket->DebugString() << endl;
		cout << "Debug String" << protoPacket->Utf8DebugString() << endl;
		protoPacket->PrintDebugString();
        LOG("Not Exist ProtoBuffer\n");
        return false;
    }

	//protoPacket = packet;
    cout << "Debug String" << protoPacket->DebugString() << endl;
    LOG("Size : %d\n", protoPacket->ByteSizeLong());
    return true;
}

server2N::PacketBody* CProtoManager::getBroadCastProtoPacket(int type)
{
	server2N::PacketBody* packet = NULL;
	if ( type == CONNECT )
	{
		packet = _connectCase;
	}
	else if ( type == DISCONNECT )
	{
		packet = _disConnectCase;
	}

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
        cout << "Debug String" << tEvent.DebugString() << endl;
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
		
        cout << "Debug String" << tConnect.DebugString() << endl;
        LOG("Has Connect\n");
    }
    else
    {
        LOG("Not Exist ProtoBuffer\n");
    }

	return type;
}


bool CProtoManager::setConnType(server2N::PacketBody* protoPacket, int32_t type, int32_t fd)
{
	if ( !protoPacket )
	{
		LOG("Not Exist User , User ProtoPacket\n");
		return false;
	}


	if ( !protoPacket->has_connect() ) 
	{
		LOG("Invalid ProtoPacket And Type\n");
		return false;
	}

	if ( type == (int32_t)TRYCONNECT )
	{
		server2N::UserConnection tConnect = protoPacket->connect();
		tConnect.set_contype(server2N::UserConnection_ConnectionType_Connect);
		tConnect.set_id(fd);
	}


	return true;
}


bool CProtoManager::setActionType(server2N::PacketBody* protoPacket, int type)
{

	return true;
}
#include <sstream>
std::string hexStr(unsigned char*data, int len)
{
    std::stringstream ss;
    ss<<std::hex;
    for(int i(0);i<len;++i)
        ss<<(int)data[i];
    return ss.str();
}


