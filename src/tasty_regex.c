/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_regex_utils.h"

/* helper macros
 * ────────────────────────────────────────────────────────────────────────── */
#ifdef __cplusplus
#	define NULL_POINTER nullptr /* use c++ null pointer constant */
#else
#	define NULL_POINTER NULL    /* use traditional c null pointer macro */
#endif /* ifdef __cplusplus */

#define TASTY_END_OF_CHUNK -1


/* typedefs, struct declarations
 * ────────────────────────────────────────────────────────────────────────── */
/* for tracking unset loose ends of DFA states */
struct TastyPatch {
	union TastyState *restrict *state;
	struct TastyPatch *next;
};

struct TastyPatchList {
	struct TastyPatch *restrict head;
	struct TastyPatch *restrict *end_ptr;
};

/* DFA chunks, used temporarily in compilation */
struct TastyChunk {
	union TastyState *start;
	struct TastyPatchList patches;
};

struct TastyOrNode {
	struct TastyChunk *chunk;
	struct TastyOrNode *next;
};


/* used for tracking accumulating matches during run */
struct TastyAccumulator {
	const union TastyState *state;	 /* currently matching regex */
	struct TastyAccumulator *next;	 /* next parallel matching state */
	const unsigned char *match_from; /* beginning of string match */
};


/* global variables
 * ────────────────────────────────────────────────────────────────────────── */



/* helper functions
 * ────────────────────────────────────────────────────────────────────────── */

/* compile helper functions
 * ────────────────────────────────────────────────────────────────────────── */
static inline void
patch_states(struct TastyPatch *restrict patch,
	     union TastyState *const restrict state)
{
	do {
		*(patch->state) = state;
		patch = patch->next;
	} while (patch != NULL_POINTER);
}

static inline void
concat_patches(struct TastyPatchList *const restrict patches1,
		   struct TastyPatchList *const restrict patches2)
{
	*(patches1->end_ptr) = patches2->head;
	patches1->end_ptr    = patches2->end_ptr;
}

static inline void
merge_states(union TastyState *const restrict state1,
	     union TastyState *const restrict state2)
{
	union TastyState *restrict *restrict state1_from;
	union TastyState *restrict *restrict state2_from;

	state1_from = &state1->skip;
	state2_from = &state2->skip;

	union TastyState *restrict *const restrict state1_until
	= state1_from + (UCHAR_MAX + 1l);

	while (1) {
		/* no conflict, merge state2's branch into state1 */
		if (*state1_from == NULL_POINTER) {
			*state1_from = *state2_from;

		/* fork on same match (NFA), need to flatten branch */
		} else if (*state2_from != NULL_POINTER) {
			merge_states(*state1_from,
				     *state2_from);
		}

		++state1_from;
		if (state1_from == state1_until)
			return;

		++state2_from;
	}
}


static inline void
merge_chunks(struct TastyChunk *const restrict chunk1,
	     struct TastyChunk *const restrict chunk2)
{
	merge_states(chunk1->start,
		     chunk2->start);

	concat_patches(&chunk1->patches,
		       &chunk2->patches);
}

static inline void
concat_chunks(struct TastyChunk *const restrict chunk1,
	      struct TastyChunk *const restrict chunk2)
{
	patch_states(chunk1->patches.head,
		     chunk2->start);

	chunk1->patches = chunk2->patches;
}

static inline void
push_wild_patches(struct TastyPatch *restrict *const restrict patch_head,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  union TastyState *const restrict state)
{
	struct TastyPatch *restrict patch;
	struct TastyPatch *restrict next_patch;
	union TastyState *restrict *restrict state_from;

	/* init list traversal vars */
	next_patch = *patch_head;
	patch	   = *patch_alloc;

	/* starting from first non-NULL match */
	state_from = &state->step[1];

	union TastyState *const restrict *restrict state_until
	= state_from + UCHAR_MAX;

	do {
		patch->state = state_from;
		patch->next  = next_patch;

		next_patch = patch;
		++patch;

		++state_from;
	} while (state_from < state_until);

	/* update head of list, alloc */
	*patch_head  = next_patch;
	*patch_alloc = patch;
}




/* run helper functions
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


/* fundamental state elements
 * ────────────────────────────────────────────────────────────────────────── */
static union TastyState *
match_one(union TastyState *restrict *const restrict state_alloc,
	  struct TastyPatch *restrict *const restrict patch_alloc,
	  struct TastyPatch *restrict *const restrict patch_head,
	  const unsigned char token)
{
	/* pop state node */
	union TastyState *const restrict state = *state_alloc;
	++(*state_alloc);

	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record pointer needing to be set */
	patch->state = &state->step[token];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* return new state */
	return state;
}

/* TODO SetWild functions */
static union TastyState *
wild_one(union TastyState *restrict *const restrict state_alloc,
	 struct TastyPatch *restrict *const restrict patch_alloc,
	 struct TastyPatch *restrict *const restrict patch_head)
{
	return NULL;
}


static union TastyState *
match_zero_or_one(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head,
		  const unsigned char token)
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
	patch->next = *patch_head;
	*patch_head = patch;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record match pointer needing to be set */
	patch->state = &state->step[token];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* return new state */
	return state;
}

static union TastyState *
match_zero_or_more(union TastyState *restrict *const restrict state_alloc,
		   struct TastyPatch *restrict *const restrict patch_alloc,
		   struct TastyPatch *restrict *const restrict patch_head,
		   const unsigned char token)
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
	patch->next = *patch_head;
	*patch_head = patch;

	/* patch match with self */
	state->step[token] = state;

	/* return new state */
	return state;
}

static union TastyState *
match_one_or_more(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head,
		  const unsigned char token)
{
	struct TastyPatch *restrict patch;

	/* pop state nodes */
	union TastyState *const restrict state_one	    = *state_alloc;
	union TastyState *const restrict state_zero_or_more = state_one + 1l;
	*state_alloc += 2l;

	/* patch first match */
	state_one->step[token] = state_zero_or_more;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state_zero_or_more->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record match pointer needing to be set */
	patch->state = &state_zero_or_more->step[token];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* return first state */
	return state_one;
}

static inline int
fetch_next_state(struct TastyState *restrict *const restrict state,
		 union TastyState *restrict *const restrict state_alloc,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 struct TastyPatch *restrict *const restrict patch_head,
		 const unsigned char *restrict *const restrict string_ptr)
{
	/* cached maps */
	typedef union TastyState *
	SetMatch(union TastyState *restrict *const restrict state_alloc,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 struct TastyPatch *restrict *const restrict patch_head,
		 const unsigned char token);

	static const SetMatch *const set_match_map[UCHAR_MAX + 1] = {
		['\0' ... ')']	     = &match_one,
		['*']		     = &match_zero_or_more,
		['+']		     = &match_one_or_more,
		[','  ... '>']	     = &match_one,
		['?']		     = &match_zero_or_one,
		['@' ...  UCHAR_MAX] = &match_one
	};

	typedef union TastyState *
	SetWild(union TastyState *restrict *const restrict state_alloc,
		struct TastyPatch *restrict *const restrict patch_alloc,
		struct TastyPatch *restrict *const restrict patch_head);

	static const SetWild *const set_wild_map[UCHAR_MAX + 1] = {
		['\0' ... ')']	     = &wild_one,
		['*']		     = &wild_zero_or_more,
		['+']		     = &wild_one_or_more,
		[','  ... '>']	     = &wild_one,
		['?']		     = &wild_zero_or_one,
		['@' ...  UCHAR_MAX] = &wild_one
	};

	static const bool valid_escape_map[UCHAR_MAX + 1] = {
		['\\'] = true,
		['*']  = true,
		['+']  = true,
		['.']  = true,
		['(']  = true,
		[')']  = true,
		['?']  = true,
		['|']  = true
	};

	const unsigned char *restrict string;
	unsigned char token;

	string = *string_ptr;
	token  = *string;

	/* fetch token */
	switch (token) {
	case '\0':
	case '(':
	case ')':
	case '|': /* let caller handle control characters */
		return TASTY_END_OF_CHUNK;

	case '*':
	case '+':
	case '?':
		return TASTY_ERROR_NO_OPERAND;

	case '.':
		++string;
		/* check for operand */
		*state = set_wild_map[*string](state_alloc,
					       patch_alloc,
					       patch_head);
		break;

	case '\\':
		++string;
		token = *string;

		if (!valid_escape_map[token])
			return TASTY_ERROR_INVALID_ESCAPE;
		/* fall through */
	default:
		++string;
		/* check for operand */
		*state = set_match_map[*string](state_alloc,
						patch_alloc,
						patch_head,
						token);
	}

	*string_ptr = string;
	return 0;
}

static inline int
fetch_next_chunk(struct TastyChunk *const restrict chunk,
		 union TastyState *restrict *const restrict state_alloc,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 const unsigned char *restrict *const restrict string_ptr)
{
	const union TastyState *restrict state;
	const unsigned char *restrict string;
	unsigned char token;
	int status;

	/* init chunk */
	chunk->patches.head    = NULL_POINTER;
	chunk->patches.end_ptr = &(*patch_alloc)->next;

	status = fetch_next_state(&chunk->start,
				  state_alloc,
				  patch_alloc,
				  string_ptr);

	if (status != 0)
		return status;


	return 0;
}






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
tasty_regex_run(const struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string)
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
			      string);

		++string;

		if (*string == '\0')
			break;

		/* traverse acc_list: update states, prune dead-end accs, and
		 * append matches */
		acc_list_process(&acc_list,
				 &match_alloc,
				 regex->matching,
				 string);
	}

	/* append matches found in acc_list */
	acc_list_final_scan(acc_list,
			    &match_alloc,
			    regex->matching,
			    string);

	/* close match interval */
	matches->until = match_alloc;

	/* free temporary storage */
	free(accumulators);

	/* return success */
	return 0;
}


/* free allocations */
extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);

extern inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches);
