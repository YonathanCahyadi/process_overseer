#ifndef LIB_OVERSEER_QUEUE_H_
#define LIB_OVERSEER_QUEUE_H_

#include <linux/limits.h>
#include <pthread.h>

#include "../global/data_structure.h"
#include "../global/network.h"
#include "logging.h"

typedef struct request_queue_node_struct {
	socket_addr* client_info;
	request* req;
	struct request_queue_node_struct* next;
} request_queue_node;

typedef struct process_record_node_struct {
	char time[TIME_FORMAT_LEN];
	unsigned long mem_usage;
	float percentage;
	struct process_record_node_struct* next;
} process_records;

typedef struct process_queue_node_struct {
	int pid;
	char arguments[PATH_MAX];
	int record_len;
	process_records* records;
	struct process_queue_node_struct* next;
} process_queue_node;

void queue_request(socket_addr* client_info, request* req);

request_queue_node* deque_request();

void free_request_queue_node(request_queue_node* node);

void free_request_queue();

void queue_process(int pid, char* arguments);

process_records* get_last_record(process_records* head);

void update_process_queue();

void free_process_queue();

process_queue_node* get_process_queue_head();

#endif /** LIB_OVERSEER_QUEUE_H_ */