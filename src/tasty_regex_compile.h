#ifndef TASTY_REGEX_TASTY_REGEX_COMPILE_H_
#define TASTY_REGEX_TASTY_REGEX_COMPILE_H_
#ifdef __cplusplus /* ensure C linkage */
extern "C" {
#	undef restrict
#	define restrict __restrict__ /* use c++ compatible '__restrict__' */
#endif /* ifdef __cplusplus */

/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_globals.h"
#include <stdbool.h>	/* bool */

#ifdef __cplusplus /* close 'extern "C" {' */
}
#endif /* ifdef __cplusplus */
#endif /* ifndef TASTY_REGEX_TASTY_REGEX_COMPILE_H_ */
