CC=gcc
DEPS = game.h regex.h
FLAGS=-Wall

ifeq ($(OS), Windows_NT)
all: win_client.o
	$(CC) -o grue_food.exe $^ $(FLAGS) $(CFLAGS)
else
all: client.o helpers.o
	$(CC) -o client $^ $(FLAGS) $(CFLAGS)
endif

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(FLAGS) $(CFLAGS)

server: server.o helpers.o parser.o regex.o
	$(CC) -o server $^ -lpthread -I/usr/local/include -L/usr/local/lib -lpcre2-8 $(FLAGS) $(CFLAGS)

clean:
	rm server client *.o
