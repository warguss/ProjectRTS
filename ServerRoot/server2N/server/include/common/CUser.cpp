#include "CUser.h"

CUser::CUser()
{
	tX = 0;
    tY = 0;
    _fd = 0;
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

}

bool CUser::moveY(int32_t tY)
{

}

void CUser::setData(int fd, char* buf, int length, int type)
{
	LOG("CUser::SetData fd[%d] buf[%s]\n", fd, buf);
	_fd = fd;
	_length = length;
	memset(_buffer, '\0', BUFFER);
	memcpy(_buffer, buf, _length);
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

