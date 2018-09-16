#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include "server.h"

inline
int ServerFunctions::get_socket() {
	return socket_fd;
}

int ServerFunctions::setup_socket(int port) {

	socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (socket_fd < 0) {
		fprintf(stderr, "There Was An Error Creating Socket");
		return -1;
	} else {

		int val = 1; // no idea what this does

		// Configures socket to be reusable. When server crashes we won't get error messages
		int socket_setup = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int));

		if (socket_setup < 0) {
			fprintf(stderr, "Error Setting Up Socket For Reuse");
			exit(-1);
		}

		int bind_result = server_bind(port);

		if (bind_result < 0) {
			fprintf(stderr, "Error Binding To Specified Port");
			exit(-1);
		}

		int listen_result = listen_socket(socket_fd);

		if (listen_result < 0) {
			fprintf(stderr, "Error Listening to Specified Socket");
			exit(-1);
		}
		
		return socket_fd;
	}

}

int ServerFunctions::server_bind(int port_num) {
	struct sockaddr_in addr;
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port_num);
	return bind(get_socket(), (struct sockaddr*) &addr, sizeof(addr));

}	

int ServerFunctions::listen_socket(int socket) {
	int listen_socket = listen(socket, 32);

	if(listen_socket < 0) {
		fprintf(stderr, "Error Listening on the Specified Socket");
		return -1;
	} else {
		return 0;
	}
}


