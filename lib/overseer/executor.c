#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <wait.h>
#include <pthread.h>
#include "executor.h"
#include "logging.h"
#include "utility.h"
#include "../global/data_structure.h"
#include "../global/macro.h"

static int process_pool[MAX_ACTIVE_PROCESS] = { [0 ... (MAX_ACTIVE_PROCESS - 1)] = -1 };
static int num_of_active_process = 0;
pthread_mutex_t process_mutex = PTHREAD_MUTEX_INITIALIZER;


void queue_process(int pid){
    for(int i = 0; i < MAX_ACTIVE_PROCESS; i++){
        if(process_pool[i] == -1){
            process_pool[i] = pid;
            num_of_active_process++;
            break;
        }
    }
}

void deque_process(int pid){
    for(int i = 0; i < MAX_ACTIVE_PROCESS; i++){
        if(process_pool[i] == pid){
            process_pool[i] = -1;
            num_of_active_process--;
            break;
        }
    }
}

void kill_all_child(){
    for(int i = 0; i < MAX_ACTIVE_PROCESS; i++){
		if(process_pool[i] != -1){
        	CHECK(kill(process_pool[i], SIGKILL), "failed to kill process %d", process_pool[i]);
		}
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
		/** close the file, file descriptor */
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
		// CHECK(execvp(result_argv[0], result_argv), "could not execute %s", req.arguments);
		EXEC(execvp, result_argv, print_log, req.arguments);
		/** any argument past this, will not be executed */

	} else if (pid > 0) { /** parrent process */


        /** store child PID in the process pool */
        pthread_mutex_lock(&process_mutex);
        queue_process(pid);
        pthread_mutex_unlock(&process_mutex);

		/** check if child exist */
		int child_status_code = 0;
		/** send SIGTERM to Child process */
		

		/** wait for child process to terminate */
		sleep(5);
		waitpid(pid, &child_status_code, 0); /** blocking until child terminate*/
		/** log child termination */
		print_log(stdout, "%d has terminated with status code %d", pid, child_status_code);

        /** remove process from the process pool */
        pthread_mutex_lock(&process_mutex);
        deque_process(pid);
        pthread_mutex_unlock(&process_mutex);

		/** if log flag is on return the STDOUT and STDERR to the parent terminal */
		if (req.log_flag) {
			dup2(terminal_fd[0], STDOUT_FILENO);
			dup2(terminal_fd[1], STDERR_FILENO);
		}

	} else { /** fork failed */
		print_log(stderr, "fork failed (%s)", strerror(errno));
	}
}