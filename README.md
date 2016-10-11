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
                           skip  [match 'b']
                             │        │
                             ↓        ↓
                           ( TastyState )←────┐
                             │        │       │
                           skip  [match 'c']  │
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
                           skip  [match 'd']    │
                             │        └─────────┘
                             ↓
                           ( TastyState )
                             │   │    │
                 ┌───────────┘   │    └────────────────┐
         [match '\0 + 1'] [match '\0 + 2'] ... [match 'UCHAR_MAX']
                 │               │                     │
                 ↓               ↓                     ↓
              matching        matching              matching
```

While this implementation PCRE *O*(*mn*)
