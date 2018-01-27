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

    public:
        int _fd;
		server2N::PacketBody _protoPacket;

	public:
        CUser();
        CUser(int fd, int32_t x, int32_t y);
        ~CUser();

        bool moveX(int32_t tX);
        bool moveY(int32_t tY);

        bool setData(int fd, int type);
		bool setPacketBody(server2N::PacketBody* packet);


	private:
		bool _userConnectionEvent(int fd);
		bool _userDisConnectionEvent(int fd);
};
#endif