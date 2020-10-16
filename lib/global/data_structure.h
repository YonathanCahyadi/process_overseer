#ifndef LIB_GLOBAL_DATA_STRUCTURE_H_
#define LIB_GLOBAL_DATA_STRUCTURE_H_

#include <linux/limits.h>


#define NULL_STR "NULL"
#define NULL_STR_LEN 5
#define OFF 0
#define ON 1

typedef struct request_struct {
	int o_flag;
	int log_flag;
	int t_flag;
	int seconds;
	int mem_flag;
	int pid;
	int memkill_flag;
	int percentage;
	int arguments_flag;
	char out_file_path[PATH_MAX];
	char log_file_path[PATH_MAX];
	char arguments[PATH_MAX];
} request;

void init_request(request *req);

void print_request(request req);

#endif /** LIB_GLOBAL_DATA_STRUCTURE_H_ */