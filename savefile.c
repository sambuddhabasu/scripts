/*
   Author: Sambuddha Basu
   The script lets users save a file from the internet by specifying the URL
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAXBUF		1024
#define h_addr		h_addr_list[0]

int main(int argc, char *argv[]) {
	int sockfd, yes = 1, port, rc;
	char ip[MAXBUF], ip_save[MAXBUF], page[MAXBUF], buffer[MAXBUF], temp[MAXBUF], request[MAXBUF], store_int[MAXBUF], *filename, *token = NULL;
	struct sockaddr_in connect_addr;
	struct hostent *he;
	FILE *fp;
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
	strcpy(ip_save, ip);
	filename = strrchr(page, '/');
	filename++;
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
	if(!inet_pton(AF_INET, ip, &(connect_addr.sin_addr.s_addr))) {
		if((he = gethostbyname(ip)) == NULL) {
			herror("gethostbyname");
			exit(1);
		}
		strcpy(ip, inet_ntoa(*((struct in_addr *)he->h_addr)));
		if(!inet_pton(AF_INET, ip, &(connect_addr.sin_addr.s_addr))) {
			fprintf(stderr, "%s\n", ip);
			exit(1);
		}
	}
	memset(&(connect_addr.sin_zero), '\0', 8);
	if(connect(sockfd, (struct sockaddr *)&connect_addr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(errno);
	}
	sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", page, ip_save);
	send(sockfd, request, strlen(request), 0);
	recv(sockfd, buffer, MAXBUF, 0);
	for(rc = 0; rc < MAXBUF - 4; rc++) {
		if(buffer[rc] == '\r' && buffer[rc + 1] == '\n' && buffer[rc + 2] == '\r' && buffer[rc + 3] == '\n')
			break;
	}
	rc += 4;
	fp = fopen(filename, "wb");
	while(rc < MAXBUF) {
		fputc(buffer[rc], fp);
		rc++;
	}
	while((rc = recv(sockfd, buffer, MAXBUF, 0)) > 0) {
		fwrite(buffer, rc, 1, fp);
	}
	fclose(fp);
	close(sockfd);
	return 0;
}
