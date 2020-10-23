#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

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

	/** if controler had mem flag ON*/
	if(req.mem_flag){
		char buf[DEFAULT_MEM_REQ_RES];
		CLEAR_CHAR_BUFFER(buf, DEFAULT_MEM_REQ_RES);
		int nbyte = CHECK(recv(socket_fd, buf, DEFAULT_MEM_REQ_RES, MSG_WAITALL), "error recv mem usage data. (%s)", strerror(errno));
		SIZE_CHECK(nbyte, DEFAULT_MEM_REQ_RES, "mem usage response is corrupted");

		if(req.pid == -1){
			printf("PID\tUsage\t<file> [arg..]\n");
		}else{
			printf("Time\t\t\tUsage\n");
		}
		printf("%s\n", buf);
	}

    /** close connection and socket */
    close_connection(connection);
    close_socket(socket_fd);

	return 0;
}