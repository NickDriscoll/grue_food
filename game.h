#include <stdint.h>
#include <stddef.h>

#define PORT 6969
#define BUFFER_SIZE 1024
#define USER_DIR "users/"
#define PCRE2_CODE_UNIT_WIDTH 8

typedef struct thread_args
{
	int socket;
	char* thread_flag;
} thread_args;

typedef struct player_identity
{
	char name[BUFFER_SIZE];
	uint64_t location_id;
} player_identity;

typedef struct location_id
{
	char name[BUFFER_SIZE];
	char description[BUFFER_SIZE];
} location_id;

void clear_buffer(char* buffer);
void thread_cleanup_routine(void* arg);
int check_for_match(const char* pattern, const char* text);
void send_message(int socket, const void* buffer, size_t len);