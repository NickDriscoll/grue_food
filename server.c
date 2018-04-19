#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "game.h"
#include <pcre2.h>

int check_for_match(const char* pattern, const char* text)
{
	pcre2_code* code;
	int result;
	int error;

	/* Even though these two variables are never used, they're necessary because pcre2_compile
	   and pcre2_match will not work without these things to point to.*/
	PCRE2_SIZE offset;
	pcre2_match_data* data;

	code = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, PCRE2_CASELESS, &error, &offset, NULL);
	if (code == NULL)
	{
		char buffer[BUFFER_SIZE];
		pcre2_get_error_message(error, buffer, sizeof(buffer));
		fprintf(stderr, "%s\n", buffer);
		pthread_exit(NULL);
	}

	data = pcre2_match_data_create_from_pattern(code, NULL);

	result = pcre2_match(code, text, strlen(text), 0, 0, data, NULL);
	pcre2_code_free(code);
	return result >= 0;
}

void send_MOTD(int socket)
{
	char* message = "****BZZZT****\n\n\n\n";
	send(socket, message, strlen(message) + 1, 0);
}

void register_user(int socket, player_identity* player)
{
	int fd;
	char buffer[BUFFER_SIZE] = "Please enter a username: ";
	char file_path[BUFFER_SIZE*2];
	struct stat st = {0};

	/* Get username */
	send_message(socket, buffer, strlen(buffer));
	clear_buffer(buffer);
	recv(socket, buffer, sizeof(buffer), 0);

	/* Place ./users/<username> in file_path */
	strcpy(file_path, USER_DIR);
	strcat(file_path, buffer);

	/* Create users directory if it doesn't already exist */
	if (stat(USER_DIR, &st) == -1)
		mkdir(USER_DIR, 0777);

	/* If the username isn't already taken, open a new file for a new user */
	while (access(file_path, F_OK) == 0)
	{
		strcpy(buffer, "Requested username has already been taken.\nPlease enter a username: ");
		send_message(socket, buffer, strlen(buffer));
		clear_buffer(buffer);
		recv(socket, buffer, sizeof(buffer), 0);

		/* Place users/<username> in file_path */
		strcpy(file_path, USER_DIR);
		strcat(file_path, buffer);
	}
	fd = open(file_path, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	write(fd, buffer, strlen(buffer));
	strcpy(player->name, buffer);

	clear_buffer(buffer);
	strcpy(buffer, "Please enter a password: ");
	send_message(socket, buffer, strlen(buffer));
	clear_buffer(buffer);
	recv(socket, buffer, sizeof(buffer), 0);
	write(fd, "\n", 1);
	write(fd, buffer, strlen(buffer));

	/* Initialize player with the default location */
	player->location = parse_level_file(LEVEL_DIR "testroom.lvl");

	close(fd);
}

void start_routine(int socket, player_identity* player)
{	
	char buffer[BUFFER_SIZE] = "Login or register a new user? ";
	send_message(socket, buffer, strlen(buffer));
	
	while (1)
	{
		clear_buffer(buffer);
		recv(socket, buffer, sizeof(buffer), 0);

		if (check_for_match("login", buffer))
		{
			/*login_user(socket, player);*/
			break;
		}
		else if (check_for_match("register", buffer))
		{
			register_user(socket, player);
			break;
		}
		else
		{
			strcpy(buffer, "Response must be login or register.\nLogin or register a new user? ");
			send_message(socket, buffer, strlen(buffer));
		}
	}
}

int parse_command(const char* command, player_identity* player, int socket)
{
	char buffer[BUFFER_SIZE];
	clear_buffer(buffer);

	if (check_for_match("quit", command) || check_for_match("exit", command))
	{
		return 0;
	}
	else if (check_for_match("l$|look", command))
	{
		sprintf(buffer, "%s\n\n%s\n>", player->location->name, player->location->description);
		send_message(socket, buffer, strlen(buffer));
	}
	else
	{
		sprintf(buffer, "I don't understand \"%s\"\n>", command);
		send_message(socket, buffer, strlen(buffer));
	}
	return 1;
}

void game_loop(int socket, player_identity* player)
{
	char buffer[BUFFER_SIZE];

	clear_buffer(buffer);
	sprintf(buffer, "%s\n\n%s\n>", player->location->name, player->location->description);
	send_message(socket, buffer, strlen(buffer));
	
	/* Display name and description of current location */
	do
	{
		clear_buffer(buffer);
		recv(socket, buffer, BUFFER_SIZE, 0);
	} while (parse_command(buffer, player, socket));
}

void* thread_main(void* raw_args)
{
	thread_args* args = raw_args;
	player_identity player;
	char c;

	/* Receive response to OK flag */
	recv(args->socket, &c, 1, 0);

	/* Push thread_cleanup_routine to cleanup stack */
	pthread_cleanup_push(thread_cleanup_routine, raw_args);

	/* Send over MOTD */
	send_MOTD(args->socket);

	/* Ask if user wants to login or register */
	start_routine(args->socket, &player);

	/* Drop user into game */
	game_loop(args->socket, &player);

	/* Send shutdown signal */
	c = 0xFF;
	send_message(args->socket, &c, 1);

	/* Let the main thread know that this thread has terminated */
	pthread_cleanup_pop(1);
}

int main(int argc, char** argv)
{
	int listening_socket, actual_socket;
	struct sockaddr_in address;
	socklen_t address_len = sizeof(address);
	char message[BUFFER_SIZE];
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
		error();
	}

	/* Set SO_REUSEADDR */
	if (setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
	{
		error();
	}	

	if (bind(listening_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
	{
		error();
	}

	/* Begin listening for connections */
	if (listen(listening_socket, MAX_NUMBER_OF_CONNECTIONS) < 0)
	{
		error();
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
				pthread_create(&threads[i], NULL, thread_main, args);
				thread_states[i] = 1;
				flag = 1;
				printf("Client at thread index: %i\n", i);
				confirmation_code = 1;
			}
		}

		if (flag == 0)
		{			
			fprintf(stderr, "ERROR: number of users at capacity!\n");
			confirmation_code = 0;
		}

		/* Tell client if a thread was available */
		send_message(actual_socket, &flag, 1);
	}

	return 0;
}