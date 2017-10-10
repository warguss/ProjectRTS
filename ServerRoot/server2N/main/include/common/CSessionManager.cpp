#include "CSessionManager.h"

CSessionManager::CSessionManager(int port)
{
	_port = port;
	_serverSock = 0;
	_epoll_fd = 0;
	connectInitialize();
}

CSessionManager::~CSessionManager()
{
	free(_events);
	close(_serverSock);
} 



void CSessionManager::_connectInitialize()
{
	struct sockaddr_in serverAddr;
	_serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if ( _serverSock <= 0 )
	{
		LOG("Error Sock Open");
		perror("socket:");
		return ;
	}
	
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
	_events = malloc(sizeof(struct epoll_event) * EPOLL_SIZE);
	_init_ev.events = EPOLLIN;
	_init_ev.data.fd = _serverSock;
	epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _serverSock, &init_ev);
} 	




static void* CSessionManager::waitEvent(void* val)
{
	while(true)
	{
		int event_count = epoll_wait(epoll_fd, events, EPOLL_SIZE, -1)
		if ( event_count == -1 )
		{
			break;
		}

		for ( int i = 0; i < event_count; ++i )
		{
			if ( events[i].data.fd == serverSock )
			{
				/* Client 접속 */
				struct sockaddr_in clntAddr;
				int clntAddrSize = sizeof(clntAddr);
				int clientSock = accept(serverSock, (sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize);
				if ( clientSock < 0 )
				{
					LOG("Error Client Set\n");
					perror("accept");
					break;
				}
				if ( !_connectUserEvent(clientSock) )
				{
					/*
					 * Pool에 User가 가득 들어찬 상황
					 * Error 문구 전송이 필요하다
					 */
					close(clientSock);
				}
			} 
			else
			{
				/* 
				 * Client Read 
				 * Main Logic 전달 위해서
				 * Queue에 저장해야한다.
				 */
				 _readUserEvent(events[i].data.fd);			

			}
		}
	}
}


bool CSessionManager::_connectUserEvent(int fd)
{
	bool isSuccess = false;
	init_ev.events = EPOLLIN;
	init_ev.data.fd = fd;
	/* Lock */
	if ( isSuccess = g_userPool.addUserInPool(clientSock, 0, 0) )
	{
		epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, clientSock, &init_ev);
		g_userPool.allSendEvent();
	} 
	/* unLock */

	return isSuccess;
}

bool CSessionManager::_readUserEvent(int fd)
{
	char buffer[BUFSIZE];
	memset(buffer, '\0', sizeof(char) * BUFSIZE);

	int readn = read(fd, buffer, BUFSIZE);
	if ( readn <= 0 )
	{
		LOG("Read Error, Delete Event\n");
		_deleteEvent(fd);
	}
	
	/* 
	 * QueueManger에 넣는다.
	 * QueueManager 내부에서 Lock 처리한다.
	 */
	int type = READ_TYPE;
	g_queue.enqueue(fd, buffer, type);

	return true;
} 

void CSessionManager::_deleteEvent(int fd)
{
	LOG("CSessionManager _deleteEvent[%d]\n", fd);
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, _events);
	close(fd);
} 
