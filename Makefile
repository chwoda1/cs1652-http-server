build:
	g++ http_client.cpp -o http_client
	g++ http_server1.cpp server.cpp -o http_server1
	g++ http_server2.cpp server.cpp -o http_server2
clean:
	rm -f http_client
	rm -f http_server1
	rm -f http_server2
	rm -f *.txt
