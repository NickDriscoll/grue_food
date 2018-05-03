#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

/* Returns 1 if text matches regex, 0 otherwise */
int check_for_match(const char* pattern, const char* text);
