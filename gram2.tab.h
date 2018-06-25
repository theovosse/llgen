/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

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
     stringsym = 274,
     numbersym = 275,
     singletoken = 276,
     starsym = 277,
     equalsym = 278,
     ignoresym = 279,
     lbracketsym = 280,
     rbracketsym = 281
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
#define stringsym 274
#define numbersym 275
#define singletoken 276
#define starsym 277
#define equalsym 278
#define ignoresym 279
#define lbracketsym 280
#define rbracketsym 281




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
/* Line 1529 of yacc.c.  */
#line 109 "gram2.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

