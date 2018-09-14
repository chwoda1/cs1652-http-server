build:
	g++ http_client.cpp -o http_client
	g++ http_server1.cpp -o http_server1

clean:
	rm -f http_client
	rm -f http_server1
	rm -f *.txt
