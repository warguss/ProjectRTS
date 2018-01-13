
#define HEADER_SIZE 4
#define HEADER_BUFFER 1024
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



#define PROTOBUF_USER 0
#define PROTOBUF_ADDRESSBOOK 1

#define HEADER 2
#define BODY 3


#define ALL_SEND 1000
#define PART_SEND 1001
