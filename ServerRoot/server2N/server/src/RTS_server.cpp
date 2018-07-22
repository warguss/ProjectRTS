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
#include <sys/types.h>
#include <sys/wait.h>
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
	LOG_DEBUG("All Send Connect");
	list<CUser*> userConnector;
	g_userPool.getAllUserList(userConnector);

	/*************************************
	 * TryConnect 
	 *************************************/
	int32_t type = eventPacket->_type;
	CUser* eventUser = g_userPool.findUserInPool(eventPacket->_fd);
	if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect || type == (int32_t)server2N::UserConnection_ConnectionType_Connect )
	{
		if ( !eventUser )
		{
			LOG_ERROR("Not Exist EventUser");
			return false;
		} 
		CProtoPacket *connectPacket = NULL;
		if ( !g_packetManager.setConnectType(type, eventUser, eventPacket->_fd, userConnector, &connectPacket) ) 
		{
			LOG_ERROR("Error Connector Type");
			return false;
		}

		cout <<  connectPacket->_proto->DebugString() << endl;
		session.m_writeQ_Manager.enqueue(connectPacket);
		type = server2N::UserConnection_ConnectionType_Connect;
	}

	int userConnectSize = userConnector.size(); 
	list<CUser*>::iterator it = userConnector.begin();
	LOG_DEBUG("Connector User Size(%d)", userConnectSize);
	for ( ; it != userConnector.end(); it++ )
	{
		CUser* user = *it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setConnectType(type, eventUser, user->_fd, userConnector, &packet) ||  !packet )
		{
			LOG_ERROR("Error Connector Type");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		cout << "Test: " <<  packet->_proto->DebugString() << endl;
		//session.m_writeQ_Manager.releaseLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Noti", packet->_fd);
	}
	return true;
}

bool ActionPartSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	LOG_DEBUG("Part Send Event sector(%d)", eventUser->_sector);
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	list<CUser*> connectList;
	g_userPool.getPartUserList(connectList, eventUser->_sector);

	/*************************************
	 * Event Send 
	 *************************************/
	int32_t type = eventUser->_type;
	list<CUser*>::iterator it = connectList.begin();
	for ( ; it != connectList.end(); it++ )
	{
		CUser* recvUser = (CUser*)*it;
		CProtoPacket *packet = NULL;
		if ( !g_packetManager.setActionType(type, recvUser, eventUser, connectList, &packet) )
		{
			LOG_ERROR("Error Connector Type");
			return false;
		} 

		if ( recvUser && (recvUser->_fd == eventUser->_fd) && !packet )
		{
			continue ;
		}

		/*********************************
		 * Enqueue
		 *********************************/
		cout << "Test: " <<  packet->_proto->DebugString() << endl;
		session.m_writeQ_Manager.enqueue(packet);
		LOG_DEBUG("User(%d) Send Noti", packet->_fd);
	}
	LOG_DEBUG("End Part Send");

	return true;
}

bool ActionAllSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	LOG_DEBUG("Action All Send Connect");
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
			LOG_ERROR("Error Connector Type");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Noti", packet->_fd);


		if ( type == (int32_t)server2N::GameEvent_action_EventDeath )
		{
			CProtoPacket *notiPacket = NULL;
			if ( !g_packetManager.setNotiType(type, user, eventUser, connectList, &notiPacket) || !notiPacket )
			{
				LOG_ERROR("Error Connector Type");
				return false;
			}

			//session.m_writeQ_Manager.releaseLock();
			session.m_writeQ_Manager.enqueue(notiPacket);
			LOG_INFO("User(%d) Send Noti", packet->_fd);
		}
	}
	LOG_DEBUG("End All Send");
	return true;
}

bool NotiAllSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	LOG_DEBUG("Noti All Send Connect");
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
			LOG_ERROR("Error Connector Type");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.releaseLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Noti", packet->_fd);
	}
	LOG_DEBUG("End All Send");
	return true;
}

bool childProcessLogic()
{
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
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventMove, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventStop, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventJump, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventShoot, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventHit, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventSpawn, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventUserSync, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::GameEvent_action_EventDeath, ActionPartSendFunc) );

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

	LOG_DEBUG("g_sector initialize(%d)", g_sectorIdx);
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
				LOG_ERROR("Error Data");
				continue ;
			}

			LOG_DEBUG("User Type (%d)", type); 
			void (*func)(CSessionManager&, CProtoPacket*) = NULL;
			func = funcMap.find(type)->second;
			if ( func )
			{
				LOG_DEBUG("Start Func");
				func(session, data);
			}
			else
			{
				LOG_ERROR("Not Register Type");
			}
		} 	
	}
	int status;
	pthread_join(thread, (void**)&status);
	google::protobuf::ShutdownProtobufLibrary();
	LOG_DEBUG("Stop SuccessFully");
	return true;
} 

int main(int argc, char* argv[]) 
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	childProcessLogic();
	do 
	{
		pid_t pid;
		pid = fork();
		/*
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		*/
		if ( pid > 0 )
		{
			int status;
			pid_t child = waitpid(pid, &status, 0);
			if ( WIFSIGNALED(status) ) 
			{
				LOG_ERROR("Child Process Error", status);
				continue ; 
			} 
		}
		else if ( pid == 0 )
		{
			childProcessLogic();
		} 
		else
		{
			LOG_ERROR("Error Fork");
			return -1;
		}
	}while(true);
	return 0;
}
