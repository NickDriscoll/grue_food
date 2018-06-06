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

item* merge_inventories(item* inv1, item* inv2)
{
	item* current1 = inv1;
	item* current2 = inv2;
	item* result;

	if (current1 == NULL)
		return inv2;
	if (current2 == NULL)
		return inv1;

	while (current1 != NULL && current2 != NULL)
	{
		if (strcmp(current1->name, current2->name) < 0)
			add_to_list(&result, current1);
		else
			add_to_list(&result, current2);
	}

	while (
}

void sort_inventory(item** inv)
{
	int i;
	item* current;
	if (*inv == NULL)
		return;

	
}
