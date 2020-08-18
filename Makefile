CC=gcc

all: server client

server: echo_server.c 
	gcc -o server echo_server.c 

client: echo_client.c 
	gcc -o client echo_client.c 

clean:
	rm server client
