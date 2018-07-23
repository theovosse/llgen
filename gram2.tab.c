/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     identifiersym = 258,
     colonsym = 259,
     semicolonsym = 260,
     periodsym = 261,
     functionreturnsym = 262,
     shifttokensym = 263,
     commasym = 264,
     outputsym = 265,
     codesym = 266,
     optionsym = 267,
     sequencesym = 268,
     keywordsym = 269,
     subtokensym = 270,
     chainsym = 271,
     lparsym = 272,
     rparsym = 273,
     breaksym = 274,
     errorsym = 275,
     onsym = 276,
     stringsym = 277,
     numbersym = 278,
     singletoken = 279,
     starsym = 280,
     equalsym = 281,
     ignoresym = 282,
     lbracketsym = 283,
     rbracketsym = 284
   };
#endif
/* Tokens.  */
#define identifiersym 258
#define colonsym 259
#define semicolonsym 260
#define periodsym 261
#define functionreturnsym 262
#define shifttokensym 263
#define commasym 264
#define outputsym 265
#define codesym 266
#define optionsym 267
#define sequencesym 268
#define keywordsym 269
#define subtokensym 270
#define chainsym 271
#define lparsym 272
#define rparsym 273
#define breaksym 274
#define errorsym 275
#define onsym 276
#define stringsym 277
#define numbersym 278
#define singletoken 279
#define starsym 280
#define equalsym 281
#define ignoresym 282
#define lbracketsym 283
#define rbracketsym 284




/* Copy the first part of user declarations.  */
#line 1 "gram2.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl3.h"
#include "llgen.h"

Node preCodeBlocks = NULL;
Node postCodeBlocks = NULL;
static Node *codeBlkPtr = &preCodeBlocks;


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 14 "gram2.y"
{
    Node node;
    NodeList nodeList;
    Name name;
    char *string;
	RuleResult ruleresult;
}
/* Line 193 of yacc.c.  */
#line 174 "gram2.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 187 "gram2.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  11
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   117

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  30
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  58
/* YYNRULES -- Number of states.  */
#define YYNSTATES  93

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   284

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    12,    17,    22,    29,
      37,    42,    46,    48,    52,    54,    56,    62,    66,    67,
      70,    71,    75,    76,    78,    82,    83,    85,    87,    91,
      93,    97,   100,   103,   107,   109,   111,   115,   120,   122,
     125,   129,   131,   135,   137,   139,   141,   143,   145,   147,
     149,   151,   153,   155,   157,   159,   163,   167,   168
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      31,     0,    -1,    32,    -1,    31,    32,    -1,    33,    -1,
      36,    -1,    35,     4,    41,     6,    -1,    34,    26,    22,
       6,    -1,    34,    26,    22,    14,    34,     6,    -1,    34,
      26,    34,    17,    34,    18,     6,    -1,    34,    15,    34,
       6,    -1,    27,    22,     6,    -1,     3,    -1,    34,    40,
      38,    -1,    11,    -1,     3,    -1,     7,    37,    37,    26,
      36,    -1,     7,    37,    37,    -1,    -1,     7,    37,    -1,
      -1,    17,    49,    18,    -1,    -1,    42,    -1,    42,     5,
      41,    -1,    -1,    43,    -1,    44,    -1,    44,     9,    43,
      -1,    45,    -1,    45,    16,    46,    -1,    45,    12,    -1,
      45,    13,    -1,    45,    13,    12,    -1,    11,    -1,    10,
      -1,    21,    34,    19,    -1,    21,    34,    20,    22,    -1,
      46,    -1,     8,    46,    -1,    34,    40,    39,    -1,    22,
      -1,    17,    41,    18,    -1,    24,    -1,     3,    -1,     4,
      -1,     5,    -1,     6,    -1,     9,    -1,    10,    -1,    22,
      -1,    23,    -1,    25,    -1,    26,    -1,    47,    -1,    17,
      49,    18,    -1,    28,    49,    29,    -1,    -1,    49,    48,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    39,    39,    39,    42,    46,    51,    52,    53,    54,
      55,    56,    58,    62,    74,    78,    83,    86,    88,    93,
      95,   100,   102,   107,   110,   115,   116,   118,   119,   122,
     125,   129,   132,   135,   139,   142,   145,   149,   155,   158,
     164,   169,   173,   184,   185,   186,   187,   188,   189,   190,
     191,   192,   193,   194,   197,   200,   204,   210,   213
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "identifiersym", "colonsym",
  "semicolonsym", "periodsym", "functionreturnsym", "shifttokensym",
  "commasym", "outputsym", "codesym", "optionsym", "sequencesym",
  "keywordsym", "subtokensym", "chainsym", "lparsym", "rparsym",
  "breaksym", "errorsym", "onsym", "stringsym", "numbersym", "singletoken",
  "starsym", "equalsym", "ignoresym", "lbracketsym", "rbracketsym",
  "$accept", "grammar", "gramelt", "rule", "identifier", "lhs", "codename",
  "identifiername", "functionreturnoption", "functionresultoption",
  "argumentoption", "rhs", "alternative", "elementlist", "element",
  "single_element_shiftoption", "single_element", "non_bracket_symbol",
  "wildcard", "wildcardsequence", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    30,    31,    31,    32,    32,    33,    33,    33,    33,
      33,    33,    34,    35,    36,    37,    38,    38,    38,    39,
      39,    40,    40,    41,    41,    42,    42,    43,    43,    44,
      44,    44,    44,    44,    44,    44,    44,    44,    45,    45,
      46,    46,    46,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    48,    48,    48,    49,    49
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     1,     4,     4,     6,     7,
       4,     3,     1,     3,     1,     1,     5,     3,     0,     2,
       0,     3,     0,     1,     3,     0,     1,     1,     3,     1,
       3,     2,     2,     3,     1,     1,     3,     4,     1,     2,
       3,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     0,     2
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    12,    14,     0,     0,     2,     4,    22,     0,     5,
       0,     1,     3,     0,    57,     0,    18,    25,    11,     0,
       0,     0,     0,     0,    13,     0,    35,    34,    25,     0,
      41,    22,     0,    23,    26,    27,    29,    38,    10,    44,
      45,    46,    47,    48,    49,    57,    21,    50,    51,    43,
      52,    53,    57,    54,    58,     7,     0,     0,    15,     0,
      39,     0,     0,    20,     6,    25,     0,    31,    32,     0,
       0,     0,     0,     0,    17,    42,    36,     0,     0,    40,
      24,    28,    33,    30,    55,    56,     8,     0,     0,    37,
      19,     9,    16
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     4,     5,     6,    31,     8,     9,    59,    24,    79,
      16,    32,    33,    34,    35,    36,    37,    53,    54,    20
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -42
static const yytype_int8 yypact[] =
{
       5,   -42,   -42,   -17,     3,   -42,   -42,    -6,     6,   -42,
       1,   -42,   -42,    16,   -42,    14,    27,    95,   -42,    32,
      45,    19,    36,    55,   -42,     9,   -42,   -42,    95,    16,
     -42,    61,    66,    74,   -42,    73,    48,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,    16,    16,   -42,    55,
     -42,    65,    46,    77,   -42,    95,    95,   -42,    75,     9,
      71,    18,    79,    68,    64,   -42,   -42,    69,    55,   -42,
     -42,   -42,   -42,   -42,   -42,   -42,   -42,    86,    89,   -42,
     -42,   -42,   -42
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -42,   -42,    97,   -42,     0,   -42,    20,   -41,   -42,   -42,
      76,   -26,   -42,    38,   -42,   -42,   -24,   -42,   -42,     7
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
       7,    60,    61,    11,     7,    10,     1,    18,     1,    13,
      17,    14,     1,    19,     2,    22,     2,     1,    74,     1,
      15,    39,    40,    41,    42,    55,    28,    43,    44,    62,
       3,    30,     3,    56,    23,    45,    21,    90,    38,    80,
      47,    48,    49,    50,    51,    83,    52,    85,    39,    40,
      41,    42,    70,    57,    43,    44,    72,    73,    58,    71,
      67,    68,    45,    46,    69,    76,    77,    47,    48,    49,
      50,    51,    64,    52,    39,    40,    41,    42,    14,    65,
      43,    44,    66,    75,    78,    86,    87,    82,    45,    84,
      88,    89,    91,    47,    48,    49,    50,    51,     1,    52,
       2,    12,     0,    25,    81,    26,    27,    63,    92,     0,
       0,     0,    28,     0,     0,     0,    29,    30
};

static const yytype_int8 yycheck[] =
{
       0,    25,    28,     0,     4,    22,     3,     6,     3,    15,
       4,    17,     3,    13,    11,    15,    11,     3,    59,     3,
      26,     3,     4,     5,     6,     6,    17,     9,    10,    29,
      27,    22,    27,    14,     7,    17,    22,    78,     6,    65,
      22,    23,    24,    25,    26,    69,    28,    29,     3,     4,
       5,     6,    45,    17,     9,    10,    56,    57,     3,    52,
      12,    13,    17,    18,    16,    19,    20,    22,    23,    24,
      25,    26,     6,    28,     3,     4,     5,     6,    17,     5,
       9,    10,     9,    18,     7,     6,    18,    12,    17,    18,
      26,    22,     6,    22,    23,    24,    25,    26,     3,    28,
      11,     4,    -1,     8,    66,    10,    11,    31,    88,    -1,
      -1,    -1,    17,    -1,    -1,    -1,    21,    22
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    11,    27,    31,    32,    33,    34,    35,    36,
      22,     0,    32,    15,    17,    26,    40,     4,     6,    34,
      49,    22,    34,     7,    38,     8,    10,    11,    17,    21,
      22,    34,    41,    42,    43,    44,    45,    46,     6,     3,
       4,     5,     6,     9,    10,    17,    18,    22,    23,    24,
      25,    26,    28,    47,    48,     6,    14,    17,     3,    37,
      46,    41,    34,    40,     6,     5,     9,    12,    13,    16,
      49,    49,    34,    34,    37,    18,    19,    20,     7,    39,
      41,    43,    12,    46,    18,    29,     6,    18,    26,    22,
      37,     6,    36
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 4:
#line 42 "gram2.y"
    {
		if (postCodeBlocks == NULL)
			codeBlkPtr = &postCodeBlocks;
	;}
    break;

  case 5:
#line 46 "gram2.y"
    {
		*codeBlkPtr = MakeNode(cLineNumber, single, MakeString(codeBlock), NULL);
		codeBlkPtr = &(*codeBlkPtr)->next;
	;}
    break;

  case 6:
#line 51 "gram2.y"
    { InsertSymbol((yyvsp[(1) - (4)].name)->name, (yyvsp[(3) - (4)].nodeList), true, identifier); ;}
    break;

  case 7:
#line 52 "gram2.y"
    { AssignString((yyvsp[(1) - (4)].name), (yyvsp[(3) - (4)].name)); (yyvsp[(3) - (4)].name)->isTerminal = true; ;}
    break;

  case 8:
#line 53 "gram2.y"
    { KeywordString((yyvsp[(1) - (6)].name), (yyvsp[(3) - (6)].name), (yyvsp[(5) - (6)].name)); ;}
    break;

  case 9:
#line 54 "gram2.y"
    { Function((yyvsp[(1) - (7)].name), (yyvsp[(3) - (7)].name), (yyvsp[(5) - (7)].name)); ;}
    break;

  case 10:
#line 55 "gram2.y"
    { KeywordString((yyvsp[(1) - (4)].name), NULL, (yyvsp[(3) - (4)].name)); ;}
    break;

  case 11:
#line 56 "gram2.y"
    { (yyvsp[(2) - (3)].name)->index = -1; (yyvsp[(2) - (3)].name)->isTerminal = true; ;}
    break;

  case 12:
#line 58 "gram2.y"
    {
		(yyval.name) = InsertSymbol((yyvsp[(1) - (1)].string), NULL, false, identifier);
	;}
    break;

  case 13:
#line 62 "gram2.y"
    {
		(yyval.name) = (yyvsp[(1) - (3)].name);
		(yyvsp[(1) - (3)].name)->lineNr = lineNumber;
		(yyvsp[(1) - (3)].name)->arglist = (yyvsp[(2) - (3)].node);
		(yyval.name)->nrParameters = nrParameters((yyvsp[(2) - (3)].node));
		(yyvsp[(1) - (3)].name)->ruleResult = (yyvsp[(3) - (3)].ruleresult);
		if (startSymbol == NULL) {
			startSymbol = (yyvsp[(1) - (3)].name);
			endOfInput = InsertSymbol("endOfInput", NULL, false, token);
		}
	;}
    break;

  case 14:
#line 74 "gram2.y"
    {
		(yyval.name) = MakeName(codeBlock, -1, output, cLineNumber);
	;}
    break;

  case 15:
#line 78 "gram2.y"
    {
		(yyval.name) = MakeName(yytext, -1, identifier, lineNumber);
	;}
    break;

  case 16:
#line 83 "gram2.y"
    {
		(yyval.ruleresult) = MakeRuleResult((yyvsp[(2) - (5)].name), (yyvsp[(3) - (5)].name), (yyvsp[(5) - (5)].name));
	;}
    break;

  case 17:
#line 86 "gram2.y"
    {
		(yyval.ruleresult) = MakeRuleResult((yyvsp[(2) - (3)].name), (yyvsp[(3) - (3)].name), NULL);
	;}
    break;

  case 18:
#line 88 "gram2.y"
    {
		(yyval.ruleresult) = NULL;
	;}
    break;

  case 19:
#line 93 "gram2.y"
    {
		(yyval.name) = (yyvsp[(2) - (2)].name);
	;}
    break;

  case 20:
#line 95 "gram2.y"
    {
		(yyval.name) = NULL;
	;}
    break;

  case 21:
#line 100 "gram2.y"
    {
		(yyval.node) = (yyvsp[(2) - (3)].node);
	;}
    break;

  case 22:
#line 102 "gram2.y"
    {
		(yyval.node) = NULL;
	;}
    break;

  case 23:
#line 107 "gram2.y"
    {
		(yyval.nodeList) = (yyvsp[(1) - (1)].nodeList);
	;}
    break;

  case 24:
#line 110 "gram2.y"
    {
		(yyvsp[(1) - (3)].nodeList)->rest = (yyvsp[(3) - (3)].nodeList);
		(yyval.nodeList) = (yyvsp[(1) - (3)].nodeList);
	;}
    break;

  case 25:
#line 115 "gram2.y"
    {(yyval.nodeList) = MakeNodeList(NULL, NULL); ;}
    break;

  case 26:
#line 116 "gram2.y"
    {(yyval.nodeList) = MakeNodeList((yyvsp[(1) - (1)].node), NULL); ;}
    break;

  case 27:
#line 118 "gram2.y"
    { (yyval.node) = (yyvsp[(1) - (1)].node); ;}
    break;

  case 28:
#line 119 "gram2.y"
    { (yyvsp[(1) - (3)].node)->next = (yyvsp[(3) - (3)].node); (yyval.node) = (yyvsp[(1) - (3)].node); ;}
    break;

  case 29:
#line 122 "gram2.y"
    {
		(yyval.node) = (yyvsp[(1) - (1)].node);
	;}
    break;

  case 30:
#line 125 "gram2.y"
    {
		(yyval.node) = MakeNode(lineNumber, chain, MakeNode(lineNumber, single, (yyvsp[(1) - (3)].node),
						MakeNode(lineNumber, single, (yyvsp[(3) - (3)].node), NULL)), NULL);
	;}
    break;

  case 31:
#line 129 "gram2.y"
    {
		(yyval.node) = MakeNode(lineNumber, option, (yyvsp[(1) - (2)].node), NULL);
	;}
    break;

  case 32:
#line 132 "gram2.y"
    {
		(yyval.node) = MakeNode(lineNumber, sequence, (yyvsp[(1) - (2)].node), NULL);
	;}
    break;

  case 33:
#line 135 "gram2.y"
    {
		(yyval.node) = MakeNode(lineNumber, option, MakeNode(lineNumber, sequence, (yyvsp[(1) - (3)].node), NULL), NULL);
		(yyval.node)->element.sub->preferShift = (yyvsp[(1) - (3)].node)->preferShift;
	;}
    break;

  case 34:
#line 139 "gram2.y"
    {
		(yyval.node) = MakeNode(cLineNumber, single, MakeName(codeBlock, -1, code, cLineNumber), NULL);
	;}
    break;

  case 35:
#line 142 "gram2.y"
    {
		(yyval.node) = MakeNode(cLineNumber, single, MakeName(codeBlock, -1, output, cLineNumber), NULL);
	;}
    break;

  case 36:
#line 145 "gram2.y"
    {
		(yyvsp[(2) - (3)].name)->isTerminal = true;
		(yyval.node) = MakeNode(lineNumber, breakToken, (yyvsp[(2) - (3)].name), NULL);
	;}
    break;

  case 37:
#line 149 "gram2.y"
    {
		(yyvsp[(2) - (4)].name)->isTerminal = true;
		(yyval.node) = MakeNode(lineNumber, errorMessage, MakeNode(lineNumber, single, (yyvsp[(2) - (4)].name), MakeNode(lineNumber, single, (yyvsp[(4) - (4)].name), NULL)), NULL);
	;}
    break;

  case 38:
#line 155 "gram2.y"
    {
		(yyval.node) = (yyvsp[(1) - (1)].node);
	;}
    break;

  case 39:
#line 158 "gram2.y"
    {
		(yyval.node) = (yyvsp[(2) - (2)].node);
		(yyval.node)->preferShift = true;
	;}
    break;

  case 40:
#line 164 "gram2.y"
    {
		(yyval.node) = MakeNode(lineNumber, single, (yyvsp[(1) - (3)].name), NULL);
		(yyval.node)->arglist = (yyvsp[(2) - (3)].node);
		(yyval.node)->assignTo = (yyvsp[(3) - (3)].name);
	;}
    break;

  case 41:
#line 169 "gram2.y"
    {
		(yyvsp[(1) - (1)].name)->isTerminal = true;
		(yyval.node) = MakeNode(lineNumber, single, (yyvsp[(1) - (1)].name), NULL);
	;}
    break;

  case 42:
#line 173 "gram2.y"
    {
		(yyval.node) = MakeNode(lineNumber, alternative, (yyvsp[(2) - (3)].nodeList), NULL);
		if (!containsGrammarSymbol((yyvsp[(2) - (3)].nodeList))) {
			/* Allowing only output here would allow writing something like
			   ( { code }) OPTION, which is meaningless.
			 */
			yyerror("cannot group output symbols");
		}
	;}
    break;

  case 43:
#line 184 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 44:
#line 185 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 45:
#line 186 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 46:
#line 187 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 47:
#line 188 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 48:
#line 189 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 49:
#line 190 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 50:
#line 191 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 51:
#line 192 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 52:
#line 193 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 53:
#line 194 "gram2.y"
    { (yyval.node) = MakeNode(nrBlanks, single, MakeString(yytext), NULL); ;}
    break;

  case 54:
#line 197 "gram2.y"
    {
		(yyval.node) = (yyvsp[(1) - (1)].node);
	;}
    break;

  case 55:
#line 200 "gram2.y"
    {
		(yyval.node) = AppendNode(MakeNode(0, single, MakeString("("), (yyvsp[(2) - (3)].node)),
						MakeNode(0, single, MakeString(")"), NULL));
	;}
    break;

  case 56:
#line 204 "gram2.y"
    {
		(yyval.node) = AppendNode(MakeNode(0, single, MakeString("["), (yyvsp[(2) - (3)].node)),
						MakeNode(0, single, MakeString("]"), NULL));
	;}
    break;

  case 57:
#line 210 "gram2.y"
    {
		(yyval.node) = NULL;
	;}
    break;

  case 58:
#line 213 "gram2.y"
    {
		(yyval.node) = AppendNode((yyvsp[(1) - (2)].node), (yyvsp[(2) - (2)].node));
	;}
    break;


/* Line 1267 of yacc.c.  */
#line 1838 "gram2.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



