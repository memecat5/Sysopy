all: client server

server: server.c
	gcc -g -O2 -Wall -Wextra server.c -o server

client: client.c
	gcc -g -O2 -Wall -Wextra client.c -o client

clean:
	rm -f server
	rm -f client
