#ifndef _CSESSION_MANAGER_H_
#define _CSESSION_MANAGER_H_
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

#define EPOLL_SIZE 300
#define LOG printf

class CSessionManager
{
private:
	int _epoll_fd;
	struct epoll_event _init_ev, *_events;

	int _serverSock;
	int _port;


public:
	CSessionManager();
	~CSessionManager();
		
	void connectInitialize();
	bool waitEvent();

private:
	bool _connectUserEvent(int fd);
	bool _deleteUserEvent(int fd);
	bool _readUserEvent(int fd);
};

#endif
