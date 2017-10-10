#ifndef _CUSER_CLASS_H_
#define _CUSER_CLASS_H_

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include "MsgString.h"

class CUser
{
	private:
		pthread_t sendThread;
		int _fd;
		int _sector;
		int32_t tX,tY;
		char _buffer[BUFFER];

	public:
		CUser(int fd, int32_t x, int32_t y);
		~CUser();

		bool moveX(int32_t tX);
		bool moveY(int32_t tY);
		
		
		
		static void* writeData(void* buf);
};
#endif
