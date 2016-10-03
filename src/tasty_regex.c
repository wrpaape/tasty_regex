#include "tasty_regex.h"

/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern)
{
	const void *restrict *head_state;

	const size_t length_pattern = string_length(pattern);

	if (length_pattern == 0lu)
		return TASTY_ERROR_EMPTY_PATTERN;

	/* allocate stack of state nodes, init all pointers to NULL */
	head_state = calloc(length_pattern,
			    sizeof(const void *restrict) * TASTY_STATE_LENGTH);

	if (UNLIKELY(head_state == NULL_POINTER))
		return TASTY_ERROR_OUT_OF_MEMORY;

	/* 'matching' state set to not-necessarily valid point in memory, (will
	 * never be accessed anyway) */
	regex->initial	= head_state;
	regex->matching = (const void *restrict)
			  (head_state + (length_pattern * TASTY_STATE_LENGTH));
	return 0;
}


int
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
{
	struct TastyMatch *restrict match;	      /* free nodes */
	struct TastyAccumulator *restrict acc_alloc;  /* free nodes */
	struct TastyAccumulator *restrict head_acc;   /* list of live matches */
	struct TastyAccumulator *restrict acc;	      /* list traversal var */
	struct TastyAccumulator *restrict *acc_ptr;   /* list traversal var */
	const void *restrict next_step;		      /* next step from state */

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

	acc_alloc = accumulators;
	head_acc  = NULL;

	/* step through each character of string
	 * ────────────────────────────────────────────────────────────────── */
	for (unsigned char token = *string; token != '\0'; token = *string) {
		/* push initial state into list of accumulators
		 * ────────────────────────────────────────────────────────── */
		acc_alloc->state      = regex->initial;
		acc_alloc->next	      = head_acc;
		acc_alloc->match_from = string;

		++string; /* === increment string here === */

		head_acc = acc_alloc;
		++acc_alloc;

		/* process accumulated matching states
		 * ────────────────────────────────────────────────────────── */
		acc_ptr = &head_acc;
		acc	= head_acc;
		do {
			next_step = acc->state[token];

			/* no token match ? */
			if (next_step == NULL_POINTER) {

				/* TODO: traverse wildcard route */
				/* try wildcard */
				next_step = acc->state['\0'];

				if (next_step == NULL_POINTER) {
					/* delete acc from list and continue */
					acc	 = acc->next;
					*acc_ptr = acc;
					continue;
				}
			}

			/* regex entirely traversed (new match) ? */
			if (next_step == regex->matching) {
				/* update match interval */
				match->from  = acc->match_from;
				match->until = string;

				++match;

				/* delete acc from list */
				acc	 = acc->next;
				*acc_ptr = acc;

			/* token match, next_step is a state node (2x ptr) */
			} else {
				/* update accumulator state */
				acc->state = (const void *restrict *) next_step;

				/* process next accumlated matching state */
				acc_ptr = &acc->next;
				acc	= *acc_ptr;
			}
		} while (acc != NULL_POINTER);
	}

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
string_length(const unsigned char *const restrict string)
{
	register const unsigned char *restrict until = string;

	while (*until != '\0')
		++until;

	return until - string;
}
