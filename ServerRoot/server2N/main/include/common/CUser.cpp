#include "CUser.h"

#define BUFFER 8096

		pthread_t sendThread;
		int _fd;
		int32_t tX,tY;
		int _sector;
		char _buffer[BUFFER];

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
		
static void* readData(void* data)
{


}



static void* writeData(void* data)
{


}
