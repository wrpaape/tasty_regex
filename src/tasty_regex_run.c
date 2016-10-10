/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_regex_run.h"
#include "tasty_regex_utils.h"


/* helper macros
 * ────────────────────────────────────────────────────────────────────────── */
#ifdef __cplusplus
#	define NULL_POINTER nullptr /* use c++ null pointer constant */
#else
#	define NULL_POINTER NULL    /* use traditional c null pointer macro */
#endif /* ifdef __cplusplus */


/* typedefs, struct declarations
 * ────────────────────────────────────────────────────────────────────────── */
/* used for tracking accumulating matches during run */
struct TastyAccumulator {
	const union TastyState *state;	 /* currently matching regex */
	struct TastyAccumulator *next;	 /* next parallel matching state */
	const unsigned char *match_from; /* beginning of string match */
};


/* helper functions
 * ────────────────────────────────────────────────────────────────────────── */
static inline void
push_next_acc(struct TastyAccumulator *restrict *const restrict acc_list,
	      struct TastyAccumulator *restrict *const restrict acc_alloc,
	      const struct TastyRegex *const restrict regex,
	      const unsigned char *const restrict string)
{
	const union TastyState *restrict state;
	const union TastyState *restrict next_state;

	state = regex->initial;

	while (1) {
		next_state = state->step[*string];

		/* if no match on string */
		if (next_state == NULL_POINTER) {
			/* check skip route */
			next_state = state->skip;

			/* if DNE or skipped all the way to end w/o explicit
			 * match, do not add new acc to acc_list */
			if (   (next_state == NULL_POINTER)
			    || (next_state == regex->matching))
				return;

		/* explicit match */
		} else {
			/* pop a fresh accumulator node */
			struct TastyAccumulator *const restrict acc
			= *acc_alloc;
			++(*acc_alloc);

			/* populate it and push into acc_list */
			acc->state	= next_state;
			acc->next	= *acc_list;
			acc->match_from	= string;

			*acc_list = acc;
			return;
		}
	}
}


static inline void
push_match(struct TastyMatch *restrict *const restrict match_alloc,
	   const unsigned char *const restrict from,
	   const unsigned char *const restrict until)
{
	/* pop a fresh match node */
	struct TastyMatch *const restrict match = *match_alloc;
	++(*match_alloc);

	/* populate */
	match->from  = from;
	match->until = until;
}


static inline void
acc_list_process(struct TastyAccumulator *restrict *restrict acc_ptr,
		 struct TastyMatch *restrict *const restrict match_alloc,
		 const union TastyState *const restrict matching,
		 const unsigned char *const restrict string)
{
	struct TastyAccumulator *restrict acc;
	const union TastyState *restrict state;
	const union TastyState *restrict next_state;

	acc = *acc_ptr;

	while (acc != NULL_POINTER) {
		state = acc->state;

		/* if last step was a match */
		if (state == matching) {
			/* push new match */
			push_match(match_alloc,
				   acc->match_from,
				   string);

			/* remove acc from list */
			acc	 = acc->next;
			*acc_ptr = acc;

		/* step to next state */
		} else {
			next_state = state->step[*string];

			/* if no match on string */
			if (next_state == NULL_POINTER) {
				/* check skip route */
				next_state = state->skip;

				/* if DNE */
				if (next_state == NULL_POINTER) {
					/* remove acc from list */
					acc	 = acc->next;
					*acc_ptr = acc;
					continue;
				}
			}

			/* update accumulator state and traversal vars */
			acc->state = next_state;

			acc_ptr = &acc->next;
			acc	= acc->next;
		}
	}
}

static inline void
acc_list_final_scan(struct TastyAccumulator *restrict acc,
		    struct TastyMatch *restrict *const restrict match_alloc,
		    const union TastyState *const restrict matching,
		    const unsigned char *const restrict string)
{
	const union TastyState *restrict state;

	while (acc != NULL_POINTER) {
		state = acc->state;

		/* if last step was a match */
		if (state == matching) {
			/* push new match */
			push_match(match_alloc,
				   acc->match_from,
				   string);

		/* check if skip route can match */
		} else {
			while (1) {
				state = state->skip;

				/* if dead end, bail */
				if (state == NULL_POINTER)
					break;

				/* if skipping yields match, record */
				if (state == matching) {
					/* push new match */
					push_match(match_alloc,
						   acc->match_from,
						   string);
					break;
				}
			}
		}

		acc = acc->next;
	}
}


/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_run(const struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const char *restrict string)
{
	struct TastyMatch *restrict match_alloc;
	struct TastyAccumulator *restrict acc_alloc;
	struct TastyAccumulator *restrict acc_list;

	/* want to ensure at least 1 non-'\0' char before start of walk */
	if (*string == '\0') {
		matches->from  = NULL_POINTER;
		matches->until = NULL_POINTER;
		return 0;
	}

	const size_t length_string = nonempty_string_length(string);

	/* at most N matches */
	match_alloc = malloc(sizeof(struct TastyMatch) * length_string);

	if (UNLIKELY(match_alloc == NULL_POINTER))
		return TASTY_ERROR_OUT_OF_MEMORY;

	/* at most N running accumulators */
	struct TastyAccumulator *const restrict accumulators
	= malloc(sizeof(struct TastyAccumulator) * length_string);

	if (UNLIKELY(accumulators == NULL_POINTER)) {
		free(match_alloc);
		return TASTY_ERROR_OUT_OF_MEMORY;
	}

	acc_alloc = accumulators;	/* point acc_alloc to valid memory */
	acc_list  = NULL_POINTER;	/* initialize acc_list to empty */
	matches->from = match_alloc;	/* set start of match interval */

	/* walk string */
	while (1) {
		/* push next acc if explicit start of match found */
		push_next_acc(&acc_list,
			      &acc_alloc,
			      regex,
			      (const unsigned char *) string);

		++string;

		if (*string == '\0')
			break;

		/* traverse acc_list: update states, prune dead-end accs, and
		 * append matches */
		acc_list_process(&acc_list,
				 &match_alloc,
				 regex->matching,
				 (const unsigned char *) string);
	}

	/* append matches found in acc_list */
	acc_list_final_scan(acc_list,
			    &match_alloc,
			    regex->matching,
			    (const unsigned char *) string);

	/* close match interval */
	matches->until = match_alloc;

	/* free temporary storage */
	free(accumulators);

	/* return success */
	return 0;
}


/* free allocations */
extern inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches);
