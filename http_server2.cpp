#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

#include "server.h"

int handle_connection(int);

const std::string ok_response = std::string("HTTP/1.0 200 OK\r\n"
                                            "Content-type: text/plain\r\n"
                                            "Content-length: %d \r\n\r\n");

const std::string notok_response =
    std::string("HTTP/1.0 404 FILE NOT FOUND\r\n"
                "Content-type: text/html\r\n\r\n"
                "<html><body bgColor=black text=white>\n"
                "<h2>404 FILE NOT FOUND</h2>\n"
                "</body></html>\n");

const int STREAM_SIZE = 4096;

int main(int argc, char* argv[]) {
	struct sockaddr_storage sa;
	int port = -1;

	fd_set read_sockets;	// array of file descriptors
	fd_set read_copy;		

	FD_ZERO(&read_sockets);
	FD_ZERO(&read_copy);

	ServerFunctions socketObjs;	// CPP Obj

	if (argc < 3) {
		fprintf(stderr, "Usage Error... http_server2 k|u port");
		return -1;
	}

	port = atoi(argv[2]);

	if (port < 1500) {
		fprintf(stderr, "You're attempting to use a reserved port");
		return -1;
	}

	int socket_fd = socketObjs.setup_socket(port);

	FD_SET(socket_fd, &read_sockets);		// add listening descriptor to set

	if (socket_fd < 0) {
		fprintf(stderr, "Error Initializing Socket");
		return -1;
	}

	int largest = socket_fd;

	for(;;) {

		read_copy = read_sockets;

		int result = select(largest + 1, &read_copy, 0, 0, 0);

		// we have something to be selected
		if (result > 0) {
		
			// Iterate over every FD 
			for (int i = 0 ; i <= largest ; i++) {

				if (FD_ISSET(i, &read_copy)) {

					// this is an incoming connection
					if (i == socket_fd) {
						
						int accept_socket = accept(socket_fd, NULL, NULL);

						if (accept_socket < 0) {
							fprintf(stderr, "Error Accepting Socket");
							continue;
						} else {

							if (largest < accept_socket) {
								largest = accept_socket;
							}

							// add the socket to the original fd_set
							FD_SET(accept_socket, &read_sockets);

						}
					} else {
						// handle the connection
						int connection_socket = handle_connection(i);

						if (connection_socket < 0) {
							fprintf(stderr, "Error Handling the Request"); // crash here??
						}

						close(i);
						FD_CLR(i, &read_copy);	// remove value from fd_set

					}

				}
			}

		}

	}

}

int handle_connection(int incoming_fd) {
	FILE* fp = fdopen(incoming_fd, "rw");
	FILE* to_return = NULL;
	char* helper;		// helper for strtok_r
	char* get_header = NULL;
	char* substring = NULL;
	long long file_length;
	int ok = 1;

	get_header = (char*) malloc(8193); // one byte bigger for null term

	read(incoming_fd, get_header, 8192);
	strtok_r(get_header, " ", &helper); // extract HTTP

	substring = strtok_r(NULL, " ", &helper); // extract filename
	to_return = fopen(substring, "r");

	if (to_return == NULL) {
		send(incoming_fd, notok_response.c_str(),  strlen(notok_response.c_str()), 0);
		ok = 0;
	} else {
		int send_count = 1;

		// this is just crap to find lenth of file
		fseek(to_return, 0, SEEK_END);
		file_length = ftell(to_return);	
		rewind(to_return);
		char* file_buffer = (char*) malloc(STREAM_SIZE);     // send 4kb at a time i guess

		int num_digits = floor(log10(abs(file_length))) + 1; // length of the file_length number
		sprintf(file_buffer, ok_response.c_str(), file_length);

		while(send_count) {
			send(incoming_fd, file_buffer, STREAM_SIZE, 0);
			memset(file_buffer, 0, STREAM_SIZE);
			send_count = fread(file_buffer, sizeof(char), STREAM_SIZE, to_return);
		}
		
		free(file_buffer);

	}
	
	fclose(fp);

	if (to_return != NULL)
		fclose(to_return);

	free(get_header);
	get_header = NULL;
	close(incoming_fd);

	if (ok) {
		return 0;
	} else {
		return -1;
	}
}
