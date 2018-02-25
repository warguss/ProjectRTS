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

#if 0 
bool CUser::setPacketBody(server2N::PacketBody* packet)
{
	if ( !packet )
	{
		LOG("Error invalid Packet Body\n");
		return false;
	}

	if ( _protoPacket )
	{
		delete _protoPacket;
		_protoPacket = NULL;
	}

	_protoPacket = packet;
	return true;
}
#endif
