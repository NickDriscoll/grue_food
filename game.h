#include <stdint.h>

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
	uint64_t password_hash;
} player_identity;

void clear_buffer(char* buffer);
void thread_cleanup_routine(void* arg);
int check_for_match(const char* pattern, const char* text);