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

void add_to_list(item** list, item* new_item)
{
	if (*list == NULL)
		*list = new_item;

	while ((*list)->next != NULL)
	{
		*list = (*list)->next;
	}

	(*list)->next = new_item;
}

item* merge_inventories(item* inv1, item* inv2)
{
	item* current1 = inv1;
	item* current2 = inv2;
	item* result = NULL;

	while (current1 != NULL && current2 != NULL)
	{
		if (strcmp(current1->name, current2->name) < 0)
		{
			add_to_list(&result, current1);
			current1 = current1->next;
		}
		else
		{
			add_to_list(&result, current2);
			current2 = current2->next;
		}
	}

	while (current1 != NULL)
	{
		add_to_list(&result, current1);
		current1 = current1->next;
	}

	while (current2 != NULL)
	{
		add_to_list(&result, current2);
		current2 = current2->next;
	}

	return result;
}

size_t list_len(item* list)
{
	size_t count = 0;
	while (list != NULL)
	{
		list = list->next;
		count++;
	}
	return count;
}

item* sort_inventory(item* inv)
{
	int i;
	size_t length;
	item* half1;
	item* half2;
	item* current = inv;

	length = list_len(inv);
	if (length < 2)
	{
		return inv;
	}

	/* Split the list in half */
	for (i = 0; i < length / 2; i++)
	{
		current = current->next;
	}
	half2 = current->next;
	current->next = NULL;
	half1 = inv;

	/* Recurse on the lists */
	half1 = sort_inventory(half1);
	half2 = sort_inventory(half2);

	/* Merge them */
	return merge_inventories(half1, half2);
}
