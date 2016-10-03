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
#define TASTY_ERROR_EMPTY_PATTERN 1	/* 'pattern' is "" */
#define TASTY_ERROR_OUT_OF_MEMORY 2	/* failed to allocate memory */


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

/* struct TastyState { */
/* 	const struct TastyState *restrict *step; /1* jump forward to next state *1/ */
/* }; */

struct TastyRegex {
	const void *restrict *initial;
	const void *restrict matching;
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


#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_H_ */
