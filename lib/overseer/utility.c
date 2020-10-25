#include "utility.h"

#include <errno.h>
#include <libgen.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../global/macro.h"
#include "../global/network.h"
#include "executor.h"
#include "logging.h"
#include "queue.h"

/**
 * @brief  Print the Usage message
 * @note   
 * @return None
 */
void usage() {
	fprintf(stderr, "%s\n", "error: usage <port>");
	exit(1);
}

/**
 * @brief  get the port number from the argument and store it in addr
 * @note   
 * @param  argc: argument count
 * @param  argv: argument vector
 * @param  addr: variable to store the port number
 * @return None
 */
void get_port(int argc, char** argv, socket_addr* addr) {
	/** check user input */
	if (argc != 2) {
		usage();
	}

	if (IS_INTERGER(argv[1])) {
		addr->port = atoi(argv[1]);
	} else {
		usage();
	}
}

/**
 * @brief  recieve incoming request from the controler and store the request in the req param
 * @note   This function will recieve request from the controler in the form of JSON.
 * 		   After recieving the JSON it will deserialize it into a request struct and store it in req 
 * @param  connection: the connection number 
 * @param  req: the variable to store the recieved request
 * @return None
 */
void recv_request(int connection, request* req) {
	char buffer[MAX_JSON_LEN];
	CLEAR_CHAR_BUFFER(buffer, MAX_JSON_LEN);

	/** recieve request */
	int nbyte = CHECK2(recv(connection, buffer, MAX_JSON_LEN, MSG_WAITALL), "failed recieving request (%s)", strerror(errno));
	SIZE_CHECK(nbyte, MAX_JSON_LEN, "request data corrupted");

	/** deserialize request */
	CHECK(sscanf(buffer,
	             REQUEST_DESERIALIZATION_FORMAT,
	             &req->o_flag,
	             req->out_file_path,
	             &req->t_flag,
	             &req->seconds,
	             &req->log_flag,
	             req->log_file_path,
	             &req->mem_flag,
	             &req->pid,
	             &req->memkill_flag,
	             &req->percentage,
	             &req->arguments_flag,
	             req->arguments),
	      "deserializing failed!");
}

/**
 * @brief  Create a new directory if not exist
 * @note   
 * @param  file_path: the file path
 * @return None
 */
void create_dir(char* file_path) {
	/** split the file path into directory path and file name */
	char file_path_cp[PATH_MAX];
	CLEAR_CHAR_BUFFER(file_path_cp, PATH_MAX);
	strncpy(file_path_cp, file_path, PATH_MAX);
	char* dir_path = dirname(file_path_cp);

	/** check if directory exist */
	struct stat st;
	if (stat(dir_path, &st) == -1) { /** directory didn't exist */
		CHECK(mkdir(dir_path, 0555), "failed to create directory %s (%s)", dir_path, strerror(errno));
	}
}

/**
 * @brief  Process the recieved request
 * @note   This function will process the recieved request from the controler
 * @param  request: the request
 * @param  pro_mutex: the mutex related to process linked list
 * @return None
 */
void process_request(request_queue_node request, pthread_mutex_t pro_mutex) {
	/** process mem request */
	if (request.req->mem_flag) {
		process_mem_req(request, pro_mutex);
	}
	/** process memkill request */
	if (request.req->memkill_flag) {
		process_memkill_req(request, pro_mutex);
	}

	/** check if request got an arguments to execute */
	if (request.req->arguments_flag) {
		request_exec(request, pro_mutex);
	}
}