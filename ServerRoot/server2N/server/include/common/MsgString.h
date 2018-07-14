#ifndef _MSG_STRING_H_
#include <errno.h>
#define _MSG_STRING_H_
#define HEADER_SIZE 4
#define HEADER_BUFFER 1024
#define BUFFER 8096
#define POOL_SIZE 300
#define EPOLL_SIZE 301
#define BACKLOG_SIZE 50

#include <stdio.h>
#include "CLoging.h"
extern CLoging g_logger;


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

//#define LOG_INFO(fmt, ...) { printf("INFO|%s|%s(%d)[%s] : " fmt "\n", __DATE__, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_INFO(...) { g_logger.log(CLoging::Info, __FILENAME__, __LINE__, __VA_ARGS__); }


//#define LOG_WARN(fmt, ...) { printf("WARN|%s|%s(%d)[%s] : " fmt "\n", __DATE__, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_WARN(...) { g_logger.log(CLoging::Warn, __FILENAME__, __LINE__, __VA_ARGS__); }


//#define LOG_DEBUG(fmt, ...) { printf("DEBUG|%s|%s(%d)[%s] : " fmt "\n", __DATE__, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_DEBUG(...) { g_logger.log(CLoging::Debug, __FILENAME__, __LINE__, __VA_ARGS__); }

//#define LOG_ERROR(fmt, ...) { printf("ERR|%s|%s(%d)[%s] : " fmt "\n", __DATE__, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }
#define LOG_ERROR(...) { g_logger.log(CLoging::Error, __FILENAME__, __LINE__, __VA_ARGS__); }


/* Type관련 Define */
#define NOT_SET 0
#define READ_TYPE 100
#define WRITE_TYPE 101

#define PROTOBUF_USER 0
#define PROTOBUF_ADDRESSBOOK 1

#define HEADER 2
#define BODY 3

#define ALL_SEND 1000
#define PART_SEND 1001

#define INVALID_USER 10001
#define X_SECTOR_MAX 20
#define Y_SECTOR_MAX 20
#define X_GAME_MAX 100
#define Y_GAME_MAX 100

#endif
