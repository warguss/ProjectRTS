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
	public:
		int _type;
        int _sector;
        float _x,_y;
        float _accelX,_accelY;
		string _nickName;

		int32_t _killInfo;
		int32_t _deathInfo;
		int _health_point;
		int _fd;
		bool _isInterested;
		int32_t _score;
	public:
        CUser();
        CUser(int fd, int32_t x, int32_t y);
        ~CUser();
        bool setData(int fd, int type);
		void userKillDeathScoreCalc();

	private:
		bool _userConnectionEvent(int fd);
		bool _userDisConnectionEvent(int fd);
};
#endif
