#ifndef LIB_OVERSEER_EXECUTOR_H_
#define LIB_OVERSEER_EXECUTOR_H_

#include "../global/data_structure.h"

#define MAX_ACTIVE_PROCESS 10

void request_exec(request req);

void kill_all_child();



#endif /** LIB_OVERSEER_EXECUTOR_H_ */