#ifndef LIB_CONTROLER_UTILITY_H_
#define LIB_CONTROLER_UTILITY_H_

#define REQUEST_SERIALIZATION_FORMAT "{o_flag: %d, out_file: %s, t_flag: %d, seconds: %d, log_flag: %d, log_file: %s, mem_flag: %d, pid: %d, memkill_flag: %d, percentage: %d, arguments_flag: %d, arguments: %s}"

#include "../global/data_structure.h"
#include "../global/network.h"

void get_user_input(int argc, char **argv, socket_addr *sock_addr, request *req);

#endif /** LIB_CONTROLER_UTILITY_H_ */