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

typedef void (*CallBackFunc)(CSessionManager&, CUser*);
bool ForAllSendFunc(CSessionManager& session, CUser* eventUser)
{
	/*********************************
	 * UserPool에서 전체 유저 가져옴
	 *********************************/ 


	/*********************************
	 * Event 발생시킨 User는 먼저넣음
	 *********************************/
	session.m_writeQ_Manager.enqueue(eventUser);

	/*********************************
	 * 그 중에서 EventUser는 제외
	 *********************************/ 
	int type;
	for ( g_userPool.it = g_userPool.userInfo.begin(); g_userPool.it != g_userPool.userInfo.end(); g_userPool.it++ )
	{
		CUser* user = (CUser*)g_userPool.it->second;
		if ( !user || user->_fd == eventUser->_fd )
		{
			continue ;
		}

		/*********************************
		 * ProtoPacket할당
		 *********************************/ 
		if ( !user->setPacketBody(g_packetManager.getBroadCastProtoPacket(type)) )
		{
			return false;
		} 
		
		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.enqueue(user);
	}
	
	return true;
}



bool ForPartSendFunc(CSessionManager& session, CUser* eventUser)
{
	LOG("Part Send Event\n");



	return true;
}






int main(int argc, char* argv[]) 
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	int port = 10001;
	pthread_t thread;
	CSessionManager session(port);
	
	map<int, CallBackFunc> funcMap;
	funcMap.insert( pair<int, CallBackFunc>(CONNECT, ForAllSendFunc) );
	funcMap.insert( pair<int, CallBackFunc>(DISCONNECT, ForAllSendFunc) );


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
		if ( session.m_readQ_Manager.isQueueDataExist() &&  (data = session.m_readQ_Manager.dequeue()) && data ) 
		{
			/*************************************
			 * Game Logic
			 * ALL_SEND
			 * PART_SEND
			 *************************************/ 
			void (*func)(CSessionManager&, CUser*) = NULL;
			if ( func )
			{
				func(session, data);
			}
			else
			{
				LOG("Not Register Type(%d)\n", 1);
			}
		} 	
	}
	LOG("MainLogic\n");

	int status;
	pthread_join(thread, (void**)&status);
	google::protobuf::ShutdownProtobufLibrary();
	return -1;

}
