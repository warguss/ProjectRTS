#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common/CProtoManager.h"
using namespace google::protobuf::io;

CProtoManager g_packetManager;


int main(int argc,char **argv)
{
    int sockfd,n;
    struct sockaddr_in servaddr;
 
	server2N::PacketBody packet;
	server2N::UserConnection con;

	con.set_id(1000);
	con.set_contype(server2N::UserConnection_ConnectionType_TryConnect);
	packet.set_allocated_connect(&con);
	


    sockfd=socket(AF_INET,SOCK_STREAM, 0);
    bzero(&servaddr,sizeof servaddr);
 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(10001);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 
    
	printf("Test\n");
#if 0 
	Local에서 Encoding & Decoding
	{
		unsigned char header[4];
		memset(header, '\0', sizeof(unsigned char) * 4);
		uint32_t bodyLength = 0;
		if ( !g_packetManager.encodingHeader(header, &packet, bodyLength) ) 
		{
			printf("Error encoding Header");
			return -1;
		}
		int writen = 0;
		printf("write Header [%d], writen[%d]\n", sizeof(header), writen);

		unsigned char body[bodyLength];
		memset(body , '\0', sizeof(body));
		if ( !g_packetManager.encodingBody(body, &packet, bodyLength) )
		{
			printf("Error encoding Body\n");
			return -1;
		}
		printf("write Body, bodyLegnth[%d] writen[%d]\n", bodyLength, writen);

		

		//memset(header, '\0', sizeof(header));
		if ( !g_packetManager.decodingHeader(header, 4, bodyLength) )
		{
			printf("Error Read Header\n");
			return -1;
		}
		printf("Read Header bodyLength[%d]\n", bodyLength);

		server2N::PacketBody* recvPacket = NULL;
		if ( !g_packetManager.decodingBody(body, bodyLength, bodyLength, recvPacket) )
		{
			printf("Error Read Body\n");
		}
		delete recvPacket;
		printf("End\n");
    }
#endif
    {
		if ( connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 )
		{	
			perror("connect");
			return -1;
		}
		unsigned char header[4];
		memset(header, '\0', sizeof(unsigned char) * 4);
		uint32_t bodyLength = 0;
		if ( !g_packetManager.encodingHeader(header, &packet, bodyLength) ) 
		{
			printf("Error encoding Header");
			return -1;
		}
		int writen = 0;
		writen = write(sockfd, header, sizeof(header));
		printf("write Header [%d], writen[%d]\n", sizeof(header), writen);

		unsigned char body[bodyLength];
		memset(body , '\0', sizeof(body));
		if ( !g_packetManager.encodingBody(body, &packet, bodyLength) )
		{
			printf("Error encoding Body\n");
			return -1;
		}
		writen = write(sockfd, body, sizeof(body));
		printf("write Body, bodyLegnth[%d] writen[%d]\n", bodyLength, writen);

		

		memset(header, '\0', sizeof(header));
		int readn = read( sockfd, header, 4);
		if ( !g_packetManager.decodingHeader(header, readn, bodyLength) )
		{
			printf("Error Read Header\n");
			return -1;
		}
		printf("Read Header\n");

		unsigned char recvBody[bodyLength];
		memset(recvBody, '\0', sizeof(recvBody));
		readn = read( sockfd, recvBody, bodyLength);
		server2N::PacketBody* recvPacket = NULL;
		if ( !g_packetManager.decodingBody(recvBody, readn, bodyLength, &recvPacket) )
		{
			printf("Error Read Body\n");
		}
		printf("End\n");
		if ( recvPacket ) 
		{
			recvPacket->clear_connect();
			recvPacket->clear_event();
			delete recvPacket;
			recvPacket = NULL;
		}
 	}
	
	return 0;
}
