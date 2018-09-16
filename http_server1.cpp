#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <math.h>

void handle_connection(int);

const char ok_response[]  = "HTTP/1.0 200 OK\r\n"             \   
                       "Content-type: text/plain\r\n"    \
                        "Content-length: %d \r\n\r\n";

const char notok_response[] = "HTTP/1.0 404 FILE NOT FOUND\r\n" \
                        "Content-type: text/html\r\n\r\n" \   
                        "<html><body bgColor=black text=white>\n" \   
                        "<h2>404 FILE NOT FOUND</h2>\n"
                        "</body></html>\n";

const int STREAM_SIZE = 4096;

class ServerFunctions {
	public:
		int setup_socket(int port);		
	private:
		int server_bind(int port);
		int get_socket();
		int listen_socket(int socket);
		int socket_fd;

};

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

void handle_connection(int incoming_fd) {
	FILE* fp = fdopen(incoming_fd, "rw");
	FILE* to_return = NULL;
	char* helper;		// helper for strtok_r
	char* get_header = NULL;
	char* substring = NULL;
	long long file_length;

	get_header = (char*) malloc(8193); // one byte bigger for null term

	read(incoming_fd, get_header, 8192);
	strtok_r(get_header, " ", &helper); // extract HTTP

	substring = strtok_r(NULL, " ", &helper); // extract filename
	to_return = fopen(substring, "r");

	if (to_return == NULL) {
		send(incoming_fd, notok_response,  strlen(notok_response), 0);
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
}
