#include "tasty_regex.h"

/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern)
{
	struct TastyState *restrict head_state;

	const size_t length_pattern = string_length(pattern);

	if (length_pattern == 0lu)
		return TASTY_ERROR_EMPTY_PATTERN;

	/* allocate stack of state nodes, init all pointers to NULL */
	head_state = calloc(length_pattern,
		      sizeof(struct TastyState));

	if (UNLIKELY(head_state == NULL_POINTER))
		return TASTY_ERROR_OUT_OF_MEMORY;

	/* 'matching' state set to not-necessarily valid point in memory, (will
	 * never be accessed anyway) */
	regex->start	= head_state;
	regex->matching = head_state + length_pattern;

	return 0;
}

int
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
{
	struct TastyMatch *restrict match;	     /* free nodes */
	struct TastyAccumulator *restrict acc_alloc; /* free nodes */
	struct TastyAccumulator *restrict head_acc;  /* list of live matches */
	struct TastyAccumulator *restrict acc;	     /* list traversal var */
	struct TastyState *restrict state;	     /* current state */
	struct TastyState *restrict next_state;	     /* next state */
	struct TastyState **restrict init_step;	     /* jump from init state */

	const size_t length_string = string_length(string);

	/* at most length(string) matches */
	match = malloc(sizeof(struct TastyMatch) * length_string);

	if (UNLIKELY(match == NULL_POINTER))
		return TASTY_ERROR_OUT_OF_MEMORY;

	/* at most length(string) parallel matching states to keep track of
	 * (free before return) */
	struct TastyAccumulator *const restrict accumulators
	= malloc(sizeof(struct TastyAccumulator) * length_string);

	if (UNLIKELY(accumulators == NULL_POINTER)) {
		free(match);
		return TASTY_ERROR_OUT_OF_MEMORY;
	}

	matches->from = match;


	while (1) {
		/* tranverse string until match with first state */
		state = regex->start;

		while (1) {
			if ((*string) == '\0')
				goto STRING_EXHAUSTED;

			next_state = state->step[*string];



			++string;
		}


	}


STRING_EXHAUSTED:
	matches->until = match;

	free(accumulators);
	return 0;
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
