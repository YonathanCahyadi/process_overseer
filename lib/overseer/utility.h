#ifndef LIB_OVERSEER_UTILITY_H_
#define LIB_OVERSEER_UTILITY_H_

#include <stdio.h>
#include <pthread.h>
#include "queue.h"

#include "../global/network.h"

#define REQUEST_DESERIALIZATION_FORMAT "{o_flag: %d, out_file: %[^\t\n,], t_flag: %d, seconds: %d, log_flag: %d, log_file: %[^\t\n,], mem_flag: %d, pid: %d, memkill_flag: %d, percentage: %f, arguments_flag: %d, arguments: %[^\t\n}]}"
#define WRITE 1
#define READ 0
#define DEFAULT_TIMEOUT 10

void get_port(int argc, char** argv, socket_addr* addr);

void recv_request(int connection, request* req);

void create_dir(char* file_path);

void process_request(request_queue_node req, pthread_mutex_t process_mutex);

#endif /** LIB_OVERSEER_UTILITY_H_ */