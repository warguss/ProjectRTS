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

void CUser::setData(int fd, char* buf, int type)
{

}

static void* CUser::writeData(void* buf)
{

	return (void*)0;
}

