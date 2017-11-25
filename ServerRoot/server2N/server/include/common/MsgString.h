
#define BUFFER 8096
#define POOL_SIZE 300
#define EPOLL_SIZE 301
#define BACKLOG_SIZE 50

#include <stdio.h>
#define LOG(fmt, ...) { printf("%s(%d)[%s] : " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); }

/* Type관련 Define */
#define NOT_SET 0
#define READ_TYPE 100
#define WRITE_TYPE 101

