#include "overseer.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lib/global/macro.h"
#include "lib/global/network.h"
#include "lib/overseer/logging.h"
#include "lib/overseer/queue.h"
#include "lib/overseer/utility.h"
#include "lib/overseer/executor.h"

/** function prototype */
void start_thread();
void close_thread();
void* handle_request_loop(void* arg);
void* mem_usage_updater(void *arg);
void set_signal();
void signal_handler(int num);

/** global variable */
pthread_t request_thread_pool[MAX_THREAD_NUMBER];
pthread_t updater_thread_id;
pthread_mutex_t req_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t handler_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t process_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;

volatile int thread_loop_running = 1;
volatile sig_atomic_t stop = 0;
volatile int sigint_flag = 0;

int socket_fd;
int connection;
request *req;
socket_addr* in_addr;

/** program main entry */
int main(int argc, char** argv) {
	socket_addr addr;
	init_sock_addr(&addr);

	/** get the user inputed port */
	get_port(argc, argv, &addr);

	/** create socket */
	socket_fd = create_socket(addr, SERVER);

	/** start thread */
	start_thread();

	/** set signal handler */
	set_signal();

	/** accept connection and put request to linked list */
	while (!stop) {
		req = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(request));
		CHECK_MEM_ALLOCATION(req);
		in_addr = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(socket_addr));
		CHECK_MEM_ALLOCATION(in_addr);
		
		/** accepting connection */
		accept_connection(socket_fd, in_addr);

		/** give connection log */
		print_log(stdout, "connection received from %s", in_addr->url);

		/** recive request */
		recv_request(in_addr->connection_fd, req);
		
		/** add request to the queue */
		pthread_mutex_lock(&req_mutex);
		queue_request(in_addr, req);
		pthread_mutex_unlock(&req_mutex);

		/** signal the thread that there is a request to be processed*/
		pthread_cond_signal(&condition_var);
	}
	
	return 0;
}




/**
 * @brief  start all the thread needed
 * @note   this function will start:
 * 		   5 Thread for request handling
 * 		   1 Thread for updating process memory usage
 * @retval None
 */
void start_thread() {
	/** set the thread attr */
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/** start the thread for handling request */
	for (int i = 0; i < MAX_THREAD_NUMBER; i++){
		
		pthread_create(&request_thread_pool[i], &attr, handle_request_loop, NULL);
	}

	/** start the thread for updating child proecess memory usage */
	pthread_create(&updater_thread_id, &attr, mem_usage_updater, NULL);
}

/**
 * @brief  close all thread used
 * @note   this function will close:
 * 		   5 Thread for request handling
 * 		   1 Thread for updating process memory usage
 * @retval None
 */
void close_thread() {
	/** stop the loop inside the thread routine */
	pthread_mutex_lock(&handler_mutex);
	thread_loop_running = 0;
	pthread_mutex_unlock(&handler_mutex);
	
	/** wake every thread */
	pthread_cond_broadcast(&condition_var);

	/** close request handler thread */
	for (int i = 0; i < MAX_THREAD_NUMBER; i++){
		pthread_join(request_thread_pool[i], NULL);
	}

	/** close updater thread */
	pthread_join(updater_thread_id, NULL);
}

/**
 * @brief  handler for request execution and response
 * @note   
 * @param  arg: 
 * @retval None
 */
void* handle_request_loop(void* arg) {
	int running = 1;
	while (running) {
		/** get request from the queue */
		pthread_mutex_lock(&req_mutex);
		request_queue_node* req_node = deque_request();
		if (!req_node) { /** there is no request */
			pthread_cond_wait(&condition_var, &req_mutex);
			req_node = deque_request();
		}
		pthread_mutex_unlock(&req_mutex);


		if (req_node) { /** there is request */
			/** process request */
			process_request(*req_node, process_mutex);
			/** close connection to the client */
			close_connection(req_node->client_info->connection_fd);
			/** free request node */
			free_request_queue_node(req_node);
		}


		/** check if thread should running */
		pthread_mutex_lock(&handler_mutex);
		if(!thread_loop_running) running = 0;
		pthread_mutex_unlock(&handler_mutex);

	}

	return NULL;
}

/**
 * @brief  update the process memory usage
 * @note   This function will run every second
 * @param  *arg: 
 * @retval None
 */
void* mem_usage_updater(void *arg){
	
	int running = 1;
	while(running){
		pthread_mutex_lock(&process_mutex);
		update_process_queue();
		pthread_mutex_unlock(&process_mutex);
		
		sleep(1);

		/** check if loop should running */
		pthread_mutex_lock(&handler_mutex);
		if(!thread_loop_running) running = 0;
		pthread_mutex_unlock(&handler_mutex);
	}

	
	
	return NULL;
}

/**
 * @brief  set the signaling for main process
 * @note   This function will set the SIGINT
 * @retval None
 */
void set_signal() {
	struct sigaction sa;
	sa.sa_flags = 0; /** disable the SA_RESTART */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = signal_handler;

	/** set SIGINT signal */
	CHECK(sigaction(SIGINT, &sa, NULL), "failed to set SIGINT signal handler (%s)", strerror(errno));
}

/**
 * @brief  handler in case SIGNAL is recieved
 * @note   
 * @param  num: interupt number
 * @retval None
 */
void signal_handler(int num) {
	if(num == SIGINT && (sigint_flag == 0)){
		print_log(stdout, "SIGINT recieved");
		sigint_flag = 1;
		/** stop the loop in main */
		stop = 1;
		
		print_log(stdout, "Cleaning Resources and terminating");

		/** free unused allocated req and address holder */
		free(req);
		free(in_addr);

		/** kill all child */
		kill_all_child(process_mutex);
		
		/** close thread */
		close_thread();

		/** free the request queue */
		free_request_queue();

		/** free the process queue */
		free_process_queue();

		/** close socket */
		close_socket(socket_fd);

		/** exit the program */
		exit(0);
		
	}
}