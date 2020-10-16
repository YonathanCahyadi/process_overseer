#include "queue.h"

#include <stdlib.h>

#include "../global/data_structure.h"
#include "../global/macro.h"

static int num_of_queue = 0;
static queue_node* head;
static queue_node* tail;

void queue_request(request* req) {
	/** create new quest node */
	queue_node* new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(request));
	CHECK_MEM_ALLOCATION(new_node);

	/** populate the new node */
	new_node->req = req;
	new_node->next = NULL;

	/** add new node to the linked list */
	if (num_of_queue == 0) { /** linked list is empty */
		head = new_node;
		tail = new_node;
	} else { /** there is something on linked list */
		tail->next = new_node;
		tail = new_node;
	}

    /** increase the num of item stored in the linked list */
	num_of_queue++;
}

queue_node* deque_request(){
    queue_node* retval = NULL;

    /** if there is something on the linked list */
    if(num_of_queue > 0){
        retval = head;
        head = head->next;

        /** if the node was last of the list */
        if(head == NULL) tail = NULL;

        /** decrease the num of item stored in the linked list */
        num_of_queue--;
    }

    return retval;
}

void free_queue_node(queue_node *node){
    free(node->req);
    free(node);
}

void free_queue(){
    for(; head != NULL; head = head->next){
        free_queue_node(head);
    }
    num_of_queue = 0;
}