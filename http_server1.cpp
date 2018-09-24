#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>

#include "server.h"


int handle_connection(int);

const char ok_response[]  = "HTTP/1.0 200 OK\r\n" \
                       "Content-type: text/plain\r\n"\
                        "Content-length: %lld \r\n\r\n";

const char notok_response[] = "HTTP/1.0 404 FILE NOT FOUND\r\n" \
                        "Content-type: text/html\r\n\r\n" \
                        "<html><body bgColor=black text=white>\n" \
                        "<h2>404 FILE NOT FOUND</h2>\n"		\
                        "</body></html>\n";

const int STREAM_SIZE = 4096;


int main(int argc, char* argv[]) {
	struct sockaddr_storage sa;
	int port = -1;

	ServerFunctions socketObjs;	// CPP Obj
	socklen_t sock_len;

	if (argc < 3) {
		fprintf(stderr, "Usage error... http_server1 k|u port");
		return -1;
	}

	port = atoi(argv[2]);

	if (port < 1500) {
		fprintf(stderr, "You're attempting to use a reserved port");
		return -1;
	}

	if (toupper(*argv[1]) == 'K') {
		// init kernel stack
	} else if (toupper(*argv[1]) == 'U') {
		// init user stack
	} else {
		fprintf(stderr, "Error... Unknown Option\n");
		return -1;
	}

	int socket_fd = socketObjs.setup_socket(port);

	sock_len = sizeof(sa);

	for(;;) {
	
		int connection_fd = accept(socket_fd, (struct sockaddr*) &sa, &sock_len);	// this is blocking

		if (connection_fd < 0) {
			fprintf(stderr, "Fatal Error\n");
			exit(-1);
		}

		/**
		  * sockaddr_storage struct Is Going to Contain What we Need
		  *	- ss_family => 2 which means AF_INET
		  *	- 
		  **/
		if (connection_fd > 0) {
			handle_connection(connection_fd);
		}

	}
	
	close(socket_fd);
	return 0;
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
		send(incoming_fd, notok_response,  strlen(notok_response), 0);
		ok = 0;
	} else {
		int send_count = 1;

		// this is just crap to find lenth of file
		fseek(to_return, 0, SEEK_END);
		file_length = ftell(to_return);	
		rewind(to_return);
		char* file_buffer = (char*) malloc(STREAM_SIZE);     // send 4kb at a time i guess

		int num_digits = floor(log10(abs(file_length))) + 1; // length of the file_length number
		sprintf(file_buffer, ok_response, file_length);

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
