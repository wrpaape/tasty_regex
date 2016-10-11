#ifndef TASTY_REGEX_TASTY_REGEX_GLOBALS_H_
#define TASTY_REGEX_TASTY_REGEX_GLOBALS_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
#	undef restrict
#	define restrict __restrict__ /* use c++ compatible '__restrict__' */
#endif /* ifdef __cplusplus */


/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include <stdlib.h>	/* size_t, m|calloc, free */
#include <limits.h>	/* UCHAR_MAX, CHAR_BIT */


/* /1* system check */
/*  * ────────────────────────────────────────────────────────────────────────── *1/ */
/* #if (CHAR_BIT != 8) */
/* #	error "system 'CHAR_BIT' not supported (char is not 8 bits)" */
/* #endif /1* if (CHAR_BIT != 8) *1/ */


/* error macros
 * ────────────────────────────────────────────────────────────────────────── */
#define TASTY_ERROR_OUT_OF_MEMORY	   1 /* failed to allocate memory */
#define TASTY_ERROR_EMPTY_EXPRESSION	   2 /* pattern or subexpr is "" */
#define TASTY_ERROR_UNBALANCED_PARENTHESES 3 /* parentheses not balanced */
#define TASTY_ERROR_INVALID_ESCAPE	   4 /* \[unescapeable char] */
#define TASTY_ERROR_NO_OPERAND		   5 /* [*+?] preceeded by nothing */
#define TASTY_ERROR_INVALID_UTF8	   6 /* invalid UTF8 code point */


/* typedefs, struct declarations
 * ────────────────────────────────────────────────────────────────────────── */
/* DFA state node, step['\0'] reserved for 'skip' field (no explicit match) */
union TastyState {
	union TastyState *step[UCHAR_MAX + 1]; /* jump tbl for non-'\0' */
	union TastyState *skip;		     /* no match option */
};

/* complete DFA */
struct TastyRegex {
	const union TastyState *restrict initial;
	const union TastyState *restrict matching;
};

#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_GLOBALS_H_ */
