all: client server

server: server.c
	gcc -g -Wall -Wextra server.c -o server

client: client.c
	gcc -g -Wall -Wextra client.c -o client

clean:
	rm -f server
	rm -f client
