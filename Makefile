all:
	gcc -o server server.c -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8
	gcc -o client client.c

client:
	gcc -o client client.c

debug:
	gcc -o server -g server.c -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8
	gcc -o client -g client.c

clean:
	rm server client
