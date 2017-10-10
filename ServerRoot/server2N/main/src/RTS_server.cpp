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
#include "common/CUserPool.h"
#include "common/CQueueManager.h"




using namespace std;
using namespace google::protobuf::io;


extern CUserPool g_userPool;
extern CQueueManager g_queue;
extern pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

void* MainGaminLogic(void* userPool)
{
	while(true)
	{
		/*
		 * Queue를 하나 두고, 값을 확인해야 할거 같다. 		
		 * Queue에 값이 없으면, Input이 없는 상황이겠지. 
		 * BusyWait가 되어버린다.
		 * Signal Wait가 필요할거 같기도하다. 
		 */
		CData* data = NULL;
		if ( data = g_queue.dequeue() ) 
		{
			/* 
			 * 해석 로직이 필요함 buffer -> Object 
			 * Buffer 내용에 따라, 일부 보낼 지, 전체 보낼 지 결정
			 * QueueData -> 재 해석 -> CData로 재 변환.
			 */
			
		} 	
		



				
	}

	return -1;
}


int main(int argc, char* argv[]) 
{
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	
	CSessionManager session;

	pthread_create();
	
	google::protobuf::ShutdownProtobufLibrary();
	return -1;

}
