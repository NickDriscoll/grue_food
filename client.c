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
	/* Magic StackOverflow code */
	printf("\e[1;1H\e[2J");
}

int main(int argc, char** argv)
{
	int sock;
	char buffer[BUFFER_SIZE];
	char recv_char;
	struct sockaddr_in address;

	if (argc < 2)
	{
		printf("Usage: %s <ip address>\n", argv[0]);
		exit(-1);
	}

	/* Populate the values of address */
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (inet_pton(AF_INET, argv[1], &address.sin_addr) <= 0)
	{
		error();
	}

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
		recv(sock, buffer, sizeof(buffer), 0);

		/* Check for term signal */
		if (*buffer == -1)
			break;

		printf("%s", buffer);
		clear_buffer(buffer);
		fgets(buffer, sizeof(buffer), stdin);

		/* Remove trailing \n*/
		buffer[strlen(buffer) - 1] = '\0';

		send(sock, buffer, strlen(buffer), 0);
	}

	return 0;
}