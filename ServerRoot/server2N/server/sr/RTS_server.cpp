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


#include "common/MsgString.h"
#include "common/CSessionManager.h"
//#include "common/CUserPool.h"
//#include "common/CQueueManager.h"



using namespace std;
using namespace google::protobuf::io;


bool decodingProtoBuf(CUser* user)
{
	tutorial::AddressBook addressBook;
	google::protobuf::uint32 pbufSize;
	google::protobuf::io::ArrayInputStream inputStream(user->_buffer, user->_length);

	CodedInputStream codedStream(&inputStream);
	codedStream.ReadVarint32(&pbufSize);

	google::protobuf::io::CodedInputStream::Limit bufferLimit = codedStream.PushLimit(pbufSize);

	addressBook.ParseFromCodedStream(&codedStream);
	codedStream.PopLimit(bufferLimit);


	LOG("\nBuffer[%s]\n", user->_buffer);

#if 0 
	int32_t writeSize = 0;
	if ( writeSize = write(user->_fd, user->_buffer, (size_t)user->_length) < 0 )
	{
		perror("Send");
		LOG("Write Size[%d]\n",writeSize);
		return false;
	} 
	if ( (writeSize = send(user->_fd, user->_buffer, (size_t)user->_length, MSG_WAITALL)) < 0 )
	{
		perror("Send");
		LOG("Write Size[%d]\n",writeSize);
		return false;
	}
#endif

	return true;
} 



//CUserPool g_userPool;
//extern pthread_mutex_t g_main_mutex = PTHREAD_MUTEX_INITIALIZER;
//extern pthread_cond_t g_main_cond = PTHREAD_COND_INITIALIZER;
int main(int argc, char* argv[]) 
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	int port = 10001;
	pthread_t thread;
	CSessionManager session(port);
	
	LOG("Test1\n");
	if ( pthread_create(&thread, NULL, session.waitEvent, (void*)&port) < 0 )
	{
		exit(0);
	}
	
	if ( pthread_create(&thread, NULL, session.writeEvent, (void*)&port) < 0 )
	{
		exit(0);
	}

	while(1)
	{
		/*
		 * Queue를 하나 두고, 값을 확인해야 할거 같다. 		
		 * Queue에 값이 없으면, Input이 없는 상황이겠지. 
		 * BusyWait가 되어버린다.
		 * Signal Wait가 필요할거 같기도하다. 
		 */
		CUser* data = NULL;
		if ( data = session.m_readQ_Manager.dequeue(READ_TYPE) ) 
		{
			decodingProtoBuf(data);
			session.m_writeQ_Manager.enqueue(data);
			/* 
			 * 해석 로직이 필요함 buffer -> Object 
			 * Buffer 내용에 따라, 일부 보낼 지, 전체 보낼 지 결정
			 * QueueData -> 재 해석 -> CData로 재 변환.
			 */
			
		} 	
	}
	LOG("MainLogic4\n");



	
	
	int status;
	pthread_join(thread, (void**)&status);
	LOG("Test2\n");
	google::protobuf::ShutdownProtobufLibrary();
	return -1;

}
