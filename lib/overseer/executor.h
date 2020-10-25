#ifndef LIB_OVERSEER_EXECUTOR_H_
#define LIB_OVERSEER_EXECUTOR_H_

#include <linux/limits.h>
#include <pthread.h>

#include "../global/data_structure.h"
#include "queue.h"

#define MAX_ACTIVE_PROCESS 10
#define DEFAULT_TIMEOUT 10

void request_exec(request_queue_node req_node, pthread_mutex_t pro_mutex);

void process_mem_req(request_queue_node req_node, pthread_mutex_t pro_mutex);

void process_memkill_req(request_queue_node req_node, pthread_mutex_t pro_mutex);

void kill_all_child(pthread_mutex_t pro_mutex);

#endif /** LIB_OVERSEER_EXECUTOR_H_ */