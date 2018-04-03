#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "game.h"

void* thread_func(void* raw_args)
{
	thread_args* args = raw_args;
	char buffer[1024];

	/* Ask if user wants to login or register */

	

	/* Let the main thread know that this thread has terminated */
	*(args->thread_flag) = 0;
}

int main(int argc, char** argv)
{
	const int MAX_NUMBER_OF_CONNECTIONS = 8;
	int listening_socket, actual_socket;
	struct sockaddr_in address;
	socklen_t address_len = sizeof(address);
	char message[1024];
	pthread_t threads[MAX_NUMBER_OF_CONNECTIONS];
	char thread_states[MAX_NUMBER_OF_CONNECTIONS];
	memset(thread_states, 0, sizeof(thread_states));

	/* Populate the values of address */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	/* Create and bind the listening socket */
	listening_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_socket < 0)
	{
		fprintf(stderr, "Error creating socket.\n");
		exit(-1);
	}

	if (bind(listening_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		fprintf(stderr, "Unable to bind listening socket.\n");
		exit(-1);
	}

	/* Begin listening for connections */
	if (listen(listening_socket, MAX_NUMBER_OF_CONNECTIONS) < 0)
	{
		fprintf(stderr, "Call to listen() failed.\n");
		exit(-1);
	}
	printf("Listening on port %i...\n", PORT);

	while (1)
	{
		int i;
		char flag = 0;
		char confirmation_code;

		actual_socket = accept(listening_socket, (struct sockaddr *)&address, &address_len);

		/* Create and populate arg_struct */
		thread_args* args = malloc(sizeof(thread_args));
		args->socket = actual_socket;

		for (i = 0; i < MAX_NUMBER_OF_CONNECTIONS && flag == 0; i++)
		{
			if (thread_states[i] == 0)
			{
				args->thread_flag = thread_states + i;
				pthread_create(&threads[i], NULL, thread_func, args);
				thread_states[i] = 1;
				flag = 1;
				printf("Thread index: %i\n", i);
				confirmation_code = 1;
			}
		}

		if (flag == 0)
		{			
			fprintf(stderr, "ERROR: number of users at capacity!\n");
			confirmation_code = 0;
		}

		/* Tell client if a thread was available */
		send(actual_socket, &confirmation_code, sizeof(confirmation_code), 0);
	}

	return 0;
}