#include "unity.h"
#include "tasty_regex.h"
#include <unistd.h>

void setUp(void)
{
}

void tearDown(void)
{
}

void test_tasty_regex(void)
{
	struct TastyRegex regex;
	struct TastyMatchInterval matches;


	TEST_ASSERT_EQUAL_INT(0,
			      tasty_regex_compile(&regex,
						  "(ooga|boga) +boo"));

	TEST_ASSERT_EQUAL_INT(0,
			      tasty_regex_run(&regex,
					      &matches,
					      "boogity oogaoogaooga boo boga boo ooga"));

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


	tasty_regex_free(&regex);
	tasty_match_interval_free(&matches);
}
