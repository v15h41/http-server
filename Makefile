all: server.c
	gcc -g -Wall -o server server.c -lpthread
clean: 
	rm server