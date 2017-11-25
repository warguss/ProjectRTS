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
        LOG("Error Sock Open");
        perror("socket:");
        return -1;
    }

	LOG("serverSock [%d]\n", _serverSock);
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(_port);

    if ( bind(_serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
    {
        LOG("Serv SOck Bind Error\n");
        perror("bind");
        return -1;
    }

    if ( listen(_serverSock , BACKLOG_SIZE) < 0 )
    {
        LOG("Errror List\n");
        perror("listen:");
        return -1;
    }

	_epoll_fd = epoll_create(EPOLL_SIZE);
	_events = (struct epoll_event*)malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
    _init_ev.events = EPOLLIN;
    _init_ev.data.fd = _serverSock;
    epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _serverSock, &_init_ev);

	LOG("Connect Initialize\n");
	return 0;
}

static void* CSessionManager::waitEvent(void* val)
{
    while(1)
    {
		LOG("epoll wait\n");
        int event_count = epoll_wait(_epoll_fd, _events, EPOLL_SIZE, -1);
        if ( event_count == -1 )
        {
            break;
        }

		LOG("Epoll Count [%d]\n", event_count);
        for ( int i = 0; i < event_count; ++i )
        {
			LOG("Wait Epoll Event\n");
            if ( _events[i].data.fd == _serverSock )
            {
				LOG("Connect Epoll Event\n");
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

				if ( !g_userPool.addUserInPool(clientSock, 0, 0) ) 
				{
					g_userPool.delUserInPool(clientSock);
					close(clientSock);
					continue ;
				} 
				epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSock, &_init_ev);
				g_userPool.allSendEvent();
                
            }
            else
            {
                /*
                 * Client Read
                 * Main Logic 전달 위해서
                 * Queue에 저장해야한다.
                 */
				char buffer[BUFFER];
				memset(buffer, '\0', sizeof(char) * BUFFER);
		
				int fd = _events[i].data.fd;
				int readn = read(fd, buffer, BUFFER);
				LOG("Client Input [%d], read size[%d]\n", fd, readn);
				if ( readn <= 0 )
				{
					LOG("Read Error, Delete Event\n");    
					LOG("CSessionManager _deleteEvent[%d]\n", fd);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
					g_userPool.delUserInPool(fd);
					close(fd);
				}

				/*
				 * QueueManger에 넣는다.
				 * QueueManager 내부에서 Lock 처리한다.
				 * userPool 에서 꺼내야할듯
				 */
				int type = READ_TYPE;
				m_readQ_Manager.enqueue(fd, buffer, readn, type);
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
		if ( user = m_writeQ_Manager.dequeue(WRITE_TYPE) )
		{
			int32_t writeSize = 0;
			if ( (writeSize = write(user->_fd, user->_buffer, (size_t)user->_length)) < 0 ) 
			{
				perror("Send");
				LOG("Write Size[%d]\n", writeSize);
			}	
		}
	}
	 

	return (void*)0;
}



#if 0 
bool CSessionManager::_connectUserEvent(int clientSock)
{
    bool isSuccess = false;
    _init_ev.events = EPOLLIN;
    _init_ev.data.fd = clientSock;
    /* Lock */
    if ( isSuccess = rPool.addUserInPool(clientSock, 0, 0) )
    {
        epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSock, &_init_ev);
        g_userPool.allSendEvent();
    }
    /* unLock */

    return isSuccess;
}
bool CSessionManager::_readUserEvent(int fd)
{
    char buffer[BUFFER];
    memset(buffer, '\0', sizeof(char) * BUFFER);

    int readn = read(fd, buffer, BUFFER);
    if ( readn <= 0 )
    {
        LOG("Read Error, Delete Event\n");
        _deleteUserEvent(fd);
    }

    /*
     * QueueManger에 넣는다.
     * QueueManager 내부에서 Lock 처리한다.
     */
    int type = READ_TYPE;
    manager.enqueue(fd, buffer, type);

    return true;
}

void CSessionManager::_deleteEvent(int fd)
{
    LOG("CSessionManager _deleteEvent[%d]\n", fd);
    epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
    close(fd);
}

#endif




