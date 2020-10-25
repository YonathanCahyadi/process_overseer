#include "executor.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "../global/data_structure.h"
#include "../global/macro.h"
#include "logging.h"
#include "queue.h"
#include "utility.h"

volatile int sleep_interupt = 0;
pthread_mutex_t sleep_interupt_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief  Check if exec is successfuly executed
 * @note   The function will wait for 1 second before checking, 
 * 		   If child process is exist print log for sucessfull execution
 * @param  pid: The process PID that need to be checked
 * @param  req_node: The request
 * @return None
 */
void check_exec(int pid, request_queue_node req_node) {
	int child_status_code;
	/** check child status */
	sleep(1);
	if (waitpid(pid, &child_status_code, WNOHANG) == 0) { /** successfully running */
		print_log(stdout, "%s has been executed with pid %d", req_node.req->arguments, pid);
	}
}

/**
 * @brief  interupt sleep 
 * @note   
 * @retval None
 */
void interupt_sleep() {
	pthread_mutex_lock(&sleep_interupt_mutex);
	sleep_interupt = 1;
	pthread_mutex_unlock(&sleep_interupt_mutex);
}

/**
 * @brief  sleep while checking for interupt
 * @note   
 * @param  time: time to sleep
 * @return None
 */
void sleep2(int time) {
	int current_sec = 0;
	int interupt = 0;
	while ((current_sec < time) && !interupt) {
		pthread_mutex_lock(&sleep_interupt_mutex);
		if (sleep_interupt) interupt = 1;
		pthread_mutex_unlock(&sleep_interupt_mutex);
		sleep(1);
		current_sec++;
	}
}

/**
 * @brief  Handle the Child Process Signaling
 * @note   This function will wait for 10 second if t_flag in req_node is not ON and req_node->seconds is not specified.
 * 		   After 10 second or n seconds, SIGTERM will be sent to the process with the specified PID and wait for 5 seconds.
 * 		   After 5 seconds ends and process is still alive the SIGKILL will be sent. 
 * @param  pid: The Child Process PID 
 * @param  req_node: The request
 * @return None
 */
void child_process_signaling(int pid, request_queue_node req_node) {
	int child_status_code = 0;

	/** child process signaling time */
	if (req_node.req->t_flag) {
		sleep2(req_node.req->seconds);
	} else {
		sleep2(DEFAULT_TIMEOUT);
	}

	/** signaling and terminating the child process */
	if (waitpid(pid, &child_status_code, WNOHANG) == 0) { /** child process still exist */
		kill(pid, SIGTERM);
		print_log(stdout, "sent SIGTERM to %d", pid);

		sleep2(5);
		if (waitpid(pid, &child_status_code, WNOHANG) == 0) { /** child process still exist */
			kill(pid, SIGKILL);
			print_log(stdout, "sent SIGKILL to %d", pid);
		} else { /** child process finished */
			print_log(stdout, "%d has terminated with status code %d", pid, WEXITSTATUS(child_status_code));
		}
	} else { /** child process finished */
		print_log(stdout, "%d has terminated with status code %d", pid, WEXITSTATUS(child_status_code));
	}
}

/**
 * @brief  Handle the request arguments execution
 * @note   This function will execute the recieved request argument from the controler
 * @param  req_node: The request
 * @param  pro_mutex: The mutex related to process queue
 * @return None
 */
void request_exec(request_queue_node req_node, pthread_mutex_t pro_mutex) {
	/** check if log flag is on, if it's on redirect the logging */
	int terminal_fd[2]; /** saving the terminal file descriptor */
	if (req_node.req->log_flag) {
		/** save the current terminal file descriptor */
		terminal_fd[0] = dup(STDOUT_FILENO);
		terminal_fd[1] = dup(STDERR_FILENO);
		/** create dir if not exist */
		create_dir(req_node.req->log_file_path);
		/** create file */
		int file_fd = CHECK(open(req_node.req->log_file_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR), "error creating logging file (%s)", strerror(errno));
		/** redirect logging output to the file */
		dup2(file_fd, STDOUT_FILENO);
		dup2(file_fd, STDERR_FILENO);
		/** close the file, file descriptor */
		close(file_fd);
	}

	/** log execution attempt */
	print_log(stdout, "attempting to execute %s", req_node.req->arguments);

	/** split string of arguments into array of string */
	char temp_arg[sizeof(req_node.req->arguments)]; /** make a copy of req.arguments */
	CLEAR_CHAR_BUFFER(temp_arg, sizeof(temp_arg));
	strncpy(temp_arg, req_node.req->arguments, sizeof(temp_arg));

	/** calculate the number of string available */
	int num_of_string = 0;
	char* token = strtok(temp_arg, " ");
	while (token) {
		num_of_string++;
		token = strtok(NULL, " ");
	}

	/** begin splitting req_node.req->arguments */
	char* result_argv[num_of_string + 1];
	CLEAR_CHAR_BUFFER(temp_arg, sizeof(temp_arg));
	strncpy(temp_arg, req_node.req->arguments, sizeof(temp_arg));

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

	/** check if the requested arguments file exist */
	if (access(result_argv[0], F_OK) != -1) { /** requested arguments exist */
		/** fork process */
		pid_t pid = fork();
		if (pid == 0) { /** child process */

			/** if log flag is on return the STDOUT and STDERR to the child terminal */
			if (req_node.req->log_flag) {
				dup2(terminal_fd[0], STDOUT_FILENO);
				dup2(terminal_fd[1], STDERR_FILENO);
			}

			/** redirect ouput if o_flag is on */
			if (req_node.req->o_flag) {
				/** create dir if not exist */
				create_dir(req_node.req->out_file_path);
				/** create the file */
				int file_fd = CHECK(open(req_node.req->out_file_path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR), "error creating output file (%s)", strerror(errno));
				/** redirect stdout and stderr to the file*/
				dup2(file_fd, STDOUT_FILENO);
				dup2(file_fd, STDERR_FILENO);
				/** close the file, file descriptor */
				close(file_fd);
			}

			/** replace child proceses with the new proceses */
			EXEC(execvp, result_argv, print_log, req_node.req->arguments);
			/** any argument past this, will not be executed */

		} else if (pid > 0) { /** parrent process */

			/** store child PID in the process pool */
			pthread_mutex_lock(&pro_mutex);
			queue_process(pid, req_node.req->arguments);
			pthread_mutex_unlock(&pro_mutex);

			/** check exec status */
			check_exec(pid, req_node);

			/** Child signaling */
			child_process_signaling(pid, req_node);

		} else { /** fork failed */
			print_log(stderr, "fork failed (%s)", strerror(errno));
		}
	} else { /** requested arguments doesn't exist */
		print_log(stdout, "could not execute %s", req_node.req->arguments);
	}

	/** if log flag is on return the STDOUT and STDERR to the parent terminal */
	if (req_node.req->log_flag) {
		dup2(terminal_fd[0], STDOUT_FILENO);
		dup2(terminal_fd[1], STDERR_FILENO);
	}
}

/**
 * @brief  Kill all Child Process
 * @note   This function will go through the process linked list, and sent SIGKILL to all of the process stored there.
 * @param  pro_mutex: The mutex related to process queue
 * @return None
 */
void kill_all_child(pthread_mutex_t pro_mutex) {
	pthread_mutex_lock(&pro_mutex);
	process_queue_node* tmp = get_process_queue_head();
	int i = 0;
	if (tmp != NULL) {
		for (; tmp != NULL; tmp = tmp->next) {
			if (tmp == NULL) break;
			kill(tmp->pid, SIGKILL);
			i++;
		}
	}
	pthread_mutex_unlock(&pro_mutex);

	/** interupt all sleeping thread processing child process signaling */
	interupt_sleep();
}

/**
 * @brief  Handle the request for process memory usage
 * @note   This function will look for the memory usage for each process and send a response back to the controler
 * @param  req_node: The request
 * @param  pro_mutex: The mutex related to process queue
 * @return None
 */
void process_mem_req(request_queue_node req_node, pthread_mutex_t pro_mutex) {
	/** construct the data to be sent */
	char buf[DEFAULT_MEM_REQ_RES];
	CLEAR_CHAR_BUFFER(buf, DEFAULT_MEM_REQ_RES);

	if (req_node.req->pid == -1) { /** return all running process and last recorded memory usage */
		/** <pid> <bytes> <file> [arg...] */
		pthread_mutex_lock(&pro_mutex);
		process_queue_node* tmp = get_process_queue_head();
		if (tmp != NULL) {
			for (; tmp != NULL; tmp = tmp->next) {
				if (tmp == NULL) break;
				/** get the PID */
				char pid_buf[10];
				snprintf(pid_buf, 10, "%d\t", tmp->pid);
				strcat(buf, pid_buf);

				/** get the last record */
				process_records* tmp_record = get_last_record(tmp->records);
				char mem_buf[100];
				snprintf(mem_buf, 100, "%lu\t", tmp_record->mem_usage);
				strcat(buf, mem_buf);

				/** get the arg */
				strcat(buf, req_node.req->arguments);
				strcat(buf, "\n");
			}
		}
		pthread_mutex_unlock(&pro_mutex);

	} else { /** return all recorded memory usage of that specific pid */
		/** %Y-%m-%d %H:%M:%S <bytes> */
		pthread_mutex_lock(&pro_mutex);
		process_queue_node* tmp = get_process_queue_head();

		/** got to the specified PID */
		for (; tmp != NULL; tmp = tmp->next) {
			if (tmp != NULL) {
				if (tmp->pid == req_node.req->pid) break;
			}
		}

		if (tmp != NULL) {
			/** <pid> <bytes> <file> [arg...] */
			/** get all the usage  */
			if (tmp->pid == req_node.req->pid) { /** process with specified pid exist */
				process_records* tmp_record = tmp->records;
				for (; tmp_record != NULL; tmp_record = tmp_record->next) {
					strcat(buf, tmp_record->time);

					char mem_buf[100];
					snprintf(mem_buf, 100, "\t%lu\n", tmp_record->mem_usage);
					strcat(buf, mem_buf);
				}
			} else { /** process with specified pid doesn't exist */
				strcat(buf, "process doesn't exist");
			}
		}
		pthread_mutex_unlock(&pro_mutex);
	}

	int nbyte = CHECK(send(req_node.client_info->connection_fd, buf, DEFAULT_MEM_REQ_RES, 0), "error sending mem response");
	SIZE_CHECK(nbyte, DEFAULT_MEM_REQ_RES, "error sending mem usage response");
}

/**
 * @brief  Handle the request for memkill
 * @note   This function will kill all process that use memory bigger than the specified percentage
 * @param  req_node: The request
 * @param  pro_mutex: The mutex relate to process queue
 * @return None
 */
void process_memkill_req(request_queue_node req_node, pthread_mutex_t pro_mutex) {
	/** get each process last record of mem usage */
	pthread_mutex_lock(&pro_mutex);
	process_queue_node* tmp = get_process_queue_head();
	if (tmp != NULL) {
		for (; tmp != NULL; tmp = tmp->next) {
			if (tmp == NULL) break;

			/** get the last record */
			process_records* tmp_record = get_last_record(tmp->records);

			/** if process mem usage is bigger that specified treshold kill it */
			if (tmp_record->percentage > req_node.req->percentage) {
				kill(tmp->pid, SIGKILL);
				print_log(stdout, "process with pid %d has been killed", tmp->pid);
			}
		}
	}
	pthread_mutex_unlock(&pro_mutex);

	/** check which process use more memory than the specified thresshold */
}