#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef unix
#define DIR_SEP_CHAR '/'
#endif
#ifdef windows
#define DIR_SEP_CHAR '\\'
#endif
#ifdef __MACH__
#define DIR_SEP_CHAR '/'
#endif
#ifndef DIR_SEP_CHAR
#error "unknown platform"
#endif
#include "storage.h"
#include "avl3.h"
#include "llgen.h"
#include "gram.tab.h"
#include "regexp.h"
#include "lang.h"

void PrintRule2(FILE *f, Node rule);
void PrintRule(FILE *f, Name lhs, Node rule);
static void PrintAlternatives(NodeList rules, TREE follow, TREE oneOf, int indent, const char *errormsg, const char *errorargs, Boolean generateCode, TREE firstNext, Boolean *usesLA);
extern int yywrap();

static TREE idTree = NULL;
Name startSymbol = NULL;
static int nodeNumber = 0;
Name endOfInput;
static Boolean errorOccurred;
static char *inputFileName = "(stdin)";
char llTokenSetTypeString[64] = "LLTokenSet";
static Boolean changes;
static Boolean notLLError = false;
static Name currProc;
FILE *outputFile = NULL;
static Node oldNodes = NULL;
static FILE *skeletonFile = NULL;
static TREE properties = NULL;
static Name *indexArray = NULL;
static RegExp dfa = NULL;
static TREE tokenSets = NULL;
static TREE tokenSets2 = NULL;
static long int tokenSetNr = 0;
static int lastLineCopied = 0;
static TREE overlap = NULL;
static char *regExpError[] = {
/* 0 */ "no error",
/* 1 */ "illegal escape character",
/* 2 */ "unexpected end of expression",
/* 3 */ ") expected",
/* 4 */ "end of expression expected",
/* 5 */ "] expected",
/* 6 */ "no such error!",
/* 7 */ "- expected",
/* 8 */ "error in range"
};
static Language* targetLanguage = &cLanguage;
int nrBitsInElement = 8;
static char* baseName = "parser";
static const char* packageName = "main";
int globalIndent = 0;
char *MakeString(char *str) {
	char *copy = mmalloc(strlen(str) + 1);

	strcpy(copy, str);
	return (copy);
}

static Boolean HasProperty(char *property) {
	if (*property == '!') {
		return (FindNode(properties, property + 1, (TreeCmp) strcmp) == NULL);
	} else {
		return (FindNode(properties, property, (TreeCmp) strcmp) != NULL);
	}
}

void AddProperty(char *property)
{
	if (FindNode(properties, property, (TreeCmp) strcmp) == NULL) {
		InsertNodeSingle(&properties, MakeString(property), NULL, (TreeCmp) strcmp);
	}
}

void RetractProperty(char *property) {
	DeleteNode(&properties, property, (TreeCmp) strcmp, NULL);
}

/* Ugly convention:
   - in declarations, the first argument is the parameter name, the rest is
     the type of the parameter.
   - in calls, everything is parameter
 */
static void openFunctionDeclaration(FILE *f, const char* functionName, Node argList, Name returnType) {
	int level = 0;
    Boolean firstStringOfParameter = true;

    targetLanguage->startFunctionDeclaration(f, functionName, returnType == NULL? "void": returnType->name);
	for (Node nPtr = argList; nPtr != NULL; nPtr = nPtr->next) {
        int nrSpaces = nPtr->lineNr;
        const char *str = (const char *) nPtr->element.name;
        if (strcmp(str, ",") == 0 && level == 0) {
            firstStringOfParameter = true;
        } else {
            if (firstStringOfParameter) {
                targetLanguage->parameterDeclarationName(f, str, nrSpaces);
                firstStringOfParameter = false;
            } else {
                targetLanguage->nextDeclarationParameterSymbol(f, str, nrSpaces);
            }
            if (strcmp(str, "{") == 0 || strcmp(str, "(") == 0 || strcmp(str, "[") == 0) {
                level++;
            } else if (strcmp(str, "}") == 0 || strcmp(str, ")") == 0 || strcmp(str, "]") == 0) {
                level--;
            }
        }
	}
}

static void closeFunctionDeclaration(FILE *f) {
    targetLanguage->endFunctionDeclaration(f);
}

static void openFunctionCall(FILE *f, const char* functionName, Node argList, Name returnType, Name assignTo) {
	int level = 0;

    fputs(targetLanguage->owner, outputFile);
    targetLanguage->startFunctionCall(f, functionName, returnType == NULL? NULL: returnType->name, assignTo == NULL? NULL: assignTo->name);
	for (Node nPtr = argList; nPtr != NULL; nPtr = nPtr->next) {
        const char *str = (const char *) nPtr->element.name;
        /* Spaces from the input */
		for (int i = nPtr->lineNr; i != 0; i--) {
			fputc(' ', f);
		}
        if (strcmp(str, ",") == 0 && level == 0) {
            targetLanguage->nextCallParameter(f);
        } else {
            targetLanguage->nextCallParameterSymbol(f, str);
            if (strcmp(str, "{") == 0 || strcmp(str, "(") == 0 || strcmp(str, "[") == 0) {
                level++;
            } else if (strcmp(str, "}") == 0 || strcmp(str, ")") == 0 || strcmp(str, "]") == 0) {
                level--;
            }
        }
	}
}

static void closeFunctionCall(FILE *f) {
    targetLanguage->endFunctionCall(f);
}

static void InterpretLine(char *str) {
	while (*str != '\0') {
		switch (*str) {
			case '%':
				str++;
                switch (*str) {
                    case 'S':
                        str++;
                        fputs(startSymbol->name, outputFile);
                        break;
                    case 'N':
                        str++;
                        fprintf(outputFile, "%d", nodeNumber + 1);
                        break;
                    case 't':
                        str++;
                        fprintf(outputFile, "%d", (nodeNumber + nrBitsInElement) / nrBitsInElement);
                        break;
                    case 'B': // Base file name
                        str++;
                        fputs(baseName, outputFile);
                        break;
                    case 'P': // Package name
                        str++;
                        fputs(packageName, outputFile);
                        break;
                    case 'E': {
                            int pos = nodeNumber + 1;
                            str++;
                            while (pos >= nrBitsInElement) {
                                fputs("0, ", outputFile);
                                pos -= nrBitsInElement;
                            }
                            fprintf(outputFile, "0x%0*lX", (nrBitsInElement + 3) / 4, (1L << pos));
                        }
                        break;
                    default:
                        fprintf(stderr, "Illegal code in skeleton file: %s\n", str - 1);
                        break;
				}
				break;
			case '#': // #S = argument for start function call
				str++;
				if (*str == 'S') {
                    int level = 0;
                    Boolean paramName = true;
                    str++;
                    targetLanguage->resetFunctionCall(outputFile);
                    if (startSymbol->arglist != NULL) {
                        for (Node nPtr = startSymbol->arglist; nPtr != NULL; nPtr = nPtr->next) {
                            const char *str = (const char *) nPtr->element.name;
                            if (strcmp(str, ",") == 0 && level == 0) {
                                targetLanguage->nextCallParameter(outputFile);
                                paramName = true;
                            } else {
                                if (paramName) {
                                    /* Spaces from the input */
                                    for (int i = nPtr->lineNr; i != 0; i--) {
                                        fputc(' ', outputFile);
                                    }
                                    targetLanguage->nextCallParameterSymbol(outputFile, str);
                                    paramName = false;
                                }
                                if (strcmp(str, "{") == 0 || strcmp(str, "(") == 0 || strcmp(str, "[") == 0) {
                                    level++;
                                } else if (strcmp(str, "}") == 0 || strcmp(str, ")") == 0 || strcmp(str, "]") == 0) {
                                    level--;
                                }
                            }
                        }
                        targetLanguage->nextCallParameter(outputFile);
                    }
                    targetLanguage->nextCallParameterSymbol(outputFile, "");
                    targetLanguage->pointerToArray(outputFile, "endOfInputSet", -1);
				} else {
					fprintf(stderr, "Illegal code in skeleton file: %s\n", str - 1);
				}
				break;
			case '*': // *S = argument for start function declaration
				str++;
				if (*str == 'S') {
                    int level = 0;
                    Boolean paramName = true;
					str++;
					targetLanguage->resetFunctionCall(outputFile);
                    if (startSymbol->arglist != NULL) {
                        for (Node nPtr = startSymbol->arglist; nPtr != NULL; nPtr = nPtr->next) {
                            const char *str = (const char *) nPtr->element.name;
                            if (strcmp(str, ",") == 0 && level == 0) {
                                targetLanguage->nextCallParameter(outputFile);
                                paramName = true;
                            } else {
                                if (paramName) {
                                    targetLanguage->parameterDeclarationName(outputFile, str, nPtr->lineNr);
                                    paramName = false;
                                } else {
                                    targetLanguage->nextDeclarationParameterSymbol(outputFile, str, nPtr->lineNr);
                                }
                                if (strcmp(str, "{") == 0 || strcmp(str, "(") == 0 || strcmp(str, "[") == 0) {
                                    level++;
                                } else if (strcmp(str, "}") == 0 || strcmp(str, ")") == 0 || strcmp(str, "]") == 0) {
                                    level--;
                                }
                            }
                        }
                    }
				} else {
					fprintf(stderr, "Illegal code in skeleton file: %s\n", str - 1);
				}
				break;
			case '\\':
				str++;
				if (*str != '\0') {
					fputc(*str++, outputFile);
				}
				break;
			default:
				fputc(*str++, outputFile);
				break;
		}
	}
}

RuleResult MakeRuleResult(Name returnVariable, Name returnType, Name initialValue) {
	RuleResult ruleResult = (RuleResult) mmalloc(sizeof(struct RuleResult));

	ruleResult->returnVariable = returnVariable;
	ruleResult->returnType = returnType;
	ruleResult->initialValue = initialValue;
	return ruleResult;
}

Name MakeName(char *str, int index, NameType nameType, short int lineNr) {
	Name name = (Name) mmalloc(sizeof(struct Name));

	name->name = MakeString(str);
	name->index = index;
	name->lineNr = lineNr;
	name->empty = false;
	name->first = NULL;
	name->nameType = nameType;
	name->rules = NULL;
	name->follow = NULL;
	name->arglist = NULL;
	name->ruleResult = NULL;
	name->keywords = NULL;
	name->usesLA = false;
	name->functions = NULL;
	return (name);
}

Name InsertSymbol(char *str, NodeList rules, Boolean lhs, NameType nameType) {
	Name name = GetInfo(FindNode(idTree, str, (TreeCmp) NameCmpStr));

	if (name == NULL) {
		name = MakeName(str, -1, nameType, lineNumber);
		InsertNodeSingle(&idTree, name, name, (TreeCmp) NameCmp);
	}
	if (lhs) {
		if (name->rules == NULL && name->nameType != token) {
			name->rules = rules;
		} else {
			yyerror("lhs %s already used", str);
		}
	}
	return (name);
}

Name InsertString(char *str) {
	Name name = GetInfo(FindNode(idTree, str, (TreeCmp) NameCmpStr));

	if (name == NULL) {
		name = MakeName(str, ++nodeNumber, string, lineNumber);
		InsertNodeSingle(&idTree, name, name, (TreeCmp) NameCmp);
	}
	return (name);
}

void AssignString(Name name, Name str) {
	if (name->rules == NULL && name->nameType == identifier) {
		name->nameType = token;
		name->index = str->index;
	} else {
		yyerror("token already defined as lhs %s", name->name);
	}
}

void KeywordString(Name name, Name str, Name ident) {
    if (name->keywords != NULL || name->functions != NULL) {
		yyerror("no keyword hierarchie allowed: %s", name->name);
	} else if (name->rules == NULL && name->nameType == identifier && (ident->nameType == identifier || ident->nameType == token)) {
		name->nameType = token;
		if (str == NULL) {
			name->index = ++nodeNumber;
		} else {
			name->index = str->index;
			str->index = 0;
		}
		ident->nameType = token;
		InsertNodeSingle(&ident->keywords, str, name, (TreeCmp) NameCmp);
		AddProperty("keywords");
	} else if (ident->nameType != identifier && ident->nameType != token) {
		yyerror("identifier already defined as lhs %s", ident->name);
	} else {
		yyerror("token already defined as lhs %s", name->name);
	}
}

void Function(Name name, Name fun, Name ident) {
	if (name->keywords != NULL || name->functions != NULL) {
		yyerror("no function hierarchie allowed: %s", name->name);
	} else if (name->rules == NULL && name->nameType == identifier && (ident->nameType == identifier || ident->nameType == token)) {
		name->nameType = token;
		name->index = ++nodeNumber;
		fun->index = 0;
		fun->nameType = function;
		ident->nameType = token;
		InsertNodeSingle(&ident->functions, fun, name, (TreeCmp) NameCmp);
		AddProperty("functions");
	} else if (ident->nameType != identifier && ident->nameType != token) {
		yyerror("identifier already defined as lhs %s", ident->name);
	} else {
		yyerror("token already defined as lhs %s", name->name);
	}
}

Node MakeNode(short int lineNr, NodeType nodeType, void *element, Node next)
{
	Node node;

	if (oldNodes == NULL)
	{
		node = (Node) mmalloc(sizeof(struct Node));
	}
	else
	{
		node = oldNodes;
		oldNodes = oldNodes->next;
	}
	node->nodeType = nodeType;
	node->arglist = NULL;
	switch (nodeType)
	{
		case single: node->element.name = element; break;
		case option: node->element.sub = element; break;
		case sequence: node->element.sub = element; break;
		case chain: node->element.sub = element; break;
		case alternative: node->element.alternatives = element; break;
		default: fprintf(stderr, "MakeNode: unknown nodetype %d\n", nodeType);
	}
	node->lineNr = lineNr;
	node->next = next;
	node->preferShift = false;
	node->assignTo = NULL;
	return (node);
}

NodeList MakeNodeList(Node first, NodeList rest)
{
	NodeList nodeList = (NodeList) mmalloc(sizeof(struct NodeList));

	nodeList->first = first;
	nodeList->rest = rest;
	return (nodeList);
}

Node AppendNode(Node node1, Node node2)
{
	Node ret = node1;

	if (node1 == NULL)
	{
		return (node2);
	}
	while (node1->next != NULL)
	{
		node1 = node1->next;
	}
	node1->next = node2;
	return (ret);
}

static Boolean DetermineEmptyRule(Node rule)
{
	Boolean empty;
	NodeList rules;

	for (empty = true; empty && rule != NULL; rule = rule->next)
	{
		switch (rule->nodeType)
		{
			case single: /* Only if identifier produces empty, not if string; ignore code and output */
				empty = (rule->element.name->nameType == identifier && rule->element.name->empty) ||
						rule->element.name->nameType == output || rule->element.name->nameType == code;
				break;
			case option: /* Bien sure */
				empty = true;
				break;
			case sequence: /* Only if subpart is empty */
				empty = DetermineEmptyRule(rule->element.sub);
				break;
			case chain: /* Only if first part of chain is empty, second doesn't matter */
				empty = DetermineEmptyRule(rule->element.sub->element.sub);
				break;
			case alternative: /* Only if one of the alternatives is empty */
				empty = false;
				for (rules = rule->element.alternatives; !empty && rules != NULL; rules = rules->rest)
				{
					empty = DetermineEmptyRule(rules->first);
				}
				break;
			default:
				fprintf(stderr, "DetermineEmptyRule: unknown nodetype %d\n", rule->nodeType);
				return (empty);
		}
	}
	return (empty);
}

static void DetermineEmpty(Name name)
{
	NodeList rules = name->rules;
	Boolean empty = false;

	if (!name->empty && name->nameType == identifier)
	{
		while (!empty && rules != NULL)
		{
			empty = DetermineEmptyRule(rules->first);
			rules = rules->rest;
		}
		if (empty)
		{
			name->empty = true;
			changes = true;
		}
	}
}

int NameCmpStr(Name str1, char *str2)
{
	return (strcmp(str1->name, str2));
}

int NameCmp(Name str1, Name str2)
{
	return (str1 == NULL? (str2 == NULL? 0: -1): (str2 == NULL? 1: strcmp(str1->name, str2->name)));
}

static Boolean InsertElement(TREE *first, Name name)
{
	Name sym = GetInfo(FindNode(*first, name, (TreeCmp) NameCmp));

	if (sym == NULL)
	{
		InsertNodeSingle(first, name, name, (TreeCmp) NameCmp);
		return (true);
	}
	else
	{
		return (false);
	}
}

static TREE Intersection(TREE t1, TREE t2) {
	TREE result = NULL;
	TREE t = GetFirstNode(t1);

	while (t != NULL) {
		if (FindNode(t2, GetKey(t), (TreeCmp) NameCmp) != NULL) {
			InsertNodeSingle(&result, GetKey(t), GetKey(t), (TreeCmp) NameCmp);
		}
		t = GetNextNode(t);
	}
	return (result);
}

static Boolean NotEmptyIntersection(TREE t1, TREE t2) {
	TREE t = GetFirstNode(t1);

	while (t != NULL) {
		if (FindNode(t2, GetKey(t), (TreeCmp) NameCmp) != NULL) {
			return (true);
		}
		t = GetNextNode(t);
	}
	return (false);
}

static Boolean Unite(TREE *set, TREE addition) {
	TREE t = GetFirstNode(addition);
	Boolean change = false;

	while (t != NULL) {
		if (InsertElement(set, GetKey(t))) {
			change = true;
		}
		t = GetNextNode(t);
	}
	return (change);
}

static TREE ExpandOverlap(TREE tr) {
	TREE ta = NULL;
	TREE t = GetFirstNode(tr);
	TREE node;

	while (t != NULL) {
		if ((node = FindNode(overlap, GetKey(t), (TreeCmp) NameCmp)) == NULL) {
			InsertNodeSingle(&ta, GetKey(t), NULL, (TreeCmp) NameCmp);
		} else {
			Unite(&ta, GetInfo(node));
		}
		t = GetNextNode(t);
	}
	return (ta);
}

/* Intersection2 also incorporates overlapping tokens */
static TREE Intersection2(TREE t1, TREE t2) {
	TREE ta1 = ExpandOverlap(t1);
	TREE ta2 = ExpandOverlap(t2);
	TREE result = Intersection(ta1, ta2);

	KillTree(ta1, NULL);
	KillTree(ta2, NULL);
	return (result);
}

static void ifCondition(Boolean open, int indent) {
    if (open) {
        fputs(targetLanguage->openIf, outputFile);
    } else {
        fputs(targetLanguage->closeIf, outputFile);
    }
}

static void elseIfCondition(Boolean open, int indent) {
    if (open) {
        fputs(targetLanguage->elseStr, outputFile);
        fputc(' ', outputFile);
        fputs(targetLanguage->openIf, outputFile);
    } else {
        fputs(targetLanguage->closeIf, outputFile);
    }
}

Boolean bracesOnNewLine = false;

void doIndent(int level) {
    fputc('\n', outputFile);
	while (level != 0)
	{
		fputs("	", outputFile);
		level--;
	}
}

void startBlock(int indentLevel) {
    if (bracesOnNewLine) {
        doIndent(indentLevel);
        fputs("{", outputFile);
    } else {
        fputs(" {", outputFile);
    }
}

void terminateBlock(int indentLevel) {
    doIndent(indentLevel);
    fputs("}", outputFile);
}

static void startUnconditionalLoop(int indent) {
    doIndent(indent);
    fputs(targetLanguage->unconditionalLoop, outputFile);
    startBlock(indent);
}

static void breakStatement(int indent) {
    doIndent(indent);
    fprintf(outputFile, "%s%s", targetLanguage->breakStatement, targetLanguage->terminateStatement);
}

static void endUnconditionalLoop(int indent) {
    terminateBlock(indent);
}

static void startDoWhileLoop(int indentLevel) {
    doIndent(indentLevel);
    targetLanguage->startDoWhileLoop(outputFile, indentLevel);
}

static void endDoWhileLoop(Boolean open, int indent) {
    targetLanguage->endDoWhileLoop(outputFile, open, indent);
}

static void PrintArrayDeclaration(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size) {
    targetLanguage->arrayDeclaration(f, scope, type, name, suffix, size, true);
}

static void PrintLocalDeclaration(FILE* f, const char* type, const char* name) {
    targetLanguage->localDeclaration(f, type, name);
    fputs(targetLanguage->terminateDeclaration, f);
}

static void PrintLocalDeclarationWithInitialization(FILE* f, const char* type, const char* name, const char* init) {
    targetLanguage->localDeclarationWithInitialization(f, type, name, init);
    fputs(targetLanguage->terminateDeclaration, f);
}

static void PrintReturnValue(FILE* f, const char* returnValue) {
    targetLanguage->returnValue(f, returnValue);
    fputs(targetLanguage->terminateStatement, f);
}

static void PrintStateTransitionStart(FILE* f, long int nr) {
    targetLanguage->stateTransitionStart(f, nr);
}

static void PrintStateTransition(FILE* f, int ch, long int destination, const char* accept) {
    targetLanguage->stateTransition(f, ch, destination, accept);
}

static void PrintEmptyTransition(FILE* f, long int nr) {
    targetLanguage->emptyStateTransition(f, nr);
}

static void PrintStateTransitionEnd(FILE* f) {
    targetLanguage->stateTransitionEnd(f);
}

static Boolean CollectFirst(TREE *firsts, TREE follow, Node rule) {
	NodeList rules;
	Boolean empty = true;
	TREE sym;

	while (empty && rule != NULL) {
		switch (rule->nodeType) {
			case single:
				if (rule->element.name->nameType == identifier) {
					for (sym = GetFirstNode(rule->element.name->first); sym != NULL; sym = GetNextNode(sym)) {
						Name key = GetKey(sym);
						if (key->nameType == token || key->nameType == string) {
							(void) InsertElement(firsts, key);
						}
					}
					empty = rule->element.name->empty;
				} else if (rule->element.name->nameType == token || rule->element.name->nameType == string) {
					(void) InsertElement(firsts, rule->element.name);
					empty = false;
				}
				break;
			case option:
				(void) CollectFirst(firsts, NULL, rule->element.sub);
				break;
			case sequence:
				empty = CollectFirst(firsts, NULL, rule->element.sub);
				break;
			case chain:
				empty = CollectFirst(firsts, NULL, rule->element.sub->element.sub);
				if (empty) /* Now also inspect the other chain part, but it remains empty */ {
					(void) CollectFirst(firsts, NULL, rule->element.sub->next->element.sub);
				}
				break;
			case alternative: /* Try each one... */
				empty = false;
				for (rules = rule->element.alternatives; rules != NULL; rules = rules->rest) {
					if (CollectFirst(firsts, NULL, rules->first)) {
						empty = true;
					}
				}
				break;
			default:
				fprintf(stderr, "CollectFirst: unknown nodetype %d\n", rule->nodeType);
				return (empty);
		}
		rule = rule->next;
	}
	if (empty) {
		Unite(firsts, follow);
	}
	return empty;
}

static void DetermineFirst(Name name)
{
	NodeList rules = name->rules;
	TREE firsts = NULL;

	while (rules != NULL)
	{
		(void) CollectFirst(&firsts, NULL, rules->first);
		rules = rules->rest;
	}
	if (Unite(&name->first, firsts))
	{
		changes = true;
	}
	KillTree(firsts, NULL);
}

static void DetermineFollowRule(TREE *follow, Node rule, Name orig, TREE followers)
{
	NodeList rules;
	TREE nFollowers;
	TREE chain1Followers;
	TREE chain2Followers;
    TREE sequenceFirstsAndFollowers;

	for (; rule != NULL; rule = rule->next)
	{
		if (rule->nodeType != single || rule->element.name == orig)
		{
			nFollowers = NULL;
			(void) CollectFirst(&nFollowers, followers, rule->next);
			switch (rule->nodeType)
			{
				case single: /* && rule->element.name == orig */
					if (Unite(follow, nFollowers))
					{
						changes = true;
					}
					break;
				case option:
					DetermineFollowRule(follow, rule->element.sub, orig, nFollowers);
					break;
				case sequence:
                    sequenceFirstsAndFollowers = NULL;
                    (void) CollectFirst(&sequenceFirstsAndFollowers, NULL, rule->element.sub);
                    Unite(&sequenceFirstsAndFollowers, nFollowers);
					DetermineFollowRule(follow, rule->element.sub, orig, sequenceFirstsAndFollowers);
                    KillTree(sequenceFirstsAndFollowers, NULL);
					break;
				case chain:
					/*	A CHAIN B, C
						FOLLOW(A) = nFollowers + FIRST(B) + (EMPTY(B)? FIRST(A) + ...: EMPTY)
						FOLLOW(B) = FIRST(A) + (EMPTY(A)? FIRST(B) + nFollowers: EMPTY) */
					chain1Followers = NULL;
					chain2Followers = NULL;
					(void) CollectFirst(&chain2Followers, nFollowers, rule->element.sub->element.sub);
					/* chain2Followers = FIRST(A) + (EMPTY(A)? nFollowers: EMPTY) */
					(void) CollectFirst(&chain1Followers, chain2Followers, rule->element.sub->next->element.sub);
					/* chain1Followers = FIRST(B) + (EMPTY(B)? chain2Followers: EMPTY) */
					(void) Unite(&chain1Followers, nFollowers);
					/* chain1Followers = nFollowers + FIRST(B) + ... */
					DetermineFollowRule(follow, rule->element.sub->element.sub, orig, chain1Followers);
					KillTree(chain1Followers, NULL);
					KillTree(chain2Followers, NULL);
					chain1Followers = NULL;
					chain2Followers = NULL;
					(void) CollectFirst(&chain2Followers, NULL, rule->element.sub->next->element.sub);
					/* chain2Followers = FIRST(B) */
					(void) Unite(&chain2Followers, nFollowers);
					/* chain2Followers = FIRST(B) + nFollowers */
					(void) CollectFirst(&chain1Followers, chain2Followers, rule->element.sub->element.sub);
					/* chain1Followers = FIRST(A) + (EMPTY(A)? chain2Followers: EMPTY) */
					DetermineFollowRule(follow, rule->element.sub->next->element.sub, orig, chain1Followers);
					KillTree(chain1Followers, NULL);
					KillTree(chain2Followers, NULL);
					break;
				case alternative: /* Try each one... */
					for (rules = rule->element.alternatives; rules != NULL; rules = rules->rest)
					{
						DetermineFollowRule(follow, rules->first, orig, nFollowers);
					}
					break;
				default:
					fprintf(stderr, "DetermineFollowRule: unknown nodetype %d", rule->nodeType);
					return;
			}
			KillTree(nFollowers, NULL);
		}
	}
}

static void DetermineFollow(Name name)
{
	TREE sym;
	Name lhs;
	NodeList rules;

	if (name->nameType == identifier)
	{
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
		{
			lhs = GetKey(sym);
			for (rules = lhs->rules; rules != NULL; rules = rules->rest)
			{
				DetermineFollowRule(&name->follow, rules->first, name, lhs->follow);
			}
		}
	}
}

static void ComputeEmptyFirstFollow(void)
{
	TREE sym;

	/* Compute empty productions */	
	do
	{
		changes = false;
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
		{
			DetermineEmpty(GetKey(sym));
		}
	}
	while (changes);
	/* Compute first sets */
	do
	{
		changes = false;
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
		{
			DetermineFirst(GetKey(sym));
		}
	}
	while (changes);
	/* Compute follow sets */
	(void) InsertElement(&startSymbol->follow, endOfInput);
	do
	{
		changes = false;
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
		{
			DetermineFollow(GetKey(sym));
		}
	}
	while (changes);
}

static void RenumberRegExp(RegExp regExp)
{
	Transition tr, tr2;
	ESet transitions;
	int mCount = CharSetSize(), count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;
	int sNum = 0;

	while (regExp != NULL)
	{
		mState = NULL;
		if (regExp->transitions == NULL)
		{
			regExp->state.number = -1;
		}
		else
		{
			regExp->state.number = sNum;
			transitions = NULL;
			tCount = CharSetSize();
			for (i = 0; i != tCount; i++) cSet[i] = false;
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (!cSet[tr->transitionChar])
				{
					mCount--;
					cSet[tr->transitionChar] = true;
					tCount--;
				}
			}
			mCount = tCount;
			/* mCount is nr. of fails, mState == NULL, the fail state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState) /* not the most efficient check, but ok */
				{
					for (count = 0, tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination) count++;
					if (count > mCount)
					{
						mCount = count;
						mState = tr->destination;
					}
				}
			}
			/* mCount is max. nr. of transitions to the same state, mState is that state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState && !SetMember(tr->destination, transitions))
				{
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination)
							sNum++;
				}
			}
			if (mState == NULL)
			{
				sNum++;
			}
			else
			{
				if (tCount != 0)
				{
					for (i = 0; i != CharSetSize(); i++)
					{
						if (!cSet[i]) sNum++;
					}
				}
				sNum++;
			}
			RemoveSet(transitions);
		}
		regExp = regExp->next;
	}
}

static Name TokenIndexName(int i, Boolean strings) {
	TREE sym;
	Name name;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
		name = GetKey(sym);
		if (name->nameType == token && name->index == i) {
			return name;
		}
	}
    if (strings) {
        for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
            name = GetKey(sym);
            if (name->nameType == string && name->index == i) {
                return name;
            }
        }
    }
	return NULL;
}

static int TokenSetCmp(void *tSet1, void *tSet2)
{
	ESet tokenSet1 = tSet1;
	ESet tokenSet2 = tSet2;

	while (tokenSet1 != NULL && tokenSet2 != NULL)
	{
		Answer answer1 = (Answer) tokenSet1->element;
		Answer answer2 = (Answer) tokenSet2->element;
		if (answer1->info > answer2->info)
		{
			return (1);
		}
		else if (answer1->info < answer2->info)
		{
			return (-1);
		}
		tokenSet1 = tokenSet1->next;
		tokenSet2 = tokenSet2->next;
	}
	return (tokenSet1 != NULL? 1: tokenSet2 != NULL? -1: 0);
}

static int IntCmp(void *k1, void *k2)
{
	return (k1 < k2? -1: k1 == k2? 0: 1);
}

static void CreateTokenSet(FILE *f, ESet tokenSet)
{
	TREE node = FindNode(tokenSets, tokenSet, TokenSetCmp);
	TREE tokens = NULL;
	int lastBoundary = 0;
	unsigned long int cValue = 0;

	if (node == NULL)
	{
		tokenSetNr++;
		InsertNodeSingle(&tokenSets, tokenSet, (void *) tokenSetNr, TokenSetCmp);
        targetLanguage->arrayDeclaration(f, "static", targetLanguage->tokenSetType,
                                         "llTokenSet", tokenSetNr, (nodeNumber + 1) / nrBitsInElement + 1, true);
		while (tokenSet != NULL)
		{
			InsertNodeSingle(&tokens, ((Answer) tokenSet->element)->info, NULL, IntCmp);
			tokenSet = tokenSet->next;
		}
        node = GetFirstNode(tokens);
        while (lastBoundary <= nodeNumber) {
            if (node != NULL) {
                int tokenIndex = (int) GetKey(node);
                if (tokenIndex == -1) tokenIndex = 0;
                while (tokenIndex >= lastBoundary + nrBitsInElement)
                {
                    fprintf(f, "0x%0*lX", (nrBitsInElement + 3) / 4, cValue);
                    cValue = 0;
                    lastBoundary += nrBitsInElement;
                    if (lastBoundary <= nodeNumber) {
                        fputs(", ", f);
                    }
                }
                cValue |= 1 << (tokenIndex - lastBoundary);
                node = GetNextNode(node);
            } else {
                fprintf(f, "0x%0*lX", (nrBitsInElement + 3) / 4, cValue);
                cValue = 0;
                lastBoundary += nrBitsInElement;
                if (lastBoundary <= nodeNumber) {
                    fputs(", ", f);
                }
            }
		}
        fprintf(f, "%s%s /*", targetLanguage->endOfArrayLiteral, targetLanguage->terminateDeclaration);
        for (node = GetFirstNode(tokens); node != NULL; node = GetNextNode(node)) {
            int tokenIndex = (int) GetKey(node);
            Name name = TokenIndexName(tokenIndex == -1? 0: tokenIndex, false);
            if (name != NULL) {
                fprintf(f, " %s", name->name);
            }
        }
        fputs(" */\n", f);
		KillTree(tokens, NULL);
	}
}

static void FormatTokenSets(FILE *f, RegExp dfa)
{
	RegExp regExp;

	for (regExp = dfa; regExp != NULL; regExp = regExp->next)
	{
		if (regExp->info != NULL)
		{
			CreateTokenSet(f, regExp->info);
		}
	}
}

static char *TokenSetName(RegExp state)
{
	static char tokenSetName[16];
	TREE node;

	if (state == NULL || state->info == NULL || (node = FindNode(tokenSets, state->info, TokenSetCmp)) == NULL)
	{
		strcpy(tokenSetName, targetLanguage->nullValue);
	}
	else
	{
		sprintf(tokenSetName, "llTokenSet%d", (int) GetInfo(node));
	}
	return (tokenSetName);
}

static void FormatRegExp(FILE *f, RegExp dfa)
{
	Transition tr, tr2;
	ESet transitions;
	int mCount = CharSetSize(), count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;
	Boolean prntd = false;
	RegExp regExp;
	int sNum = 0;

	for (regExp = dfa; regExp != NULL; regExp = regExp->next)
	{
		if (prntd) fputs("\n", f);
		if (regExp->transitions != NULL)
		{
            PrintStateTransitionStart(f, regExp->state.number);
			prntd = true;
			mState = NULL;
			transitions = NULL;
			tCount = CharSetSize();
			for (i = 0; i != tCount; i++) cSet[i] = false;
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (!cSet[tr->transitionChar])
				{
					cSet[tr->transitionChar] = true;
					tCount--;
				}
			}
			mCount = tCount;
			/* mCount is nr. of fails, mState == NULL, the fail state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState) /* not the most efficient check, but ok */
				{
					for (count = 0, tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination) count++;
					if (count > mCount)
					{
						mCount = count;
						mState = tr->destination;
					}
				}
			}
			/* mCount is max. nr. of transitions to the same state, mState is that state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState && !SetMember(tr->destination, transitions))
				{
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next)
					{
						if (tr2->destination == tr->destination)
						{
							if (prntd) prntd = false;
							else fputs("		  ", f);
                            PrintStateTransition(f, tr2->transitionChar, tr->destination->state.number, TokenSetName(tr->destination));
							sNum++;
						}
					}
				}
			}
			if (mState == NULL)
			{
				if (prntd) prntd = false;
				else fputs("		  ", f);
                PrintStateTransition(f, -1, -1, targetLanguage->nullValue);
				sNum++;
			}
			else
			{
				if (tCount != 0)
				{
					for (i = 0; i != CharSetSize(); i++)
					{
						if (!cSet[i])
						{
							if (prntd) prntd = false;
							else fputs("		  ", f);
                            PrintStateTransition(f, i, -1, targetLanguage->nullValue);
							sNum++;
						}
					}
				}
				if (prntd) prntd = false;
				else fputs("		  ", f);
                PrintStateTransition(f, -1, mState->state.number, TokenSetName(mState));
				sNum++;
			}
			RemoveSet(transitions);
            PrintStateTransitionEnd(f);
		} else {
            PrintEmptyTransition(f, regExp->state.number);
        }
	}
	if (prntd) fputs("\n", f);
}

static void regexpcopy(char *copy, char *regexp)
{
	regexp++;
	while (*regexp != '\0')
	{
		if (*regexp == '"')
		{
			if (regexp[1] != '\0')
			{
				yyerror("quote error in regexp\n");
				errorOccurred = true;
			}
			*copy = '\0';
			return;
		}
		else if (*regexp == '\\')
		{
			if (regexp[1] == '\0')
			{
				yyerror("escape error in regexp\n");
				errorOccurred = true;
				regexp++;
			}
			else if (regexp[1] == '"')
			{
				*copy++ = '"';
				regexp += 2;
			}
			else if (regexp[1] == '\\')
			{
				*copy++ = '\\';
				*copy++ = '\\';
				regexp += 2;
			}
			else
			{
				*copy++ = *regexp++;
			}
		}
		else
		{
			*copy++ = *regexp++;
		}
	}
}

static void VerifyTokenOverlap(ESet oTokenSet)
{
	int nrTokens = 0;
	ESet tokenSet = oTokenSet;
	ESet token;

	while (tokenSet != NULL)
	{
        int tokenIndex = (int) ((Answer) tokenSet->element)->info;
        if (tokenIndex != -1) {
            /* Ignore overlap with endOfInput/IGNORE, since endOfInput can't
               happen, and IGNORE is skipped during parsing when the overlapping
               token is not in the expected set. */
            nrTokens++;
        }
		tokenSet = tokenSet->next;
	}
	if (nrTokens > 1)
	{
		fprintf(stderr, "Overlap between tokens:\n");
		for (tokenSet = oTokenSet; tokenSet != NULL; tokenSet = tokenSet->next)
		{
			int tokenIndex = (int) ((Answer) tokenSet->element)->info;
			Name name = TokenIndexName(tokenIndex, true);
			TREE node = InsertNodeSingle(&overlap, name, NULL, (TreeCmp) NameCmp);
			TREE nOverlap = GetInfo(node);
			fprintf(stderr, "%s:%d: token %d: %s\n", inputFileName, name->lineNr, tokenIndex, name->name);
			for (token = oTokenSet; token != NULL; token = token->next)
			{
				InsertNodeSingle(&nOverlap, TokenIndexName((int) ((Answer) token->element)->info, true), NULL, (TreeCmp) NameCmp);
			}
			SetInfo(node, nOverlap);
		}
	}
}

static void CheckOverlap(RegExp dfa)
{
	Transition tr, tr2;
	ESet transitions;
	int mCount = CharSetSize(), count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;
	RegExp regExp;

	for (regExp = dfa; regExp != NULL; regExp = regExp->next)
	{
		if (regExp->transitions != NULL)
		{
			mState = NULL;
			transitions = NULL;
			tCount = CharSetSize();
			for (i = 0; i != tCount; i++) cSet[i] = false;
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (!cSet[tr->transitionChar])
				{
					mCount--;
					cSet[tr->transitionChar] = true;
					tCount--;
				}
			}
			/* mCount is nr. of fails, mState == NULL, the fail state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState) /* not the most efficient check, but ok */
				{
					for (count = 0, tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination) count++;
					if (count > mCount)
					{
						mCount = count;
						mState = tr->destination;
					}
				}
			}
			/* mCount is max. nr. of transitions to the same state, mState is that state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState && !SetMember(tr->destination, transitions))
				{
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next)
					{
						if (tr2->destination == tr->destination)
						{
							TREE node = FindNode(tokenSets, tr->destination->info, TokenSetCmp);
							if (node != NULL)
							{
								VerifyTokenOverlap(GetKey(node));
							}
						}
					}
				}
			}
			if (mState != NULL)
			{
				TREE node = FindNode(tokenSets, mState->info, TokenSetCmp);
				if (node != NULL)
				{
					VerifyTokenOverlap(GetKey(node));
				}
			}
			RemoveSet(transitions);
		}
	}
}

static void CompileScanner(void)
{
	TREE sym;
	Name name;
	RegExp nfa = NULL;
	int res;
	char regexpbuf[256];

	if (!HasProperty("escregexp"))
	{
		viMode = true;
	}
	push_localadm();
	nfa = InitRegExp();
	SetSingleAnswer(false);
	if (HasProperty("extcharset"))
	{
		SetCharSetSize(256);
	}
	else
	{
		SetCharSetSize(128);
	}

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
	{
		name = GetKey(sym);
		if (name->nameType == string && name->index != 0)
		{
			regexpcopy(regexpbuf, name->name);
			if ((res = AddRegExp(nfa, regexpbuf, (void *) name->index)) != 0)
			{
				fprintf(stderr, "%s:%d: %s\n", inputFileName, name->lineNr, regExpError[res]);
				errorOccurred = true;
			}
		}
	}
	dfa = N2DFA(nfa);
	MinimizeDFA(dfa);
}

void PrintRule2(FILE *f, Node rule)
{
	NodeList rules;

	while (rule != NULL)
	{
		switch (rule->nodeType)
		{
			case single:
				switch (rule->element.name->nameType)
				{
					case code: fprintf(f, " { %s }", rule->element.name->name); break;
					default: fprintf(f, " %s", rule->element.name->name); break;
				}
				break;
			case option:
				fputs("(", f);
				PrintRule2(f, rule->element.sub);
				fputs(") OPTION", f);
				break;
			case sequence:
				fputs("(", f);
				PrintRule2(f, rule->element.sub);
				fputs(") SEQUENCE", f);
				break;
			case chain:
				fputs("(", f);
				PrintRule2(f, rule->element.sub->element.sub);
				fputs(") CHAIN (", f);
				PrintRule2(f, rule->element.sub->next->element.sub);
				fputs(")", f);
				break;
			case alternative: /* Try each one... */
				fputs("(", f);
				for (rules = rule->element.alternatives; rules != NULL; rules = rules->rest)
				{
					fputs("(", f);
					PrintRule2(f, rules->first);
					fputs(")", f);
					if (rules->rest != NULL)
					{
						fputs(";", f);
					}
				}
				fputs(")", f);
				break;
			default:
				fprintf(stderr, "DetermineFollowRule: unknown nodetype %d", rule->nodeType);
				return;
		}
		rule = rule->next;
		if (rule != NULL)
		{
			fputs(",", f);
		}
	}
}

void PrintRule(FILE *f, Name lhs, Node rule)
{
	fprintf(f, "%s:", lhs->name);
	PrintRule2(f, rule);
	fputs(".\n", f);
}

static void PrintTree(FILE *f, TREE set)
{
	set = GetFirstNode(set);
	while (set != NULL)
	{
		fprintf(f, "%s", ((Name) GetKey(set))->name);
		set = GetNextNode(set);
		if (set != NULL)
		{
			fputs(", ", f);
		}
	}
}

static int tabWidth = 4;

static void PrintCode(char *str, NameType prev, NameType next, int indent)
{
	int prefixLength = 0;
	char *sstr = str;
	int length;

    doIndent(indent);
    /* Skip until first printable character; count whitespace from line start */
	for (;;) {
		if (*sstr == '\n') { prefixLength = 0; str = sstr + 1; lastLineCopied++; }
		else if (*sstr == ' ') prefixLength++;
		else if (*sstr == '\t') prefixLength = (prefixLength / tabWidth + 1) * tabWidth;
		else break;
		sstr++;
	}
    /* Copy code block, but replace leading whitespace with given indent */
	while (*str != '\0') {
		length = 0; sstr = str;
		while (*sstr != '\0' && (*sstr == ' ' || *sstr == '\t')) {
			if (*sstr == ' ') length++;
			else if (*sstr == '\t') length = (length / tabWidth + 1) * tabWidth;
			sstr++;
		}
        if (*sstr == '\0' || (*sstr == '\n' && sstr[1] == '\0')) {
            // Ignore final newline
            if (*sstr == '\n') {
                lastLineCopied++;
            }
            break;
        }
		length -= prefixLength;
        doIndent(indent);
        while (length > 0) {
            fputc(' ', outputFile);
            length--;
        }
		str = sstr;
		while (*str != '\0' && *str != '\n') {
			fputc(*str++, outputFile);
		}
		if (*str == '\n') {
			str++;
			lastLineCopied++;
		}
	}
}

static int TokenSetCmp2(void *tSet1, void *tSet2)
{
	TREE t1 = GetFirstNode(tSet1);
	TREE t2 = GetFirstNode(tSet2);

	while (t1 != NULL && t2 != NULL)
	{
		Name token1 = (Name) GetKey(t1);
		Name token2 = (Name) GetKey(t2);
		int res = strcmp(token1->name, token2->name);
		if (res != 0)
		{
			return (res);
		}
		t1 = GetNextNode(t1);
		t2 = GetNextNode(t2);
	}
	return (t1 != NULL? 1: t2 != NULL? -1: 0);
}

static void TokenSetName2(TREE tokenSet) {
	TREE node = FindNode(tokenSets2, tokenSet, TokenSetCmp2);

    targetLanguage->pointerToArray(outputFile, "llTokenSet", (long int) GetInfo(node));
}

static void CreateTokenSet2(FILE *f, TREE tokenSet)
{
	TREE node = FindNode(tokenSets2, tokenSet, TokenSetCmp2);
	int lastBoundary = 0;
	unsigned long int cValue = 0;
	TREE tokens = NULL;

	if (node == NULL)
	{
		tokenSetNr++;
		InsertNodeSingle(&tokenSets2, CopyTree(tokenSet, NULL, NULL), (void *) tokenSetNr, TokenSetCmp2);
        targetLanguage->arrayDeclaration(f, "static", targetLanguage->tokenSetType,
                                         "llTokenSet", tokenSetNr, (nodeNumber + 1) / nrBitsInElement + 1, true);
		for (node = GetFirstNode(tokenSet); node != NULL; node = GetNextNode(node))
		{
			Name name = (Name) GetKey(node);
			long int tokenIndex = name->index;
			if (tokenIndex == -1) tokenIndex = 0;
			InsertNodeSingle(&tokens, (void *) tokenIndex, NULL, IntCmp);
		}
        node = GetFirstNode(tokens);
        while (lastBoundary <= nodeNumber) {
            if (node != NULL) {
                int tokenIndex = (int) GetKey(node);
                while (tokenIndex >= lastBoundary + nrBitsInElement)
                {
                    fprintf(f, "0x%0*lX", (nrBitsInElement + 3) / 4, cValue);
                    cValue = 0;
                    lastBoundary += nrBitsInElement;
                    if (lastBoundary <= nodeNumber) {
                        fputs(", ", f);
                    }
                }
                cValue |= 1 << (tokenIndex - lastBoundary);
                node = GetNextNode(node);
            } else {
                fprintf(f, "0x%0*lX", (nrBitsInElement + 3) / 4, cValue);
                cValue = 0;
                lastBoundary += nrBitsInElement;
                if (lastBoundary <= nodeNumber) {
                    fputs(", ", f);
                }
            }
		}
		fprintf(f, "%s%s /*", targetLanguage->endOfArrayLiteral, targetLanguage->terminateDeclaration);
		for (node = GetFirstNode(tokenSet); node != NULL; node = GetNextNode(node))
		{
			Name name = (Name) GetKey(node);
            fprintf(f, " %s", name->name);
		}
        fputs(" */\n", f);
		KillTree(tokens, NULL);
	}
}

static void CreateGuard(Boolean guard, Boolean generateCode, Boolean empty, int indent, TREE firsts, Boolean firstOfRule, Boolean *usesLA) {
	if (guard) {
		if (generateCode) {
			doIndent(indent);
			fprintf(outputFile, "%swaitForToken(", targetLanguage->owner);
			if (firsts == NULL) {
				fputs("follow\n", outputFile);
			} else if (empty || firstOfRule) {
				fputs("uniteTokenSets(", outputFile);
                if (targetLanguage->requiresLocalLTSetStorage) {
                    targetLanguage->pointerToArray(outputFile, "ltSet", -1);
                    fputs(", ", outputFile);
                }
                TokenSetName2(firsts);
				fputs(", follow)", outputFile);
			} else {
				TokenSetName2(firsts);
			}
            fprintf(outputFile, ", follow)%s", targetLanguage->terminateStatement);
			if (HasProperty("showfirsts")) {
				fputs(" /* ", outputFile);
				PrintTree(outputFile, firsts);
				fputs(" */", outputFile);
			}
			if (firstOfRule) {
				ifCondition(true, indent);
				fprintf(outputFile, "!tokenInCommon(%scurrSymbol, ", targetLanguage->owner);
                TokenSetName2(firsts);
                fputs(")", outputFile);
                ifCondition(false, indent);
			}
		} else {
			if (firsts != NULL && (empty || firstOfRule)) {
				*usesLA = true;
			}
			CreateTokenSet2(outputFile, firsts);
		}
	}
}

static void PrintFirstSymbols(TREE ofirsts, TREE oneOf, void (*cond)(Boolean, int), int indent, Boolean positive, Boolean generateCode, TREE *symbols, int lineNr, Boolean emptyFollow, Boolean* usesLA) {
	TREE common = Intersection2(ofirsts, oneOf);

	if (generateCode) {
        cond(true, indent);
        fprintf(outputFile, "%stokenInCommon(%scurrSymbol, ", (positive? "": "!"), targetLanguage->owner);
		if (emptyFollow) {
			fputs("uniteTokenSets(", outputFile);
            if (targetLanguage->requiresLocalLTSetStorage) {
                targetLanguage->pointerToArray(outputFile, "ltSet", -1);
                fputs(", ", outputFile);
            }
            fputs("follow, ", outputFile);
		}
        TokenSetName2(common);
		if (emptyFollow) {
            fputs(")", outputFile);
        }
        fputs(")", outputFile);
        cond(false, indent);
		if (common == NULL || (symbols != NULL && NotEmptyIntersection(*symbols, common))) {
			fputs(" /* Not an LL(1)-grammar */", outputFile);
			yyerror("not an LL(1)-grammar\n");
			notLLError = true;
		}
		if (HasProperty("showfirsts")) {
			fputs(" /* firsts: ", outputFile);
			PrintTree(outputFile, ofirsts);
			fputs(" */", outputFile);
		}
		if (symbols != NULL) {
			Unite(symbols, common);
		}
	} else {
		CreateTokenSet2(outputFile, common);
		if (emptyFollow) {
			*usesLA = true;
		}
	}
	KillTree(common, NULL);
}

static void GenerateSingle(Node rule, NameType previous, int indent, Boolean generateCode)
{
    if (generateCode) {
        if (HasProperty("line") && lastLineCopied != rule->lineNr) {
            fprintf(outputFile, "#line %d \"%s\"\n", rule->lineNr, inputFileName);
            lastLineCopied = rule->lineNr;
        }
        if (rule->element.name->nameType == output) {
            doIndent(indent);
            if (strcmp(rule->element.name->name, "'%s'") == 0) {
                fprintf(outputFile, "printf(\"%%s\", lastSymbol);");
            } else {
                fputs("printf(\"", outputFile);
                fwrite(rule->element.name->name + 1, 1, strlen(rule->element.name->name) - 2, outputFile);
                fputs("\");", outputFile);
            }
        } else if (rule->element.name->nameType == code) {
            PrintCode(rule->element.name->name, previous,
                      (rule->next == NULL || rule->next->nodeType != single? string: rule->next->element.name->nameType),
                      indent);
        }
    }
}

static void GenerateRule(Node rule, TREE follow, TREE oneOf, int indent, Boolean generateCode, Boolean guard, TREE firstNext, Boolean *usesLA) {
	NameType previous = string;
	TREE chain1Followers;
	TREE chain2Followers;
	TREE overlap;
	TREE directFollowers;
	Boolean empty1, empty2;

    if (generateCode && HasProperty("debugFF")) {
        doIndent(indent);
        fputs("/* follow: ", outputFile); PrintTree(outputFile, follow);
        fputs(", oneOf: ", outputFile); PrintTree(outputFile, oneOf);
        fputs(", firstNext: ", outputFile); PrintTree(outputFile, firstNext);
        fputs("*/\n", outputFile);
    }
	while (rule != NULL) {
		TREE firsts = NULL;
		TREE followers = NULL;
		TREE lFirstNext = NULL;
		TREE common = NULL;
        TREE sequenceFollowers = NULL;
		(void) CollectFirst(&followers, follow, rule->next);
		empty2 = CollectFirst(&lFirstNext, firstNext, rule->next);
		switch (rule->nodeType) {
			case single:
				if (rule->element.name->nameType == identifier) {
					directFollowers = NULL;
					if (rule->next != NULL) {
						empty1 = CollectFirst(&directFollowers, firstNext, rule->next->next);
					}
					Unite(&directFollowers, lFirstNext);
					if (generateCode) {
						RuleResult res = rule->element.name->ruleResult;
						Name returnType = res == NULL? NULL: res->returnType;
						doIndent(indent);
						openFunctionCall(outputFile, rule->element.name->name, rule->arglist, returnType, rule->assignTo);
                        targetLanguage->nextCallParameter(outputFile);
						if (empty1 || empty2) {
							if (directFollowers == NULL) {
								targetLanguage->nextCallParameterSymbol(outputFile, "follow");
							} else {
								fputs("uniteTokenSets(", outputFile);
                                if (targetLanguage->requiresLocalLTSetStorage) {
                                    targetLanguage->pointerToArray(outputFile, "ltSet", -1);
                                    fputs(", ", outputFile);
                                }
                                fputs("follow, ", outputFile);
                                TokenSetName2(directFollowers);
                                fputs(")", outputFile);
							}
						} else {
							TokenSetName2(directFollowers);
						}
						closeFunctionCall(outputFile);
						fputs(targetLanguage->terminateStatement, outputFile);
					} else {
						if ((empty1 || empty2) && directFollowers != NULL) {
							*usesLA = true;
						}
						CreateTokenSet2(outputFile, directFollowers);
					}
					KillTree(directFollowers, NULL);
				} else if (rule->element.name->nameType == token || rule->element.name->nameType == string) {
					if (generateCode) {
						doIndent(indent);
                        fprintf(outputFile, "%sgetToken(", targetLanguage->owner);
                        targetLanguage->tokenEnum(outputFile, rule->element.name->index, rule->element.name->name);
						fputs(", ", outputFile);
                        TokenSetName2(lFirstNext);
						fprintf(outputFile,", follow)%s", targetLanguage->terminateStatement);
					} else {
						CreateTokenSet2(outputFile, lFirstNext);
					}
				} else if (rule->element.name->nameType == output || rule->element.name->nameType == code) {
					GenerateSingle(rule, previous, indent, generateCode);
				} else {
					fprintf(stderr, "GenerateRule: Unknown nameType: %d\n", rule->element.name->nameType);
				}
				previous = rule->element.name->nameType;
				break;
			case option:
				empty1 = CollectFirst(&firsts, NULL, rule->element.sub);
				CreateGuard(guard, generateCode, true, indent, firsts, false, usesLA);
				if (empty1) Unite(&firsts, followers);
				common = Intersection(firsts, oneOf);
				if (generateCode) {
					doIndent(indent);
				}
				PrintFirstSymbols(firsts, common, ifCondition, indent, true, generateCode, NULL, 0, empty1, usesLA);
				if (generateCode) {
					startBlock(indent);
					overlap = Intersection2(firsts, followers);
					if (overlap != NULL) {
						doIndent(indent + 1);
						fputs("/* Not an LL(1)-grammar */\n", outputFile);
						fprintf(stderr, "%s:%d: not an LL(1)-grammar\n", inputFileName, rule->lineNr);
						KillTree(overlap, NULL);
						notLLError = true;
					}
				}
				GenerateRule(rule->element.sub, followers, common, indent + 1, generateCode, false, lFirstNext, usesLA);
				if (generateCode) {
					terminateBlock(indent);
				}
				break;
			case sequence:
				empty1 = CollectFirst(&firsts, NULL, rule->element.sub);
				Unite(&lFirstNext, firsts); /* a sequence can be followed by itself */
				CreateGuard(guard, generateCode, true, indent, firsts, false, usesLA);
				if (empty1) {
                    Unite(&firsts, followers);
                    sequenceFollowers = firsts;
                } else {
                    Unite(&sequenceFollowers, firsts);
                    Unite(&sequenceFollowers, followers);
                }
				Unite(&common, firsts); Unite(&common, oneOf);
				if (generateCode) {
					startDoWhileLoop(indent);
				}
				GenerateRule(rule->element.sub, sequenceFollowers, common, indent + 1, generateCode, false, lFirstNext, usesLA);
				CreateGuard(true, generateCode, empty2, indent + 1, lFirstNext, false, usesLA);
				if (generateCode) {
					overlap = Intersection2(firsts, followers);
					if (overlap != NULL) {
						doIndent(indent);
						fputs("/* Not an LL(1)-grammar */\n", outputFile);
						fprintf(stderr, "%s:%d: not an LL(1)-grammar\n", inputFileName, rule->lineNr);
						KillTree(overlap, NULL);
						notLLError = true;
					}
				}
				PrintFirstSymbols(firsts, common, endDoWhileLoop, indent, true, generateCode, NULL, 0, false, usesLA);
                if (sequenceFollowers != firsts) {
                    KillTree(sequenceFollowers, NULL);
                }
				break;
			case chain:
				if (generateCode) {
					startUnconditionalLoop(indent);
				}
				chain1Followers = NULL;
				chain2Followers = NULL;
				(void) CollectFirst(&chain2Followers, followers, rule->element.sub->element.sub);
				(void) CollectFirst(&chain1Followers, chain2Followers, rule->element.sub->next->element.sub);
				(void) Unite(&chain1Followers, followers);
				directFollowers = NULL;
				if (CollectFirst(&directFollowers, NULL, rule->element.sub->next->element.sub)) {
					(void) CollectFirst(&directFollowers, NULL, rule->element.sub->element.sub);
				}
				(void) Unite(&directFollowers, lFirstNext);
				GenerateRule(rule->element.sub->element.sub, chain1Followers, idTree, indent + 1, generateCode, true, directFollowers, usesLA);
				KillTree(directFollowers, NULL);
				KillTree(chain1Followers, NULL);
				KillTree(chain2Followers, NULL);
				chain1Followers = NULL;
				(void) CollectFirst(&chain1Followers, followers, rule->element.sub->element.sub);
				(void) CollectFirst(&firsts, chain1Followers, rule->element.sub->next->element.sub);
				KillTree(chain1Followers, NULL);
				if (generateCode) {
					overlap = Intersection2(firsts, followers);
					if (overlap != NULL) {
						doIndent(indent + 1);
						fputs("/* Not an LL(1)-grammar */\n", outputFile);
						fprintf(stderr, "%s:%d: not an LL(1)-grammar\n", inputFileName, rule->lineNr);
						KillTree(overlap, NULL);
						notLLError = true;
					}
					doIndent(indent + 1);
				}
				PrintFirstSymbols(firsts, idTree, ifCondition, indent, false, generateCode, NULL, 0, false, usesLA);
				if (generateCode) {
                    startBlock(indent + 1);
					breakStatement(indent + 2);
					terminateBlock(indent + 1);
				}
				chain1Followers = NULL;
				chain2Followers = NULL;
				(void) CollectFirst(&chain2Followers, NULL, rule->element.sub->next->element.sub);
				(void) Unite(&chain2Followers, followers);
				(void) CollectFirst(&chain1Followers, chain2Followers, rule->element.sub->element.sub);
				directFollowers = NULL;
				if (CollectFirst(&directFollowers, NULL, rule->element.sub->element.sub)) {
					(void) CollectFirst(&directFollowers, NULL, rule->element.sub->next->element.sub);
					Unite(&directFollowers, lFirstNext);
				}
				GenerateRule(rule->element.sub->next->element.sub, chain1Followers, firsts, indent + 1, generateCode, false, directFollowers, usesLA);
				KillTree(directFollowers, NULL);
				KillTree(chain1Followers, NULL);
				KillTree(chain2Followers, NULL);
				if (generateCode) {
                    endUnconditionalLoop(indent);
				}
				break;
			case alternative:
				PrintAlternatives(rule->element.alternatives, followers, oneOf, indent, targetLanguage->errorMsg, targetLanguage->errorArgs, generateCode, lFirstNext, usesLA);
				break;
			default:
				break;
		}
		rule = rule->next;
		KillTree(followers, NULL);
		KillTree(firsts, NULL);
		KillTree(common, NULL);
		KillTree(lFirstNext, NULL);
		oneOf = idTree;
	}
}

static void PrintAlternatives(NodeList pRules, TREE follow, TREE oneOf, int indent, const char *errormsg, const char* errorargs, Boolean generateCode, TREE firstNext, Boolean *usesLA)
{
	int cnt = 0;
	TREE firsts = NULL;
	TREE common;
	TREE symbols = NULL;
	NodeList rules;
	Node rule;

	if (pRules == NULL) {
		yyerror("error in grammar: %s not defined!\n", errormsg);
		return;
	}
	rule = pRules->first;
	if (pRules != NULL && pRules->rest == NULL) {
		GenerateRule(rule, follow, oneOf, indent, generateCode, false, firstNext, usesLA);
	} else {
		Boolean empty = false, empty1;
		for (rules = pRules; rules != NULL; rules = rules->rest) {
			firsts = NULL;
			empty1 = CollectFirst(&firsts, follow, rules->first);
			if (empty1) {
				if (empty) {
					if (generateCode) {
						fputs("/* Not an LL(1)-grammar */\n", outputFile);
					} else if (rules->first == NULL) {
						fprintf(stderr, "%s: not an LL(1)-grammar: no rules?\n", inputFileName);
                    } else {
						fprintf(stderr, "%s:%d: not an LL(1)-grammar\n", inputFileName, rules->first->lineNr);
					}
				}
				empty = true;
			}
			if (generateCode) {
                if (bracesOnNewLine || cnt == 0) {
                    doIndent(indent);
                } else if (!bracesOnNewLine) {
                    fputs(" ", outputFile);
                }
			}
			PrintFirstSymbols(firsts, oneOf, (cnt == 0? ifCondition: elseIfCondition), indent, true, generateCode, &symbols, (rules->first == NULL? 0: rules->first->lineNr), empty1, usesLA);
			if (generateCode) {
				startBlock(indent);
			}
			common = Intersection(firsts, oneOf);
			GenerateRule(rules->first, follow, common, indent + 1, generateCode, false, firstNext, usesLA);
			KillTree(firsts, NULL);
			KillTree(common, NULL);
			cnt++;
			if (generateCode) {
				terminateBlock(indent);
			}
		}
		if (generateCode) {
			if (cnt == 0) {
				fprintf(stderr, "error in grammar: no alternatives for %s!\n", errormsg);
				doIndent(indent);
				fputs("/* NO ALTERNATIVES! */\n", outputFile);
			}
            if (bracesOnNewLine) {
                doIndent(indent);
            } else {
                fputs(" ", outputFile);
            }
            fputs("else", outputFile);
			startBlock(indent);
			doIndent(indent + 1);
            fprintf(outputFile, "%s(%s\"%s\"%s)%s", targetLanguage->errorReporter, targetLanguage->errorReporterArg, errormsg, errorargs, targetLanguage->terminateStatement);
			terminateBlock(indent);
		}
		KillTree(symbols, NULL);
	}
}

static void CreateTokenSets(Name name, void *dummy)
{
#pragma unused(dummy)

	if (name->nameType == identifier)
	{
		PrintAlternatives(name->rules, name->follow, idTree, 1, name->name, "", false, NULL, &name->usesLA);
	}
}

static void CreateProcs(Name name, void *dummy) {
#pragma unused(dummy)
	char errormsg[256];
	Boolean dummyLA;

	if (name->nameType == identifier) {
		RuleResult res = name->ruleResult;
		Name returnVariable = res == NULL? NULL: res->returnVariable;
		Name returnType = res == NULL? NULL: res->returnType;
		Name initialValue = res == NULL? NULL: res->initialValue;
		currProc = name;
		sprintf(errormsg, "%s expected", name->name);
		if (HasProperty("showfollow")) {
			fputs(" /* firsts: ", outputFile);
			PrintTree(outputFile, name->first);
			fputs(", follow: ", outputFile);
			PrintTree(outputFile, name->follow);
			fputs(" */\n", outputFile);
		}
		if (HasProperty("line")) {
			fprintf(outputFile, "#line %d \"%s\"\n", name->lineNr, inputFileName);
		}
        doIndent(globalIndent);
        openFunctionDeclaration(outputFile, name->name, name->arglist, returnType);
        targetLanguage->parameterDeclarationName(outputFile, "follow", 0);
        targetLanguage->nextDeclarationParameterSymbol(outputFile, llTokenSetTypeString, 1);
        closeFunctionDeclaration(outputFile);
        startBlock(globalIndent);
		if (returnType != NULL && returnVariable != NULL) {
            doIndent(globalIndent + 1);
            PrintLocalDeclarationWithInitialization(outputFile, returnType->name, returnVariable->name, initialValue->name);
		}
		if (name->usesLA && targetLanguage->requiresLocalLTSetStorage) {
            doIndent(globalIndent + 1);
            PrintLocalDeclaration(outputFile, llTokenSetTypeString, "ltSet");
		}
		PrintAlternatives(name->rules, name->follow, idTree, globalIndent + 1, name->name, "", true, NULL, &dummyLA);
		if (returnType != NULL && returnVariable != NULL) {
            doIndent(globalIndent + 1);
            PrintReturnValue(outputFile, returnVariable->name);
		}
        doIndent(globalIndent);
		fputs("}\n\n", outputFile);
	}
}

static void ShowAllRules(void)
{
	TREE sym;
	Name name;
	NodeList rules;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
	{
		name = GetKey(sym);
		if (name->nameType == identifier)
		{
			for (rules = name->rules; rules != NULL; rules = rules->rest)
			{
				PrintRule(stdout, name, rules->first);
			}
		}
	}
}

static void DefineProcs(void) {
	TREE sym;
	Name name;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
		name = GetKey(sym);
		if (name->nameType == identifier) {
			RuleResult res = name->ruleResult;
			Name returnType = res == NULL? NULL: res->returnType;
			if (HasProperty("line")) {
				fprintf(outputFile, "#line %d \"%s\"\n", name->lineNr, inputFileName);
			}
            openFunctionDeclaration(outputFile, name->name, name->arglist, returnType);
            targetLanguage->parameterDeclarationName(outputFile, "follow", 0);
            targetLanguage->nextDeclarationParameterSymbol(outputFile, llTokenSetTypeString, 1);
            closeFunctionDeclaration(outputFile);
            fprintf(outputFile, "%s\n", targetLanguage->terminateStatement);
		}
	}
}

static void DefineEnums(void)
{
	TREE sym;
	Name name;
	Boolean prntd = false;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
	{
		name = GetKey(sym);
		if (name->nameType == token && name->index > 0)
		{
			if (prntd) fputs(",\n", outputFile);
			fprintf(outputFile, "	%s = %ld", name->name, name->index);
			prntd = true;
		}
	}
	if (prntd) fputs("\n", outputFile);
}

static Boolean HasEnums(void)
{
	TREE sym;
	Name name;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym))
	{
		name = GetKey(sym);
		if (name->nameType == token && name->index > 0)
		{
			return (true);
		}
	}
	return (false);
}

static void CreateTokenNames(void)
{
	Name name;
	int i;

    PrintArrayDeclaration(outputFile, "static", targetLanguage->stringType, "tokenName", -1, nodeNumber + 2);
	fputs("\n\t\"IGNORE\",\n", outputFile);
	for (i = 1; i <= nodeNumber; i++)
	{
		name = TokenIndexName(i, true);
		if (name != NULL && name->nameType == token)
		{
			fprintf(outputFile, "	\"%s\",\n", name->name);
		}
		else if (name != NULL && name->nameType == string)
		{
			fprintf(outputFile, "	%s,\n", name->name);
		}
		else
		{
			fputs("	\"@&^@#&^\",\n", outputFile);
		}
	}
	fputs("	\"EOF\",\n", outputFile);
	fprintf(outputFile, "%s%s\n", targetLanguage->endOfArrayLiteral, targetLanguage->terminateDeclaration);
}

static long int CountNonEmpty(void *key, void *info)
{
#pragma unused(info)

	return (key == NULL? 0: 1);
}

/* Only called for C/C++ */
static void DefineFunctionList(void) {
	TREE sym;
	Name name;
	int i;
	const char* argType = targetLanguage->stringType;

	if (HasProperty("functions")) {
		indexArray = (Name *) mmalloc(sizeof(Name) * (nodeNumber + 1));
		for (i = 0; i <= nodeNumber; i++) {
			indexArray[i] = NULL;
		}
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
			name = GetKey(sym);
			if (name->nameType == token && name->functions != NULL) {
				TREE function;
				for (function = GetFirstNode(name->functions); function != NULL; function = GetNextNode(function)) {
					if (GetKey(function) != NULL) {
						fprintf(outputFile, "extern int %s(%s);\n", ((Name) GetKey(function))->name, argType);
					}
				}
				fprintf(outputFile, "static Function %sFunctions[] =\n{\n", name->name);
				for (function = GetFirstNode(name->functions); function != NULL; function = GetNextNode(function)) {
					if (GetKey(function) != NULL) {
						fprintf(outputFile, "	{%s, %s},\n", ((Name) GetKey(function))->name, ((Name) GetInfo(function))->name);
					}
				}
				fputs("};\n", outputFile);
				indexArray[name->index] = name;
			}
		}
		fputs("static FunctionList functionList[] = {\n", outputFile);
		for (i = 0; i <= nodeNumber; i++) {
			name = indexArray[i];
			if (name == NULL)
			{
				fputs("	{NULL, 0},\n", outputFile);
			} else {
				fprintf(outputFile, "	{%sFunctions, %ld},\n", name->name, CountTree(name->functions, CountNonEmpty));
			}
		}
		fputs("};\n", outputFile);
	}
}

static void DefineKeywordList(void)
{
	TREE sym;
	Name name;
	int i;
    const char* kwlType = targetLanguage->keyWordListType;

	if (HasProperty("keywords")) {
        indexArray = (Name *) mmalloc(sizeof(Name) * (nodeNumber + 1));
        for (i = 0; i <= nodeNumber; i++) {
            indexArray[i] = NULL;
        }
        for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
            name = GetKey(sym);
            if (name->nameType == token && name->keywords != NULL) {
                if (targetLanguage == &cLanguage || targetLanguage == &cppLanguage) {
                    TREE keyword;
                    fprintf(outputFile, "static KeyWord %sKeyWords[] =\n{\n", name->name);
                    for (keyword = GetFirstNode(name->keywords); keyword != NULL; keyword = GetNextNode(keyword)) {
                        if (GetKey(keyword) != NULL) {
                            fprintf(outputFile, "	{%s, %s},\n", ((Name) GetKey(keyword))->name, ((Name) GetInfo(keyword))->name);
                        }
                    }
                    fputs("};\n", outputFile);
                }
                indexArray[name->index] = name;
            }
        }
        if (targetLanguage == &rustLanguage) {
            for (i = 0; i <= nodeNumber; i++) {
                name = indexArray[i];
                if (name != NULL) {
                    TREE keyword;
                    fprintf(outputFile, "    let mut keyword_list_%d = HashMap::<String, u32>::new();\n", i);
                    for (keyword = GetFirstNode(name->keywords); keyword != NULL; keyword = GetNextNode(keyword)) {
                        Name key = (Name) GetKey(keyword);
                        if (key != NULL) {
                            fprintf(outputFile, "    keyword_list_%d.insert(%s.to_string(), %ld);\n", i, key->name, ((Name) GetInfo(keyword))->index);
                        }
                    }
                    fprintf(outputFile, "    keyword_list.insert(%d, keyword_list_%d);\n", i, i);
                }
            }
        } else {
            PrintArrayDeclaration(outputFile, "static", kwlType, "keywordList", -1, -1);
            fputc('\n', outputFile);
            for (i = 0; i <= nodeNumber; i++) {
                name = indexArray[i];
                fputs("\t", outputFile);
                if (name == NULL) {
                    fputs(targetLanguage->emptyKeyWordListEntry, outputFile);
                } else {
                    if (targetLanguage == &cLanguage || targetLanguage == &cppLanguage) {
                        fprintf(outputFile, "{%sKeyWords, %ld}", name->name, CountTree(name->keywords, CountNonEmpty));
                    } else {
                        TREE keyword;
                        Boolean first = true;
                        fputs("{", outputFile);
                        for (keyword = GetFirstNode(name->keywords); keyword != NULL; keyword = GetNextNode(keyword)) {
                            if (GetKey(keyword) != NULL) {
                                if (!first)
                                    fputs(", ", outputFile);
                                fprintf(outputFile, "%s: %ld", ((Name) GetKey(keyword))->name, ((Name) GetInfo(keyword))->index);
                                first = false;
                            }
                        }
                        fputs("}", outputFile);
                    }
                }
                fputs(",\n", outputFile);
            }
            fprintf(outputFile, "%s%s\n", targetLanguage->endOfArrayLiteral, targetLanguage->terminateDeclaration);
        }
    }
}

static void OpenSkeleton(const char* argv0) {
    char llgenFileName[256];

    strcpy(llgenFileName, "llgen.skel.");
    strcpy(llgenFileName + 11, targetLanguage->extension);
    if ((skeletonFile = fopen(llgenFileName, "r")) == NULL) {
        char namebuf[1024];
        char* tail;
        strcpy(namebuf, argv0);
        if ((tail = strrchr(namebuf, DIR_SEP_CHAR)) != NULL) {
            strcpy(tail + 1, llgenFileName);
            skeletonFile = fopen(namebuf, "r");
        }
    }
    if (skeletonFile == NULL) {
        perror("skeleton file");
        exit(1);
    }
}

static void CopySkeleton(const char* argv0) {
	char buf[256];
	int nrIgnoreLevels = 0;
    int lineNr = 0;

	while (fgets(buf, sizeof(buf), skeletonFile) != NULL) {
        lineNr++;
		if (buf[0] == '%') {
			if (buf[1] == '%') {
				if (nrIgnoreLevels == 0) {
					InterpretLine(buf + 2);
				}
			} else if (buf[1] == '{') {
				buf[strlen(buf) - 1] = '\0';
				if (!HasProperty(buf + 2)) {
                    if (nrIgnoreLevels == 0 && HasProperty("debugskel")) {
                        fprintf(stderr, "%d: start ignore: %s\n", lineNr, buf + 2);
                    }
                    nrIgnoreLevels++;
				} else if (nrIgnoreLevels == 0 && HasProperty("debugskel")) {
                    fprintf(stderr, "%d: accept: %s\n", lineNr, buf + 2);
                }
			} else if (buf[1] == '}') {
				buf[strlen(buf) - 1] = '\0';
				if (!HasProperty(buf + 2)) {
					nrIgnoreLevels--;
                    if (nrIgnoreLevels < 0 && HasProperty("debugskel")) {
                        fprintf(stderr, "%d: nrIgnoreLevels < 0\n", lineNr);
                    } else if (nrIgnoreLevels == 0 && HasProperty("debugskel")) {
                        fprintf(stderr, "%d: end ignore: %s\n", lineNr, buf + 2);
                    }
				} else if (nrIgnoreLevels == 0 && HasProperty("debugskel")) {
                    fprintf(stderr, "%d: end accept: %s\n", lineNr, buf + 2);
                }
            } else if (buf[1] == '=') {
                if (nrIgnoreLevels == 0) {
                    buf[strlen(buf) - 1] = '\0';
                    switch (buf[2]) {
                        case 'T':
                            strcpy(llTokenSetTypeString, buf + 3);
                            break;
                    }
                }
			} else if (buf[1] != '#') {
				if (nrIgnoreLevels == 0) {
					buf[strlen(buf) - 1] = '\0';
					if (strcmp(buf + 1, "initcode") == 0) {
						Node cPtr = preCodeBlocks;
						int pLineNr = 0;
						while (cPtr != NULL) {
                            const char* txt = (const char*) cPtr->element.name;
                            if (txt[0] != '%') {
                                if (cPtr->lineNr != pLineNr && HasProperty("line")) {
                                    fprintf(outputFile, "#line %d \"%s\"\n", cPtr->lineNr, inputFileName);
                                    pLineNr = cPtr->lineNr;
                                    lastLineCopied = pLineNr;
                                }
                                fputs(txt, outputFile);
                            }
							cPtr = cPtr->next;
						}
                    } else if (strncmp(buf + 1, "code", 4) == 0) {
						Node cPtr = preCodeBlocks;
						int pLineNr = 0;
                        char blockNr = buf[5];
						while (cPtr != NULL) {
                            const char* txt = (const char*) cPtr->element.name;
                            if (txt[0] == '%' && txt[1] == blockNr && isspace(txt[2])) {
                                if (cPtr->lineNr != pLineNr && HasProperty("line")) {
                                    fprintf(outputFile, "#line %d \"%s\"\n", cPtr->lineNr, inputFileName);
                                    pLineNr = cPtr->lineNr;
                                    lastLineCopied = pLineNr;
                                }
                                fputs(txt + 3, outputFile);
                            }
							cPtr = cPtr->next;
						}
					} else if (strcmp(buf + 1, "exitcode") == 0) {
						Node cPtr = postCodeBlocks;
						int pLineNr = 0;
						while (cPtr != NULL) {
							if (cPtr->lineNr != pLineNr && HasProperty("line")) {
								fprintf(outputFile, "#line %d \"%s\"\n", cPtr->lineNr, inputFileName);
								pLineNr = cPtr->lineNr;
								lastLineCopied = pLineNr;
							}
							fputs((char *) cPtr->element.name, outputFile);
							cPtr = cPtr->next;
						}
					} else if (strcmp(buf + 1, "enum") == 0) {
						DefineEnums();
					} else if (strcmp(buf + 1, "prototypes") == 0) {
						DefineProcs();
					} else if (strcmp(buf + 1, "keywordlist") == 0) {
						DefineKeywordList();
					} else if (strcmp(buf + 1, "functionlist") == 0) {
						DefineFunctionList();
					} else if (strcmp(buf + 1, "scantable") == 0) {
						if (dfa == NULL) fprintf(stderr, "Error in skeleton file: scantable not preceded by tokenset\n");
                        if (targetLanguage == &cLanguage || targetLanguage == &cppLanguage || targetLanguage == &goLanguage || targetLanguage == &rustLanguage) {
                            RenumberRegExp(dfa);
                        }
						FormatRegExp(outputFile, dfa);
					} else if (strcmp(buf + 1, "tokensets") == 0) {
						if (dfa == NULL) CompileScanner();
						FormatTokenSets(outputFile, dfa);
						CreateTokenNames();
						CheckOverlap(dfa);
					} else if (strcmp(buf + 1, "parsefunctions") == 0) {
						TraverseTreeInOrder(idTree, (TreeProc) CreateTokenSets);
						fputc('\n', outputFile);
						TraverseTreeInOrder(idTree, (TreeProc) CreateProcs);
					} else if (strcmp(buf + 1, "createtokensets") == 0) {
						TraverseTreeInOrder(idTree, (TreeProc) CreateTokenSets);
					} else if (strcmp(buf + 1, "parsefunctionbodies") == 0) {
						TraverseTreeInOrder(idTree, (TreeProc) CreateProcs);
					} else {
						fprintf(stderr, "Error in skeleton file: %s\n", buf);
					}
				}
			}
		} else if (nrIgnoreLevels == 0) {
			fputs(buf, outputFile);
		}
	}
}

static void determineBaseName(const char* str) {
    const char* lastSlash = strrchr(str, '/');
    const char* fileName = lastSlash != NULL? lastSlash + 1: str;
    const char* lastDot = strchr(fileName, '.');
    const int baseNameLength = lastDot == NULL? strlen(fileName): lastDot - fileName;

    baseName = mmalloc(baseNameLength + 1);
    strncpy(baseName, fileName, baseNameLength);
    baseName[baseNameLength] = '\0';
}

int main(int argc, char *argv[]) {
	char* argv0;

	argv0 = argv[0];
	argv++;
	while (argc > 1 && ((**argv == '-' || **argv == '+') || strchr(*argv, '='))) {
        if (**argv == '-') {
            RetractProperty(*argv + 1);
        } else if (**argv == '+') {
            AddProperty(*argv + 1);
        } else {
            const char* eq = strchr(*argv, '=');
            if (strncmp(*argv, "packageName", eq - *argv) == 0) {
                packageName = eq + 1;
            } else {
                fprintf(stderr, "unknown option: %s", *argv);
            }
        }
		argc--;
		argv++;
	}
    if (HasProperty("cpp")) {
        targetLanguage = &cppLanguage;
    } else if (HasProperty("js")) {
        targetLanguage = &jsLanguage;
    } else if (HasProperty("ts")) {
        targetLanguage = &tsLanguage;
    } else if (HasProperty("go")) {
        targetLanguage = &goLanguage;
    } else if (HasProperty("rust")) {
        targetLanguage = &rustLanguage;
    } else {
        targetLanguage = &cLanguage;
        AddProperty("c");
    }
    targetLanguage->initialize();
	argv--;
	if (argc >= 2) {
		if (strcmp(argv[1], "-") == 0) {
			yyin = stdin;
		} else {
			if ((yyin = fopen(argv[1], "r")) == NULL) {
				perror(argv[1]);
				exit(1);
			}
			inputFileName = argv[1];
            determineBaseName(inputFileName);
		}
		if (argc >= 3) {
			if (strcmp(argv[2], "-") != 0) {
				if ((outputFile = fopen(argv[2], "w")) == NULL) {
					perror(argv[2]);
					exit(1);
				}
			} else {
				outputFile = stdout;
			}
		} else {
			char outName[256];
			char *suffix;
			const char* extension = targetLanguage->extension;
			strcpy(outName, argv[1]);
			suffix = strrchr(outName, '.');
			if (suffix != NULL && (strcmp(suffix, ".l") == 0 || strcmp(suffix, ".L") == 0 || strcmp(suffix, ".ll") == 0 || strcmp(suffix, ".LL") == 0)) {
				strcpy(suffix + 1, extension);
			} else {
				strcat(outName, ".");
				strcat(outName, extension);
			}
			if ((outputFile = fopen(outName, "w")) == NULL) {
				perror(outName);
				exit(1);
			}
		}
	}
	errorOccurred = false;
	yyparse();
	if (errorOccurred) {
		exit(1);
	}
	if (HasEnums() != 0) {
		AddProperty("enumid");
	}
	ComputeEmptyFirstFollow();
    if (HasProperty("showallrules")) {
        ShowAllRules();
    } else {
        OpenSkeleton(argv0);
        CopySkeleton(argv0);
    }
    if (HasProperty("tab8")) {
        tabWidth = 8;
    }
    if (HasProperty("newlineBeforeBrace")) {
        bracesOnNewLine = true;
    }
	if (yyin != stdin) {
		fclose(yyin);
	}
	if (outputFile != stdout) {
		fclose(outputFile);
	}
	if (skeletonFile != NULL) {
		fclose(skeletonFile);
	}
	return 0;
}

void yyerror(char *format, ...) { /* Called by yyparse on error */
	va_list args;

	errorOccurred = true;
	(void) fprintf(stderr, "%s:%d: ", inputFileName, lineNumber);
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
}

int yywrap() {
	return (1);
}
