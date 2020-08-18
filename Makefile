CC=gcc

all: server client

server: daytimeserver.c 
	gcc -o server daytimeserver.c 

client: daytimeclient.c 
	gcc -o client daytimeclient.c 

clean:
	rm server client
