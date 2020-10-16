#include "utility.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "../global/macro.h"
#include "../global/network.h"
#include "logging.h"

void usage() {
	fprintf(stderr, "%s\n", "error: usage <port>");
	exit(1);
}

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

void recv_request(int connection, request* req) {
	char buffer[MAX_JSON_LEN];
	CLEAR_CHAR_BUFFER(buffer, MAX_JSON_LEN);

	/** recieve request */
	int nbyte = CHECK(recv(connection, buffer, MAX_JSON_LEN, 0), "failed recieving request");
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

void request_exec(request req) {
	/** check if log flag is on, if it's on redirect the logging */
	int terminal_fd[2]; /** saving the terminal file descriptor */
	if (req.log_flag) {
		/** save the current terminal file descriptor */
		terminal_fd[0] = dup(STDOUT_FILENO);
		terminal_fd[1] = dup(STDERR_FILENO);
		/** create dir if not exist */
		create_dir(req.log_file_path);
		/** create file */
		int file_fd = CHECK(open(req.log_file_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR), "error creating logging file (%s)", strerror(errno));
		/** redirect logging output to the file */
		dup2(file_fd, STDOUT_FILENO);
		dup2(file_fd, STDERR_FILENO);
		/** close the file, file descriptoe */
		close(file_fd);
	}

	/** log execution attempt */
	print_log(stdout, "attempting to execute %s", req.arguments);

	/** fork process */
	pid_t pid = fork();
	if (pid == 0) { /** child process */
		/** split string of arguments into array of string */
		char temp_arg[sizeof(req.arguments)]; /** make a copy of req.arguments */
		CLEAR_CHAR_BUFFER(temp_arg, sizeof(temp_arg));
		strncpy(temp_arg, req.arguments, sizeof(temp_arg));

		/** calculate the number of string available */
		int num_of_string = 0;
		char* token = strtok(temp_arg, " ");
		while (token) {
			num_of_string++;
			token = strtok(NULL, " ");
		}

		/** begin splitting req.arguments */
		char* result_argv[num_of_string + 1];
		CLEAR_CHAR_BUFFER(temp_arg, sizeof(temp_arg));
		strncpy(temp_arg, req.arguments, sizeof(temp_arg));

		/** split arguments and store it in result_argv */
		int i = 0;
		char* token2 = strtok(temp_arg, " ");
		while (token2) {
			result_argv[i] = token2;
			i++;
			token2 = strtok(NULL, " ");
		}
		/** add null to the end of result_argv */
		result_argv[num_of_string] = NULL;

		/** if log flag is on return the STDOUT and STDERR to the child terminal */
		if (req.log_flag) {
			dup2(terminal_fd[0], STDOUT_FILENO);
			dup2(terminal_fd[1], STDERR_FILENO);
		}

		/** redirect ouput if o_flag is on */
		if (req.o_flag) {
			/** create dir if not exist */
			create_dir(req.out_file_path);
			/** create the file */
			int file_fd = CHECK(open(req.out_file_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR), "error creating output file (%s)", strerror(errno));
			/** redirect stdout and stderr to the file*/
			dup2(file_fd, STDOUT_FILENO);
			dup2(file_fd, STDERR_FILENO);
			/** close the file, file descriptor */
			close(file_fd);
		}

		/** replace child proceses with the new proceses */
		CHECK(execvp(result_argv[0], result_argv), "could not execute %s", req.arguments);
		/** any argument past this, will not be executed */

	} else if (pid > 0) { /** parrent process */

		/** check if child exist */
		int child_status_code = 0;


		/** send SIGTERM to Child process */
		

		/** wait for child process to terminate */
		waitpid(pid, &child_status_code, 0); /** blocking until child terminate*/
		/** log child termination */
		print_log(stdout, "%d has terminated with status code %d", pid, child_status_code);

		/** if log flag is on return the STDOUT and STDERR to the parent terminal */
		if (req.log_flag) {
			dup2(terminal_fd[0], STDOUT_FILENO);
			dup2(terminal_fd[1], STDERR_FILENO);
		}

	} else { /** fork failed */
		print_log(stderr, "fork failed (%s)", strerror(errno));
	}
}

void process_request(request req) {
	/** check if request got an arguments to execute */
	if (req.arguments_flag) {
		request_exec(req);
	}
}