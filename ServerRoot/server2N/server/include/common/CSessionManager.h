#ifndef _MODULE_CSESSION_MANAGER_H_
#define _MODULE_CSESSION_MANAGER_H_

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CQueueManager.h"
#include "MsgString.h"
#include "CUserPool.h"
#include "CProtoManager.h"

using namespace std;

class CSessionManager
{
private:
	int _port;

public:
	static int _epoll_fd;
    static struct epoll_event _init_ev, *_events;

    static int _serverSock;

    static CQueueManager m_readQ_Manager;
    static CQueueManager m_writeQ_Manager;
    CSessionManager(int port);
    ~CSessionManager();

    int connectInitialize();
    static void* waitEvent(void* val);
    static void* writeEvent(void* val);

private:
    bool _connectUserEvent(int fd);
    bool _deleteUserEvent(int fd);
    bool _readUserEvent(int fd);
};
#endif

