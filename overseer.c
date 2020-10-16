#include "overseer.h"

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "./lib/global/macro.h"
#include "./lib/global/network.h"
#include "./lib/overseer/logging.h"
#include "./lib/overseer/queue.h"
#include "./lib/overseer/utility.h"

/** function prototype */
void start_thread();
void close_thread();
void* handle_request_loop(void* arg);
void set_signal();
void signal_handler(int num);

/** global variable */
pthread_t request_thread_pool[MAX_THREAD_NUMBER];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_var = PTHREAD_COND_INITIALIZER;
volatile sig_atomic_t stop = 0;

/** program main entry */
int main(int argc, char** argv) {
	socket_addr addr;
	init_sock_addr(&addr);

	/** get the user inputed port */
	get_port(argc, argv, &addr);

	/** create socket */
	int socket_fd = create_socket(addr, SERVER);

	/** start thread */
	start_thread();

	/** set signal handler */
	set_signal();

	
	
	/** accept connection and put request to linked list */
	while (!stop) {
		request* req = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(request));
		CHECK_MEM_ALLOCATION(req);
		socket_addr* in_addr = calloc(DEFAULT_ALLOCATION_SIZE, sizeof(socket_addr));
		CHECK_MEM_ALLOCATION(in_addr);
		
		/** accepting connection */
		int connection = accept_connection(socket_fd, in_addr);

		/** give connection log */
		print_log(stdout, "connection received from %s", in_addr->url);

		/** recive request */
		recv_request(connection, req);
		
		/** add request to the queue */
		pthread_mutex_lock(&mutex);
		queue_request(in_addr, req);
		pthread_mutex_unlock(&mutex);

		/** signal the thread that there is a request to be processed*/
		pthread_cond_signal(&condition_var);

	
		/** close connection to the cient */
		close_connection(connection);
	}

	/** print cleaning up resource log */
	print_log(stdout, "cleaning resource");

	/** close thread */
	close_thread();
	/** free the queue */
	free_queue();
	/** close socket */
	close_socket(socket_fd);
	
	
	return 0;
}

void start_thread() {
	/** set the thread attr */
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	/** start the thread */
	for (int i = 0; i < MAX_THREAD_NUMBER; i++)
		pthread_create(&request_thread_pool[i], &attr, handle_request_loop, NULL);
}

void close_thread() {
	for (int i = 0; i < MAX_THREAD_NUMBER; i++)
		pthread_join(request_thread_pool[i], NULL);
}

void* handle_request_loop(void* arg) {
	while (1) {
		/** get request from the queue */
		pthread_mutex_lock(&mutex);
		queue_node* req_node = deque_request();
		if (!req_node) { /** there is no request */
			pthread_cond_wait(&condition_var, &mutex);
			req_node = deque_request();
		}
		pthread_mutex_unlock(&mutex);

		if (req_node) { /** there is request */
			/** process request */
			process_request(*req_node->req);
			/** free request node */
			free_queue_node(req_node);
		}
	}

	return NULL;
}

void set_signal() {
	struct sigaction sa;
	sa.sa_flags = 0; /** disable the SA_RESTART */
	sigemptyset(&sa.sa_mask);
	sa.sa_handler = signal_handler;

	/** set SIGINT signal */
	CHECK(sigaction(SIGINT, &sa, NULL), "failed to set SIGINT signal handler (%s)", strerror(errno));
}

void signal_handler(int num) {
	stop = 1;
	print_log(stdout, "SIGINT recieved");
}