#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "game.h"

#define PORT 6969

void* thread_func(void* args)
{
	/* Only arg is the accept socket */
	int sock = (*(int*)(args));
	char buffer[1024];

	recv(sock, buffer, sizeof(buffer), 0);
	printf("Message: %s\n", buffer);
}

int main(int argc, char** argv)
{
	const int MAX_NUMBER_OF_CONNECTIONS = 5;
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
		actual_socket = accept(listening_socket, (struct sockaddr *)&address, &address_len);
		printf("Accepting a connection.\n");
		for (i = 0; i < MAX_NUMBER_OF_CONNECTIONS && flag == 0; i++)
		{
			if (thread_states[i] == 0)
			{
				pthread_create(&threads[i], NULL, thread_func, &actual_socket);
				thread_states[i] = 1;
				flag = 1;
				printf("Thread index: %i\n", i);
			}
		}

		if (i == MAX_NUMBER_OF_CONNECTIONS - 1 && flag == 0)
		{
			fprintf(stderr, "ERROR: number of users at capacity!\n");
		}
	}

	return 0;
}