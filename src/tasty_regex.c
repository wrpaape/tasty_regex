#include "tasty_regex.h"

/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern)
{
	struct TastyState *restrict head;

	const size_t length_pattern = string_length(pattern);

	if (length_pattern == 0lu)
		return TASTY_ERROR_EMPTY_PATTERN;

	/* allocate stack of state nodes, init all pointers to NULL */
	head = calloc(length_pattern,
		      sizeof(struct TastyState));

	if (UNLIKELY(head == NULL_POINTER))
		return TASTY_ERROR_OUT_OF_MEMORY;

	/* 'matching' state set to not-necessarily valid point in memory, (will
	 * never be accessed anyway) */
	regex->start	= head;
	regex->matching = head + length_pattern;

	return 0;
}

void
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
{
}

/* free allocations */
extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);
extern inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches);


/* helper functions
 * ────────────────────────────────────────────────────────────────────────── */
static inline size_t
string_length(const char *const restrict string)
{
	register const char *restrict until = string;

	while (*until != '\0')
		++until;

	return until - string;
}
