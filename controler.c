#include <stdio.h>
#include <stdlib.h>

#include "./lib/controler/utility.h"
#include "./lib/global/network.h"
#include "./lib/global/data_structure.h"
#include "./lib/global/macro.h"

int main(int argc, char** argv) {
	socket_addr addr;
	init_sock_addr(&addr);
	request req;
	init_request(&req);
    

	/** get user input */
	get_user_input(argc, argv, &addr, &req);

    /** create the TCP socket */
	int socket_fd = create_socket(addr, CLIENT);

    /** open connection to the server */
	int connection = open_connection(socket_fd, addr);

    /** send the request */
	send_request(socket_fd, req);

    /** close connection and socket */
    close_connection(connection);
    close_socket(socket_fd);

	return 0;
}