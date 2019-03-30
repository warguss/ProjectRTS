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
#include "common/CProtoLogicFactory.h"

using namespace std;
bool ConnectAllSendFunc(CSessionManager& session, CProtoPacket* eventUser);
bool ActionPartSendFunc(CSessionManager& session, CProtoPacket* eventUser);
bool ActionAllSendFunc(CSessionManager& session, CProtoPacket* eventUser);
bool NotiAllSendFunc(CSessionManager& session, CProtoPacket* eventUser);
bool preActionFunc(CSessionManager& session, CProtoPacket* eventUser);
typedef bool (*CallBackFunc)(CSessionManager&, CProtoPacket*);
extern CLS_CALLBACK afxCreateClass(int32_t type);


extern int32_t g_sectorIdx;
extern CUserPool g_userPool;
extern CThreadLockManager g_lockManager;

bool ConnectAllSendFunc(CSessionManager& session, CProtoPacket* eventPacket)
{
	/*************************************
	 * 전체 유저 연결 정보 획득
	 *************************************/
	list<CUser*> userConnector;
	g_userPool.getAllUserList(userConnector);

	/*****************************************
	 * TryConnect : 접속 시도 
	 * Connect : Connect로 바꿔서 전체로 던짐
	 *
	 *****************************************/
	int32_t type = eventPacket->_type;
	CUser* eventUser = g_userPool.findUserInPool(eventPacket->_fd);
	if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect )
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
		session.m_writeQ_Manager.enqueue(connectPacket);
		type = server2N::UserConnection_ConnectionType_Connect;
	}

	int userConnectSize = userConnector.size(); 
	list<CUser*>::iterator it = userConnector.begin();
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
		session.m_writeQ_Manager.unLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) All Send Connection Event", packet->_fd);
	}

	return true;
}

bool ActionPartSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
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
			LOG_ERROR("Error Part Action Type");
			return false;
		} 

		if ( recvUser && (recvUser->_fd == eventUser->_fd) && !packet )
		{
			continue ;
		}

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.unLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_DEBUG("User(%d) Part Send Action Event", packet->_fd);
	}

	if ( type == (int32_t)server2N::UserEvent_action_EventDeath )
	{
		NotiAllSendFunc(session, eventUser);
	}

	return true;
}

bool ActionAllSendFunc(CSessionManager& session, CProtoPacket* eventUser)
{
	/*************************************
	 * 파트 유저 연결 정보 획득
	 *************************************/
	list<CUser*> connectList;
	g_userPool.getAllUserList(connectList);
	LOG_DEBUG("connectList(%d)", connectList.size());

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
			LOG_ERROR("Error All Action Type (%d)", user->_fd);
			continue;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.unLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Action All Send", packet->_fd);
	}

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
			LOG_ERROR("Error Noti All Type");
			return false;
		} 

		/*********************************
		 * Enqueue
		 *********************************/
		session.m_writeQ_Manager.unLock();
		session.m_writeQ_Manager.enqueue(packet);
		LOG_INFO("User(%d) Send Noti", packet->_fd);
	}

	return true;
}

// Factory 형식으로 수정 필요함, receive, onMessage, response (현재 너무 복잡함)
bool childProcessLogic()
{
	int port = 10001;
	pthread_t thread;
	CSessionManager session(port);
	
	map<int, CallBackFunc> funcMap;
	/*******************************************************
	 * Connection 관련 함수 Add
	 *******************************************************/
	//funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_TryConnect, ConnectAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_Connect, ConnectAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_DisConnect, ConnectAllSendFunc) );

	
	/*******************************************************
	 * User Event Action 관련 함수 Add
	 *******************************************************/
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_Nothing, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventStop, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventJump, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventShoot, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventHit, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventSpawn, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventUserSync, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventDeath, ActionPartSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserEvent_action_EventMove, ActionPartSendFunc) );

	/*******************************************************
	 * System Event Action 관련 함수 Add
	 *******************************************************/
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::SystemEvent_action_EventItemSpawn, ActionAllSendFunc) );
	funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::SystemEvent_action_EventItemGet, ActionAllSendFunc) );



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

	session.m_readQ_Manager.lock();
	session.m_writeQ_Manager.lock();

	LOG_DEBUG("g_sector initialize(%d)", g_sectorIdx);
	if ( pthread_create(&thread, NULL, session.waitEvent, (void*)&port) < 0 )
	{
		return false;
	}
	
	if ( pthread_create(&thread, NULL, session.writeEvent, (void*)&port) < 0 )
	{
		return false;
	}

	while(true)
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
			if ( data->_authAgent )
			{
				LOG_DEBUG("agent Send Write Session Q");
				session.m_writeQ_Manager.unLock();
				session.m_writeQ_Manager.enqueue(data);
				continue ;
			}

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

	//funcMap.insert( pair<int32_t, CallBackFunc>((int32_t)server2N::UserConnection_ConnectionType_TryConnect, ConnectAllSendFunc) );
			if ( type == (int32_t)server2N::UserConnection_ConnectionType_TryConnect || type == (int32_t)server2N::UserEvent_action_EventMove || type == (int32_t)server2N::SystemEvent_action_RequestUserInfo )
			{
				LOG_DEBUG("TEST");
				CLS_CALLBACK fnc = afxCreateClass(type);
				CProtoLogicBase* logic = fnc(false);
				if ( logic && logic->onPreProcess(data->_sector) && logic->onProcess(session, data) )
				{
					logic->onPostProcess(session);	
				} 

				delete logic;
				continue;
			}


			void (*func)(CSessionManager&, CProtoPacket*) = NULL;
			func = funcMap.find(type)->second;
			if ( func )
			{
				func(session, data);
			}
			else
			{
				LOG_ERROR("Not FuncCallback Register Type");
				
			}
		} 	
	}
	int status;
	pthread_join(thread, (void**)&status);
	google::protobuf::ShutdownProtobufLibrary();
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
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		pid = fork();
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
