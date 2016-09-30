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



#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_H_ */
