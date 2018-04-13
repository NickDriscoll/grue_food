#include "game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

void error()
{
	fprintf(stderr, "ERROR: %s\n", strerror(errno));
	exit(-1);
}

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

/* Handling of disconnects happens here */
void send_message(int socket, const void* buffer, size_t len)
{
	if (send(socket, buffer, len, MSG_NOSIGNAL) < 0 && errno == EPIPE)
		pthread_exit(NULL);
}