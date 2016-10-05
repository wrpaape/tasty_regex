#include "tasty_regex.h"

/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern)
{
	/* const union TastyState *restrict state_alloc; */
	/* const union TastyState *restrict state; */
	/* struct TastyChunk *restrict chunk_stack; */

	/* const size_t length_pattern = string_length(pattern); */

	/* if (length_pattern == 0lu) */
	/* 	return TASTY_ERROR_EMPTY_PATTERN; */

	/* /1* allocate stack of chunks (worst case (N + 1) / 2 nodes deep when */
	/*  * pattern like "a|b|c|d" *1/ */
	/* struct TastyChunk *const restrict chunk_base */
	/* = malloc(sizeof(struct TastyChunk) * ((length_pattern + 1lu) / 2lu)); */

	/* if (UNLIKELY(chunk_base == NULL_POINTER)) */
	/* 	return TASTY_ERROR_OUT_OF_MEMORY; */

	/* /1* allocate worst case length_pattern count of state nodes, allocate all */
	/*  * pointers to NULL (i.e. no match) *1/ */
	/* state_alloc = calloc(length_pattern, */
	/* 		     sizeof(union TastyState)); */

	/* if (UNLIKELY(state_alloc == NULL_POINTER)) { */
	/* 	free(chunk_base); */
	/* 	return TASTY_ERROR_OUT_OF_MEMORY; */
	/* } */

	/* regex->initial	= state_alloc; */

	/* chunk	    = chunk_base; */
	/* chunk_stack = chunk + 1l; */


	/* regex->matching = state_alloc; */

	/* free(chunk_base); */

	return 0;
}


int
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
{
	struct TastyMatch *restrict match_alloc;
	struct TastyAccumulator *restrict acc_alloc;
	struct TastyAccumulator *restrict acc_list;

	const size_t length_string = string_length(string);

	/* want to ensure at least 1 non-'\0' char before start of walk */
	if (length_string == 0lu) {
		matches->from  = NULL_POINTER;
		matches->until = NULL_POINTER;
		return 0;
	}

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


	/* push first acc if start of match found */
	push_next_acc(&acc_list,
		      &acc_alloc,
		      regex,
		      string);
	++string;

	/* walk string */
	while (*string != '\0') {
		/* traverse acc_list: update states, prune dead-end accs, and
		 * append matches */
		acc_list_process(&acc_list,
				 &match_alloc,
				 regex->matching,
				 string);

		/* push next acc if start of match found */
		push_next_acc(&acc_list,
			      &acc_alloc,
			      regex,
			      string);

		++string;
	}

	/* append matches found in acc_list */
	acc_list_final_scan(acc_list,
			    &match_alloc,
			    regex->matching,
			    string);

	/* close match interval */
	matches->until = match_alloc;

	/* free temp storage for accs */
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
inline size_t
string_length(const unsigned char *const restrict string)
{
	register const unsigned char *restrict until = string;

	while (*until != '\0')
		++until;

	return until - string;
}

inline void
patch_states(struct TastyPatch *restrict patch,
	     const union TastyState *const restrict state)
{
	do {
		*(patch->state) = state;
		patch = patch->next;
	} while (patch != NULL_POINTER);
}

inline void
push_wild_patches(struct TastyPatch *restrict *const restrict patch_list,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  union TastyState *const restrict state)
{
	struct TastyPatch *restrict patch;
	struct TastyPatch *restrict next_patch;
	const union TastyState **restrict state_from;

	/* init list traversal vars */
	next_patch = *patch_list;
	patch	   = *patch_alloc;

	/* starting from first non-NULL match */
	state_from = &state->step[1];

	const union TastyState *const restrict *restrict state_until
	= state_from + UCHAR_MAX;

	do {
		patch->state = state_from;
		patch->next  = next_patch;

		next_patch = patch;
		++patch;

		++state_from;
	} while (state_from < state_until);

	/* update head of list, alloc */
	*patch_list  = next_patch;
	*patch_alloc = patch;
}




inline void
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

inline void
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


inline void
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

inline void
acc_list_final_scan(struct TastyAccumulator *restrict acc,
		    struct TastyMatch *restrict *const restrict match_alloc,
		    const union TastyState *const restrict matching,
		    const unsigned char *const restrict string)
{
	const union TastyState *restrict state;

	while (acc != NULL) {
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
				if (state == NULL)
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


/* fundamental state elements
 * ────────────────────────────────────────────────────────────────────────── */
inline union TastyState *
match_one(union TastyState *restrict *const restrict state_alloc,
	  struct TastyPatch *restrict *const restrict patch_alloc,
	  struct TastyPatch *restrict *const restrict patch_list,
	  const unsigned char match)
{
	/* pop state node */
	union TastyState *const restrict state = *state_alloc;
	++(*state_alloc);

	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record pointer needing to be set */
	patch->state = &state->step[match];

	/* push patch into head of patch_list */
	patch->next = *patch_list;
	*patch_list = patch;

	/* return new state */
	return state;
}

inline union TastyState *
match_zero_or_one(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_list,
		  const unsigned char match)
{
	struct TastyPatch *restrict patch;

	/* pop state node */
	union TastyState *const restrict state = *state_alloc;
	++(*state_alloc);

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_list;
	*patch_list = patch;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record match pointer needing to be set */
	patch->state = &state->step[match];

	/* push patch into head of patch_list */
	patch->next = *patch_list;
	*patch_list = patch;

	/* return new state */
	return state;
}

inline union TastyState *
match_zero_or_more(union TastyState *restrict *const restrict state_alloc,
		   struct TastyPatch *restrict *const restrict patch_alloc,
		   struct TastyPatch *restrict *const restrict patch_list,
		   const unsigned char match)
{
	/* pop state node */
	union TastyState *const restrict state = *state_alloc;
	++(*state_alloc);

	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_list;
	*patch_list = patch;

	/* patch match with self */
	state->step[match] = state;

	/* return new state */
	return state;
}

inline union TastyState *
match_one_or_more(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_list,
		  const unsigned char match)
{
	struct TastyPatch *restrict patch;

	/* pop state nodes */
	union TastyState *const restrict state_one	    = *state_alloc;
	union TastyState *const restrict state_zero_or_more = state_one + 1l;
	*state_alloc += 2l;

	/* patch first match */
	state_one->step[match] = state_zero_or_more;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state_zero_or_more->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_list;
	*patch_list = patch;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record match pointer needing to be set */
	patch->state = &state_zero_or_more->step[match];

	/* push patch into head of patch_list */
	patch->next = *patch_list;
	*patch_list = patch;

	/* return first state */
	return state_one;
}
