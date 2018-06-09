#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "game.h"

#pragma comment(lib,"WS2_32")

void clear()
{
	int i;
	for (i = 0; i < 50; i++)
		printf("\n");
}

int main(int argc, char** argv)
{
	char addr_string[BUFFER_SIZE];
	char buffer[BUFFER_SIZE];
	char recv_char;
	WSADATA wsaData;
	SOCKET connect_socket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	/* Handle command line args */
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

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		fprintf(stderr, "WSAStartup failed.\n");
		return -1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(addr_string, PORT, &hints, &result) != 0)
	{
		fprintf(stderr, "getaddrinfo failed.\n");
		WSACleanup();
		return -1;
	}
	ptr = result;

	connect_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (connect_socket == INVALID_SOCKET)
	{
		fprintf(stderr, "Error creating socket.\n");
		freeaddrinfo(result);
		WSACleanup();
		return -1;
	}

	if (connect(connect_socket, ptr->ai_addr, (int)ptr->ai_addrlen) != 0)
	{
		fprintf(stderr, "Unable to connect to server.\n");
		closesocket(connect_socket);
		WSACleanup();
		return -1;
	}
	freeaddrinfo(result);

	/* Receive confirmation a thread was available */
	recv(connect_socket, &recv_char, sizeof(recv_char), 0);
	if (!recv_char)
	{
		fprintf(stderr, "Server at capacity.\n");
		return -1;
	}
	send(connect_socket, &recv_char, sizeof(recv_char), 0);

	/* Clear screen */
	clear();

	/* Display startup message */
	recv(connect_socket, buffer, BUFFER_SIZE, 0);
	printf("%s\n", buffer);

	/* Main loop */
	while (1)
	{
		ZeroMemory(buffer, BUFFER_SIZE);
		recv(connect_socket, buffer, BUFFER_SIZE, 0);

		/* Check for term signal */
		if ((unsigned char)(*buffer) == SHUTDOWN_SIGNAL)
			break;

		printf("%s", buffer);
		ZeroMemory(buffer, BUFFER_SIZE);

		fgets(buffer, BUFFER_SIZE, stdin);

		/* Remove trailing \n*/
		buffer[strlen(buffer) - 1] = '\0';

		send(connect_socket, buffer, strlen(buffer) + 1, 0);
	}

	return 0;
}