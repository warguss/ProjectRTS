#include "CUser.h"

CUser::CUser()
{
	_x = 0;
    _y = 0;
	_accelX = 0;
	_accelY = 0;

    _fd = 0;
}

CUser::CUser(int fd, int32_t x, int32_t y)
{	
	_x = 0;
    _y = 0;
	_accelX = 0;
	_accelY = 0;


    _fd = fd;
    _x = x;
    _y = y;
}

CUser::~CUser()
{
#if 0 
	if ( _protoPacket )
	{
		_protoPacket->clear_connect();
		_protoPacket->clear_event();
		delete _protoPacket;
	}
	_protoPacket = NULL;
#endif
}

bool CUser::setData(int fd, int type)
{
	LOG("CUser::SetData fd[%d]\n", fd);
	_fd = fd;
	_type = type;
	
	return true;
}
