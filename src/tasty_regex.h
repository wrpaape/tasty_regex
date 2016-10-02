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

struct TastyRegex {
	struct TastyRegex *step[UCHAR_MAX]; /* jump forward for this state */
	struct TastyRegex *skip; /* shortcut forward if no match is needed */
};

struct TastyState {
	const struct TastyRegex *live;	/* currently matching regex */
	struct TastyState *next;	/* next parallel matching state */
	const unsigned char *match;	/* beginning of string match */
};


/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const unsigned char *restrict pattern);

void
tasty_regex_run(struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const unsigned char *restrict string);

inline void
tasty_regex_free(struct TastyRegex *const restrict regex)
{
	free(regex);
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
