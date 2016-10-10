#ifndef TASTY_REGEX_TASTY_REGEX_RUN_H_
#define TASTY_REGEX_TASTY_REGEX_RUN_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
#	undef restrict
#	define restrict __restrict__ /* use c++ compatible '__restrict__' */
#endif /* ifdef __cplusplus */


/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_regex_globals.h" /* TastyState|Regex, m|calloc/free, ERROR* */


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


/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_run(const struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const char *restrict string);

/* free allocations */
inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches)
{
	free((void *) matches->from);
}

#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_RUN_H_ */
