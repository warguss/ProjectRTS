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
#include "common/MsgString.h"
#include "common/CSessionManager.h"
#include "common/CProtoManager.h"

using namespace std;
bool ForAllSendFunc(CSessionManager& session, CUser* eventUser);
bool ForPartSendFunc(CSessionManager& session, CUser* eventUser);
int32_t UserActionType(CUser* user);
typedef bool (*CallBackFunc)(CSessionManager&, CUser*);


int32_t UserActionType(CUser* user)
{
	/******************************************
	 * User Type을 분류해준다.
	 * 이 단계에서 Move, Hit같은경우
	 * 계산할 필요가 있으며 
	 * Connect, DisConnect의 경우 바로 리턴
	 ******************************************/
	if ( !user  )
	{
		LOG("Invalid User\n");
		return -INVALID_USER;
	}

	if ( !user->_protoPacket )
	{
		LOG("Invalid User ProtoPacket\n");
		return -INVALID_USER;
	}

	/*****************************************
	 * 대분류 형식으로 가야할듯
	 * Conn & Event
	 *****************************************/
	LOG("User[%d] TRYCONNECT\n", user->_fd);
	int32_t type = g_packetManager.typeReturn(user->_protoPacket);
	if ( type == TRYCONNECT )
	{
		/*******************************
		 * User가 들어왔다고 판단.
		 * _protoPacket의 id와
		 * Data를 변경한다.
		 *******************************/
		LOG("User[%d] TRYCONNECT\n", user->_fd);
		g_packetManager.setConnType(user->_protoPacket, type, user->_fd);
	}
	
	return type;
}



bool ForAllSendFunc(CSessionManager& session, CUser* eventUser)
{
	/*********************************
	 * Event 발생시킨 User는 먼저넣음
	 *********************************/
	LOG("User[%d] Event All Send\n", eventUser->_fd);
	cout << eventUser->_protoPacket->DebugString() << endl;
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

		LOG("User(%d) Send Noti\n", user->_fd);
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
	LOG("End All Send\n");

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
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)TRYCONNECT, ForAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)DISCONNECT, ForAllSendFunc) );


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
			LOG("Dequeue Start\n");
			int32_t type = UserActionType(data);
			if ( type < 0 )
			{
				LOG("Error Data\n");
				continue ;
			}

			LOG("User Type (%d)\n", type); 
			void (*func)(CSessionManager&, CUser*) = NULL;
			func = funcMap.find(type)->second;
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
