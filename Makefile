all:
	gcc -o server server.c -lpthread
	gcc -o client client.c
clean:
	rm server client
