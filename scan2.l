%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __MWERKS__
#include <unix.h>
#endif
#include "avl3.h"
#include "llgen.h"
#include "gram2.tab.h"

static int clevel;
int lineNumber = 1;
int cLineNumber;
char codeBlock[8192];
static char *cptr;
static Boolean commentInCode = false;
int nrBlanks;

static struct KeyWord
{
	char *keyword;
	int index;
} keywordStrings[] =
{
	{"CHAIN", chainsym},
	{"IGNORE", ignoresym},
	{"KEYWORD", keywordsym},
	{"OPTION", optionsym},
	{"SEQUENCE", sequencesym},
	{"SHIFT", shifttokensym},
	{"SUBTOKEN", subtokensym},
};

static int KeywordCmp(const void *k1, const void *k2)
{
	return (strcmp((char *) k1, ((struct KeyWord *) k2)->keyword));
}

static int KeywordOrIdentifier(char *str)
{
	struct KeyWord *keyword = bsearch(str, keywordStrings, sizeof(keywordStrings) / sizeof(keywordStrings[0]), sizeof(keywordStrings[0]), KeywordCmp);

	return (keyword == NULL? identifiersym: keyword->index);
}

extern int yylexlocal(void);

int yylex(void)
{
	nrBlanks = 0;
	return (yylexlocal());
}

#define yylex yylexlocal

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) if ((result = fread(buf, 1, max_size, yyin)) < 0 ) YY_FATAL_ERROR("fread() in flex scanner failed");

%}

%x comment code argument

%%
<comment>\*\/			{ BEGIN INITIAL; }
<comment>.				;
<comment>\n				lineNumber++;
<comment><<EOF>>		{ yyerror("unterminated comment"); BEGIN INITIAL; return (0); }
<code>\{                {
						    if (!commentInCode)
						    {
								clevel++;
						    }
						    *cptr++ = yytext[0];
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code>\}                {
						    if (!commentInCode)
						    {
								clevel--;
						    }
						    if (clevel == 0)
						    {
								*cptr = '\0';
								BEGIN INITIAL;
								return (codesym);
						    }
						    else
						    {
								*cptr++ = yytext[0];
						    }
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code>.                 {
						    *cptr++ = yytext[0];
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code>\\[{}]            {
						    *cptr++ = yytext[1];
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code>\n                {
						    lineNumber++;
						    *cptr++ = yytext[0];
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code>\/\*              {
						    commentInCode = true;
						    *cptr++ = yytext[0]; *cptr++ = yytext[1];
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code>\*\/              {
						    commentInCode = false;
						    *cptr++ = yytext[0]; *cptr++ = yytext[1];
						    if (cptr - codeBlock >= 8000)
						    {
								*cptr = '\0';
								cptr = codeBlock;
								return (codesym);
						    }
						}
<code><<EOF>>           { yyerror("unterminated code"); BEGIN INITIAL; return (codesym); }
<INITIAL>\{             { clevel = 1; cptr = codeBlock; cLineNumber = lineNumber; BEGIN code; }
<INITIAL>\/\*           { BEGIN comment; }
<INITIAL>\*\/           { yyerror("unopened comment"); }
<INITIAL>\}             { yyerror("unopened code block"); }
<INITIAL>\(             { return (lparsym); }
<INITIAL>\)             { return (rparsym); }
<INITIAL>\[				{ return (lbracketsym); }
<INITIAL>\]				{ return (rbracketsym); }
<INITIAL>[a-zA-Z_][a-zA-Z0-9_]* {
							int kVal = KeywordOrIdentifier(yytext);
							if (kVal == identifiersym) yylval.string = strcpy(codeBlock, yytext);
							return (kVal);
						}
<INITIAL>((([0-9]+(\.[0-9]*)?)|(\.[0-9]+))([eE][+\-]?[0-9]+)?)|(0[Xx][0-9a-fA-F]+) { return (numbersym); }
<INITIAL>\"([^"\n]|\\\")*\" { yylval.name = InsertString(yytext); return (stringsym); }
<INITIAL>\'([^'\n]|\\\')*\' { strcpy(codeBlock, yytext); return (outputsym); }
<INITIAL>\-\>           { return (functionreturnsym); }
<INITIAL>\=             { return (equalsym); }
<INITIAL>\:             { return (colonsym); }
<INITIAL>\;             { return (semicolonsym); }
<INITIAL>\.             { return (periodsym); }
<INITIAL>\,             { return (commasym); }
<INITIAL>\*             { return (starsym); }
<INITIAL>^#.*			{ if (yytext[1] == '!') RetractProperty(yytext + 2); else AddProperty(yytext + 1); }
<INITIAL>[+\-/%&|\^\<\>~!?]|-\>|\+\+|--|\<\<|\>\>|&&|\|\||\.\.\.|((\<\<|\>\>|[\<\>!=%^&*\-+|/])=) { return (singletoken); }
<INITIAL>[ \t]          { nrBlanks++; }
<INITIAL>\n             { lineNumber++; nrBlanks++; }
.                       { yyerror("unknown character: %c", yytext[0]); }
