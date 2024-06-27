#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#include "HTTP_server.h"
#include "Routes.h"
#include "Response.h"
#include "Queue.h"

#define PORT 4221
#define THREAD_POOL_SIZE 30

pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* handle_connection(void* p_client_socket, struct Route* p_route);
void* thread_function(void* arg);

int main() {
	for(int i = 0; i < THREAD_POOL_SIZE; i++) {
		pthread_create(&thread_pool[i], NULL, thread_function, NULL);
	}
	// initiate HTTP_Server
	HTTP_Server http_server;
	init_server(&http_server, PORT);
	int client_socket;

	// registering Routes
	struct Route * route = initRoute("/", "index.html"); 
	route = addRoute(route, "/skills", "skills.html");
	route = addRoute(route, "/projects", "projects.html");
	route = addRoute(route, "/contact", "contact.html");
	route = addRoute(route, "/certifications", "certifications.html");

	printf("\n====================================\n");
	printf("=========ALL AVAILABLE ROUTES========\n");
	// display all available routes
	inorder(route);

	while (true) {
		client_socket = accept(http_server.socket, NULL, NULL);

		int* p_client = malloc(sizeof(int));
		struct Route* p_route = malloc(sizeof(struct Route));
		*p_client = client_socket;
		*p_route = *route;
		pthread_mutex_lock(&mutex);
		enqueue(p_client, p_route);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		//handle_connection(client_socket);
	}
	return 0;
}

void* handle_connection(void* p_client_socket, struct Route* p_route){
	int client_socket = *((int*)p_client_socket);
	free(p_client_socket);
	struct Route route = *p_route;
	free(p_route);

	if(client_socket == -1) {
			printf("Error in accepting connection: %s\n", strerror(errno));
			exit(1);
		}
	char client_msg[4096] = "";
	read(client_socket, client_msg, 4095);
	printf("%s\n", client_msg);

	// parsing client socket header to get HTTP method, route
	char *method = "";
	char *urlRoute = "";

	char *client_http_header = strtok(client_msg, "\n");
		
	printf("\n\n%s\n\n", client_http_header);

	char *header_token = strtok(client_http_header, " ");	
	int header_parse_counter = 0;

	while (header_token != NULL) {

		switch (header_parse_counter) {
			case 0:
				method = header_token;
			case 1:
				urlRoute = header_token;
		}
		header_token = strtok(NULL, " ");
		header_parse_counter++;
	}

	printf("The method is %s\n", method);
	printf("The route is %s\n", urlRoute);

	char template[100] = "";
	
	if (strstr(urlRoute, "/css/") != NULL) {
		//strcat(template, urlRoute+1);
		strcat(template, "css/index.css");
	}else if(strstr(urlRoute, "/img/background.jpg") != NULL){
		strcat(template, "img/background.jpg");
	}
	else {
		struct Route * destination = search(&route, urlRoute);
		strcat(template, "html/");

		if (destination == NULL) {
			strcat(template, "404.html");
		}else {
			strcat(template, destination->value);
		}
	}

	char * response_data = render_static_file(template);

	// The bigger the array, the more caracters can be put in the html file, compiler accept 18446744073709551615 as max size
	//char http_header[8000000] = "HTTP/1.1 200 OK\r\n\r\n";

	// Calcul de la taille totale de l'en-tête HTTP
	size_t http_header_size = strlen("HTTP/1.1 200 OK\r\n\r\n") + strlen(response_data) + 4;


	// Allocation dynamique de la mémoire pour l'en-tête HTTP
	char *http_header = malloc(sizeof(char) * (http_header_size+1));


	// Vérification de l'allocation de mémoire
	if (http_header == NULL) {
		fprintf(stderr, "Erreur lors de l'allocation de mémoire pour l'en-tête HTTP\n");
		exit(EXIT_FAILURE);
	}



	// Construction de l'en-tête HTTP
	strcpy(http_header, "HTTP/1.1 200 OK\r\n\r\n");

	strcat(http_header, response_data);
	strcat(http_header, "\r\n\r\n");

	send(client_socket, http_header, strlen(http_header), 0);
	close(client_socket);
	//free(response_data);
	return NULL;
}

void* thread_function(void* arg) {
    while(true){
        int* client_socket;
		struct Route* route;
		node_t* node = malloc(sizeof(node_t));
        pthread_mutex_lock(&mutex);
        if((node = dequeue()) == NULL){
            pthread_cond_wait(&cond, &mutex);
            node = dequeue();
        }
        
        pthread_mutex_unlock(&mutex);

        if(node->client_socket != NULL) {
            // handle client request
            handle_connection(node->client_socket, node->route);
        }
    }
}