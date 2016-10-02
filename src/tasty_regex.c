#include "tasty_regex.h"

int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern)
{

	return 0;
}

void
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
{
}

extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);


extern inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches);
