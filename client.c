#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#include "game.h"

void display_startup_message(int socket)
{
	char buffer[BUFFER_SIZE];
	recv(socket, buffer, BUFFER_SIZE, 0);
	printf("%s\n", buffer);
}

void clear()
{
#ifdef __linux__
	/* Magic StackOverflow code https://stackoverflow.com/questions/2347770/how-do-you-clear-the-console-screen-in-c */
	printf("\e[1;1H\e[2J");
#elif __WIN32
	int i;
	for (i = 0; i < 50; i++)
		printf("\n");
#endif
}

int main(int argc, char** argv)
{
	int sock;
	char* buffer = malloc(BUFFER_SIZE);
	size_t n = BUFFER_SIZE;
	char recv_char;
	struct sockaddr_in address;
	char addr_string[BUFFER_SIZE];

	if (argc < 2)
	{
		printf("Enter the IP address of the server: ");
		fgets(addr_string, BUFFER_SIZE, stdin);
		addr_string[strlen(addr_string) - 1] = '\0';
	}
	else
	{
		strcpy(addr_string, argv[1]);
	}

	/* Populate the values of address */
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (inet_pton(AF_INET, addr_string, &address.sin_addr) <= 0)
	{
		error();
	}

	printf("Connecting to server...\n");

	if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		error();
	}

	/* Receive confirmation that a thread was available */
	recv(sock, &recv_char, sizeof(recv_char), 0);
	if (!recv_char)
	{
		fprintf(stderr, "Server at capacity.\n");
		exit(-1);
	}
	send(sock, &recv_char, sizeof(recv_char), 0);

	/* Clear screen */
	clear();

	/* Receive server MOTD / greeting / whatever */
	display_startup_message(sock);

	/* The client really does nothing more than send and receive strings forever */
	while (1)
	{
		clear_buffer(buffer);
		recv(sock, buffer, BUFFER_SIZE, 0);

		/* Check for term signal */
		if ((unsigned char)(*buffer) == SHUTDOWN_SIGNAL)
			break;

		printf("%s", buffer);
		clear_buffer(buffer);
		getline(&buffer, &n, stdin);

		/* Remove trailing \n*/
		buffer[strlen(buffer) - 1] = '\0';

		send(sock, buffer, strlen(buffer) + 1, 0);
	}

	return 0;
}