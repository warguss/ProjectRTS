#ifndef _CUSER_CLASS_H_
#define _CUSER_CLASS_H_

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#define BUFFER 8096

class CUser
{
	private:
		pthread_t sendThread;
		int _fd;
		int32_t tX,tY;
		int _sector;
		char _buffer[BUFFER];

	public:
		CUser(int fd, int32_t x, int32_t y);
		~CUser();

		bool moveX(int32_t tX);
		bool moveY(int32_t tY);
		
		static void* readData();
		static void* writeData();
};
#endif
