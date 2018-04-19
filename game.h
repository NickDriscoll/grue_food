#include <stdint.h>
#include <stddef.h>

#define PORT 6969
#define BUFFER_SIZE 4096
#define USER_DIR "users/"
#define LEVEL_DIR "levels/"
#define PCRE2_CODE_UNIT_WIDTH 8
#define MAX_NUMBER_OF_CONNECTIONS 8

typedef struct thread_args
{
	int socket;
	char* thread_flag;
} thread_args;

typedef struct location
{
	char name[BUFFER_SIZE];
	char description[BUFFER_SIZE];
	char* north;
} location;

typedef struct player_identity
{
	char name[BUFFER_SIZE];
	location* location;
} player_identity;

typedef struct token
{
	char token[BUFFER_SIZE];
	struct token* next_token;
} token;

void error();
void clear_buffer(char* buffer);
void thread_cleanup_routine(void* arg);
int check_for_match(const char* pattern, const char* text);
void send_message(int socket, const void* buffer, size_t len);
location* parse_level_file(const char* path);