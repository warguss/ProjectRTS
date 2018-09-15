#include "CUser.h"

CUser::CUser()
{
	_x = 0;
    _y = 0;
	_accelX = 0;
	_accelY = 0;

	_killInfo = 0;
	_deathInfo = 0;
	_nickName = "unknown";
    _fd = 0;

	_health_point = 0;
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
}

bool CUser::setData(int fd, int type)
{
	_fd = fd;
	_type = type;
	
	return true;
}
