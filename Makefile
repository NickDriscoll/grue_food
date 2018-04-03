all:
	gcc -o server server.c -lpthread
	gcc -o client client.c

debug:
	gcc -o server -g server.c -lpthread
	gcc -o client -g client.c

clean:
	rm server client
