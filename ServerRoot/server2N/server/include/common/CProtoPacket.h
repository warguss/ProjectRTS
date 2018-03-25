#ifndef _PROTO_PACKET_H_
#define _PROTO_PACKET_H_
#include "proto/gameContent.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "CUser.h"

class CProtoPacket
{
	public:
		int _fd;
		int32_t _type;

		CProtoPacket();
		~CProtoPacket();

		void addConnectorId(int32_t id);

		server2N::PacketBody* _proto;
		server2N::UserConnection* _protoConnect;
		server2N::GameEvent* _protoEvent;
		server2N::GlobalNotice* _protoNoti;

		bool SyncUser(CUser* user);
};
#endif
