# Introduction

LLGen is an LL(1) parser and scanner generator for grammars in a EBNF-like format (the difference being that the operators are words rather than symbols). It produces a recursive descent scanner/parser code for multiple languages: C, C++, JavaScript, TypeScript and Go. Code added to the grammar rules will be executed at exactly the indicated point, so you can even execute code before a symbol has been recognized, provided the grammar is LL(1), of course.

# Building

The code is POSIX compliant, I hope, so run `make llgen` should do it, but it's only been tested on a few platforms.

# Grammar format

- symbols are defined as regular expressions

    `identifier = "[a-zA-Z_][a-zA-Z_0-9]*".`

  It is possible (and necessary) to define symbols as special cases of
  other symbols, e.g. for keywords, or as functions that the file provides, which allows dynamic tokenizing.

- a rule is a non-terminal, colon, production rule, period.

    `S: A; B; C.`

- alternatives are separated by semicolons

- subsequent symbols are separated by commas

    `A: X, Y, Z.`

- an optional symbol is followed by the word `OPTION`, a symbols that can be repeated one or more times by the word `SEQUENCE` (so `SEQUENCE OPTION` means 0 or more times), two alternating symbols are combined by `CHAIN`: `a CHAIN b` recognizes: a, a b a, a b a b a, etc.

- any symbol can be replaced by an alternative or a sequence between parentheses, e.g. `(a; (b; c) SEQUENCE)`

- code can be inserted at any point between braces, e.g.

    expr: number, { value = atoi(lastSymbol); }.

  Code can also be added before the grammar, or after, and will simply be copied at that point.

- non-terminals can have parameters and return values, e.g.

    `expr(context any) -> result string = { "" }:`

  would receive a parameter `context` when called (provided by the grammar author) and return a value in the local variable `result` which would be initialized with "".

# Usage

Compile a grammar using

    llgen [options] gram.ll

The options are binary: `+a` adds an option, `-a` removes it. The target language is an option, of which you should specify only one: `+c`, `+cpp`, `+js`, `+ts`, or `+go`. C is the default.

Other options are `+/-llerror` to include or exclude a default error message function, `+/-main` to include a simple main() function (or not), and some to set the type of input (`stringinput`, `noinput`) which select between reading from a file (the default), from a string, or using an external function.

# Skeleton files

The different languages are generated via dedicated functions in lang.c and skeleton files, which provide support for scannning and set operations. These files have an #if(n)def-like mechanism using the options given in the command line: %{A ... %}A will copy the text only when option A is set, and %{!A ... %}!A only when option A isn't set. The skeleton files also handle options set during grammar processing, such as keywords and functions.

# Unicode

The C/C++ target also has the option for unicode input, but that doesn't work very well.
