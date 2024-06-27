#ifndef QUEUE_H_
#define QUEUE_H_

struct node {
    struct node *next;
    int* client_socket;
    struct Route* route;
};
typedef struct node node_t;

void enqueue(int* client_socket, struct Route* route);
node_t* dequeue();

#endif  // QUEUE_H_