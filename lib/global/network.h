#ifndef LIB_GLOBAL_NETWORK_H_
#define LIB_GLOBAL_NETWORK_H_

#include "data_structure.h"

#define MAX_JSON_LEN 15000
#define DEFAULT_MEM_REQ_RES 100000
#define MAX_URL_LEN 2048
#define MAX_CONNECTION_QUEUE 20

typedef struct socket_addr_struct {
	char url[MAX_URL_LEN];
	int port;
	int connection_fd;
} socket_addr;

typedef enum socket_type_enum {
	CLIENT,
	SERVER
} socket_type;

int create_socket(socket_addr addr, socket_type type);

int open_connection(int socket_fd, socket_addr des_addr);

void init_sock_addr(socket_addr *addr);

void send_request(int socket_fd, request req);

int accept_connection(int socket_fd, socket_addr *in_addr);

void close_connection(int connection_fd);

void close_socket(int socket_fd);

void print_sock_addr(socket_addr addr);

#endif /**  LIB_GLOBAL_NETWORK_H_ */