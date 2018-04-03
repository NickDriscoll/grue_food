#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "game.h"

int convert_command_to_code(char* command)
{

}

int main(int argc, char** argv)
{
	int sock;
	char message[1024];
	char recv_char;
	struct sockaddr_in address;

	/* Populate the values of address */
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) <= 0)
	{
		fprintf(stderr, "Invalid address.\n");
		exit(-1);
	}

	if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		fprintf(stderr, "Connection failed.\n");
		exit(-1);
	}

	/* Receive confirmation that a thread was available */
	recv(sock, &recv_char, sizeof(recv_char), 0);
	if (!recv_char)
	{
		fprintf(stderr, "Server at capacity.\n");
		exit(-1);
	}

	/* Get line from user */
	printf("Enter a message: ");
	fgets(message, sizeof(message), stdin);

	/* Remove trailing \n*/
	message[strlen(message) - 1] = '\0';

	send(sock, message, strlen(message), 0);

	return 0;
}