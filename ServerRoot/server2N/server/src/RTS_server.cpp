#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/epoll.h>
//#include "google/protobuf/io/coded_stream.h"
//#include "google/protobuf/io/zero_copy_stream_impl.h"
//#include "proto/gameContent.pb.h"
#include "common/MsgString.h"
#include "common/CSessionManager.h"

using namespace std;

#if 0 
using namespace google::protobuf::io;
bool decodingProtoBuf(CUser* user)
{
	char* ptr = user->_buffer;
	for ( int idx = 0; idx < (user->_length - user->_protoLength); idx++ )
	{
		if ( !ptr )
		{
			return false;
		}
		ptr++;
	} 

	google::protobuf::uint32 pbufSize;
	ArrayInputStream inputStream(user->_buffer, user->_length);
	CodedInputStream codedStream(&inputStream);


	//google::protobuf::io::CodedInputStream::Limit bufferLimit = codedStream.PushLimit(pbufSize);

	codedStream.ReadVarint32(&pbufSize);
	server2N::PacketBody proto_packet;
	//inputStream.Skip(proto_packet.size);

	if ( !proto_packet.ParseFromCodedStream(&codedStream) )
	{
		LOG("ParseFromCodedStream False\n");
		return false;
	}

	LOG("Packet Parsing\n");
	if ( proto_packet.has_connect() ) 
	{
		server2N::UserConnection connect = proto_packet.connect();
		LOG("Exist UserConnection id(%d)\n", connect.id());
	} 
	else if ( proto_packet.has_event() )
	{
		server2N::GameEvent event = proto_packet.event();
		LOG("Exist GameEvent action id(%d)\n", event.act());
	}

	LOG("Buffer[%s]\n", user->_buffer);
	//codedStream.PopLimit(bufferLimit);
	return true;
}
#endif

int main(int argc, char* argv[]) 
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	int port = 10001;
	pthread_t thread;
	CSessionManager session(port);
	
	if ( pthread_create(&thread, NULL, session.waitEvent, (void*)&port) < 0 )
	{
		exit(0);
	}
	
	if ( pthread_create(&thread, NULL, session.writeEvent, (void*)&port) < 0 )
	{
		exit(0);
	}

	while(1)
	{
		/*
		 * Queue를 하나 두고, 값을 확인해야 할거 같다. 		
		 * Queue에 값이 없으면, Input이 없는 상황이겠지. 
		 * BusyWait가 되어버린다.
		 * Signal Wait가 필요할거 같기도하다. 
		 */
		CUser* data = NULL;
		if ( data = session.m_readQ_Manager.dequeue() ) 
		{
			LOG("Read DeQueue\n");
			session.m_writeQ_Manager.enqueue(data);
			/* 
			 * 해석 로직이 필요함 buffer -> Object 
			 * Buffer 내용에 따라, 일부 보낼 지, 전체 보낼 지 결정
			 * QueueData -> 재 해석 -> CData로 재 변환.
			 */
			
		} 	
	}
	LOG("MainLogic4\n");

	int status;
	pthread_join(thread, (void**)&status);
	google::protobuf::ShutdownProtobufLibrary();
	return -1;

}
