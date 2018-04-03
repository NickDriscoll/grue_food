#include "game.h"
#include <string.h>
#include <stdio.h>

void clear_buffer(char* buffer)
{
	bzero(buffer, BUFFER_SIZE);
}

void thread_cleanup_routine(void* arg)
{
	char* code = arg;
	*code = 0;
}