#include "CSessionManager.h"
int CSessionManager::_serverSock = 0;
int CSessionManager::_epoll_fd = 0;
struct epoll_event CSessionManager::_init_ev;
struct epoll_event* CSessionManager::_events;

CQueueManager CSessionManager::m_readQ_Manager;
CQueueManager CSessionManager::m_writeQ_Manager;

static void* CSessionManager::waitEvent(void* val);
static void* CSessionManager::writeEvent(void* val);

CUserPool g_userPool;
CProtoManager g_packetManager;

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
        LOG("---ConnectInitialize() Error Sock Open\n");
        perror("socket:");
        return -1;
    }

	LOG("---ConnectInitialize() serverSock [%d]\n", _serverSock);
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_port);

    if ( bind(_serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        LOG("---ConnectInitialize() Serv SOck Bind Error\n");
        perror("bind");
        return -1;
    }

    if ( listen(_serverSock , BACKLOG_SIZE) < 0 )
    {
        LOG("---ConnectInitialize() Errror List\n");
        perror("listen:");
        return -1;
    }

	_epoll_fd = epoll_create(EPOLL_SIZE);
	_events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    _init_ev.events = EPOLLIN;
    _init_ev.data.fd = _serverSock;
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _serverSock, &_init_ev);

	LOG("---connectInitialize() Connect Initialize Complete\n");
	return 0;
}

static void* CSessionManager::waitEvent(void* val)
{
    while(1)
    {
		LOG("---waitEvent() epoll wait\n");
        int event_count = epoll_wait(_epoll_fd, _events, EPOLL_SIZE, -1);
        if ( event_count == -1 )
        {
            break;
        }

		LOG("---waitEvent() Epoll Count [%d]\n", event_count);
        for ( int i = 0; i < event_count; ++i )
        {
			LOG("---waitEvent() Epoll Event\n");
            if ( _events[i].data.fd == _serverSock )
            {
				LOG("---waitEvent() Connect Epoll Event\n");
                /* Client 접속 */
                struct sockaddr_in clntAddr;
                int clntAddrSize = sizeof(clntAddr);
                int clientSock = accept(_serverSock, (sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize);
                if ( clientSock < 0 )
                {
                    LOG("Error Client Set\n");
                    perror("accept");
                    break;
                }

				bool isSuccess = false;
				_init_ev.events = EPOLLIN;
				_init_ev.data.fd = clientSock;

#if 0 
				if ( !g_userPool.addUserInPool(clientSock, 0, 0) ) 
				{
					g_userPool.delUserInPool(clientSock);
					close(clientSock);
					continue ;
				} 
#endif
				epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSock, &_init_ev);
				//g_userPool.allSendEvent();
            }
            else
            {
				int fd = _events[i].data.fd;
				char buffer[HEADER_SIZE];
				memset(buffer, '\0', sizeof(char) * HEADER_SIZE);



				/******************************************
				 * fd해당하는 User를 Pool에서 먼저 찾는다.
				 ******************************************/
				CUser* user = g_userPool.findUserInPool(fd);
				if ( !user )
				{
					user = new CUser;
					user->setData(fd, READ_TYPE);
					g_userPool.addUserInPool(user);
				}

				/******************************************
				 * Read Header
				 ******************************************/
				int readn = read(fd, buffer, HEADER_SIZE);
				LOG("Client Input [%d], read size[%d]", fd, readn);
				if ( readn <= 0 )
				{
					LOG("Error, Delete Socket[%d]\n", fd);    
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					g_userPool.delUserInPool(fd);
					close(fd);
					continue;
				}
#if 0 
				if ( !user->decodingHeader(buffer, readn) )
				{
					LOG("Error, Decoding Header Error[%d]\n", fd);
					continue;
				}
#endif
				uint32_t bodyLength = 0;
				if ( !g_packetManager.decodingHeader(buffer, readn, bodyLength) || bodyLength <= 0  )
				{
					LOG("Error, Decoding Header Error[%d]\n", fd);
					continue;
				}


				/******************************************
				 * Read Body 
				 ******************************************/
				LOG("Body Set\n");
				unsigned char bodyBuf[bodyLength];
				memset(bodyBuf, '\0', sizeof(unsigned char) * bodyLength);
				readn = read(fd, bodyBuf, bodyLength);
				if ( readn <= 0 )
				{
					LOG("Error, Delete Socket[%d]\n", fd);    
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					g_userPool.delUserInPool(fd);
					close(fd);
					continue;
				}
				
				if ( !g_packetManager.decodingBody(buffer, readn, bodyLength, user->_protoPacket) )
				{
					LOG("Error, Decoding Header Error[%d]\n", fd);
					continue;
				}

				/******************************************
				 * QueueManger에 넣는다.
				 * QueueManager 내부에서 Lock 처리한다.
				 * userPool 에서 꺼내야할듯
				 ******************************************/
				m_readQ_Manager.enqueue(user);
				
            }
        }
    }
}

static void* CSessionManager::writeEvent(void* val)
{
	while(1) 
	{
		/* signal ... */
		CUser* user = NULL;
		if ( user = m_writeQ_Manager.dequeue() )
		{
			/***************************************
			 * Write Header 
			 ***************************************/ 
			uint32_t writeSize = 0;
			uint32_t bodyLength = 0;
			unsigned char header[HEADER_SIZE] = {'\0' , };
			if ( !g_packetManager.encodingHeader(header, user->_protoPacket, bodyLength) || bodyLength <= 0 )
			{
				LOG("Error User ProtoPacket Not Exist\n");
				continue;
			}

			if ( (writeSize = write(user->_fd, header, sizeof(unsigned char) * HEADER_SIZE )) < 0 ) 
			{
				perror("Send");
				LOG("---writeEvent() Write Error Socket[%d] writeSize[%d]", user->_fd, writeSize);
				continue ; 
			}
			LOG("Write User WriteSize(%d)\n", writeSize);


			/***************************************
			 * Write Body 
			 ***************************************/ 
			unsigned char body[(int)bodyLength];
			memset(body, '\0', sizeof(unsigned char) * bodyLength);
			if ( !g_packetManager.encodingBody(body, user->_protoPacket, bodyLength) ) 
			{
				LOG("Error Invalid Body Encoding");
				continue ;
			}

			if ( (writeSize = write(user->_fd, body, sizeof(unsigned char) * bodyLength )) < 0 ) 
			{
				perror("Send");
				LOG("---writeEvent() Write Error Socket[%d] writeSize[%d]", user->_fd, writeSize);
				continue ;
			}

			LOG("Write User WriteBody(%d)\n", writeSize);
		}
	}

	return (void*)0;
}
