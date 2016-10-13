#include "unity.h"
#include "tasty_regex.h"
#include <unistd.h>

void
setUp(void)
{
}

void
tearDown(void)
{
}

void
test_tasty_regex(void)
{
	struct TastyRegex regex;
	struct TastyMatchInterval matches;


	TEST_ASSERT_EQUAL_INT(0,
			      tasty_regex_compile(&regex,
						  "I (love|(dis)?like) (cat|dog|gopher)s"));
						  /* "I ((dis)?like|hate) (cat|dog|gopher)s")); */
						  /* "I (love|(dis)?like) (cat|dog|gopher)s")); */

	TEST_ASSERT_EQUAL_INT(0,
			      tasty_regex_run(&regex,
					      &matches,
					      "I love cats, and I like dogs, but I dislike gophers"));

	tasty_regex_free(&regex);

	for (struct TastyMatch *restrict match = matches.from;
	     match < matches.until;
	     ++match) {
		TEST_ASSERT_TRUE(write(STDOUT_FILENO,
				       match->from,
				       match->until - match->from) >= 0);

		TEST_ASSERT_TRUE(write(STDOUT_FILENO,
				       "\n",
				       sizeof("\n")) >= 0);
	}

	tasty_match_interval_free(&matches);
}
