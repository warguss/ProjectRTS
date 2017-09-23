

#include "CSessionManager.h"

CSessionManager::CSessionManager(int port)
{
	_port = port;
	_serverSock = 0;
	_epoll_fd = 0;
}

CSessionManager::~CSessionManager()
{
	free(_events);
	close(_serverSock);
} 



void CSessionManager::connectInitialize()
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




void CSessionManager::waitEvent()
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
				}
				_connectUserEvent(clientSock);
			} 
			else
			{

			}
		}
	}
}


bool CSessionManager::_connectUserEvent(int fd)
{
	init_ev.events = EPOLLIN;
	init_ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSock, &init_ev);
	/* Lock */
	g_userPool.addUserInPool(clientSock, 0, 0);
	/* unLock */

	return true;
}

bool CSessionManager::_readUserEvent(int fd)
{
	char buffer[8192];
	memset(buffer, '\0', sizeof(char) * 8192);

	int readn = read(fd, buffer, 8192);
	if ( readn <= 0 )
	{
		close(fd);
	}
	
	/* Lock */
	/* Queue에 넣는다. */
	/* UnLock */
} 

bool CSessionManager::_deleteEvent(int fd)
{


	return true;
} 






