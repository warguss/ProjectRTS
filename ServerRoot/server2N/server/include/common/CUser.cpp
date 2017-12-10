#include "CUser.h"

CUser::CUser()
{
	tX = 0;
    tY = 0;
    _fd = 0;
	_protoType = 0;
	_protoLength = 0;
}

CUser::CUser(int fd, int32_t x, int32_t y)
{
    _fd = fd;
    tX = x;
    tY = y;
    memset(_buffer, '\0', sizeof(char) * BUFFER);
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

void CUser::setData(int fd, char* buf, int length, int type)
{
	LOG("CUser::SetData fd[%d] buf[%s]\n", fd, buf);
	_fd = fd;

	memset(_buffer, '\0', BUFFER);
	memcpy(_buffer, buf, _length);

	
	/*******************************************
	 * Message구분을 위한, Proto Buffer Ptr이동
	 * [0 - 4] Type, Length
	 * [5 - length] ProtoBuffer
	 *******************************************/
	char* ptr = buf;
	_protoType = _buffer[0];
	LOG("CUser::SetData protoType[%c]\n", _protoType);

	_protoLength = _buffer[1] | _buffer[2] << 8 | _buffer[3] << 16 | _buffer[4] << 24; 
	LOG("CUser::SetData protoLength[%c]\n", _protoLength);



	
	_length = length;
	_type = type;
}

#if 0 
static void* CUser::writeData(void* buf)
{
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

