#include "network.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"

/**
 * @brief  initialize socket addr struct
 * @note   
 * @param  *addr: socket addr struct
 * @return None
 */
void init_sock_addr(socket_addr *addr) {
	addr->port = 0;
	CLEAR_CHAR_BUFFER(addr->url, MAX_URL_LEN);
}

/**
 * @brief  print the socket addr struct 
 * @note   
 * @param  addr: the socket addr struct
 * @return None
 */
void print_sock_addr(socket_addr addr) {
	printf("\033[0;32m");
	printf("sock_addr:\n");
	printf("\033[0m");
	printf("url: %s\n", addr.url);
	printf("port: %d\n", addr.port);
}

/**
 * @brief  create a new socket based on the given type
 * @note   create a new socket, if server type is give the socket will be bind and listen to the socket
 * @param  sock_addr: the socket address info
 * @param  type: socket type <CLIENT, SERVER>
 * @return 
 */
int create_socket(socket_addr sock_addr, socket_type type) {
	int socket_fd = CHECK(socket(AF_INET, SOCK_STREAM, 0), "socket creation failed!");

	if (type == SERVER) {
		/** configure the socket option */
		int opt_enable = 1;
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_enable, sizeof(opt_enable));
		setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &opt_enable, sizeof(opt_enable));

		/** bind the socket */
		struct sockaddr_in addr;
		CLEAR_STRUCT(&addr, sizeof(addr));
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_family = AF_INET;
		addr.sin_port = htons(sock_addr.port);

		CHECK(bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)), "binding socket failed! (%s)", strerror(errno));

		/** listen on the specified socket */
		CHECK(listen(socket_fd, MAX_CONNECTION_QUEUE), "listening on the socket failed! (%s)", strerror(errno));
	}

	return socket_fd;
}

/**
 * @brief  convert url to ip
 * @note   
 * @param  *url: the url
 * @return ip address
 */
char *url_to_ip(char *url) {
	struct hostent *hostname_ptr;
	hostname_ptr = gethostbyname(url);
	CHECK_IF_NULL(hostname_ptr, "failed to get IP addr");

	return inet_ntoa(*((struct in_addr *)hostname_ptr->h_addr_list[0]));
}

/**
 * @brief  open connection based on the socket fd
 * @note   
 * @param  socket_fd: socket file descriptor
 * @param  des_addr: info about the destination address
 * @return connection file descriptor
 */
int open_connection(int socket_fd, socket_addr des_addr) {
	int connection;
	struct sockaddr_in addr;
	CLEAR_STRUCT(&addr, sizeof(addr));

	char *ip = url_to_ip(des_addr.url);

	/** convert destination url to binary */
	CHECK(inet_pton(AF_INET, ip, &addr.sin_addr), "IP to binary conversion failed!");

	/** populate the rest of the connection details */
	addr.sin_family = AF_INET;
	addr.sin_port = htons(des_addr.port);

	/** create the connection */
	connection = CHECK(connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr)), "could not connect to oveerseer at %s %d!", ip, des_addr.port);

	return connection;
}

/**
 * @brief  accept incoming connection
 * @note   
 * @param  socket_fd: socket file descriptor
 * @param  *in_addr: variable to store the incoming address info
 * @return incoming connection file descriptor
 */
int accept_connection(int socket_fd, socket_addr *in_addr) {
	struct sockaddr_in client;
	CLEAR_STRUCT(&client, sizeof(struct sockaddr_in));
	socklen_t client_len = sizeof(client);

	/** accept incomming connection */
	int client_fd = CHECK2(accept(socket_fd, (struct sockaddr *)&client, &client_len), "failed to accept incomming connection! (%s)", strerror(errno));

	/** get incomming connection ip addr */
	char ip[INET_ADDRSTRLEN];
	CLEAR_CHAR_BUFFER(ip, INET_ADDRSTRLEN);

	inet_ntop(AF_INET, &client.sin_addr, ip, INET_ADDRSTRLEN);
	/** store connection ip addr, port and the connection_fd */
	strncpy(in_addr->url, ip, INET_ADDRSTRLEN);
	in_addr->port = (int)ntohs(client.sin_port);
	in_addr->connection_fd = client_fd;

	return client_fd;
}

/**
 * @brief  close connection
 * @note   
 * @param  connection_fd: the connection file descriptor
 * @return None
 */
void close_connection(int connection_fd) {
	shutdown(connection_fd, SHUT_RDWR);
	close(connection_fd);
}

/**
 * @brief  close socket
 * @note   
 * @param  socket_fd: the socket file descriptor
 * @return None
 */
void close_socket(int socket_fd) {
	shutdown(socket_fd, SHUT_RDWR);
	close(socket_fd);
}