#define PORT 6969

typedef struct thread_args
{
	int socket;
	char* thread_flag;
} thread_args;