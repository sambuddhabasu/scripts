/*
   Author: Sambuddha Basu
   The script lets users save a file from the internet by specifying the URL
 */

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAXBUF		1024

int main(int argc, char *argv[]) {
	int sockfd, yes = 1, content_len, is_content, port;
	char ip[MAXBUF], page[MAXBUF], buffer[MAXBUF], temp[MAXBUF], *token = NULL;
	struct sockaddr_in connect_addr;
	if(argc == 1) {
		fprintf(stderr, "No URL provided\n");
		exit(1);
	}
	bzero(ip, MAXBUF);
	bzero(page, MAXBUF);
	if(sscanf(argv[1], "http://%99[^:]:%99d/%99[^\n]", ip, &port, page) != 3) {
		sscanf(argv[1], "http://%99[^/]/%99[^\n]", ip, page);
		port = 80;
	}
	printf("%s\n%d\n%s\n", ip, port, page);
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(errno);
	}
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(errno);
	}
	connect_addr.sin_family = AF_INET;
	connect_addr.sin_port = htons(port);
	if(inet_aton("192.168.0.102", &connect_addr.sin_addr.s_addr) == -1) {
		perror("server");
		exit(errno);
	}
	memset(&(connect_addr.sin_zero), '\0', 8);
	if(connect(sockfd, (struct sockaddr *)&connect_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(errno);
	}
	bzero(buffer, MAXBUF);
	send(sockfd, "GET / HTTP/1.1\r\nHost: 192.168.0.102\r\n\r\n", 39, 0);
	recv(sockfd, buffer, sizeof(buffer), 0);
	is_content = 0;
	token = strtok(buffer, "\n");
	while(token) {
		if(sscanf(token, "Content-Length: %d", &content_len)) {
			is_content = 1;
			break;
		}
		token = strtok(NULL, "\n");
	}
	if(!is_content) {
		fprintf(stderr, "Could not find Content-Length in the response headers\n");
		exit(1);
	}
	printf("%d\n", content_len);
	close(sockfd);
	return 0;
}
