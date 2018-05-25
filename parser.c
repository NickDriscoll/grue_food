#include "game.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void display_linked_list(token* list)
{
	while (list->next_token != NULL)
	{
		printf("%s\n", list->token);
		list = list->next_token;
	}
	printf("%s\n", list->token);
}

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
		if ((c == ' ' || c == '\n' || c == '\r'))
		{
			if (current_string[0] == '\0')
				continue;

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

	/* Place final token in list */
	current_string[i] = '\0';
	strcpy(current_token->token, current_string);
	current_token->next_token = NULL;
	add_to_list(token_list, current_token);

	close(fd);
	return token_list;
}

void parse_chunk(token** list, char* field, char* tag)
{
	char buffer[BUFFER_SIZE];
	clear_buffer(buffer);

	while (strcmp((*list)->token, tag) != 0)
	{
		strcat(buffer, (*list)->token);
		strcat(buffer, " ");
		*list = (*list)->next_token;
	}

	/* Remove trailing space */
	if (strlen(buffer) != 0)
		buffer[strlen(buffer) - 1] = '\0';
	*list = (*list)->next_token;

	strcpy(field, buffer);
}

location* parse_level_file(const char* path)
{
	token* current = tokenize_file(path);
	location* l = calloc(1, sizeof(location));

	/* Recursive descent parse until all tokens have been consumed */
	while (current != NULL)
	{
		if (strcmp(current->token, "<label>") == 0)
		{
			current = current->next_token;
			parse_chunk(&current, l->name, "</label>");
		}
		else if (strcmp(current->token, "<des>") == 0)
		{
			current = current->next_token;
			parse_chunk(&current, l->description, "</des>");
		}
		else if (strcmp(current->token, "<north>") == 0)
		{
			current = current->next_token;
			parse_chunk(&current, l->directions[north], "</north>");
		}
		else if (strcmp(current->token, "<south>") == 0)
		{
			current = current->next_token;
			parse_chunk(&current, l->directions[south], "</south>");
		}
		else if (strcmp(current->token, "<west>") == 0)
		{
			current = current->next_token;
			parse_chunk(&current, l->directions[west], "</west>");
		}
		else if (strcmp(current->token, "<east>") == 0)
		{
			current = current->next_token;
			parse_chunk(&current, l->directions[east], "</east>");
		}
		else if (strcmp(current->token, "") == 0)
		{
			current = current->next_token;
		}
	}

	return l;
}
