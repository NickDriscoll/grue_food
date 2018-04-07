#include "game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void add_to_list(token* list, token* new_item)
{
	token* current = list;

	/* Prevent issues with the first item */
	if (list == new_item)
		return;

	while (current->next_token != NULL)
	{
		current = current->next_token;
	}
	current->next_token = new_item;
}

/* Returns a linked list of tokens */
token* tokenize_file(const char* path)
{
	int fd = open(path, O_RDONLY);
	char current_string[BUFFER_SIZE];
	char c;
	int i = 0;
	token* token_list = malloc(sizeof(token));
	token* current_token = token_list;

	while (read(fd, &c, 1) > 0)
	{
		if ((c == ' ' || c == '\n' || c == '\r') && current_string[0] != '\0')
		{
			current_string[i] = '\0';
			strcpy(current_token->token, current_string);
			clear_buffer(current_string);
			current_token->next_token = NULL;
			add_to_list(token_list, current_token);
			i = 0;
			current_token = malloc(sizeof(token));
		}
		else
		{
			current_string[i] = c;
			i++;
		}
	}

	close(fd);
	return token_list;
}

location* parse_level_file(const char* path)
{
	token* list = tokenize_file(path);
	token* current;
	location* l = malloc(sizeof(location));
	memset(l, 0, sizeof(location));

	current = list->next_token;

	/* Fill the label */
	while (strcmp(current->token, "</label>") != 0)
	{
		strcat(l->name, current->token);
		strcat(l->name, " ");
		current = current->next_token;
	}

	/* Replace final space with \0 */
	l->name[strlen(l->name) - 1] = '\0';

	/* Fill the description */
	current = current->next_token;
	while (strcmp(current->token, "</des>") != 0)
	{
		strcat(l->description, current->token);
		strcat(l->description, " ");
		current = current->next_token;
	}
	l->description[strlen(l->description) - 1] = '\0';

	return l;
}