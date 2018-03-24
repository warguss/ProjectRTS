#ifndef _PROTO_MANAGER_H_
#define _PROTO_MANAGER_H_
#include <list>
#include "MsgString.h"
#include "proto/gameContent.pb.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "CProtoPacket.h"

using namespace std;
using namespace google::protobuf::io;

class CProtoManager
{
	private:
		/*********************************
		 * 전체 유저에게 보여줘야 할 상황은
		 * 미리 만들어두고 사용한다.
		 *********************************/
		server2N::PacketBody _tryConnectPacket;
		server2N::PacketBody _connectPacket;
		server2N::PacketBody _disConnectPacket;

		server2N::UserConnection* _tryConnectCase;
		server2N::UserConnection* _connectCase;
		server2N::UserConnection* _disConnectCase;

	public:
		CProtoManager();
		~CProtoManager();

		/********************************
		 * BroadCast 해제
		 ********************************/
		void initialize(); 
		void close();

		bool encodingHeader(unsigned char* outputBuf, server2N::PacketBody* protoPacket, uint32_t& bodyLength);
		bool encodingBody(unsigned char* buffer, server2N::PacketBody* protoPacket, uint32_t bodyLength);

		bool decodingHeader(unsigned char* buffer, uint32_t bufLength, uint32_t& bodyLength);
		bool decodingBody(unsigned char* buffer, uint32_t bufLength, uint32_t bodyLength, CProtoPacket** protoPacket);

		server2N::PacketBody* getBroadCastProtoPacket(int type);

		int32_t typeReturn(server2N::PacketBody* protoPacket);
		bool setActionType(int type, CUser* senderUser, CProtoPacket* eventUser, list<CUser*> allUser, CProtoPacket** packet);
		bool setConnectType(int type, CUser* senderUser, int eventFd, list<CUser*> allUser, CProtoPacket** packet);

		void resetProtoPacket(CProtoPacket* protoPacket);
};
extern CProtoManager g_packetManager;
#endif
