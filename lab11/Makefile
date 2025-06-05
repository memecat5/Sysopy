all: server client

server: server.c
	gcc -g server.c -o server

client: client.c
	gcc -g client.c -o client

run: server
	./server 2137

run_client: client
	./client 127.0.0.1 2137

clean:
	rm -f server
