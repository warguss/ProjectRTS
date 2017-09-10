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
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "proto/addressbook.pb.h"

#define LOG printf
using namespace std;
using namespace google::protobuf::io;

#define READ_BUFSIZE 8096
#define BACKLOG_SIZE 50

extern CUserPool g_userPool;
extern pthread_mutex_t g mutex = PTHREAD_MUTEX_INITIALIZER;
extern pthread_cond_t g cond = PTHREAD_COND_INITIALIZER;
void* MainGaminLogic(void* userPool)
{
	while(true)
	{
		

	}

	return -1;
}


int main(int argc, char* argv[]) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;
	

	/************************
	 * Serv Socket Declare
	 ************************/
	int serverSock;
	int port = 10001;
	struct sockaddr_in serverAddr;

	serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock <= 0)
	{
		LOG("Error Socket Open");
		perror("socket:");
		return -1;
	}
	
	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	if ( bind(serverSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
	{
		LOG("Serv SOck Bind Error\n");
		perror("bind");
		return -1;
	} 

	if ( listen(serverSock , BACKLOG_SIZE) < 0 )
	{	
		LOG("Errror List\n");
		perror("listen:");
		return -1;
	} 

	/**************************
	 * Read Protocol Buffer
	 **************************/
	char buffer[READ_BUFSIZE];
	size_t servAddrSize = sizeof(serverAddr);

	pthread_create();
	while(true)
	{
		struct sockaddr_in clntAddr;
		int clntAddrSize = sizeof(clntAddr);
		int clientSock = accept(serverSock, (sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize); 
		if ( clientSock < 0 )
		{		
			LOG("Error Client Set\n");
			perror("accept");
		}
		
		/**************************
		 * 초기 게임 생성 위치 랜덤
		 **************************/

		g_userPool.addUserInPool(clientSock, 0, 0);
	}
	
	google::protobuf::ShutdownProtobufLibrary();
	return -1;

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
}
