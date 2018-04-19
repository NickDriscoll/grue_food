#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

/* Returns 1 if text matches regex, 0 otherwise */
int check_for_match(PCRE2_SPTR8 pattern, PCRE2_SPTR8 text);