#include "utility.h"

#include <getopt.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../global/data_structure.h"
#include "../global/macro.h"
#include "../global/network.h"

/**
 * @brief  print help message
 * @note   
 * @return None
 */
void help() {
	printf("Usage <address> <port> {[-o out_file] [-log log_file] [-t seconds] <file> [arg...] | mem [pid] | memkill <percent>}\n");
	printf("\t--help:\t print usage message\n");
	printf("\t-t:\t specifies the timeout for SIGTERM to be sent\n");
	printf("\t-log:\t redirect stdout of the overseer's managment of <file> to log_file\n");
	printf("\t-o:\t redirect stdout and stderr of the executed <file> to out_file\n");
	exit(0);
}

/**
 * @brief  print an error usage message
 * @note   
 * @return None
 */
void usage() {
	fprintf(stderr, "%s\n", "Error: Usage <address> <port> {[-o out_file] [-log log_file] [-t seconds] <file> [arg...] | mem [pid] | memkill <percent>}\n");
	exit(1);
}



/**
 * @brief  get the user input from the command line arguments, and store it in the params
 * @note   
 * @param  argc: arguments count
 * @param  **argv: arguments vector
 * @param  *sock_addr: pointer type sock_addr, where it store the network info <url, port> 
 * @param  *req: pointer type request, where it store the request to sent to the overseer
 * @return None
 */
void get_user_input(int argc, char **argv, socket_addr *sock_addr, request *req) {
	/** validate user input */
	if (argc < 3) {
		if (argc == 2) {
			if (strcmp(argv[1], "--help") == 0) {
				help();

			} else {
				usage();
			}
		} else {
			usage();
		}
	}

	/** get the inputed url and port */
	strncpy(sock_addr->url, argv[1], MAX_URL_LEN);
	if (IS_INTERGER(argv[2])) {
		sock_addr->port = atoi(argv[2]);
	} else {
		fprintf(stderr, "%s\n", "port must be an interger");
		exit(1);
	}

	if(argv[3] != NULL){
	int c;
	int log_flag = 0;
	int t_flag = 0;
	int mem_flag = 0;
	int memkill_flag = 0;
	int argument_flag = 0;


	/** check for mem */
	if(strncmp(argv[3], "mem", sizeof("mem")) == 0 ){
		if(memkill_flag || argument_flag) usage();
		mem_flag = 1;
		req->mem_flag = ON;
		if(argv[4] != NULL){
			if(IS_INTERGER(argv[4])){
				req->pid = atoi(argv[4]);
			}else{
				usage();
			}
		}else {
			req->pid = -1;
		}
	}

	/** check for memkill */
	if(strncmp(argv[3], "memkill", sizeof("memkill")) == 0 ){
		if(mem_flag || argument_flag) usage();
		memkill_flag = 1;
		req->memkill_flag = ON;
		if(argv[4] != NULL){	
			if(IS_FLOAT(argv[4])){
				req->percentage = atof(argv[4]);
			}else{
				usage();
			}
		}else {
			usage();
		}
	}

	/** id user got other things inputed */
	if (argc > 2) {
		/** get the inputed options */
		struct option options[] = {
		    {"o", required_argument, NULL, 'o'},
		    {"t", required_argument, NULL, 't'},
		    {"log", required_argument, NULL, 'l'},
		    {0, 0, 0, 0}};

		while ((c = getopt_long_only(argc, argv, "o:l:t:", options, NULL)) != EOF) {
			switch (c) {
			case 'o':
				if (memkill_flag || mem_flag || log_flag || t_flag || argument_flag) usage();
				// o_flag = 1;
				req->o_flag = ON;
				strncpy(req->out_file_path, optarg, PATH_MAX);
				break;
			case 'l':
				if (memkill_flag || mem_flag || t_flag || argument_flag) usage();
				log_flag = 1;
				req->log_flag = ON;
				strncpy(req->log_file_path, optarg, PATH_MAX);
				break;
			case 't':
				if (memkill_flag || mem_flag || argument_flag) usage();
				if (IS_INTERGER(optarg)) {
					t_flag = 1;
					req->t_flag = ON;
					req->seconds = atoi(optarg);
				} else {
					usage();
				}
				break;
			case ':':
			case '?':
			default:
				argument_flag = 1;
				break;
			}
		}

		if(!(mem_flag || memkill_flag)){ /** only take arguments only and only if mem and memkill falg is OFF */
			/** get the <file> [args...] */
			optind += 2;
			if (optind < argc) {
				char buffer[PATH_MAX];
				CLEAR_CHAR_BUFFER(buffer, PATH_MAX);
				while (optind < argc) {
					strcat(buffer, argv[optind++]);
					strcat(buffer, " ");
				}
				argument_flag = 1;
				req->arguments_flag = ON;
				strncpy(req->arguments, buffer, PATH_MAX);
			}
		}
	}
	}
}

/**
 * @brief  Make request struct into JSON format
 * @note   
 * @param  req: the request to serialize
 * @param  *result: variable to store the result of serialization 
 * @return None
 */
void serialize_request(request req, char *result) {
	/** serialize the request struct */
	CHECK(snprintf(result,
	               MAX_JSON_LEN,
	               REQUEST_SERIALIZATION_FORMAT,
	               req.o_flag,
	               req.out_file_path,
	               req.t_flag,
	               req.seconds,
	               req.log_flag,
	               req.log_file_path,
	               req.mem_flag,
	               req.pid,
	               req.memkill_flag,
	               req.percentage,
	               req.arguments_flag,
	               req.arguments),
	      "serializing failed!");
}

/**
 * @brief  send request through the socket 
 * @note   this function will serialize the request into JSON and send it over the socket
 * @param  socket_fd: the socket file descriptor
 * @param  req: the request to be sended
 * @return None
 */
void send_request(int socket_fd, request req) {
	char buffer[MAX_JSON_LEN];
	CLEAR_CHAR_BUFFER(buffer, MAX_JSON_LEN);
	serialize_request(req, buffer);

	int nbyte = send(socket_fd, buffer, sizeof(buffer), 0);
	SIZE_CHECK(nbyte, sizeof(buffer), "sending failed!");
}
