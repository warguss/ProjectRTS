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

using namespace std;
using namespace google::protobuf::io;
class CUser
{
    private:
		int _type;
        int _sector;
        int32_t tX,tY;
		/*******************************
		 * BroadCase위해 ptr로 할당
		 *******************************/
		server2N::PacketBody* _protoPacket;

    public:
        int _fd;
		int _sendType;
		uint32_t bodyLength;

	public:
        CUser();
        CUser(int fd, int32_t x, int32_t y);
        ~CUser();

        bool moveX(int32_t tX);
        bool moveY(int32_t tY);

        bool setData(int fd, int type);
		//bool setHeader(unsigned char* buf);
#if 0 
		bool encodingHeader(unsigned char* outputBuf);
		bool encodingBody(unsigned char* buffer);

		bool decodingHeader(unsigned char* buffer, uint32_t bufLength);
		bool decodingBody(unsigned char* buffer, uint32_t bufLength);
#endif

	private:
		bool _userConnectionEvent(int fd);
		bool _userDisConnectionEvent(int fd);
};
#endif
