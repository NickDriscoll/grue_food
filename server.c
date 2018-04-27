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
#include "regex.h"

void send_MOTD(int socket)
{
	char* message = "****BZZZT****\n\n\n\n";
	send_message(socket, message, strlen(message) + 1);
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
	sprintf(file_path, "%s%s", USER_DIR, buffer);

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
		sprintf(file_path, "%s%s", USER_DIR, buffer);
	}

	fd = open(file_path, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);

	strcpy(player->name, buffer);

	clear_buffer(buffer);
	strcpy(buffer, "Please enter a password: ");
	send_message(socket, buffer, strlen(buffer));
	clear_buffer(buffer);
	recv(socket, buffer, sizeof(buffer), 0);
	write(fd, buffer, strlen(buffer));
	write(fd, "\n", 1);

	/* Initialize player with the default location */
	char* default_location = LEVEL_DIR "testroom.lvl";
	player->location = parse_level_file(default_location);
	write(fd, default_location, strlen(default_location));
	write(fd, "\n", 1);

	close(fd);
}

void login_user(int socket, player_identity* player)
{
	char buffer[BUFFER_SIZE];
	char buffer2[BUFFER_SIZE];
	char file_path[BUFFER_SIZE * 2];
	char password[BUFFER_SIZE];
	char c;
	int fd;

	sprintf(buffer, "Please enter your username: ");
	send_message(socket, buffer, strlen(buffer));
	clear_buffer(buffer);
	recv(socket, buffer, BUFFER_SIZE, 0);

	sprintf(file_path, "%s%s", USER_DIR, buffer);

	while (access(file_path, F_OK) != 0)
	{
		sprintf(buffer2, "No user named %s exists.\nPlease enter your username: ", buffer);
		send_message(socket, buffer2, strlen(buffer2));
		clear_buffer(buffer);
		recv(socket, buffer, BUFFER_SIZE, 0);
		sprintf(file_path, "%s%s", USER_DIR, buffer);
	}
	fd = open(file_path, O_RDONLY);

	strcpy(player->name, buffer);

	/* Validate password */
	sprintf(buffer, "Please enter your password: ");
	send_message(socket, buffer, strlen(buffer));
	clear_buffer(buffer);
	recv(socket, buffer, BUFFER_SIZE, 0);

	/* Read the first line from the user file */
	while (read(fd, &c, 1) > 0 && c != '\n')
	{
		strncat(password, &c, 1);
	}

	/* Compare */
	while (strcmp(buffer, password) != 0)
	{
		sprintf(buffer, "Incorrect password\nPlease enter your password: ");
		send_message(socket, buffer, strlen(buffer));
		clear_buffer(buffer);
		recv(socket, buffer, BUFFER_SIZE, 0);
	}

	/* At this point, the password entered is correct */
	/* Read path to current location */
	bzero(file_path, sizeof(file_path));
	while (read(fd, &c, 1) > 0 && c != '\n')
	{
		strncat(file_path, &c, 1);
	}

	player->location = parse_level_file(file_path);

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

		if (check_for_match((PCRE2_SPTR8)"^l$|login", (PCRE2_SPTR8)buffer))
		{
			login_user(socket, player);
			break;
		}
		else if (check_for_match((PCRE2_SPTR8)"^r$|register", (PCRE2_SPTR8)buffer))
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

void look(int socket, player_identity* player)
{
	char buffer[BUFFER_SIZE];
	sprintf(buffer, "%s\n\n%s\n>", player->location->name, player->location->description);
	send_message(socket, buffer, strlen(buffer));
}

int try_move_north(player_identity* player)
{
	char path[BUFFER_SIZE];
	if (player->location->north[0] == '\0')
		return 0;

	sprintf(path, "%s%s", LEVEL_DIR, player->location->north);
	player->location = parse_level_file(path);

	return 1;
}

int parse_command(const char* command, player_identity* player, int socket)
{
	char buffer[BUFFER_SIZE];
	clear_buffer(buffer);

	if (check_for_match((PCRE2_SPTR8)"quit|exit", (PCRE2_SPTR8)command))
	{
		return 0;
	}
	else if (check_for_match((PCRE2_SPTR8)"^l$|look", (PCRE2_SPTR8)command))
	{
		look(socket, player);
	}
	else if (check_for_match((PCRE2_SPTR8)"^n$|north", (PCRE2_SPTR8)command))
	{
		if (try_move_north(player))
		{
			look(socket, player);
		}
		else
		{
			strcpy(buffer, "Unable to move north.\n>");
			send_message(socket, buffer, strlen(buffer));
		}
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
	c = SHUTDOWN_SIGNAL;
	send_message(args->socket, &c, 1);

	/* Let the main thread know that this thread has terminated */
	pthread_cleanup_pop(1);

	return NULL;
}

int main(int argc, char** argv)
{
	int listening_socket, actual_socket;
	struct sockaddr_in address;
	socklen_t address_len = sizeof(address);
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
			}
		}

		if (flag == 0)
		{			
			fprintf(stderr, "ERROR: number of users at capacity!\n");
		}

		/* Tell client if a thread was available */
		send_message(actual_socket, &flag, 1);
	}

	return 0;
}