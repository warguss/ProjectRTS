#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "proto/addressbook.pb.h"

#define LOG printf
using namespace std;
using namespace google::protobuf::io;

#define READ_BUFSIZE 8096
#define BACKLOG_SIZE 50
#define EPOLL_SIZE 301

extern CUserPool g_userPool;
extern pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

extern CQueue g_queue;
void* MainGaminLogic(void* userPool)
{
	int preUserCount = 0;
	while(true)
	{
		/* 유저 변화량을 체크한한다 */
		/* lock */
		if ( preUserCount != g_userPool.userCount() )
		{
			preUserCount = g_userPool.userCount();
			g_userPool.allSendEvent();
		}
		/* unLock */

		/* Queue를 하나 두고, 값을 확인해야 할거 같다. */		
		/* Queue에 값이 없으면, Input이 없는 상황이겠지. */
		

		/*
		 * UserOutput을 출력. 
		 * 우선적으로 시간에 따른 전체 초기화와
		 * 전체 이벤트 출력 조건이 필요
		 * 이외에는 구역  출력
		 */
		
	}

	return -1;
}


int main(int argc, char* argv[]) 
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	CSessionManager session;
	session.connectInitilize();
	session.waitEvent();
	

	pthread_create();
	
	google::protobuf::ShutdownProtobufLibrary();
	return -1;

}
#if 0 	
	while(true)
	{
		memset(buffer, '\0', (size_t)READ_BUFSIZE);
		int32_t readSize;
		LOG("accept Success\n");
		if ( (readSize = recv(clientSock, buffer, (int32_t)READ_BUFSIZE, 0)) > 0 )
		{
			LOG("buffer:[%s]", buffer);
			tutorial::AddressBook addressBook;
			/**************************************
			 * Read Buffer Save inputStream
			 * And Get Size used by ReadVariant32
			 * Parsing addressBook 
			 *************************************/
			google::protobuf::uint32 pbufSize;
			google::protobuf::io::ArrayInputStream inputStream(buffer, readSize);
			CodedInputStream codedStream(&inputStream);
			codedStream.ReadVarint32(&pbufSize);
		
			google::protobuf::io::CodedInputStream::Limit bufferLimit = codedStream.PushLimit(pbufSize);
			addressBook.ParseFromCodedStream(&codedStream);
			codedStream.PopLimit(bufferLimit);
			cout << "Message is " << addressBook.DebugString();

			/**************************************
			 * Write Buffer
			 **************************************/
			int32_t writeSize;
			if ( (writeSize = send(clientSock, buffer, (size_t)readSize, MSG_WAITALL) ) < 0 )
			{
				LOG("Error Send");
				perror("send");
			} 

		} 
		LOG("Success\n");
	}
#endif
	// Optional:  Delete all global objects allocated by libprotobuf.
