#ifndef LIB_OVERSEER_QUEUE_H_
#define LIB_OVERSEER_QUEUE_H_

#include "../global/data_structure.h"

typedef struct queue_node_struct {
    request* req;
	struct queue_node_struct* next;
} queue_node;

void queue_request(request* req);

queue_node* deque_request();

void free_queue_node(queue_node *node);

void free_queue();

#endif /** LIB_OVERSEER_QUEUE_H_ */