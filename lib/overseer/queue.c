#include "queue.h"

#include <stdlib.h>

#include "../global/data_structure.h"
#include "../global/network.h"
#include "../global/macro.h"

static int num_of_queued_request = 0;
static request_queue_node* request_head;
static request_queue_node* request_tail;


void queue_request(socket_addr* client_info, request* req) {
	/** create new quest node */
	request_queue_node* new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(request_queue_node));
	CHECK_MEM_ALLOCATION(new_node);

	/** populate the new node */
    new_node->client_info = client_info;
	new_node->req = req;
	new_node->next = NULL;

	/** add new node to the linked list */
	if (num_of_queued_request == 0) { /** linked list is empty */
		request_head = new_node;
		request_tail = new_node;
	} else { /** there is something on linked list */
		request_tail->next = new_node;
		request_tail = new_node;
	}

    /** increase the num of item stored in the linked list */
	num_of_queued_request++;
}

request_queue_node* deque_request(){
    request_queue_node* retval = NULL;

    /** if there is something on the linked list */
    if(num_of_queued_request > 0){
        retval = request_head;
        request_head = request_head->next;

        /** if the node was last of the list */
        if(request_head == NULL) request_tail = NULL;

        /** decrease the num of item stored in the linked list */
        num_of_queued_request--;
    }

    return retval;
}

void free_request_queue_node(request_queue_node *node){
    free(node->client_info);
    free(node->req);
    free(node);
}

void free_request_queue(){
    request_queue_node* tmp;

   while (request_head != NULL){
       tmp = request_head;
       request_head = request_head->next;
       free_request_queue_node(tmp);
       num_of_queued_request--;
    }
}

