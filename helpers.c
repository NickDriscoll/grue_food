#include "game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void clear_buffer(char* buffer)
{
	bzero(buffer, BUFFER_SIZE);
}

void thread_cleanup_routine(void* arg)
{
	thread_args* args = arg;
	*(args->thread_flag) = 0;
	free(args);
}