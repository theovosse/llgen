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
  Initialization is optional. The call would have to look like this:

    `rule: ... expr(some_context) -> expr_result ...`

  In the code following that call, `expr_result` will be available for use.

- Comment must be placed between `/*` and `*/`.

- If nothing is specified, whitespace is significant like any character. To ignore a string, add

  `IGNORE "regexp".`

  to the grammar. This can not only be used to ignore whitespace, but also
  comment symbols, e.g.

    `IGNORE "[ \t\r\n]+".`

    `IGNORE "//.*".`
  
  Note that these strings are ignored, but do separate tokens.

- There is a primitive error handling mechanism: `ON <terminal>`. If the terminal is encountered at that point in a rule, you can perform one of these actions:

    `ON <terminal> BREAK`, which breaks out of a sequence or chain.

    `ON <terminal> ERROR <string>` which prints the string as an error message via llerror (so it shows at the current token and should be seen as an error by the environment).

Note that the terminal is not consumed.

# Usage

Compile a grammar using

    llgen [options] gram.ll [outputfile.ext]

The options are binary: `+a` adds an option, `-a` removes it. The target language is an option, of which you should specify only one: `+c`, `+cpp`, `+js`, `+ts`, or `+go`. C is the default. The output file by default has the same name as the input file, but with the extension for the target language.

Other options are `+/-llerror` to include or exclude a default error message function, `+/-main` to include a simple main() function (or not), and some to set the type of input (`stringinput`, `noinput`) which select between reading from a file (the default), from a string, or using an external function.

### Example

The following command generates a typescript file that does not contain main, nor the default llerror from cdl.ll:

    llgen -llerror -main +ts cdl.ll

# Skeleton files

The different languages are generated via dedicated functions in lang.c and skeleton files, which provide support for scannning and set operations. These files have an #if(n)def-like mechanism using the options given in the command line: %{A ... %}A will copy the text only when option A is set, and %{!A ... %}!A only when option A isn't set. The skeleton files also handle options set during grammar processing, such as keywords and functions.

# Unicode

The C/C++ target also has the option for unicode input, but that doesn't work very well.

# IDE support

There's also extensive support for editing in VSCode; there's a vsix file in the client folder.

# About the code

LLGen is written entirely in C, because that was the only compiler I had when I started it. Although rewriting in C++ could make some parts more readable, I decided it's not worth it, as there was nothing to be gained from using inheritance, until I added support for multiple languages. Using templates will make some of the code a bit easier, but it's never been worth a complete rewrite.

So:
- rules and are represented as a tree of nodes
- symbol sets are AVL trees (sorting/comparison is passed along as a function)

The regexp code is even older, and uses the classic regexps -> NFA -> DFA algorithm. It even has an option to generate 68k machine code from the DFA.

# Skeleton file properties

- llerror: by default on; provides an error reporting mechanism.

- main: by default off; makes the output stand-alone.

- ignorebuffer: by default off; stores all ignored tokens before the current; only implemented for ts/js.

- keepignorebuffer: off by default; when set, does not clear ignoreBuffer on every token, but requires the grammar to do so.