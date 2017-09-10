#ifndef _CUSER_CLASS_H_
#define _CUSER_CLASS_H_

#include <iostream>
#include <stdio.h>

class CUser
{
	private:
		int _fd;
		int32_t x,y;
		int sector;

	public:
		CUser(int fd, int32_t x, int32_t y);
		~CUser();

		bool moveX(int32_t tX);
		bool moveY(int32_t tY);
		
		int32_t readData();
		int32_t writeData();

		
};

#endif
