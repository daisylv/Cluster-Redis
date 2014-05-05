///*
// * test.c
// *
// *  Created on: Apr 26, 2014
// *      Author: daisy
// */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <poll.h>
#include <limits.h>

int main()
{
	const char* addr = "127.0.0.1";
	int sfd, rv;
	char _port[6];
	struct addrinfo hints, *servinfo, *p;
	snprintf(_port, 6, "%d", 6379);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
         hints.ai_family = AF_INET6;
         if ((rv = getaddrinfo(addr,_port,&hints,&servinfo)) != 0) {
            printf("error address\n");
            return -1;
        }
    }
    p = servinfo;
    if ((sfd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1)
    {
        printf("error create socket\n");
        return -1;
    }
    if (connect(sfd,p->ai_addr,p->ai_addrlen) == -1)
    {
    	printf("connect error\n");
    	return -1;
    }
    const char *command = "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n";
	char buffer[1024] = "";
	strcpy(buffer, command);
	send(sfd, buffer, 1024, 0);
	bzero(buffer, 1024);

	int length = 0;
	length = read(sfd, buffer, 1024);
	while (length)
	{
		if (length < 0)
		{
			printf("error receiving data\n");
			break;
		}
		int write_len = write(1, buffer, length);
		if (write_len < length)
		{
			printf("write Failed\n");
			break;
		}
		length = read(sfd, buffer, 1024);
	}
	return 0;
}







//#include <netinet/in.h>
//#include <sys/types.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <string.h>
//
//#define REDIS_PORT 6379
//#define BUFFER_SIZE 1024
//
//int main(int argc, char **argv)
//{
//	if (argc != 2)
//	{
//		printf("Error, need ip address...");
//		exit(1);
//	}
//	struct sockaddr_in client_addr;
//	bzero(&client_addr, sizeof(client_addr));
//	client_addr.sin_family = AF_INET;
//	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
//	client_addr.sin_port = htons(0);
//
//	printf("%s\n", client_addr.sin_addr);
//	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
//	if (client_socket < 0)
//	{
//		printf("Create Socket Failed!\n");
//		exit(1);
//	}
//	if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
//	{
//		printf("Client bind Failed\n");
//		exit(1);
//	}
//
//	struct sockaddr_in server_addr;
//	bzero(&server_addr, sizeof(server_addr));
//	server_addr.sin_family = AF_INET;
//	printf("%s\n", argv[1]);
//	if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
//	{
//		printf("server ip Error\n");
//		exit(1);
//	}
//	server_addr.sin_port = 6379;
//	socklen_t server_addr_length = sizeof(server_addr);
//	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
//	{
//		printf("can not connect to server\n");
//		exit(1);
//	}
//
//	const char *command = "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$7\r\nmyvalue\r\n";
//	char buffer[BUFFER_SIZE] = "";
//	strcpy(buffer, command);
//	send(client_socket, buffer, BUFFER_SIZE, 0);
//
//	bzero(buffer, BUFFER_SIZE);
//	int length = 0;
//	while (length = recv(client_socket, buffer, BUFFER_SIZE, 0))
//	{
//		if (length < 0)
//		{
//			printf("error receiving data\n");
//			break;
//		}
//		int write_len = write(1, buffer, length);
//		if (write_len < length)
//		{
//			printf("write Failed\n");
//			break;
//		}
//	}
//
//	return 0;
//}
//
//int
//
