#ifndef TASTY_REGEX_TASTY_REGEX_UTILS_H_
#define TASTY_REGEX_TASTY_REGEX_UTILS_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
#	undef restrict
#	define restrict __restrict__ /* use c++ compatible '__restrict__' */
#endif /* ifdef __cplusplus */


/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include <stdlib.h>	/* size_t */


/* helper macros
 * ────────────────────────────────────────────────────────────────────────── */
#define LIKELY(BOOL)   __builtin_expect(BOOL, 1)
#define UNLIKELY(BOOL) __builtin_expect(BOOL, 0)


/* know string is at least 1 char long */
inline size_t
nonempty_string_length(const char *const restrict string)
{
	register const char *restrict until = string;

	do {
		++until;
	} while (*until != '\0');

	return until - string;
}

#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_UTILS_H_ */
