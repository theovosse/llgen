%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avl3.h"
#include "llgen.h"

Node preCodeBlocks = NULL;
Node postCodeBlocks = NULL;
static Node *codeBlkPtr = &preCodeBlocks;
%}

%union
{
    Node node;
    NodeList nodeList;
    Name name;
    char *string;
	RuleResult ruleresult;
}

%token <string> identifiersym
%token colonsym semicolonsym periodsym functionreturnsym shifttokensym
%token commasym outputsym codesym optionsym sequencesym keywordsym subtokensym
%token chainsym lparsym rparsym
%token <name> stringsym
%token numbersym singletoken starsym equalsym ignoresym lbracketsym rbracketsym

%type <nodeList> alternative rhs
%type <node> elementlist element single_element single_element_shiftoption
%type <name> lhs identifier codename identifiername functionresultoption
%type <node> argumentoption wildcardsequence wildcard non_bracket_symbol
%type <ruleresult> functionreturnoption

%start grammar

%%

grammar: gramelt | grammar gramelt;

gramelt:
	rule {
		if (postCodeBlocks == NULL)
			codeBlkPtr = &postCodeBlocks;
	} |
	codename {
		*codeBlkPtr = MakeNode(cLineNumber, single, MakeString(codeBlock), NULL);
		codeBlkPtr = &(*codeBlkPtr)->next;
	};

rule: lhs colonsym rhs periodsym { InsertSymbol($1->name, $3, true, identifier); } |
      identifier equalsym stringsym periodsym { AssignString($1, $3); } |
      identifier equalsym stringsym keywordsym identifier periodsym { KeywordString($1, $3, $5); } |
      identifier equalsym identifier lparsym identifier rparsym periodsym { Function($1, $3, $5); } |
      identifier subtokensym identifier periodsym { KeywordString($1, NULL, $3); } |
      ignoresym stringsym periodsym { $2->index = -1; } ;

identifier: identifiersym {
		$$ = InsertSymbol($1, NULL, false, identifier);
	};

lhs: identifier argumentoption functionreturnoption {
		$$ = $1;
		$1->lineNr = lineNumber;
		$1->arglist = $2;
		$$->nrParameters = nrParameters($2);
		$1->ruleResult = $3;
		if (startSymbol == NULL) {
			startSymbol = $1;
			endOfInput = InsertSymbol("endOfInput", NULL, false, token);
		}
	};

codename: codesym {
		$$ = MakeName(codeBlock, -1, output, cLineNumber);
	} ;

identifiername: identifiersym {
		$$ = MakeName(yytext, -1, identifier, lineNumber);
	} ;

functionreturnoption:
	functionreturnsym identifiername identifiername equalsym codename {
		$$ = MakeRuleResult($2, $3, $5);
	} | {
		$$ = NULL;
	};

functionresultoption:
	functionreturnsym identifiername {
		$$ = $2;
	} | {
		$$ = NULL;
	};

argumentoption:
	lparsym wildcardsequence rparsym {
		$$ = $2;
	} | {
		$$ = NULL;
	} ;

rhs:
	alternative {
		$$ = $1;
	} |
	alternative semicolonsym rhs {
		$1->rest = $3;
		$$ = $1;
	};

alternative: {$$ = MakeNodeList(NULL, NULL); }
	    | elementlist {$$ = MakeNodeList($1, NULL); };

elementlist: element { $$ = $1; }
	   | element commasym elementlist { $1->next = $3; $$ = $1; }

element:
	single_element_shiftoption {
		$$ = $1;
	} |
	single_element_shiftoption chainsym single_element {
		$$ = MakeNode(lineNumber, chain, MakeNode(lineNumber, single, $1,
						MakeNode(lineNumber, single, $3, NULL)), NULL);
	} |
	single_element_shiftoption optionsym {
		$$ = MakeNode(lineNumber, option, $1, NULL);
	} |
	single_element_shiftoption sequencesym {
		$$ = MakeNode(lineNumber, sequence, $1, NULL);
	} |
	single_element_shiftoption sequencesym optionsym {
		$$ = MakeNode(lineNumber, option, MakeNode(lineNumber, sequence, $1, NULL), NULL);
		$$->element.sub->preferShift = $1->preferShift;
	} |
	codesym {
		$$ = MakeNode(cLineNumber, single, MakeName(codeBlock, -1, code, cLineNumber), NULL);
	} |
	outputsym {
		$$ = MakeNode(cLineNumber, single, MakeName(codeBlock, -1, output, cLineNumber), NULL);
	} ;

single_element_shiftoption:
	single_element {
		$$ = $1;
	} |
	shifttokensym single_element {
		$$ = $2;
		$$->preferShift = true;
	} ;

single_element:
	identifier argumentoption functionresultoption {
		$$ = MakeNode(lineNumber, single, $1, NULL);
		$$->arglist = $2;
		$$->assignTo = $3;
	} |
	stringsym {
		$$ = MakeNode(lineNumber, single, $1, NULL);
	} |
	lparsym rhs rparsym {
		$$ = MakeNode(lineNumber, alternative, $2, NULL);
		if (!containsGrammarSymbol($2)) {
			/* Allowing only output here would allow writing something like
			   ( { code }) OPTION, which is meaningless.
			 */
			yyerror("cannot group output symbols");
		}
	} ;

non_bracket_symbol:
	singletoken { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	identifiersym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	colonsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	semicolonsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	periodsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	commasym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	outputsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	stringsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	numbersym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	starsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } |
	equalsym { $$ = MakeNode(nrBlanks, single, MakeString(yytext), NULL); } ;

wildcard:
	non_bracket_symbol {
		$$ = $1;
	} |
	lparsym wildcardsequence rparsym {
		$$ = AppendNode(MakeNode(0, single, MakeString("("), $2),
						MakeNode(0, single, MakeString(")"), NULL));
	} |
	lbracketsym wildcardsequence rbracketsym {
		$$ = AppendNode(MakeNode(0, single, MakeString("["), $2),
						MakeNode(0, single, MakeString("]"), NULL));
	} ;

wildcardsequence:
	{
		$$ = NULL;
	} |
	wildcardsequence wildcard {
		$$ = AppendNode($1, $2);
	} ;
