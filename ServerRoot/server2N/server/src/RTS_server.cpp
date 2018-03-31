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
#include "common/CProtoPacket.h"
#include "common/CUserPool.h"

using namespace std;
bool ConnectAllSendFunc(CSessionManager& session, CProtoPacket* eventUser);
bool ActionPartSendFunc(CSessionManager& session, CProtoPacket* eventUser);
bool ActionAllSendFunc(CSessionManager& session, CProtoPacket* eventUser);
typedef bool (*CallBackFunc)(CSessionManager&, CProtoPacket*);

extern int32_t g_sectorIdx;
extern CUserPool g_userPool;

bool ConnectAllSendFunc(CSessionManager& session, CProtoPacket* eventPacket)
{
	/*************************************
	 * 전체 유저 연결 정보 획득
	 *************************************/
	LOG("All Send Connect\n");
	list<CUser*> userConnector;
	g_userPool.getAllUserList(userConnector);

	/*************************************
	 * TryConnect 
	 *************************************/
	int32_t type = eventPacket->_type;
	if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect || type == (int32_t)server2N::UserConnection_ConnectionType_Connect )
	{
		CProtoPacket *connectPacket = NULL;
		if ( !g_packetManager.setConnectType(type, eventPacket->_fd, eventPacket->_fd, userConnector, &connectPacket) ) 
		{
			LOG("Error Connector Type\n");
			return false;
		}

		LOG("User[%d] Pre Enqueue\n", eventPacket->_fd);
		cout <<  connectPacket->_proto->DebugString() << endl;
		session.m_writeQ_Manager.enqueue(connectPacket);

		type = server2N::UserConnection_ConnectionType_Connect;
	}

	int userConnectSize = userConnector.size(); 
	list<CUser*>::iterator it = userConnector.begin();
	LOG("Connector User Size(%d)", userConnectSize);
	for ( ; it != userConnector.end(); it++ )
	{
		CUser* user = *it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setConnectType(type, user, eventPacket->_fd, userConnector, &packet) ||  !packet )
		{
			LOG("Error Connector Type\n");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		cout << "Test: " <<  packet->_proto->DebugString() << endl;
		session.m_writeQ_Manager.enqueue(packet);
		LOG("User(%d) Send Noti\n", packet->_fd);
	}
	LOG("End All Send\n");
	return true;
}

bool ActionPartSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	LOG("Part Send Event\n");
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	list<CUser*> connectList;
	g_userPool.getAllUserList(connectList);

	/*************************************
	 * Event Send 
	 *************************************/
	int32_t type = eventUser->_type;
	list<CUser*>::iterator it = connectList.begin();
	for ( ; it != connectList.end(); it++ )
	{
		CUser* user = (CUser*)*it;
		CProtoPacket *packet = NULL;
		if ( user && (user->_fd == eventUser->_fd) )
		{
			continue ;
		}

		if ( !g_packetManager.setActionType(type, user, eventUser, connectList, &packet) || !packet )
		{
			LOG("Error Connector Type\n");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		cout << "Test: " <<  packet->_proto->DebugString() << endl;
		session.m_writeQ_Manager.enqueue(packet);
		LOG("User(%d) Send Noti\n", packet->_fd);
	}
	LOG("End All Send\n");

	return true;
}


bool ActionAllSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	LOG("Action All Send Connect\n");
	list<CUser*> connectList;
	g_userPool.getAllUserList(connectList);

	/*************************************
	 * Event Send 
	 *************************************/
	int32_t type = eventUser->_type;
	list<CUser*>::iterator it = connectList.begin();
	for ( ; it != connectList.end(); it++ )
	{
		CUser* user = (CUser*)*it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setActionType(type, user, eventUser, connectList, &packet) || !packet )
		{
			LOG("Error Connector Type\n");
			return false;
		} 

		if ( user->_fd == eventUser->_fd )
		{
			continue ;
		}

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.enqueue(packet);
		LOG("User(%d) Send Noti\n", packet->_fd);
	}
	LOG("End All Send\n");

	return true;
}

bool NotiAllSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	LOG("Noti All Send Connect\n");
	list<CUser*> connectList;
	g_userPool.getAllUserList(connectList);

	/*************************************
	 * Event Send 
	 *************************************/
	int32_t type = eventUser->_type;
	list<CUser*>::iterator it = connectList.begin();
	for ( ; it != connectList.end(); it++ )
	{
		CUser* user = (CUser*)*it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setNotiType(type, user, eventUser, connectList, &packet) || !packet )
		{
			LOG("Error Connector Type\n");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.enqueue(packet);
		LOG("User(%d) Send Noti\n", packet->_fd);
	}
	LOG("End All Send\n");

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
	/*******************************************************
	 * Connection 관련 함수 Add
	 *******************************************************/
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_TryConnect, ConnectAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_Connect, ConnectAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_DisConnect, ConnectAllSendFunc) );

	
	/*******************************************************
	 * Action 관련 함수 Add
	 *******************************************************/
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_Move, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_Stop, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_Jump, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_Shoot, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_GetHit, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_Spawn, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_UserSync, ActionAllSendFunc) );

	/*******************************************************
	 * Noti 관련 함수 Add
	 *******************************************************/
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GlobalNotice_NoticeInfo_KillInfo, NotiAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GlobalNotice_NoticeInfo_Notice, NotiAllSendFunc) );

	/*******************************************************
	 * Sector Index Setting 
	 *******************************************************/
	g_sectorIdx = ((X_GAME_MAX * Y_GAME_MAX)) / ((X_SECTOR_MAX * Y_SECTOR_MAX));
	g_userPool.initialize();
	LOG("g_sector initialize(%d)", g_sectorIdx);
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
		/***********************************************
		 * Queue를 하나 두고, 값을 확인해야 할거 같다. 		
		 * Queue에 값이 없으면, Input이 없는 상황이겠지. 
		 * BusyWait가 되어버린다.
		 * Signal Wait가 필요할거 같기도하다. 
		 ***********************************************/
		CProtoPacket* data = NULL;
		if ( session.m_readQ_Manager.isQueueDataExist() && (data = session.m_readQ_Manager.dequeue()) && data ) 
		{
			/*************************************
			 * Game Logic
			 * ALL_SEND
			 * PART_SEND
			 *
			 * Type은 이벤트 형식
			 *************************************/ 
			int32_t type = data->_type;
			if ( type < 0 )
			{
				LOG("Error Data\n");
				continue ;
			}

			LOG("User Type (%d)\n", type); 
			void (*func)(CSessionManager&, CProtoPacket*) = NULL;
			func = funcMap.find(type)->second;
			if ( func )
			{
				LOG("Start Func\n");
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
