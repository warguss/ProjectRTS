#include "CUser.h"

CUser::CUser()
{
	tX = 0;
    tY = 0;
    _fd = 0;
	bodyLength = 0;
}

CUser::CUser(int fd, int32_t x, int32_t y)
{
    _fd = fd;
    tX = x;
    tY = y;
}

CUser::~CUser()
{

}

bool CUser::moveX(int32_t tX)
{
	return true;
}

bool CUser::moveY(int32_t tY)
{
	return true;
}

bool CUser::setData(int fd, int type)
{
	LOG("CUser::SetData fd[%d]\n", fd);
	_fd = fd;
	_type = type;
	
	return true;
}

bool CUser::encodingHeader(unsigned char* outputBuf)
{
	/*************************************
	 * unsigned Char의 단위 계산을 위해
	 * 256 나머지와 나누기를 병행
	 *************************************/
	int bitDigit = 256;
	uint32_t headerSize = (uint32_t)_protoPacket.ByteSizeLong();
	LOG("User(%d) Header Size:%d\n", _fd, headerSize);
	bodyLength = headerSize; 
	for ( int idx = 0; idx < HEADER_SIZE ; idx++, outputBuf++, headerSize /= bitDigit )
	{
		if ( !outputBuf )
		{
			LOG("User(%d) ERROR Output Buf\n", _fd);
			return false;
		}
		*outputBuf = (unsigned char*)(headerSize % bitDigit);
	}

	return true;
} 


bool CUser::encodingBody(unsigned char* buffer)
{
	if ( !_protoPacket.SerializeToArray(buffer, bodyLength) )
	{
		LOG("Packet SerializeToArray Error\n");
		return false;
	}

	return true;
}

bool CUser::decodingHeader(unsigned char* buffer, uint32_t bufLength)
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

bool CUser::decodingBody(unsigned char *buffer, uint32_t bufLength)
{
	if ( bufLength != bodyLength ) 
	{
		LOG("Error length\n");
		return false;
	}
	
	LOG("Body Length Start\n");
	google::protobuf::io::CodedInputStream is(buffer, bodyLength);
	if ( !_protoPacket.MergeFromCodedStream(&is) ) 
	{
		LOG("Error CodedStream\n");
		return false;
	}

	// 디버깅 용도, 있는지 없는 지만 체크하도 리턴해도 충분하다.
	if ( _protoPacket.has_event() )
	{
		server2N::GameEvent tEvent = _protoPacket.event();
		cout << "Debug String" << tEvent.DebugString() << endl;
		LOG("Has Event\n");
	}
	else if ( _protoPacket.has_connect() )
	{
		server2N::UserConnection tConnect = _protoPacket.connect();
		cout << "Debug String" << tConnect.DebugString() << endl;

		LOG("Has Connect\n");
	}
	else
	{
		LOG("Not Exist ProtoBuffer\n");
		return false;
	}



	cout << "Debug String" << _protoPacket.DebugString() << endl;
	LOG("Size : %d\n", _protoPacket.ByteSizeLong());
	return true;
}

#if 0 
bool CUser::allocBody()
{
	if ( bodyLength <= 0 )
	{

	}



	_body = (unsigned char)malloc(sizeof(unsigned char) * bodyLength);
    	

}
#endif

#if 0 
static void* CUser::writeData(void* buf)
{
	q
	int32_t writeSize = 0;
	if ( writeSize = write(_fd, _buffer, (size_t)user->_length) < 0 )
	{
        perror("Send");
        LOG("Write Size[%d]\n",writeSize);
        return false;
	}

	return (void*)0;
}
#endif

