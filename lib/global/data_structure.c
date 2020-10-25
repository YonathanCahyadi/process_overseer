#include "./data_structure.h"

#include <stdio.h>
#include <string.h>

#include "./macro.h"


/**
 * @brief  intiialize request struct with default value
 * @note   
 * @param  *req: the pointer to request struct
 * @return None
 */
void init_request(request *req) {
    CLEAR_STRUCT(req, sizeof(request));
	strncpy(req->arguments, NULL_STR, NULL_STR_LEN);
	strncpy(req->log_file_path, NULL_STR, NULL_STR_LEN);
	strncpy(req->out_file_path, NULL_STR, NULL_STR_LEN);
	req->mem_flag = OFF;
	req->log_flag = OFF;
	req->o_flag = OFF;
	req->memkill_flag = OFF;
	req->t_flag = OFF;
	req->arguments_flag = OFF;
	req->seconds = 0;
	req->pid = 0;
	req->percentage = 0.0f;
}


/**
 * @brief  print the value contained in the stuct request
 * @note   
 * @param  req: the request struct
 * @return None
 */
void print_request(request req) {
	printf("\033[0;32m");
	printf("request:\n");
	printf("\033[0m");
	printf("o_flag: %d\n", req.o_flag);
	printf("out_file_path: %s\n", req.out_file_path);
	printf("log_flag: %d\n", req.log_flag);
	printf("log_file_path: %s\n", req.log_file_path);
	printf("t_flag: %d\n", req.t_flag);
	printf("seconds: %d\n", req.seconds);
	printf("mem_flag: %d\n", req.mem_flag);
	printf("pid: %d\n", req.pid);
	printf("memkill_flag: %d\n", req.memkill_flag);
	printf("percent: %f\n", req.percentage);
	printf("arguments_flag: %d\n", req.arguments_flag);
    printf("arguments: %s\n", req.arguments);
}