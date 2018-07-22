#include "CSessionManager.h"
#include "CUserPool.h"
int CSessionManager::_serverSock = 0;
int CSessionManager::_epoll_fd = 0;
struct epoll_event CSessionManager::_init_ev;
struct epoll_event* CSessionManager::_events;

CQueueManager CSessionManager::m_readQ_Manager;
CQueueManager CSessionManager::m_writeQ_Manager;

static void* CSessionManager::waitEvent(void* val);
static void* CSessionManager::writeEvent(void* val);

CProtoManager g_packetManager;
extern CUserPool g_userPool;

CSessionManager::CSessionManager(int port)
{
	_port = port;
	_serverSock = 0;
	_epoll_fd = 0;

	m_readQ_Manager.setType(READ_TYPE);
	m_writeQ_Manager.setType(WRITE_TYPE);
    connectInitialize();

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

static void* CSessionManager::waitEvent(void* val)
{
    while(1)
    {
		LOG_DEBUG("epoll wait");
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
				 * fd해당하는 User를 Pool에서 먼저 찾는다.
				 * 이건 필요함, 
				 * Sector를 위해서도
				 *
				 * 전체 조회
				 ******************************************/
				bool isFirst = false;
				CUser* user = g_userPool.findUserInPool(fd);
				if ( !user )
				{
					user = new CUser;
					user->setData(fd, READ_TYPE);
					isFirst = true;
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
					LOG_ERROR("Error, Delete Socket[%d](%d)(%s)", fd, errno, strerror(errno));    
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					/**********************************
					 * Disconnect Event 보내야함
					 **********************************/
					CProtoPacket* disConnPacket = new CProtoPacket;
					disConnPacket->_type = (int32_t)server2N::UserConnection_ConnectionType_DisConnect;
					disConnPacket->_fd = fd;
					m_readQ_Manager.enqueue(disConnPacket);
					close(fd);
					LOG_SUCC("@SUCC UID:%d NName:%s ATP:LOGOUT SECTOR:%d", disConnPacket->_fd, disConnPacket->_nickName.c_str(), disConnPacket->_sector);
					continue;
				}

				uint32_t bodyLength = 0;
				if ( !g_packetManager.decodingHeader(header, readn, bodyLength) || bodyLength <= 0  )
				{
					LOG_ERROR("Error, Decoding Header Error[%d] length[%d]", fd, bodyLength);
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
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					g_userPool.delUserInPool(fd);
					close(fd);
					continue;
				}
				
				LOG_DEBUG("Body Set headerSize(%d) readSize(%d)", bodyLength, readn);
				CProtoPacket* packet = NULL;
				if ( !g_packetManager.decodingBody(bodyBuf, readn, bodyLength, &packet) || !packet || !packet->SyncUser(user))
				{
					LOG_ERROR("Error, Decoding Body Error[%d]", fd);
					continue;
				}
	
				if ( isFirst )
				{
					LOG_DEBUG("Set addUserInPool");
					g_userPool.addUserInPool(user);
				}
				packet->_fd = fd;
				packet->_sector = user->_sector;
				packet->_nickName = user->_nickName;
				LOG_INFO("Sector Set event(%d) user(%d)", packet->_sector, user->_sector);
				/******************************************
				 * QueueManger에 넣는다.
				 * QueueManager 내부에서 Lock 처리한다.
				 * userPool 에서 꺼내야할듯
				 ******************************************/
				m_readQ_Manager.enqueue(packet);
				LOG_SUCC("@SUCC UID:%d NName:%s ATP:LOGIN SECTOR:%d", packet->_fd, packet->_nickName.c_str(), packet->_sector);
            }
        }
    }
}

static void* CSessionManager::writeEvent(void* val)
{
	while(1) 
	{
		/* signal ... */
		//m_writeQ_Manager.setStartLock();
		CProtoPacket* packet = NULL;
		if ( packet = m_writeQ_Manager.dequeue() )
		{
			/***************************************
			 * Write Header 
			 ***************************************/ 
			uint32_t writeSize = 0;
			uint32_t bodyLength = 0;
			unsigned char header[HEADER_SIZE] = {'\0' , };
			LOG_DEBUG("Server -> User(%d) String(%s)", packet->_fd, packet->_proto->DebugString().c_str());
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
			LOG_SUCC("@SUCC UID:%d NName:%s ATP:LOGIN SECTOR:%d", packet->_fd, packet->_nickName.c_str(), packet->_sector);
		}
	}

	return (void*)0;
}
