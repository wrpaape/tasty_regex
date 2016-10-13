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



#define TASTY_CONTROL_END_OF_PATTERN	-1
#define TASTY_CONTROL_PARENTHESES_OPEN	-2
#define TASTY_CONTROL_PARENTHESES_CLOSE	-3
#define TASTY_CONTROL_OR_BAR		-4

#define IS_TASTY_CONTROL(STATUS) (STATUS < 0)


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
static inline unsigned int
utf8_head_width(const unsigned char head)
{
	if ((head & 0x80) == 0x00)
		return 1u;	/* ASCII character*/

	if ((head & 0x40) == 0x00)
		return 0u;	/* 10xxxxxx (not utf8) */

	if ((head & 0x20) == 0x00)
		return 2u;	/* 110xxxxx */

	if ((head & 0x10) == 0x00)
		return 3u;	/* 1110xxxx */

	if ((head & 0x08) == 0x00)
		return 4u;	/* 11110xxx */

	return 0u; /* 11111xxx (not utf8) */
}


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

	/* flatten skip route in state1 */
	if (*state1_from != NULL_POINTER)
		merge_states(*state1_from,
			     state2);

	state2_from = &state2->skip;

	/* flatten skip route in state2 */
	if (*state2_from != NULL_POINTER)
		merge_states(state1,
			     *state2_from);

	++state1_from;
	union TastyState *restrict *const restrict state1_until
	= state1_from + UCHAR_MAX;

	++state2_from;

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
copy_steps(union TastyState *const restrict state1,
	   union TastyState *const restrict state2)
{
	union TastyState *restrict *restrict state1_from;
	union TastyState *restrict *restrict state2_from;

	state1_from = &state1->step[1];
	state2_from = &state2->step[1];

	union TastyState *restrict *const restrict state1_until
	= state1_from + UCHAR_MAX;

	while (1) {
		*state1_from = *state2_from;

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
sub_chunk_zero_or_more(struct TastyChunk *const restrict chunk)
{
	struct TastyPatchList *const restrict chunk_patches = &chunk->patches;

	struct TastyPatch *const restrict chunk_patches_head
	= chunk_patches->head;

	patch_states(chunk_patches_head,
		     chunk->start);

	chunk_patches_head->state = &chunk->start->skip;
	chunk_patches_head->next  = NULL_POINTER;
	chunk_patches->end_ptr	  = &chunk_patches_head->next;
}

static inline void
sub_chunk_one_or_more(struct TastyChunk *const restrict chunk,
		      union TastyState *restrict *const restrict state_alloc)
{
	/* pop state node */
	union TastyState *const restrict state = *state_alloc;
	++(*state_alloc);

	/* append state node that will loop on a match or skip */
	copy_steps(state,
		   chunk->start);

	struct TastyPatchList *const restrict chunk_patches = &chunk->patches;
	struct TastyPatch *const restrict chunk_patches_head
	= chunk_patches->head;

	patch_states(chunk_patches->head,
		     state);

	chunk_patches_head->state = &state->skip;
	chunk_patches_head->next  = NULL_POINTER;
	chunk_patches->end_ptr	  = &chunk_patches_head->next;
}

static inline void
sub_chunk_zero_or_one(struct TastyChunk *const restrict chunk,
		      struct TastyPatch *restrict *const restrict patch_alloc)
{
	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip over entire chunk */
	patch->state = &chunk->start->skip;

	/* push patch into head of patch_list */
	patch->next  = chunk->patches.head;
	chunk->patches.head = patch;
}

static inline void
close_sub_chunk(struct TastyChunk *const restrict chunk,
		union TastyState *restrict *const restrict state_alloc,
		struct TastyPatch *restrict *const restrict patch_alloc,
		const unsigned char *restrict *const restrict pattern_ptr)
{
	switch (**pattern_ptr) {
	case '*':
		++(*pattern_ptr);
		sub_chunk_zero_or_more(chunk);
		return;

	case '+':
		++(*pattern_ptr);
		sub_chunk_one_or_more(chunk,
				      state_alloc);
		return;

	case '?':
		++(*pattern_ptr);
		sub_chunk_zero_or_one(chunk,
				      patch_alloc);
		return;

	default: /* no nothing */
		return;
	}
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
static inline void
match_one(union TastyState *const restrict state,
	  struct TastyPatch *restrict *const restrict patch_alloc,
	  struct TastyPatch *restrict *const restrict patch_head,
	  const unsigned char token)
{
	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record pointer needing to be set */
	patch->state = &state->step[token];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;
}

static inline void
wild_one(union TastyState *const restrict state,
	 struct TastyPatch *restrict *const restrict patch_alloc,
	 struct TastyPatch *restrict *const restrict patch_head)
{
	/* push a patch node for every UCHAR */
	push_wild_patches(patch_head,
			  patch_alloc,
			  state);
}

static inline void
match_wide_one(union TastyState *const restrict state_last,
	       struct TastyPatch *restrict *const restrict patch_alloc,
	       struct TastyPatch *restrict *const restrict patch_head,
	       const unsigned char token_last)
{
	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record pointer needing to be set */
	patch->state = &state_last->step[token_last];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;
}


static inline void
match_zero_or_one(union TastyState *const restrict state,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head,
		  const unsigned char token)
{
	struct TastyPatch *restrict patch;

	/* pop patch node */
	 patch = *patch_alloc;
	*patch_alloc += 2l; /* account for second patch */

	/* record skip pointer needing to be set */
	patch->state = &state->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* pop patch node */
	++patch;

	/* record match pointer needing to be set */
	patch->state = &state->step[token];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;
}

static inline void
wild_zero_or_one(union TastyState *const restrict state,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 struct TastyPatch *restrict *const restrict patch_head)
{
	struct TastyPatch *restrict patch;

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
}

static inline void
match_wide_zero_or_one(union TastyState *const restrict state_first,
		       union TastyState *const restrict state_last,
		       struct TastyPatch *restrict *const restrict patch_alloc,
		       struct TastyPatch *restrict *const restrict patch_head,
		       const unsigned char token_last)
{
	struct TastyPatch *restrict patch;

	/* pop patch node */
	 patch = *patch_alloc;
	*patch_alloc += 2l; /* account for second patch */

	/* record skip pointer needing to be set */
	patch->state = &state_first->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* pop patch node */
	++patch;

	/* record match pointer needing to be set */
	patch->state = &state_last->step[token_last];

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;
}


static inline void
match_zero_or_more(union TastyState *const restrict state,
		   struct TastyPatch *restrict *const restrict patch_alloc,
		   struct TastyPatch *restrict *const restrict patch_head,
		   const unsigned char token)
{
	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* patch match with start of self */
	state->step[token] = state;
}

static inline void
wild_zero_or_more(union TastyState *const restrict state,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head)
{
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
}

static inline void
match_wide_zero_or_more(union TastyState *const restrict state_first,
			union TastyState *const restrict state_last,
			struct TastyPatch *restrict *const restrict patch_alloc,
			struct TastyPatch *restrict *const restrict patch_head,
			const unsigned char token_last)
{
	/* pop patch node */
	struct TastyPatch *const restrict patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state_first->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* patch match with start of self */
	state_last->step[token_last] = state_first;
}


static inline void
match_one_or_more(union TastyState *const restrict state_one,
		  union TastyState *const restrict state_zero_or_more,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_head,
		  const unsigned char token)
{
	struct TastyPatch *restrict patch;

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
}

static inline void
wild_one_or_more(union TastyState *const restrict state_one,
		 union TastyState *const restrict state_zero_or_more,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 struct TastyPatch *restrict *const restrict patch_head)
{
	struct TastyPatch *restrict patch;

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
}

static inline void
match_wide_one_or_more(union TastyState *const restrict state_second,
		       union TastyState *const restrict state_last,
		       union TastyState *const restrict state_zero_or_more,
		       struct TastyPatch *restrict *const restrict patch_alloc,
		       struct TastyPatch *restrict *const restrict patch_head,
		       const unsigned char token_first,
		       const unsigned char token_last)
{
	struct TastyPatch *restrict patch;

	/* patch first match */
	state_last->step[token_last] = state_zero_or_more;

	/* pop patch node */
	 patch = *patch_alloc;
	++(*patch_alloc);

	/* record skip pointer needing to be set */
	patch->state = &state_zero_or_more->skip;

	/* push patch into head of patch_list */
	patch->next = *patch_head;
	*patch_head = patch;

	/* patch looping state with second in chain */
	state_zero_or_more->step[token_first] = state_second;
}



/* fetch next state node from pattern
 * ────────────────────────────────────────────────────────────────────────── */
static inline int
fetch_next_state(union TastyState *restrict *const restrict state,
		 union TastyState *restrict *const restrict state_alloc,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 struct TastyPatch *restrict *const restrict patch_head,
		 const unsigned char *restrict *const restrict pattern_ptr)
{
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

	const unsigned char *restrict pattern;
	unsigned char token;
	union TastyState *restrict state_prev;
	union TastyState *restrict state_next;

	union TastyState *const restrict state_first = *state_alloc;
	*state	   = state_first;
	state_prev = state_first;
	state_next = state_prev + 1l;

	pattern = *pattern_ptr;
	token   = *pattern;

	switch (utf8_head_width(token)) {
	case 4u:
		state_prev->step[token] = state_next;
		state_prev = state_next;
		++state_next;
		++pattern;
		/* fall through */
	case 3u:
		state_prev->step[*pattern] = state_next;
		state_prev = state_next;
		++state_next;
		++pattern;
		/* fall through */
	case 2u:
		state_prev->step[*pattern] = state_next;
		++pattern;
		const unsigned char token_last = *pattern;

		++pattern;
		switch (*pattern) {
		case '*':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next + 1l; /* pop state nodes */
			match_wide_zero_or_more(state_first,
						state_next,
						patch_alloc,
						patch_head,
						token_last);
			return 0;

		case '+':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next + 2l; /* pop state nodes */
			match_wide_one_or_more(state_first + 1l,
					       state_next,
					       state_next + 1l,
					       patch_alloc,
					       patch_head,
					       token,
					       token_last);
			return 0;

		case '?':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next + 1l; /* pop state nodes */
			match_wide_zero_or_one(state_first,
					       state_next,
					       patch_alloc,
					       patch_head,
					       token_last);
			return 0;

		default:
			*pattern_ptr = pattern;
			*state_alloc = state_next + 1l; /* pop state nodes */
			match_wide_one(state_next,
				       patch_alloc,
				       patch_head,
				       token_last);
			return 0;
		}

	case 1u:
		break; /* ASCII character found */
	default:
		return TASTY_ERROR_INVALID_UTF8;
	}


	/* fetch token */
	switch (token) {
	case '\0': /* let caller handle control characters */
		return TASTY_CONTROL_END_OF_PATTERN;
	case '(':
		return TASTY_CONTROL_PARENTHESES_OPEN;
	case ')':
		return TASTY_CONTROL_PARENTHESES_CLOSE;
	case '|':
		return TASTY_CONTROL_OR_BAR;

	case '*':
	case '+':
	case '?':
		return TASTY_ERROR_NO_OPERAND;

	case '.': /* wild state, check for operator */
		++pattern;
		switch (*pattern) {
		case '*':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next; /* pop state node */
			wild_zero_or_more(state_first,
					  patch_alloc,
					  patch_head);
			return 0;

		case '+':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next + 1l; /* pop 2 state nodes */
			wild_one_or_more(state_first,
					 state_next,
					 patch_alloc,
					 patch_head);
			return 0;

		case '?':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next; /* pop state node */
			wild_zero_or_one(state_first,
					 patch_alloc,
					 patch_head);
			return 0;

		default:
			*pattern_ptr = pattern;
			*state_alloc = state_next; /* pop state node */
			wild_one(state_first,
				 patch_alloc,
				 patch_head);
			return 0;
		}


	case '\\':
		++pattern;
		token = *pattern;

		if (!valid_escape_map[token])
			return TASTY_ERROR_INVALID_ESCAPE;
		/* fall through */
	default: /* literal match state, check for operator */
		++pattern;
		switch (*pattern) {
		case '*':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next; /* pop state node */
			match_zero_or_more(state_first,
					   patch_alloc,
					   patch_head,
					   token);
			return 0;

		case '+':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next + 1l; /* pop 2 state nodes */
			match_one_or_more(state_first,
					  state_next,
					  patch_alloc,
					  patch_head,
					  token);
			return 0;

		case '?':
			*pattern_ptr = pattern + 1l;
			*state_alloc = state_next; /* pop state node */
			match_zero_or_one(state_first,
					  patch_alloc,
					  patch_head,
					  token);
			return 0;

		default:
			*pattern_ptr = pattern;
			*state_alloc = state_next; /* pop state node */
			match_one(state_first,
				  patch_alloc,
				  patch_head,
				  token);
			return 0;
		}
	}
}


int
fetch_next_sub_chunk(struct TastyChunk *const restrict chunk,
		     union TastyState *restrict *const restrict state_alloc,
		     struct TastyPatch *restrict *const restrict patch_alloc,
		     const unsigned char *restrict *const restrict pattern_ptr)
{
	struct TastyChunk next_chunk;
	union TastyState *restrict state;
	struct TastyPatchList next_patches;
	int status;

	struct TastyPatchList *const restrict chunk_patches = &chunk->patches;

	/* ensure at least 1 state in pattern chunk */
	chunk_patches->head    = NULL_POINTER;
	chunk_patches->end_ptr = &(*patch_alloc)->next;

	status = fetch_next_state(&chunk->start,
				  state_alloc,
				  patch_alloc,
				  &chunk_patches->head,
				  pattern_ptr);

	switch (status) {
	case 0: /* found next state */
		break;

	case TASTY_CONTROL_END_OF_PATTERN:
		return TASTY_ERROR_UNBALANCED_PARENTHESES;

	case TASTY_CONTROL_OR_BAR:
	case TASTY_CONTROL_PARENTHESES_CLOSE:
		return TASTY_ERROR_EMPTY_EXPRESSION;

	case TASTY_CONTROL_PARENTHESES_OPEN:
		++(*pattern_ptr);
		status = fetch_next_sub_chunk(chunk,
					      state_alloc,
					      patch_alloc,
					      pattern_ptr);
		if (status == 0) {
			close_sub_chunk(chunk,
					state_alloc,
					patch_alloc,
					pattern_ptr);
			break;
		} /* fall through */
	default: /* error */
		return status;
	}

	next_patches.head = NULL_POINTER;

	while (1) {
		next_patches.end_ptr = &(*patch_alloc)->next;

		status = fetch_next_state(&state,
					  state_alloc,
					  patch_alloc,
					  &next_patches.head,
					  pattern_ptr);

		switch (status) {
		case 0: /* patch previous with next state */
			patch_states(chunk_patches->head,
				     state);
			*chunk_patches = next_patches;
			next_patches.head = NULL_POINTER;
			continue;

		case TASTY_CONTROL_PARENTHESES_CLOSE:
			++(*pattern_ptr);
			return 0;

		case TASTY_CONTROL_END_OF_PATTERN:
			return TASTY_ERROR_UNBALANCED_PARENTHESES;

		case TASTY_CONTROL_OR_BAR:
			++(*pattern_ptr);
			status = fetch_next_sub_chunk(&next_chunk,
						      state_alloc,
						      patch_alloc,
						      pattern_ptr);
			if (status == 0)
				merge_chunks(chunk,
					     &next_chunk);

			return status;

		case TASTY_CONTROL_PARENTHESES_OPEN:
			++(*pattern_ptr);
			status = fetch_next_sub_chunk(&next_chunk,
						      state_alloc,
						      patch_alloc,
						      pattern_ptr);
			if (status == 0) {
				close_sub_chunk(&next_chunk,
						state_alloc,
						patch_alloc,
						pattern_ptr);
				concat_chunks(chunk,
					      &next_chunk);
				continue;
			} /* fall through */
		default: /* error */
			return status;
		}
	}
}

int
fetch_next_chunk(struct TastyChunk *const restrict chunk,
		 union TastyState *restrict *const restrict state_alloc,
		 struct TastyPatch *restrict *const restrict patch_alloc,
		 const unsigned char *restrict *const restrict pattern_ptr)
{
	struct TastyChunk next_chunk;
	union TastyState *restrict state;
	struct TastyPatchList next_patches;
	int status;

	struct TastyPatchList *const restrict chunk_patches = &chunk->patches;

	/* ensure at least 1 state in pattern chunk */
	chunk_patches->head    = NULL_POINTER;
	chunk_patches->end_ptr = &(*patch_alloc)->next;

	status = fetch_next_state(&chunk->start,
				  state_alloc,
				  patch_alloc,
				  &chunk_patches->head,
				  pattern_ptr);

	switch (status) {
	case 0: /* found next state */
		break;

	case TASTY_CONTROL_OR_BAR:
	case TASTY_CONTROL_END_OF_PATTERN:
		return TASTY_ERROR_EMPTY_EXPRESSION;

	case TASTY_CONTROL_PARENTHESES_CLOSE:
		return TASTY_ERROR_UNBALANCED_PARENTHESES;

	case TASTY_CONTROL_PARENTHESES_OPEN:
		++(*pattern_ptr);
		status = fetch_next_sub_chunk(chunk,
					      state_alloc,
					      patch_alloc,
					      pattern_ptr);
		if (status == 0) {
			close_sub_chunk(chunk,
					state_alloc,
					patch_alloc,
					pattern_ptr);
			break;
		}
		/* fall through */
	default: /* error */
		return status;
	}

	next_patches.head = NULL_POINTER;

	while (1) {
		next_patches.end_ptr = &(*patch_alloc)->next;

		status = fetch_next_state(&state,
					  state_alloc,
					  patch_alloc,
					  &next_patches.head,
					  pattern_ptr);

		switch (status) {
		case 0: /* patch previous with next state */
			patch_states(chunk_patches->head,
				     state);
			*chunk_patches = next_patches;
			next_patches.head = NULL_POINTER;
			continue;

		case TASTY_CONTROL_END_OF_PATTERN: /* return success */
			return 0;

		case TASTY_CONTROL_PARENTHESES_CLOSE:
			return TASTY_ERROR_UNBALANCED_PARENTHESES;

		case TASTY_CONTROL_OR_BAR:
			++(*pattern_ptr);
			status = fetch_next_chunk(&next_chunk,
						  state_alloc,
						  patch_alloc,
						  pattern_ptr);
			if (status == 0)
				merge_chunks(chunk,
					     &next_chunk);
			return status;

		case TASTY_CONTROL_PARENTHESES_OPEN:
			++(*pattern_ptr);
			status = fetch_next_sub_chunk(&next_chunk,
						      state_alloc,
						      patch_alloc,
						      pattern_ptr);
			if (status == 0) {
				close_sub_chunk(&next_chunk,
						state_alloc,
						patch_alloc,
						pattern_ptr);
				concat_chunks(chunk,
					      &next_chunk);
				continue;
			} /* fall through */
		default: /* error */
			return status;
		}
	}
}

static inline int
compile_pattern(struct TastyRegex *const restrict regex,
		union TastyState *restrict state_alloc,
		struct TastyPatch *restrict patch_alloc,
		const unsigned char *restrict pattern)
{

	struct TastyChunk chunk;
	int status;

	status = fetch_next_chunk(&chunk,
				  &state_alloc,
				  &patch_alloc,
				  &pattern);

	if (status == 0) {
		regex->initial	= chunk.start;
		regex->matching = state_alloc;

		patch_states(chunk.patches.head,
			     state_alloc);
	}

	return status;
}


/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const char *restrict pattern)
{
	if (*pattern == '\0')
		return TASTY_ERROR_EMPTY_EXPRESSION;

	const size_t length_pattern = nonempty_string_length(pattern);

	/* allocate buffer of patch nodes for worst case pattern:
	 * "........" (all wild, no operators) */
	struct TastyPatch *const restrict patch_alloc
	= malloc(sizeof(struct TastyPatch) * (length_pattern * UCHAR_MAX));

	if (UNLIKELY(patch_alloc == NULL_POINTER))
		return TASTY_ERROR_OUT_OF_MEMORY;

	/* allocate buffer of state nodes for worst case pattern:
	 * "abcdefgh" (no operators)
	 * and initialize all pointers to NULL */
	union TastyState *const restrict state_alloc
	= calloc(length_pattern,
		 sizeof(union TastyState));

	if (UNLIKELY(state_alloc == NULL_POINTER)) {
		free(patch_alloc);
		return TASTY_ERROR_OUT_OF_MEMORY;
	}

	const int status = compile_pattern(regex,
					   state_alloc,
					   patch_alloc,
					   (const unsigned char *) pattern);

	if (status != 0)
		free(state_alloc);

	free(patch_alloc);

	return status;
}


/* free allocations */
extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);
