# tasty_regex

##Overview
`tasty_regex` is a simplified, light-weight regular expression library that guarantees worst-case *O*(*mn*) performance where *m* := *length*(`pattern`) and *n* := *length*(`string`). UTF8 patterns are supported along with the following operators:

| Operator | Use     | Description                             |
| :------: | :-----: | :-------------------------------------- |
| `?`      | `X?`    | match expression *X* zero or one time   |
| `*`      | `X*`    | match expression *X* zero or more times |
| `+`      | `X+`    | match expression *X* one or more times  |
| `|`      | `X|Y`   | match expression *X* or *Y*             |
| `.`      | `.`     | match any character                     |
| `()`     | `(xyz)` | declare a matching expression *xyz*     |
| `\`      | `\x`    | escape character *x* in set `?*+|.()\`  |

Matching via `tasty_regex_run` is *greedy* (match as many characters as possible) and *global* (all valid matches are recorded).

##Usage

###tasty_regex_compile

####"Compile"s a regular expression, `regex`, from an input `pattern`
```
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
                    const char *restrict pattern);

```
| Return Value                         | Description                                                                               |
| :----------------------------------: | :---------------------------------------------------------------------------------------- |
| `0`                                  | compiled successfully                                                                     |
| `TASTY_ERROR_OUT_OF_MEMORY`          | failed to allocate sufficient memory                                                      |
| `TASTY_ERROR_EMPTY_EXPRESSION`	     | empty `pattern` or subexpression (i.e. `()`, `||`, '|)', etc ...)                         |
| `TASTY_ERROR_UNBALANCED_PARENTHESES` | unbalanced parentheses (i.e. `((ab)`, `aab)`, etc ...)                                    |
| `TASTY_ERROR_INVALID_ESCAPE`	       | character following `\` is not in set `?*+|.()\`                                          |
| `TASTY_ERROR_NO_OPERAND`		         | no matchable expression preceeding '?', '*', or '+' (i.e. `*abc`, 'b|?b', 'a++', etc ...) |
| `TASTY_ERROR_INVALID_UTF8`	         | `pattern` includes at least 1 invalid (non-UTF8) byte sequence                            |

**example**  
```
struct TastyRegex good_regex;
struct TastyRegex bad_regex;
int status;

status = tasty_regex_compile(&good_regex,
                             "(apples)*|oranges"); /* should succeed (return 0) */

if (status != 0) {
        /* handle failure */
}

status = tasty_regex_compile(&bad_regex,
                             "I forgot to close my (parentheses"); /* should fail (return TASTY_ERROR_UNBALANCED_PARENTHESES) */

if (status != 0) {
        /* handle failure */
}
```

If `0` is returned, compilation succeeded and `regex`'s internals (see [Implementation](#implementation)) have been allocated onto the heap. To avoid memory leaks, all calls to `tasty_regex_compile` must be paired with `tasty_regex_free`:  


###tasty_regex_free

####Frees dynamically-allocated memory after a successful call to `tasty_regex_compile`
```
extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);
```
**example**  
```
struct TastyRegex regex;
int status;

status = tasty_regex_compile(&regex,
                             "b?oogity");

if (status != 0) {
        /* handle failure */
}

/* pattern matching stuff */

tasty_regex_free(&regex);
```


```
int
tasty_regex_run(const struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const char *restrict string);

extern inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches);
```

```
/* defines an match interval on a string: from ≤ token < until */
struct TastyMatch {
        const char *restrict from;
        const char *restrict until;
};

/* defines an array of matches: from ≤ match < until */
struct TastyMatchInterval {
        struct TastyMatch *restrict from;
        struct TastyMatch *restrict until;
};
```

##Build



##Implementation
Under the hood `tasty_regex` "compile"s an input `pattern` into a deterministic finite automaton (DFA), a graph data structure that can be traversed alongside an input `string` to efficiently locate matches:

```
/* state node */
union TastyState {
        union TastyState *step[UCHAR_MAX + 1]; /* jump tbl for non-'\0' */
        union TastyState *skip;                /* no match option */
};

/* complete DFA */
struct TastyRegex {
        const union TastyState *restrict initial;
        const union TastyState *restrict matching;
};
```

`TastyState` nodes are linked according the `pattern` specification in a roughly linear fashion. For example, the `pattern` "ab?c*(de|dfg)+." would compile to the DFA:
```
                     initial
                        │
                        ↓
                  ( TastyState )
                        │
                   [match 'a']
                        │
                        ↓
                  ( TastyState )
                    │        │
                 [skip] [match 'b']
                    │        │
                    ↓        ↓
                  ( TastyState )←────┐
                    │        │       │
                 [skip] [match 'c']  │
                    │        └───────┘
                    ↓
                  ( TastyState )
                        |
                   [match 'd']
                        |
                        ↓
                  ( TastyState )←──────┐
                    │        │         │
              [match 'e'] [match 'f']  │
                    │        │         │
                    │        ↓         │
                    │  ( TastyState )  │
                    │        │         │
                    │   [match 'g']    │
                    │        │         │
                    ↓        ↓         │
                  ( TastyState )       │
                    │        │         │
                 [skip] [match 'd']    │
                    │        └─────────┘
                    ↓
                  ( TastyState )
        ┌───────────┘   │    └────────────────┐
[match '\0 + 1'] [match '\0 + 2'] ... [match 'UCHAR_MAX']
        │               │                     │
        ↓               ↓                     ↓
     matching        matching              matching
```
where links labeled  `[match 'CHAR']` represent explicit character matches and `[skip]` links represent a valid non-matching path. A list of accumulating matches is updated while an input `string` is traversed one character at a time (without backtracking).
A `TastyMatch` is populated and added to the `TastyMatchInterval` when an accumulating match has traversed the entirety of the compiled DFA.

While this implementation PCRE *O*(*mn*)
