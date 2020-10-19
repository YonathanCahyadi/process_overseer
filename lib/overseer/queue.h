#ifndef LIB_OVERSEER_QUEUE_H_
#define LIB_OVERSEER_QUEUE_H_

#include "../global/data_structure.h"
#include "../global/network.h"

typedef struct request_queue_node_struct {
    socket_addr *client_info;
    request* req;
	struct request_queue_node_struct* next;
} request_queue_node;


void queue_request(socket_addr* client_info, request* req) ;

request_queue_node* deque_request();

void free_request_queue_node(request_queue_node *node);

void free_request_queue();



#endif /** LIB_OVERSEER_QUEUE_H_ */