
class ServerFunctions {
	public:
		int setup_socket(int port);
	private:
		int server_bind(int port);
		int get_socket();
		int listen_socket(int socket);
		int socket_fd;
};
