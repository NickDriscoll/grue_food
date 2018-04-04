all:
	gcc -o client client.c helpers.c

client:
	gcc -o client client.c helpers.c

server:
	gcc -o server server.c helpers.c -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8

debug:
	gcc -o server -g server.c helpers.c -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8
	gcc -o client -g client.c helpers.c

clean:
	rm server client
