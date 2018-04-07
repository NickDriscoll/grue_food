CC=gcc
DEPS = game.h

all: client.o helpers.o
	$(CC) -o client client.o helpers.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: server.o helpers.o
	$(CC) -o server $^ -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8 $(CFLAGS)

clean:
	rm server client *.o
