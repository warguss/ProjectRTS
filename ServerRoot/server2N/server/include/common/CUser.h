#ifndef _MODULE_CUSER_H_
#define _MODULE_CUSER_H_

#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "MsgString.h"

using namespace std;
class CUser
{
    private:
        pthread_t sendThread;
        int _sector;
		int _type;
        int32_t tX,tY;
			

    public:
        char _buffer[BUFFER];
        int _fd;
		int _length;
		char _protoType;
		int _protoLength;

        CUser();
        CUser(int fd, int32_t x, int32_t y);
        ~CUser();

        bool moveX(int32_t tX);
        bool moveY(int32_t tY);

        void setData(int fd, char* buf, int length, int type);

        static void* writeData(void* buf);
};
#endif

