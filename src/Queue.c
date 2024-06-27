#include "Queue.h"
#include "Routes.h"
#include <stdlib.h>
#include <stdio.h>

node_t* head = NULL;
node_t* tail = NULL;


void enqueue(int* client_socket, struct Route* route) {
    node_t* new_node = malloc(sizeof(node_t));
    new_node->client_socket = client_socket;
    new_node->route = route;
    new_node->next = NULL;

    if (tail == NULL) {
        head = new_node;
    } else {
        tail->next = new_node;
    }
    tail = new_node;
}

// Returns NULL if the queue is empty
// Returns the pointer to a client_socket, if there is no one to get
node_t* dequeue() {
    if (head == NULL) {
        return NULL;
    }else{
        int* client_socket = head->client_socket;
        struct Route* route = head->route;
        node_t* temp = head;
        head = head->next;
        if(head == NULL) {
            tail = NULL;
        }
        free(temp);
        node_t* node = malloc(sizeof(node_t));
        node->client_socket = client_socket;
        node->route = route;
        return node;
    }
}