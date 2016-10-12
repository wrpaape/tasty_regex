# tasty_regex

##Overview
`tasty_regex` is a simplified, light-weight regular expression library that guarantees worst-case *O*(*mn*) performance where  
*m* := *length*(`pattern`) and
*n* := *length*(`string`).
UTF8 patterns are supported along with the following operators:  

| Operator | Use     | Description                             |
| :------: | :-----: | :-------------------------------------- |
| `?`      | `X?`    | match expression *X* zero or one time   |
| `*`      | `X*`    | match expression *X* zero or more times |
| `+`      | `X+`    | match expression *X* one or more times  |
| `|`      | `X|Y`   | match expression *X* or *Y*             |
| `.`      | `.`     | match any character                     |
| `()`     | `(xyz)` | declare a matching expression *xyz*     |
| `\`      | `\x`    | escape character *x* in set `?*+|.()\`  |

Matching via `tasty_regex_run` is *greedy* (match as many characters as possible) and *global* (all valid greedy matches are recorded).



##Usage

###tasty_regex_compile

####"Compile"s a regular expression, `regex`, from an input `pattern`

```
int
tasty_regex_compile(struct TastyRegex *const restrict regex,
                    const char *restrict pattern);
```

| Return Value                         | Description                                                                                    |
| :----------------------------------: | :--------------------------------------------------------------------------------------------- |
| `0`                                  | compiled successfully                                                                          |
| `TASTY_ERROR_OUT_OF_MEMORY`          | failed to allocate sufficient memory                                                           |
| `TASTY_ERROR_EMPTY_EXPRESSION`	     | empty `pattern` or subexpression (i.e. `()`, `&#124;&#124;`, `&#124;)`, etc ...)               |
| `TASTY_ERROR_UNBALANCED_PARENTHESES` | unbalanced parentheses (i.e. `((ab)`, `aab)`, etc ...)                                         |
| `TASTY_ERROR_INVALID_ESCAPE`	       | character following `\` is not in set `?*+&#124;.()\`                                          |
| `TASTY_ERROR_NO_OPERAND`		         | no matchable expression preceeding '?', '*', or '+' (i.e. `*abc`, 'b&#124;?b', 'a++', etc ...) |
| `TASTY_ERROR_INVALID_UTF8`	         | `pattern` includes at least 1 invalid (non-UTF8) byte sequence                                 |

**example**  
```
struct TastyRegex good_regex;
struct TastyRegex bad_regex;
int status;

/* should succeed (return 0) */
status = tasty_regex_compile(&good_regex,
                             "(apples)*|oranges");

if (status != 0) {
        /* handle failure */
}

/* should fail (return TASTY_ERROR_UNBALANCED_PARENTHESES) */
status = tasty_regex_compile(&bad_regex,
                             "I forgot to close my (parentheses");

if (status != 0) {
        /* handle failure */
}
```

If `0` is returned, compilation succeeded and `regex`'s internals (see [Implementation](#implementation)) have been allocated onto the heap. To avoid memory leaks, all calls to `tasty_regex_compile` must be paired with `tasty_regex_free`:  



###tasty_regex_free

####Frees dynamically-allocated memory referred to in a `TastyRegex` after a successful call to `tasty_regex_compile`

```
extern inline void
tasty_regex_free(struct TastyRegex *const restrict regex);
```

**example**  
```
struct TastyRegex regex;
int status;

/* should succeed (return 0) */
status = tasty_regex_compile(&regex,
                             "b?oogity");

if (status != 0) {
        /* handle failure */
}

tasty_regex_free(&regex);
```

###tasty_regex_run

####Matches `string` against compiled regular expression, `regex`

```
int
tasty_regex_run(const struct TastyRegex *const restrict regex,
		struct TastyMatchInterval *const restrict matches,
		const char *restrict string);
```

| Return Value                         | Description                          |
| :----------------------------------: | :----------------------------------- |
| `0`                                  | ran successfully (0 or more matches) |
| `TASTY_ERROR_OUT_OF_MEMORY`          | failed to allocate sufficient memory |

**example**  
```
struct TastyRegex regex;
struct TastyMatchInterval matches;
int status;

status = tasty_regex_compile(&regex,
                             "b?oo(gity)?");

if (status != 0) {
        /* handle failure */
}

/* should succeed (return 0) and match accordingly:
 * ├────┤
 *        ├─────┤
 *         ├────┤
 *                ├─┤
 *                 ├┤
 * oogity boogity boo
 */
status = tasty_regex_run(&regex,
                         &matches,
                         "oogity boogity boo");

if (status != 0) {
        /* handle failure */
}
```
If `0` is returned, execution has succeeded and `matches` should refer to a freeable interval of `TastyMatch`s that describe all matching substrings for `regex` against input `string` (see [Traversing TastyMatchInterval](#traversing-TastyMatchInterval)). Accordingly, all calls to `tasty_regex_run` must be paired with `tasty_match_interval_free`:  



###tasty_match_interval_free

####Frees dynamically-allocated memory referred to by a `TastyMatchInterval` after a successful call to `tasty_regex_run`

```
extern inline void
tasty_match_interval_free(struct TastyMatchInterval *const restrict matches);
```
**example**  
```
struct TastyRegex regex;
struct TastyMatchInterval matches;
int status;

/* should succeed (return 0) */
status = tasty_regex_compile(&regex,
                             "I (dis)?like (cat|dog|gopher)s");

if (status != 0) {
        /* handle failure */
}

status = tasty_regex_run(&regex,
                         &matches,
                         "I love cats, and I like dogs, but I hate gophers");

tasty_regex_free(&regex);

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

###Traversing TastyMatchInterval
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
