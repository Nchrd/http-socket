#include "HTTP_server.h"
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>

void init_server(HTTP_Server * http_server, int port) {
	http_server->port = port;

	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(server_socket == -1){
		printf("Error in socket creation: %s\n", strerror(errno));
		exit(1);
	}

	struct sockaddr_in server_address = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = {htonl(INADDR_ANY)},
	};

	int reuse = 1;
	if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0){ //Initially was SO_REUSEPORT instead but error was thrown
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		exit(1);
	}

	if(bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) != 0){
		printf("Error in binding: %s\n", strerror(errno));
		exit(1);
	}

	if(listen(server_socket, 5) != 0){
		printf("Error in listening: %s\n", strerror(errno));
		exit(1);
	}

	http_server->socket = server_socket;
	printf("HTTP Server Initialized\nPort: %d\n", http_server->port);
}