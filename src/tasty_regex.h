#ifndef TASTY_REGEX_TASTY_REGEX_H_
#define TASTY_REGEX_TASTY_REGEX_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
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
#define TASTY_ERROR_OUT_OF_MEMORY	   1 /* failed to allocate memory */
#define TASTY_ERROR_EMPTY_PATTERN	   2 /* 'pattern' is "" */
#define TASTY_ERROR_UNBALANCED_PARENTHESES 3 /* parentheses not balanced */
#define TASTY_ERROR_INVAILD_ESCAPE	   4 /* preceded unescapable char w \ */


/* typedefs, struct declarations
 * ────────────────────────────────────────────────────────────────────────── */
/* defines an match interval on a string: from ≤ token < until */
struct TastyMatch {
	const unsigned char *restrict from;
	const unsigned char *restrict until;
};

/* defines an array of matches: from ≤ match < until */
struct TastyMatchInterval {
	struct TastyMatch *restrict from;
	struct TastyMatch *restrict until;
};

/* DFA state node, step['\0'] reserved for 'skip' field (no explicit match) */
union TastyState {
	const union TastyState *step[UCHAR_MAX + 1]; /* jump tbl for non-'\0' */
	const union TastyState *skip;		     /* no match option */
};

/* complete DFA */
struct TastyRegex {
	const union TastyState *restrict initial;
	const union TastyState *restrict matching;
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

#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_H_ */
