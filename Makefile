CC=gcc
DEPS = game.h regex.h
FLAGS=-Wall

ifeq ($(OS), Windows_NT)
else
all: client.o helpers.o
	$(CC) -o client client.o helpers.o $(FLAGS) $(CFLAGS)
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(FLAGS) $(CFLAGS)

server: server.o helpers.o parser.o regex.o
	$(CC) -o server $^ -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8 $(FLAGS) $(CFLAGS)

clean:
	rm server client *.o
