#include "queue.h"
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>


#include "../global/data_structure.h"
#include "../global/network.h"
#include "../global/macro.h"
#include "proc.h"
#include "logging.h"

static int num_of_queued_request = 0;
static request_queue_node* request_head = NULL;
static request_queue_node* request_tail = NULL;

static int num_of_active_process = 0;
static process_queue_node* process_head = NULL;
static process_queue_node* process_tail = NULL;

/**
 * @brief  Add request to the request linked list
 * @note   This function will add request and client info into the linked list 
 * @param  client_info: client info regarding where the request coming from
 * @param  req: the request recieved from the client
 * @return None
 */
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

/**
 * @brief  Get request from the request linked list
 * @note   This function will get the request from the head. (LIFO)
 * @return the request queue node
 */
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

/**
 * @brief  Free the dynamically allocated resource inside request queue node
 * @note   This function will free all of the dynamic resources inside the request queue node struct
 * @param  *node: Node to be freed 
 * @return None
 */
void free_request_queue_node(request_queue_node *node){
    
    /** free allocated dynamic resources */
    free(node->client_info);
    free(node->req);
    free(node);
}

/**
 * @brief  Remove all item in the request linked list
 * @note   This function will iterate through all the node in the request linked list and freed the node one by one
 * @return None
 */
void free_request_queue(){
    request_queue_node* tmp;
    while (request_head != NULL){
       tmp = request_head;
       request_head = request_head->next;
       free_request_queue_node(tmp);
       num_of_queued_request--;
    }
}

/**
 * @brief  Get the process linked list head
 * @note   This function will return the head of linked list where process history is stored
 * @return pointer to the process linked list head
 */
process_queue_node* get_process_queue_head(){
    return process_head;
}

/**
 * @brief  Add a new record in the record linked list
 * @note   This will add a new record in the process record linked list
 * @param  pid: the process pid
 * @param  *head: the process record linked list head
 * @return None
 */
void add_record(int pid, process_records *head){
    /** create new node */
    process_records *new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(process_records));
    CHECK_MEM_ALLOCATION(new_node);

    /** get the current time and store it */
    time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);
    strftime(new_node->time, TIME_FORMAT_LEN, TIME_FORMAT, tm_info);

    /** get the current mem usage */
    new_node->mem_usage = mem_usage(pid);

    /** percentage of memory used by the process */
    new_node->percentage = mem_usage_percentage(new_node->mem_usage);

    /** assign the next address */
    new_node->next = NULL;

    if(head->next == NULL){ /** check if record only have 1 item */
        head->next = new_node;
    }else{
        /** go to the last node */
        process_records *tmp = head;
        for(; tmp->next != NULL; tmp = tmp->next);
        tmp->next = new_node; /** add new record to the last node */
    }

}

/**
 * @brief  Get the lastest record stored in the process record linked list
 * @note   This will return the last node of the process record linked list specified by the head
 * @param  *head: the process record head
 * @return the latest record
 */
process_records* get_last_record(process_records *head){
    process_records *tmp_record = head;
	for(;tmp_record->next != NULL; tmp_record = tmp_record->next);
    return tmp_record;
}

/**
 * @brief  Delete all the recorded record
 * @note   This will delete all the node in the process record linked list specified by the head
 * @param  *head: tje process record head
 * @return None
 */
void free_process_record(process_records *head){
    for(;head != NULL; head = head->next){
        if(head == NULL) break;
        free(head);
    }
}

/**
 * @brief  Remove all item in the process linked list
 * @note   This function will iterate through all the node in the process linked list 
 *         and freed the node one by one, including the recorded process record
 * @return None
 */
void free_process_queue(){
    process_queue_node* tmp;
    while (process_head != NULL){
       tmp = process_head;
       process_head = process_head->next;
       free_process_record(tmp->records);
       free(tmp);
       num_of_active_process--;
    }

}

/**
 * @brief  Add process to the process linked list
 * @note   This function will intialize a record, the process info and add it to the process linked list  
 * @param  pid: the process pid
 * @param  arguments: the process executed arguments
 * @return None
 */
void queue_process(int pid, char* arguments){
    /** create new node */
    process_queue_node *new_node = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(process_queue_node));
    CHECK_MEM_ALLOCATION(new_node);

    /** populate the new node */
    new_node->pid = pid; 
    strcpy(new_node->arguments, arguments);   
    new_node->next = NULL;

    /** initiate the record */
    process_records* record = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(process_records));
    CHECK_MEM_ALLOCATION(record);

    record->mem_usage = 0;
    record->next = NULL;

    /** get the current time and store it */
    time_t timer = time(NULL);
    struct tm* tm_info = localtime(&timer);
    strftime(record->time, TIME_FORMAT_LEN, TIME_FORMAT, tm_info);
    
    /** add the record to the new node */
    new_node->records = record;
    new_node->record_len = new_node->record_len + 1;


    /** add new node to the linked list */
	if (num_of_active_process == 0) { /** linked list is empty */
		process_head = new_node;
		process_tail = new_node;
	} else { /** there is something on linked list */
		process_tail->next = new_node;
		process_tail = new_node;
	}

    /** increase the num of item stored in the linked list */
	num_of_active_process++;

 
}

/**
 * @brief  Update the record of all process in the linked list
 * @note   This will add new record to the stored process in the linked list
 * @return None
 */
void update_process_queue(){
    
    if(num_of_active_process > 0){ /** if ther is something to update */
        
        process_queue_node *tmp;
        tmp = process_head;

        /** update the record */
        while(tmp != NULL){
            add_record(tmp->pid, tmp->records);
            tmp->record_len = tmp->record_len + 1;
            tmp = tmp->next;
        }
    }
}



