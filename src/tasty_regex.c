#include "tasty_regex.h"

/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern)
{
	/* const void *restrict *state_alloc; */
	/* const void *restrict *state; */
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


	char token;
	char next_token;

	return 0;
}


int
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
{
	/* struct TastyMatch *restrict match;	      /1* free nodes *1/ */
	/* struct TastyAccumulator *restrict acc_alloc;  /1* free nodes *1/ */
	/* struct TastyAccumulator *restrict head_acc;   /1* list of live matches *1/ */
	/* struct TastyAccumulator *restrict acc;	      /1* list traversal var *1/ */
	/* struct TastyAccumulator *restrict *acc_ptr;   /1* list traversal var *1/ */
	/* const void *restrict next_step;		      /1* next step from state *1/ */

	/* const size_t length_string = string_length(string); */

	/* /1* at most length(string) matches *1/ */
	/* match = malloc(sizeof(struct TastyMatch) * length_string); */

	/* if (UNLIKELY(match == NULL_POINTER)) */
	/* 	return TASTY_ERROR_OUT_OF_MEMORY; */

	/* /1* at most length(string) parallel matching states to keep track of */
	/*  * (free before return) *1/ */
	/* struct TastyAccumulator *const restrict accumulators */
	/* = malloc(sizeof(struct TastyAccumulator) * length_string); */

	/* if (UNLIKELY(accumulators == NULL_POINTER)) { */
	/* 	free(match); */
	/* 	return TASTY_ERROR_OUT_OF_MEMORY; */
	/* } */

	/* matches->from = match; */

	/* acc_alloc = accumulators; */
	/* head_acc  = NULL_POINTER; */

	/* /1* step through each character of string */
	/*  * ────────────────────────────────────────────────────────────────── *1/ */
	/* for (unsigned char token = *string; token != '\0'; token = *string) { */
	/* 	/1* push initial state into list of accumulators */
	/* 	 * ────────────────────────────────────────────────────────── *1/ */
	/* 	acc_alloc->state      = regex->initial; */
	/* 	acc_alloc->next	      = head_acc; */
	/* 	acc_alloc->match_from = string; */

	/* 	++string; /1* === increment string here === *1/ */

	/* 	head_acc = acc_alloc; */
	/* 	++acc_alloc; */

	/* 	/1* process accumulated matching states */
	/* 	 * ────────────────────────────────────────────────────────── *1/ */
	/* 	acc_ptr = &head_acc; */
	/* 	acc	= head_acc; */
	/* 	do { */
	/* 		next_step = acc->state[token]; */

	/* 		/1* no token match ? *1/ */
	/* 		if (next_step == NULL_POINTER) { */

	/* 			/1* TODO: traverse wildcard route *1/ */
	/* 			/1* try wildcard *1/ */
	/* 			next_step = acc->state['\0']; */

	/* 			if (next_step == NULL_POINTER) { */
	/* 				/1* delete acc from list and continue *1/ */
	/* 				acc	 = acc->next; */
	/* 				*acc_ptr = acc; */
	/* 				continue; */
	/* 			} */
	/* 		} */

	/* 		/1* regex entirely traversed (new match) ? *1/ */
	/* 		if (next_step == regex->matching) { */
	/* 			/1* update match interval *1/ */
	/* 			match->from  = acc->match_from; */
	/* 			match->until = string; */

	/* 			++match; */

	/* 			/1* delete acc from list *1/ */
	/* 			acc	 = acc->next; */
	/* 			*acc_ptr = acc; */

	/* 		/1* token match, next_step is a state node (2x ptr) *1/ */
	/* 		} else { */
	/* 			/1* update accumulator state *1/ */
	/* 			acc->state = (const void *restrict *) next_step; */

	/* 			/1* process next accumlated matching state *1/ */
	/* 			acc_ptr = &acc->next; */
	/* 			acc	= *acc_ptr; */
	/* 		} */
	/* 	} while (acc != NULL_POINTER); */
	/* } */

	/* matches->until = match; */

	/* free(accumulators); */
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

static inline union TastyState *
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
