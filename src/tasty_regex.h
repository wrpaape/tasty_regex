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
	unsigned char *restrict from;
	const unsigned char *restrict until;
};

struct TastyMatchInterval {
	struct TastyMatch *restrict from;
	const struct TastyMatch *restrict until;
};

struct TastyState {
	struct TastyState *next_step[UCHAR_MAX]; /* jump forward for this state */
	struct TastyState *next_peer;		 /* next parallel matching state */
	unsigned char *match_from;
};


struct TastyRegex {
	struct TastyMatchInterval matches;
	struct TastyState *initial;
	struct TastyState *current;
};


typedef struct _TastyStateJumpBuffer {
	struct TastyState *_ptrs[UCHAR_MAX];
} TastyStateJumpBuffer;



#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_H_ */
