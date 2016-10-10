#ifndef TASTY_REGEX_TASTY_REGEX_COMPILE_H_
#define TASTY_REGEX_TASTY_REGEX_COMPILE_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
#	undef restrict
#	define restrict __restrict__ /* use c++ compatible '__restrict__' */
#endif /* ifdef __cplusplus */

/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_regex_globals.h"
#include <stdbool.h>	/* bool */


/* API
 * ────────────────────────────────────────────────────────────────────────── */
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
		    const char *restrict pattern);

/* free allocations */
inline void
tasty_regex_free(struct TastyRegex *const restrict regex)
{
	free((void *) regex->initial);
}

#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_COMPILE_H_ */
