#ifndef _MODULE_CUSER_H_
#define _MODULE_CUSER_H_

#include <stdint.h>
#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "MsgString.h"
#include "proto/gameContent.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

//#include "CPacket.h"

using namespace std;
using namespace google::protobuf::io;
class CUser
{
    private:
        int _sector;
		int _type;
        int32_t tX,tY;
				
        pthread_t sendThread;
		server2N::PacketBody _protoPacket;


    public:
        int _fd;
		char _protoType;
		uint32_t bodyLength;
		unsigned char* _body;

	public:
        CUser();
        CUser(int fd, int32_t x, int32_t y);
        ~CUser();

        bool moveX(int32_t tX);
        bool moveY(int32_t tY);

        static void* writeData(void* buf);
        bool setData(int fd, int type);

		bool setHeader(unsigned char* buf);

		bool encodingHeader(unsigned char* outputBuf);
		bool encodingBody(unsigned char* buffer);

		bool decodingHeader(unsigned char* buffer, uint32_t bufLength);
		bool decodingBody(unsigned char* buffer, uint32_t bufLength);

		bool allocBody();
};
#endif
