/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_regex_compile.h"
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


/* helper functions
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

static inline void
join_wild_state(union TastyState *const restrict wild_state,
		union TastyState *const restrict next_state)
{
	union TastyState *restrict *restrict state_from;

	/* starting from first non-NULL match */
	state_from = &wild_state->step[1];

	union TastyState *const restrict *restrict state_until
	= state_from + UCHAR_MAX;

	do {
		*state_from = next_state;
		++state_from;
	} while (state_from < state_until);
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

static union TastyState *
wild_one(union TastyState *restrict *const restrict state_alloc,
	 struct TastyPatch *restrict *const restrict patch_alloc,
	 struct TastyPatch *restrict *const restrict patch_head)
{
	/* pop state node */
	union TastyState *const restrict state = *state_alloc;
	++(*state_alloc);

	/* push a patch node for every UCHAR */
	push_wild_patches(patch_head,
			  patch_alloc,
			  state);

	/* return new state */
	return state;
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
wild_zero_or_one(union TastyState *restrict *const restrict state_alloc,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 struct TastyPatch *restrict *const restrict patch_head)
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

	/* push a patch node for every UCHAR */
	push_wild_patches(patch_head,
			  patch_alloc,
			  state);

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
wild_zero_or_more(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head)
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
	join_wild_state(state,
			state);

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

	/* patch second state with self on match */
	state_zero_or_more->step[token] = state_zero_or_more;

	/* return first state */
	return state_one;
}

static union TastyState *
wild_one_or_more(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head)
{
	struct TastyPatch *restrict patch;

	/* pop state nodes */
	union TastyState *const restrict state_one	    = *state_alloc;
	union TastyState *const restrict state_zero_or_more = state_one + 1l;
	*state_alloc += 2l;

	/* patch first match */
	join_wild_state(state_one,
			state_zero_or_more);

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state_zero_or_more->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* patch second state with self on wild */
	join_wild_state(state_zero_or_more,
			state_zero_or_more);

	/* return first state */
	return state_one;
}



/* fetch next atomic state from pattern
 * ────────────────────────────────────────────────────────────────────────── */
static inline int
fetch_next_state(union TastyState *restrict *const restrict state,
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

	static SetMatch *const set_match_map[UCHAR_MAX + 1] = {
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

	static SetWild *const set_wild_map[UCHAR_MAX + 1] = {
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
				  &chunk->patches.head,
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


/* free allocations */
extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);
