#ifndef TASTY_REGEX_TASTY_REGEX_H_
#define TASTY_REGEX_TASTY_REGEX_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
#	ifndef restrict /* use c++ compatible '__restrict__' */
#		define restrict __restrict__
#	endif
#	ifndef NULL_POINTER /* use c++ null pointer macro */
#		define NULL_POINTER nullptr
#	endif
#else
#	ifndef NULL_POINTER /* use traditional c null pointer macro */
#		define NULL_POINTER NULL
#	endif
#endif /* ifdef __cplusplus */


/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include <stdlib.h>	/* size_t, m|calloc, free */
#include <limits.h>	/* UCHAR_MAX */


/* helper macros
 * ────────────────────────────────────────────────────────────────────────── */
#define LIKELY(BOOL)   __builtin_expect(BOOL, 1)
#define UNLIKELY(BOOL) __builtin_expect(BOOL, 0)

#define TASTY_STATE_LENGTH (UCHAR_MAX + 1)

/* error macros
 * ────────────────────────────────────────────────────────────────────────── */
#define TASTY_ERROR_EMPTY_PATTERN	1	/* 'pattern' is "" */
#define TASTY_ERROR_OUT_OF_MEMORY	2	/* failed to allocate memory */
#define TASTY_ERROR_UNBALANCED_PARENTHS 3	/* parentheses not balanced */


/* typedefs, struct declarations
 * ────────────────────────────────────────────────────────────────────────── */
struct TastyMatch {
	const unsigned char *restrict from;
	const unsigned char *restrict until;
};

struct TastyMatchInterval {
	struct TastyMatch *restrict from;
	struct TastyMatch *restrict until;
};


/* DFA state node, step['\0'] reserved for 'skip' field (no explicit match) */
union TastyState {
	const union TastyState *step[UCHAR_MAX + 1]; /* jump tbl for non-'\0' */
	const union TastyState *skip;		     /* no match option */
};

/* keep list of trailing pointers needing to be set to next state */
struct TastyPatch {
	const union TastyState **state;
	struct TastyPatch *next;
};

/* DFA chunks, used temporarily in compilation */
struct TastyChunk {
	const union TastyState *start;
	struct TastyPatch *patch_list;
};

/* complete DFA */
struct TastyRegex {
	const union TastyState *restrict initial;
	const union TastyState *restrict matching;
};

struct TastyAccumulator {
	const void *restrict *state;	 /* currently matching regex */
	struct TastyAccumulator *next;	 /* next parallel matching state */
	const unsigned char *match_from; /* beginning of string match */
};


/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern);

int
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string);

/* free allocations */
inline void
tasty_regex_free(struct TastyRegex *const restrict regex)
{
	free((void *) regex->initial);
}

inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches)
{
	free(matches->from);
}

/* helper functions
 * ────────────────────────────────────────────────────────────────────────── */
static inline size_t
string_length(const unsigned char *const restrict string);

static inline void
patch_states(struct TastyPatch *restrict patch,
	     const union TastyState *const restrict state);

static inline void
push_wild_patches(struct TastyPatch *restrict *const restrict patch_list,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  union TastyState *const restrict state);

/* fundamental state elements
 * ────────────────────────────────────────────────────────────────────────── */
static inline union TastyState *
match_one(union TastyState *restrict *const restrict state_alloc,
	  struct TastyPatch *restrict *const restrict patch_alloc,
	  struct TastyPatch *restrict *const restrict patch_list,
	  const unsigned char match);

static inline union TastyState *
match_zero_or_one(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_list,
		  const unsigned char match);

static inline union TastyState *
match_zero_or_more(union TastyState *restrict *const restrict state_alloc,
		   struct TastyPatch *restrict *const restrict patch_alloc,
		   struct TastyPatch *restrict *const restrict patch_list,
		   const unsigned char match);

static inline union TastyState *
match_one_or_more(union TastyState *restrict *const restrict state_alloc,
		  struct TastyPatch *restrict *const restrict patch_alloc,
		  struct TastyPatch *restrict *const restrict patch_list,
		  const unsigned char match);
#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_H_ */
