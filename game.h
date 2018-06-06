#include <stdint.h>
#include <stddef.h>

#define PORT 6969
#define BUFFER_SIZE 2048
#define USER_DIR "users/"
#define LEVEL_DIR "levels/"
#define MAX_NUMBER_OF_CONNECTIONS 8
#define SHUTDOWN_SIGNAL 0xFF

typedef struct thread_args
{
	int socket;
	char* thread_flag;
} thread_args;

typedef struct location
{
	char name[BUFFER_SIZE];
	char description[BUFFER_SIZE];

	/* These are all file paths to the locations that are logically around the current location */
	char directions[4][BUFFER_SIZE];
} location;

typedef struct item
{
	char name[BUFFER_SIZE];
	char description[BUFFER_SIZE];
	struct item* next;
} item;

typedef struct player_identity
{
	char name[BUFFER_SIZE];
	location* location;
	item* inventory;
} player_identity;

typedef struct token
{
	char token[BUFFER_SIZE];
	struct token* next_token;
} token;

typedef enum direction
{
	north = 0,
	south = 1,
	west = 2,
	east = 3
} direction;

void error();
void clear_buffer(char* buffer);
void thread_cleanup_routine(void* arg);

/* Wrapper around send() that has our settings hardcoded in */
void send_message(int socket, const void* buffer, size_t len);
location* parse_level_file(const char* path);

/* Sorts inventory in ascending alphabetical order */
item* sort_inventory(item* inventory);
