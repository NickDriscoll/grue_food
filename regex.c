#include "regex.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>

int check_for_match(const char* pattern, const char* text)
{
	pcre2_code* code;
	int result;
	int error;

	/* Even though these two variables are never used, they're necessary because pcre2_compile
	   and pcre2_match will not work without these things to point to.*/
	PCRE2_SIZE offset;
	pcre2_match_data* data;

	code = pcre2_compile((PCRE2_SPTR8)pattern, PCRE2_ZERO_TERMINATED, PCRE2_CASELESS, &error, &offset, NULL);
	if (code == NULL)
	{
		PCRE2_UCHAR buffer[1024];
		pcre2_get_error_message(error, buffer, sizeof(buffer));
		fprintf(stderr, "%s\n", buffer);
		pthread_exit(NULL);
	}

	data = pcre2_match_data_create_from_pattern(code, NULL);

	result = pcre2_match(code, (PCRE2_SPTR8)text, strlen(text), 0, 0, data, NULL);
	pcre2_code_free(code);
	return result >= 0;
}
