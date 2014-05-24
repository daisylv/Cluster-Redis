/*
 * client.c
 *
 *  Created on: May 17, 2014
 *      Author: daisy
 */
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
#include <unistd.h>

int argcintlen(int i) {
	int len = 0;
	if (i < 0) {
		len++;
		i = -i;
	}
	do {
		len++;
		i /= 10;
	} while (i);
	return len;
}
size_t argvbulklen(size_t len) {
	return 1 + argcintlen(len) + 2 + len + 2;
}
int appendleftCommandArgv(char *target, int argc, const char **argv,
		const size_t *argvlen) {
	/* final command */
	int pos; /* position in final command */
	size_t len;
	int totlen, j;

	int first = argcintlen(argc);
	/* Calculate number of bytes needed for the command */
	totlen = 1 + first + 2;
	for (j = 0; j < argc; j++) {
		len = argvlen ? argvlen[j] : strlen(argv[j]);
		totlen += argvbulklen(len);
	}

	/* Build the command at protocol level */
	char* cmd = "";
	cmd = malloc(totlen + 1);

	pos = sprintf(cmd, "*%d\r\n", argc);
	for (j = 0; j < argc; j++) {
		len = argvlen ? argvlen[j] : strlen(argv[j]);
		pos += sprintf(cmd + pos, "$%d\r\n", len);
		memcpy(cmd + pos, argv[j], len);
		pos += len;
		cmd[pos++] = '\r';
		cmd[pos++] = '\n';
	}
	cmd[pos] = '\0';

	strcpy(target, cmd);
	return totlen;
}

int getSocket() {
	const char* addr = "127.0.0.1";
	int sfd, rv;
	char _port[6];
	struct addrinfo hints, *servinfo, *p;
	snprintf(_port, 6, "%d", 6379);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(addr, _port, &hints, &servinfo)) != 0) {
		hints.ai_family = AF_INET6;
		if ((rv = getaddrinfo(addr, _port, &hints, &servinfo)) != 0) {
			printf("error address\n");
			return -1;
		}
	}
	p = servinfo;
	printf("%d\n", p->ai_protocol);
	if ((sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		printf("error create socket\n");
		return -1;
	}
	if (connect(sfd, p->ai_addr, p->ai_addrlen) == -1) {
		printf("connect error\n");
		return -1;
	}
	return sfd;
}

int main() {


	char cmd[256] = "";
	char buf[256] = "";
	gets(buf);
	char **argv = malloc(10 * sizeof(char*));
	int argc = 0;
	while (strcmp(buf, "exit") != 0) {
		int sfd = getSocket();
		char *tmp = buf;
		argv[argc++] = tmp;
		while (*tmp != '\0') {
			if (*tmp != ' ') {
				++tmp;
			} else {
				*tmp = '\0';
				argv[argc++] = ++tmp;
			}
		}
		size_t *argvlen = malloc(argc * sizeof(size_t *));
		int i = 0;
		while (i < argc) {
			int len = strlen(argv[i]);
			argvlen[i] = len;
			++i;
		}
//		free(cmd);
		bzero(cmd, 256);
		appendleftCommandArgv(cmd, argc, argv, argvlen);
		write(sfd, cmd, 256);

		char reply[1024] = "";
		int ret = read(sfd, reply, 1024);
		char* s = reply;
		char* pos = reply;
		while(*s != '\0') {
			if (*s == '$'  || *s == '*') {
				++s;
				while(*s != '\r'){
					if(*s == '-') {
						break;
					}
					++s;
				}
			}
			if (*s == '\r' || *s == '\n') {
				++s;
			} else {
				*pos = *s;
				++pos;
				++s;
			}
		}
		*pos = '\r';
		++pos;
		*pos = '\n';
		++pos;
		*pos = '\0';
		if(ret == 0) {
			printf("wrong msg... please try again^-^\n");
		} else {
			printf("%s", reply);
		}
		close(sfd);
		argc = 0;
		bzero(buf, 256);
		gets(buf);
	}
}

