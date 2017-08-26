#include <iostream>
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"

#include "proto/addressbook.pb.h"

#define LOG printf
using namespace std;
using namespace google::protobuf::io;

#define READ_BUFSIZE 8096
#define BACKLOG_SIZE 50
#if 0 
// Iterates though all people in the AddressBook and prints info about them.
void ListPeople(const tutorial::AddressBook& address_book) {
	for (int i = 0; i < address_book.people_size(); i++) {
		const tutorial::Person& person = address_book.people(i);

		cout << "Person ID: " << person.id() << endl;
		cout << "  Name: " << person.name() << endl;
		if (person.has_email()) {
			cout << "  E-mail address: " << person.email() << endl;
		}

		for (int j = 0; j < person.phones_size(); j++) {
			const tutorial::Person::PhoneNumber& phone_number = person.phones(j);

			switch (phone_number.type()) {
				case tutorial::Person::MOBILE:
					cout << "  Mobile phone #: ";
					break;
				case tutorial::Person::HOME:
					cout << "  Home phone #: ";
					break;
				case tutorial::Person::WORK:
					cout << "  Work phone #: ";
					break;
			}
			cout << phone_number.number() << endl;
		}
	}
}
#endif




// Main function:  Reads the entire address book from a file and prints all
//   the information inside.
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

	struct sockaddr_in clntAddr;
	int clntAddrSize = sizeof(clntAddr);
	int clientSock = accept(serverSock, (sockaddr*)&clntAddr, (socklen_t*)&clntAddrSize); 
	if ( clientSock < 0 )
	{		
		LOG("Error Client Set\n");
		perror("accept");
	}
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

	// Optional:  Delete all global objects allocated by libprotobuf.
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
