#ifndef TASTY_REGEX_TASTY_REGEX_H_
#define TASTY_REGEX_TASTY_REGEX_H_

/* system check
 * ────────────────────────────────────────────────────────────────────────── */
#include <limits.h> /* CHAR_BIT */
#if (CHAR_BIT != 8)
#	error "system 'CHAR_BIT' not supported (char is not 8 bits)"
#endif /* if (CHAR_BIT != 8) */


/* external dependencies
 * ────────────────────────────────────────────────────────────────────────── */
#include "tasty_regex_compile.h"
#include "tasty_regex_run.h"

#endif /* ifndef TASTY_REGEX_TASTY_REGEX_H_ */
