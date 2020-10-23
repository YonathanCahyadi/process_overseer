#include "network.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "macro.h"

void init_sock_addr(socket_addr *addr) {
	addr->port = 0;
	CLEAR_CHAR_BUFFER(addr->url, MAX_URL_LEN);
}

void print_sock_addr(socket_addr addr) {
	printf("\033[0;32m");
	printf("sock_addr:\n");
	printf("\033[0m");
	printf("url: %s\n", addr.url);
	printf("port: %d\n", addr.port);
}

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

		CHECK(bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)), "binding socket failed!");

		/** listen on the specified socket */
		CHECK(listen(socket_fd, MAX_CONNECTION_QUEUE), "listening on the socket failed!");
	}

	return socket_fd;
}

char *url_to_ip(char *url) {
	struct hostent *hostname_ptr;
	hostname_ptr = gethostbyname(url);
	CHECK_IF_NULL(hostname_ptr, "failed to get IP addr");

	return inet_ntoa(*((struct in_addr *)hostname_ptr->h_addr_list[0]));
}

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

int accept_connection(int socket_fd, socket_addr *in_addr) {
	struct sockaddr_in client;
	CLEAR_STRUCT(&client, sizeof(struct sockaddr_in));
	socklen_t client_len = sizeof(client);

	/** accept incomming connection */
	int client_fd = CHECK(accept(socket_fd, (struct sockaddr *)&client, &client_len), "failed to accept incomming connection!");

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

void close_connection(int connection_fd) {
	shutdown(connection_fd, SHUT_RDWR);
	close(connection_fd);
}

void close_socket(int socket_fd) {
	shutdown(socket_fd, SHUT_RDWR);
	close(socket_fd);
}