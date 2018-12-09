#include "CSessionManager.h"
#include "CUserPool.h"
#include "CAuthManager.h"

int CSessionManager::_serverSock = 0;
int CSessionManager::_epoll_fd = 0;
struct epoll_event CSessionManager::_init_ev;
struct epoll_event* CSessionManager::_events;

CQueueManager CSessionManager::m_readQ_Manager;
CQueueManager CSessionManager::m_writeQ_Manager;

static void* CSessionManager::waitEvent(void* val);
static void* CSessionManager::writeEvent(void* val);
static bool CSessionManager::_deleteUserAndEvent(CUser** user, bool isFirst);

CProtoManager g_packetManager;
extern CUserPool g_userPool;

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
CThreadLockManager g_lockManager(&g_mutex, &g_cond, false);

CSessionManager::CSessionManager(int port)
{
	_port = port;
	_serverSock = 0;
	_epoll_fd = 0;

	m_readQ_Manager.setType(READ_TYPE);
	m_writeQ_Manager.setType(WRITE_TYPE);
    connectInitialize();

	pthread_mutex_init(&g_mutex, (const pthread_mutexattr_t*)NULL);
	pthread_cond_init(&g_cond, (const pthread_condattr_t*)NULL);
}

CSessionManager::~CSessionManager()
{
    free(_events);
    close(_serverSock);
}

int CSessionManager::connectInitialize()
{
    struct sockaddr_in serverAddr;
	_serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if ( _serverSock <= 0 )
    {
        LOG_ERROR("Error Sock Open(%d)(%s)", errno, strerror(errno));
        return -1;
    }

	/************************
	 * SO_REUSEADDR Option
	 ************************/ 
	int option = 1;
	setsockopt(_serverSock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_port);

    if ( bind(_serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        LOG_ERROR("Serv SOck Bind Error(%d)(%s)", errno, strerror(errno));
        return -1;
    }

    if ( listen(_serverSock , BACKLOG_SIZE) < 0 )
    {
        LOG_ERROR("Errror List(%d)(%s)", errno, strerror(errno));
        return -1;
    }

	_epoll_fd = epoll_create(EPOLL_SIZE);
	_events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    _init_ev.events = EPOLLIN;
    _init_ev.data.fd = _serverSock;
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _serverSock, &_init_ev);

	LOG_DEBUG("Connect Initialize Complete");
	return 0;
}

bool CSessionManager::_deleteUserAndEvent(CUser** user, bool isFirst)
{
	if ( !*user )
	{
		return false;
	}

	CProtoPacket* disConnPacket = new CProtoPacket;
	disConnPacket->_type = (int32_t)server2N::UserConnection_ConnectionType_DisConnect;
	disConnPacket->_fd = (*user)->_fd;
	disConnPacket->_nickName = (*user)->_nickName.c_str();
	disConnPacket->_sector = (*user)->_sector;
	if ( isFirst )
	{
		delete *user;
		*user = NULL;
	}
	else
	{
		g_userPool.delUserInPool((*user)->_fd, (*user)->_sector);
	}

	m_readQ_Manager.enqueue(disConnPacket);
	return true;
}

static void* CSessionManager::waitEvent(void* val)
{
    while(true)
    {
		int event_count = epoll_wait(_epoll_fd, _events, EPOLL_SIZE, -1);
        if ( event_count == -1 )
        {
            break;
        }

		LOG_DEBUG("Epoll Count [%d]", event_count);
        for ( int i = 0; i < event_count; ++i )
        {
            if ( _events[i].data.fd == _serverSock )
            {
				LOG_DEBUG("Connect Epoll Event");
                /* Client 접속 */
                struct sockaddr_in clntAddr;
                int clntAddrSize = sizeof(clntAddr);
                int clientSock = accept(_serverSock, (sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize);
                if ( clientSock < 0 )
                {
                    LOG_ERROR("Error Client Set(%d)(%s)", errno, strerror(errno));
                    break;
                }

				bool isSuccess = false;
				_init_ev.events = EPOLLIN;
				_init_ev.data.fd = clientSock;
				epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSock, &_init_ev);
            }
            else
            {
				int fd = _events[i].data.fd;
				/******************************************
				 * 전체 조회(기존 접속 유저인지 판별)
				 ******************************************/
				bool isFirst = false;
				CUser* inputUser = g_userPool.findUserInPool(fd);
				if ( !inputUser )
				{
					/**************************************
					 * 신규 접속 유저의 경우
					 * Agent를 우선 확인한다.
					 **************************************/
					CAuthManager auth;
					bool isAgentSucc = true;
					do 
					{
						char buffer[AUTH_BYTE] = {'\0',};
						int32_t readn = read(fd, buffer, AUTH_BYTE);
						if ( readn <= 0 )
						{
							LOG_ERROR("Agent Buffer Not Exist (%d)", fd);
							isAgentSucc = false;
						} 

						isFirst = true;
						if ( !auth.authAgent(buffer) )
						{
							LOG_ERROR("Agent Fail (%d)", fd);
							isAgentSucc = false;
						}
					}
					while(false);

					CProtoPacket* authPacket = new CProtoPacket();
					auth.agentReturnBuffer(authPacket->_buffer, isAgentSucc);

					inputUser = new CUser;
					inputUser->setData(fd, READ_TYPE);
					authPacket->_fd = fd;
					authPacket->_authAgent = true;
					LOG_DEBUG("auth Buffer(%s)", authPacket->_buffer);

					int sector = g_userPool.addUserInPool(inputUser);
					if ( sector < 0 )
					{
						LOG_ERROR("Pool Not Available");
						_deleteUserAndEvent(&inputUser, isFirst);
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
						close(fd);
						continue ; 
					} 
					LOG_DEBUG("Sector Check(%d)", sector);
					inputUser->_sector = sector;
					m_readQ_Manager.unLock();
					m_readQ_Manager.enqueue(authPacket);
					continue ;
				}

				/******************************************
				 * Read Header
				 ******************************************/
				unsigned char header[HEADER_SIZE];
				memset(header, '\0', sizeof(header));
				int readn = read(fd, header, HEADER_SIZE);
				LOG_DEBUG("Client Input [%d], read size[%d]", fd, readn);
				if ( readn <= 0 )
				{
					LOG_INFO("@SUCC UID:%d NName:%s ATP:LOGOUT SECTOR:%d DES:(%d)(%s)", inputUser->_fd, inputUser->_nickName.c_str(), inputUser->_sector, errno, strerror(errno));
					_deleteUserAndEvent(&inputUser, isFirst);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					close(fd);
					continue;
				}

				uint32_t bodyLength = 0;
				if ( !g_packetManager.decodingHeader(header, readn, bodyLength) || bodyLength <= 0  )
				{
					LOG_ERROR("Error, Decoding Header Error[%d] length[%d]", fd, bodyLength);
					_deleteUserAndEvent(&inputUser, isFirst);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					close(fd);
					continue;
				}


				/******************************************
				 * Read Body 
				 ******************************************/
				unsigned char bodyBuf[bodyLength];
				memset(bodyBuf, '\0', sizeof(bodyBuf));
				readn = read(fd, bodyBuf, bodyLength);
				if ( readn <= 0 )
				{
					LOG_ERROR("Error, Delete Socket[%d]", fd);
					/**********************************
					 * Disconnect Event 보내야함
					 **********************************/

					LOG_INFO("@SUCC UID:%d NName:%s ATP:LOGOUT SECTOR:%d DES:Networks Error", inputUser->_fd, inputUser->_nickName.c_str(), inputUser->_sector);
					_deleteUserAndEvent(&inputUser, isFirst);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					close(fd);
					continue;
				}
				
				CProtoPacket* packet = NULL;
				if ( !g_packetManager.decodingBody(bodyBuf, readn, bodyLength, &packet) || !packet || !packet->SyncUser(inputUser))
				{
					LOG_ERROR("Error, Decoding Body Error[%d]", fd);
					_deleteUserAndEvent(&inputUser, isFirst);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					close(fd);
					continue;
				}
	
#if 0 
				if ( isFirst )
				{
					int sector = g_userPool.addUserInPool(inputUser);
					if ( sector < 0 )
					{
						LOG_ERROR("Pool Not Available");
						_deleteUserAndEvent(&inputUser, isFirst);
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
						close(fd);
						continue ; 
					} 
					LOG_DEBUG("Sector Check(%d)", sector);
					inputUser->_sector = sector;
				}
#endif

				packet->_fd = fd;
				packet->_sector = inputUser->_sector;
				packet->_nickName = inputUser->_nickName;
				LOG_INFO("Sector Set event(%d) user(%d)", packet->_sector, inputUser->_sector);
				/******************************************
				 * QueueManger에 넣는다.
				 * QueueManager 내부에서 Lock 처리한다.
				 * userPool 에서 꺼내야할듯
				 ******************************************/
				m_readQ_Manager.unLock();
				m_readQ_Manager.enqueue(packet);
				LOG_INFO("@SUCC UID:%d NName:%s ATP:%s SECTOR:%d", packet->_fd, packet->_nickName.c_str(), packet->_act.c_str(), packet->_sector);

				/******************************************
				 * 유저 Pool Size, 특정 Time에 따라
				 * Item Sapwn
				 ******************************************/
#if 0 
				if ( )
				{

				}

				CProtoPacket* itemPacket = new CProtoPacket();

				itemPacket->_proto = new server2N::PacketBody();
				itemPacket->_proto->set_senderid(fd);
				itemPacket->_fd = fd;
				itemPacket->_sector = 0;
				itemPacket->_nickName = "item";
				itemPacket->_protoEvent = new server2N::GameEvent();
				itemPacket->_type = server2N::GameEvent_action_EventItemSpawn;
				//packet->_item = g_itemManager.lastSpawnItemReturn();
				itemPacket->_protoEvent->set_acttype(server2N::GameEvent_action_EventItemSpawn);
				server2N::EventItemSpawn* protoItem = new server2N::EventItemSpawn();
				server2N::InfoItem* protoInfoItem = new server2N::InfoItem();
				protoInfoItem->set_weaponid(2);
				protoInfoItem->set_amount(5);
				protoInfoItem->set_itemtype(1);
				protoItem->set_itemid(1);
				protoItem->set_allocated_item(protoInfoItem);
				itemPacket->_protoEvent->set_allocated_itemspawnevent(protoItem);
				itemPacket->_protoEvent->set_eventpositionx((float)15);
				itemPacket->_protoEvent->set_eventpositiony((float)15);

				itemPacket->_proto->set_msgtype(server2N::PacketBody_messageType_GameEvent);
				itemPacket->_proto->set_allocated_event(itemPacket->_protoEvent);
				m_readQ_Manager.unLock();
				m_readQ_Manager.enqueue(itemPacket);
				LOG_DEBUG("Item ProtoPacket Send");
#endif
            }
        }
    }
}

static void* CSessionManager::writeEvent(void* val)
{
	while(true) 
	{
		/* signal ... */
		CProtoPacket* packet = NULL;
		if ( m_writeQ_Manager.isQueueDataExist() && (packet = m_writeQ_Manager.dequeue()) && packet )
		{
			uint32_t writeSize = 0;
			if ( packet->_authAgent )
			{
				LOG_DEBUG("auth Agent Send(%s) length(%d)", packet->_buffer, sizeof(char) * AUTH_BYTE);
				if ( writeSize = write(packet->_fd, packet->_buffer, sizeof(char) * AUTH_BYTE) <= 0 )
				{
					LOG_ERROR("Agent Packet Send Error (%d)", packet->_fd);
				}
				continue ;
			}

			/***************************************
			 * Write Header 
			 ***************************************/ 
			uint32_t bodyLength = 0;
			unsigned char header[HEADER_SIZE] = {'\0' , };
			if ( !g_packetManager.encodingHeader(header, packet->_proto, bodyLength) || bodyLength <= 0 )
			{
				LOG_ERROR("Error User ProtoPacket Not Exist");
				continue;
			}

			if ( (writeSize = write(packet->_fd, header, sizeof(unsigned char) * HEADER_SIZE )) < 0 ) 
			{
				LOG_ERROR("Write Error Socket[%d] writeSize[%d] (%d)(%s)", packet->_fd, writeSize, errno, strerror(errno));
				continue ; 
			}

			/***************************************
			 * Write Body 
			 ***************************************/ 
			unsigned char body[(int)bodyLength];
			memset(body, '\0', sizeof(unsigned char) * bodyLength);
			if ( !g_packetManager.encodingBody(body, packet->_proto, bodyLength) ) 
			{
				LOG_ERROR("Error Invalid Body Encoding");
				continue ;
			}

			if ( (writeSize = write(packet->_fd, body, sizeof(unsigned char) * bodyLength )) < 0 ) 
			{
				LOG_ERROR("Write Error Socket[%d] writeSize[%d](%d)(%s)", packet->_fd, writeSize, errno, strerror(errno));
				continue ;
			}
			/****************************************
			 * Packet Write 하고
			 * delete가 필요함
			 ****************************************/
			LOG_INFO("@SUCC UID:%d NName:%s ATP:%s SECTOR:%d", packet->_fd, packet->_nickName.c_str(), packet->_act.c_str(), packet->_sector);
		}
	}

	return (void*)0;
}
