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
#include "gram2.tab.h"
#include "regexp.h"
#include "lang.h"


// TODO: if (HasProperty("regexpcontext")) {

void PrintRule2(FILE *f, Node rule);
void PrintRule(FILE *f, Name lhs, Node rule);
static void PrintAlternatives(Name ruleName, NodeList rules, Boolean emptyToEndOfRule, TREE follow, int indent, const char *errormsg, const char *errorargs, Boolean generateCode, TREE firstNext, Boolean *usesLA, int startLineNr);
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

void AddProperty(char *property) {
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

int nrParameters(Node argList) {
	int level = 0;
	int nrParameters = argList == NULL? 0: 1;

	for (Node nPtr = argList; nPtr != NULL; nPtr = nPtr->next) {
        const char *str = (const char *) nPtr->element.name;
        if (strcmp(str, ",") == 0 && level == 0) {
			nrParameters++;
        } else {
            if (strcmp(str, "{") == 0 || strcmp(str, "(") == 0 || strcmp(str, "[") == 0) {
                level++;
            } else if (strcmp(str, "}") == 0 || strcmp(str, ")") == 0 || strcmp(str, "]") == 0) {
                level--;
            }
        }
	}
	return nrParameters;
}

static int openFunctionCall(FILE *f, const char* functionName, Node argList, Name returnType, Name assignTo, Name ruleName) {
	int level = 0;
	int nrParameters = argList == NULL? 0: 1;
	Boolean assignToRuleOutput = assignTo != NULL && ruleName->ruleResult != NULL && ruleName->ruleResult->returnVariable != NULL && strcmp(assignTo->name, ruleName->ruleResult->returnVariable->name) == 0;

    fputs(targetLanguage->owner, outputFile);
    targetLanguage->startFunctionCall(f, functionName, returnType == NULL || assignToRuleOutput? NULL: returnType->name, assignTo == NULL? NULL: assignTo->name);
	for (Node nPtr = argList; nPtr != NULL; nPtr = nPtr->next) {
        const char *str = (const char *) nPtr->element.name;
        /* Spaces from the input */
		for (int i = nPtr->lineNr; i != 0; i--) {
			fputc(' ', f);
		}
        if (strcmp(str, ",") == 0 && level == 0) {
            targetLanguage->nextCallParameter(f);
			nrParameters++;
        } else {
            targetLanguage->nextCallParameterSymbol(f, str);
            if (strcmp(str, "{") == 0 || strcmp(str, "(") == 0 || strcmp(str, "[") == 0) {
                level++;
            } else if (strcmp(str, "}") == 0 || strcmp(str, ")") == 0 || strcmp(str, "]") == 0) {
                level--;
            }
        }
	}
	return nrParameters;
}

static void closeFunctionCall(FILE *f) {
    targetLanguage->endFunctionCall(f);
}

static int IntCmp(void *k1, void *k2) {
	return (k1 < k2? -1: k1 == k2? 0: 1);
}

static void printTokenSet(FILE *f, TREE tokenSet) {
	TREE node;
	int lastBoundary = 0;
	unsigned long int cValue = 0;
	TREE tokens = NULL;

	for (node = GetFirstNode(tokenSet); node != NULL; node = GetNextNode(node)) {
		Name name = (Name) GetKey(node);
		long int tokenIndex = name->index;
		if (tokenIndex == -1) tokenIndex = 0;
		InsertNodeSingle(&tokens, (void *) tokenIndex, NULL, IntCmp);
	}
	node = GetFirstNode(tokens);
	while (lastBoundary <= nodeNumber) {
		if (node != NULL) {
			int tokenIndex = (int) GetKey(node);
			while (tokenIndex >= lastBoundary + nrBitsInElement) {
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
	for (node = GetFirstNode(tokenSet); node != NULL; node = GetNextNode(node)) {
		Name name = (Name) GetKey(node);
		fprintf(f, " %s", name->name);
	}
	fputs(" */\n", f);
	KillTree(tokens, NULL);
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
                    case 'F': {
                            str++;
                            fputs(targetLanguage->startOfArrayLiteral, outputFile);
							printTokenSet(outputFile, startSymbol->first);
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
	name->nrParameters = 0;
	name->isTerminal = false;
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

Node MakeNode(short int lineNr, NodeType nodeType, void *element, Node next) {
	Node node;

	if (oldNodes == NULL) {
		node = (Node) mmalloc(sizeof(struct Node));
	} else {
		node = oldNodes;
		oldNodes = oldNodes->next;
	}
	node->nodeType = nodeType;
	node->arglist = NULL;
	switch (nodeType) {
		case single: node->element.name = element; break;
		case option: node->element.sub = element; break;
		case sequence: node->element.sub = element; break;
		case chain: node->element.sub = element; break;
		case alternative: node->element.alternatives = element; break;
		case breakToken: node->element.name = element; break;
		case errorMessage: node->element.sub = element; break;
		// default: fprintf(stderr, "MakeNode: unknown nodetype %d\n", nodeType);
	}
	node->lineNr = lineNr;
	node->next = next;
	node->preferShift = false;
	node->empty = false;
	node->assignTo = NULL;
	return (node);
}

NodeList MakeNodeList(Node first, NodeList rest) {
	NodeList nodeList = (NodeList) mmalloc(sizeof(struct NodeList));

	nodeList->first = first;
	nodeList->rest = rest;
	nodeList->empty = false;
	return (nodeList);
}

NodeList appendNodeList(NodeList list, NodeList app) {
	if (list == NULL) {
		return app;
	} else {
		NodeList ptr = list;
		while (ptr->rest != NULL) {
			ptr = ptr->rest;
		}
		ptr->rest = app;
		return list;
	}
}

static Boolean nodeContainsGrammarSymbol(Node node) {
	while (node != NULL) {
		switch (node->nodeType) {
			case single:
				if (node->element.name->nameType != code && node->element.name->nameType != output) {
					return true;
				}
				break;
			case option:
				if (nodeContainsGrammarSymbol(node->element.sub)) {
					return true;
				}
				break;
			case sequence:
				if (nodeContainsGrammarSymbol(node->element.sub)) {
					return true;
				}
				break;
			case chain:
				if (nodeContainsGrammarSymbol(node->element.sub->element.sub) ||
					  nodeContainsGrammarSymbol(node->element.sub->next->element.sub)) {
					return true;
				}
			case alternative:
				if (containsGrammarSymbol(node->element.alternatives)) {
					return true;
				}
				break;
			case breakToken:
			case errorMessage:
				return true;
		}
		node = node->next;
	}
	return false;
}

Boolean containsGrammarSymbol(NodeList list) {
	NodeList ptr = list;

	while (ptr != NULL) {
		if (nodeContainsGrammarSymbol(ptr->first)) {
			return true;
		}
		ptr = ptr->rest;
	}
	return false;
}

Node AppendNode(Node node1, Node node2) {
	Node ret = node1;

	if (node1 == NULL) {
		return (node2);
	}
	while (node1->next != NULL) {
		node1 = node1->next;
	}
	node1->next = node2;
	return (ret);
}

/*
 * Returns true when a rule is empty, but also reorders alternatives so that an
 * empty alternative will go to the end. That generates slightly nicer code by
 * making the empty alternative unconditional, which also avoids a problem with
 * checking first vs follow.`
 */
static Boolean determineEmptyRule(Node rule) {
	Boolean empty = true, firstPrinted;
	NodeList* rules, emptyRule;

	if (rule == NULL || rule->empty) {
		return true;
	}
	for (Node ptr = rule; ptr != NULL; ptr = ptr->next) {
		switch (ptr->nodeType) {
			case single: /* Only if identifier produces empty, not if string; ignore code and output */
				if (!((ptr->element.name->nameType == identifier && ptr->element.name->empty) ||
					  ptr->element.name->nameType == output ||
					  ptr->element.name->nameType == code)) {
					empty = false;
				}
				break;
			case option: /* Bien sure */
				break;
			case sequence: /* Only if subpart is empty */
				if (!determineEmptyRule(ptr->element.sub)) {
					empty = false;
				}
				break;
			case chain: /* Only if first part of chain is empty, second doesn't matter */
				if (!determineEmptyRule(ptr->element.sub->element.sub)) {
					empty = false;
				}
				break;
			case alternative: /* Only if one of the alternatives is empty */
				firstPrinted = false;
				rules = &ptr->element.alternatives;
				emptyRule = NULL;
				if (!ptr->element.alternatives->empty) {
					while (*rules != NULL) {
						if (determineEmptyRule((*rules)->first)) {
							if (emptyRule != NULL) {
								if (!firstPrinted) {
									if (emptyRule->first != NULL) {
										fprintf(stderr, "%s:%d:error: more than one empty alternative\n",
												inputFileName, emptyRule->first->lineNr);
									} else {
										fprintf(stderr, "%s:%d:error: more than one empty alternative following this line\n",
												inputFileName, ptr->lineNr);
									}
									firstPrinted = true;
								}
								if ((*rules)->first != NULL) {
									fprintf(stderr, "%s:%d:error: and here\n",
											inputFileName, (*rules)->first->lineNr);
								} else {
									fprintf(stderr, "%s:%d:error: and following this line\n",
											inputFileName, ptr->lineNr);
								}
								notLLError = true;
								rules = &(*rules)->rest;
							} else {
								// Take this out of the list and move it to the end
								emptyRule = *rules;
								*rules = emptyRule->rest;
								emptyRule->rest = NULL;
							}
						} else {
							rules = &(*rules)->rest;
						}
					}
					if (emptyRule != NULL) {
						// Reorder such that emptyRule goes to the end.
						*rules = emptyRule;
						ptr->element.alternatives->empty = true;
					} else {
						empty = false;
					}
				}
				break;
			case breakToken:
			case errorMessage:
				break;
			// default:
			// 	fprintf(stderr, "determineEmptyRule: unknown nodetype %d\n", ptr->nodeType);
			// 	return empty;
		}
	}
	rule->empty = empty;
	return empty;
}

static void determineEmpty(Name name) {
	NodeList rules = name->rules;
	Boolean empty = false;

	if (!name->empty && name->nameType == identifier) {
		while (rules != NULL) {
			if (determineEmptyRule(rules->first)) {
				empty = true;
			}
			rules = rules->rest;
		}
		if (empty) {
			name->empty = true;
			changes = true;
		}
	}
}

int NameCmpStr(Name str1, char *str2) {
	return (strcmp(str1->name, str2));
}

int NameCmp(Name str1, Name str2) {
	return (str1 == NULL? (str2 == NULL? 0: -1): (str2 == NULL? 1: strcmp(str1->name, str2->name)));
}

static Boolean InsertElement(TREE *first, Name name) {
	Name sym = GetInfo(FindNode(*first, name, (TreeCmp) NameCmp));

	if (sym == NULL) {
		InsertNodeSingle(first, name, name, (TreeCmp) NameCmp);
		return (true);
	} else {
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
	while (level != 0) {
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

static void PrintLocalDeclarationWithInitialization(FILE* f, const Name type, const Name name, const Name init) {
    targetLanguage->localDeclarationWithInitialization(f, type->name, name->name, init != NULL? init->name: NULL);
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

/**
 * Collects the first symbols of rule, and unites them with follow if the
 * rule can be empty. Returns true when rule can be empty.
 */
static Boolean collectFirst(TREE *firsts, TREE follow, Node rule) {
	NodeList rules;
	Boolean empty = true;
	TREE sym;
	Boolean emptyAlternative;

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
				(void) collectFirst(firsts, NULL, rule->element.sub);
				break;
			case sequence:
				empty = collectFirst(firsts, NULL, rule->element.sub);
				break;
			case chain:
				empty = collectFirst(firsts, NULL, rule->element.sub->element.sub);
				if (empty) /* Now also inspect the other chain part, but it remains empty */ {
					(void) collectFirst(firsts, NULL, rule->element.sub->next->element.sub);
				}
				break;
			case alternative: /* Try each one... */
				emptyAlternative = false;
				for (rules = rule->element.alternatives; rules != NULL; rules = rules->rest) {
					if (collectFirst(firsts, NULL, rules->first)) {
						emptyAlternative = true;
					}
				}
				if (!emptyAlternative) {
					empty = false;
				}
				break;
			case breakToken:
			case errorMessage:
				break;
			default:
				fprintf(stderr, "collectFirst: unknown nodetype %d\n", rule->nodeType);
				return (empty);
		}
		rule = rule->next;
	}
	if (empty) {
		Unite(firsts, follow);
	}
	return empty;
}

static void collectFirstOn(TREE *firsts, Node rule) {
	Name name;

	switch (rule->nodeType) {
		case breakToken:
			name = rule->element.name;
			break;
		case errorMessage:
			name = rule->element.sub->element.name;
			break;
		default:
			fprintf(stderr, "collectFirstOn: unknown nodetype %d\n", rule->nodeType);
			return;
	}
	if (name->nameType == identifier) {
		fprintf(stderr, "%d: on non-terminal\n", rule->lineNr);
	} else if (name->nameType == token || name->nameType == string) {
		(void) InsertElement(firsts, name);
	}
}

static void DetermineFirst(Name name) {
	NodeList rules = name->rules;
	TREE firsts = NULL;

	while (rules != NULL) {
		(void) collectFirst(&firsts, NULL, rules->first);
		rules = rules->rest;
	}
	if (Unite(&name->first, firsts)) {
		changes = true;
	}
	KillTree(firsts, NULL);
}

static void DetermineFollowRule(TREE *follow, Node rule, Name orig, TREE followers) {
	NodeList rules;
	TREE nFollowers;
	TREE chain1EltFollowers;
	TREE chain2EltFollowers;
    TREE sequenceFirstsAndFollowers;

	for (; rule != NULL; rule = rule->next) {
		if (rule->nodeType != single || rule->element.name == orig) {
			nFollowers = NULL;
			(void) collectFirst(&nFollowers, followers, rule->next);
			switch (rule->nodeType) {
				case single: /* && rule->element.name == orig */
					if (Unite(follow, nFollowers)) {
						changes = true;
					}
					break;
				case option:
					DetermineFollowRule(follow, rule->element.sub, orig, nFollowers);
					break;
				case sequence:
                    sequenceFirstsAndFollowers = NULL;
                    (void) collectFirst(&sequenceFirstsAndFollowers, NULL, rule->element.sub);
                    Unite(&sequenceFirstsAndFollowers, nFollowers);
					DetermineFollowRule(follow, rule->element.sub, orig, sequenceFirstsAndFollowers);
                    KillTree(sequenceFirstsAndFollowers, NULL);
					break;
				case chain:
					/*	A CHAIN B, C
						FOLLOW(A) = nFollowers + FIRST(B) + (EMPTY(B)? FIRST(A) + ...: EMPTY)
						FOLLOW(B) = FIRST(A) + (EMPTY(A)? FIRST(B) + nFollowers: EMPTY) */
					chain1EltFollowers = NULL;
					chain2EltFollowers = NULL;
					(void) collectFirst(&chain2EltFollowers, nFollowers, rule->element.sub->element.sub);
					/* chain2EltFollowers = FIRST(A) + (EMPTY(A)? nFollowers: EMPTY) */
					(void) collectFirst(&chain1EltFollowers, chain2EltFollowers, rule->element.sub->next->element.sub);
					/* chain1EltFollowers = FIRST(B) + (EMPTY(B)? chain2EltFollowers: EMPTY) */
					(void) Unite(&chain1EltFollowers, nFollowers);
					/* chain1EltFollowers = nFollowers + FIRST(B) + ... */
					DetermineFollowRule(follow, rule->element.sub->element.sub, orig, chain1EltFollowers);
					KillTree(chain1EltFollowers, NULL);
					KillTree(chain2EltFollowers, NULL);
					chain1EltFollowers = NULL;
					chain2EltFollowers = NULL;
					(void) collectFirst(&chain2EltFollowers, NULL, rule->element.sub->next->element.sub);
					/* chain2EltFollowers = FIRST(B) */
					(void) Unite(&chain2EltFollowers, nFollowers);
					/* chain2EltFollowers = FIRST(B) + nFollowers */
					(void) collectFirst(&chain1EltFollowers, chain2EltFollowers, rule->element.sub->element.sub);
					/* chain1EltFollowers = FIRST(A) + (EMPTY(A)? chain2EltFollowers: EMPTY) */
					DetermineFollowRule(follow, rule->element.sub->next->element.sub, orig, chain1EltFollowers);
					KillTree(chain1EltFollowers, NULL);
					KillTree(chain2EltFollowers, NULL);
					break;
				case alternative: /* Try each one... */
					for (rules = rule->element.alternatives; rules != NULL; rules = rules->rest) {
						DetermineFollowRule(follow, rules->first, orig, nFollowers);
					}
					break;
				case breakToken:
				case errorMessage:
					// Just empty productions
					break;
				// default:
				// 	fprintf(stderr, "DetermineFollowRule: unknown nodetype %d", rule->nodeType);
				// 	return;
			}
			KillTree(nFollowers, NULL);
		}
	}
}

static void DetermineFollow(Name name) {
	TREE sym;
	Name lhs;
	NodeList rules;

	if (name->nameType == identifier) {
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
			lhs = GetKey(sym);
			for (rules = lhs->rules; rules != NULL; rules = rules->rest) {
				DetermineFollowRule(&name->follow, rules->first, name, lhs->follow);
			}
		}
	}
}

static void ComputeEmptyFirstFollow(void) {
	TREE sym;

	/* Compute empty productions */	
	do {
		changes = false;
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
			determineEmpty(GetKey(sym));
		}
	}
	while (changes);
	/* Compute first sets */
	do {
		changes = false;
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
			DetermineFirst(GetKey(sym));
		}
	}
	while (changes);
	/* Compute follow sets */
	(void) InsertElement(&startSymbol->follow, endOfInput);
	do {
		changes = false;
		for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
			DetermineFollow(GetKey(sym));
		}
	}
	while (changes);
}

static void RenumberRegExp(RegExp regExp) {
	Transition tr, tr2;
	ESet transitions;
	int mCount = CharSetSize(), count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;
	int sNum = 0;

	while (regExp != NULL) {
		mState = NULL;
		if (regExp->transitions == NULL) {
			regExp->state.number = -1;
		} else {
			regExp->state.number = sNum;
			transitions = NULL;
			tCount = CharSetSize();
			for (i = 0; i != tCount; i++) cSet[i] = false;
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (!cSet[tr->transitionChar]) {
					mCount--;
					cSet[tr->transitionChar] = true;
					tCount--;
				}
			}
			mCount = tCount;
			/* mCount is nr. of fails, mState == NULL, the fail state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (tr->destination != mState) /* not the most efficient check, but ok */
				{
					for (count = 0, tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination) count++;
					if (count > mCount) {
						mCount = count;
						mState = tr->destination;
					}
				}
			}
			/* mCount is max. nr. of transitions to the same state, mState is that state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (tr->destination != mState && !SetMember(tr->destination, transitions)) {
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination)
							sNum++;
				}
			}
			if (mState == NULL) {
				sNum++;
			} else {
				if (tCount != 0) {
					for (i = 0; i != CharSetSize(); i++) {
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

static int TokenSetCmp(void *tSet1, void *tSet2) {
	ESet tokenSet1 = tSet1;
	ESet tokenSet2 = tSet2;

	while (tokenSet1 != NULL && tokenSet2 != NULL) {
		Answer answer1 = (Answer) tokenSet1->element;
		Answer answer2 = (Answer) tokenSet2->element;
		if (answer1->info > answer2->info) {
			return (1);
		} else if (answer1->info < answer2->info) {
			return (-1);
		}
		tokenSet1 = tokenSet1->next;
		tokenSet2 = tokenSet2->next;
	}
	return (tokenSet1 != NULL? 1: tokenSet2 != NULL? -1: 0);
}

static void CreateTokenSet(FILE *f, ESet tokenSet) {
	TREE node = FindNode(tokenSets, tokenSet, TokenSetCmp);
	TREE tokens = NULL;
	int lastBoundary = 0;
	unsigned long int cValue = 0;

	if (node == NULL) {
		tokenSetNr++;
		InsertNodeSingle(&tokenSets, tokenSet, (void *) tokenSetNr, TokenSetCmp);
        targetLanguage->arrayDeclaration(f, "static", targetLanguage->tokenSetType,
                                         "llTokenSet", tokenSetNr, (nodeNumber + 1) / nrBitsInElement + 1, true);
		while (tokenSet != NULL) {
			InsertNodeSingle(&tokens, ((Answer) tokenSet->element)->info, NULL, IntCmp);
			tokenSet = tokenSet->next;
		}
        node = GetFirstNode(tokens);
        while (lastBoundary <= nodeNumber) {
            if (node != NULL) {
                int tokenIndex = (int) GetKey(node);
                if (tokenIndex == -1) tokenIndex = 0;
                while (tokenIndex >= lastBoundary + nrBitsInElement) {
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

static void FormatTokenSets(FILE *f, RegExp dfa) {
	RegExp regExp;

	for (regExp = dfa; regExp != NULL; regExp = regExp->next) {
		if (regExp->info != NULL) {
			CreateTokenSet(f, regExp->info);
		}
	}
}

static char *TokenSetName(RegExp state) {
	static char tokenSetName[16];
	TREE node;

	if (state == NULL || state->info == NULL || (node = FindNode(tokenSets, state->info, TokenSetCmp)) == NULL) {
		strcpy(tokenSetName, targetLanguage->nullValue);
	} else {
		sprintf(tokenSetName, "llTokenSet%d", (int) GetInfo(node));
	}
	return (tokenSetName);
}

static void FormatRegExp(FILE *f, RegExp dfa) {
	Transition tr, tr2;
	ESet transitions;
	int mCount = CharSetSize(), count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;
	Boolean prntd = false;
	RegExp regExp;
	int sNum = 0;

	for (regExp = dfa; regExp != NULL; regExp = regExp->next) {
		if (prntd) fputs("\n", f);
		if (regExp->transitions != NULL) {
            PrintStateTransitionStart(f, regExp->state.number);
			prntd = true;
			mState = NULL;
			transitions = NULL;
			tCount = CharSetSize();
			for (i = 0; i != tCount; i++) cSet[i] = false;
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (!cSet[tr->transitionChar]) {
					cSet[tr->transitionChar] = true;
					tCount--;
				}
			}
			mCount = tCount;
			/* mCount is nr. of fails, mState == NULL, the fail state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (tr->destination != mState) /* not the most efficient check, but ok */
				{
					for (count = 0, tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination) count++;
					if (count > mCount) {
						mCount = count;
						mState = tr->destination;
					}
				}
			}
			/* mCount is max. nr. of transitions to the same state, mState is that state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (tr->destination != mState && !SetMember(tr->destination, transitions)) {
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next) {
						if (tr2->destination == tr->destination) {
							if (prntd) prntd = false;
							else fputs("		  ", f);
                            PrintStateTransition(f, tr2->transitionChar, tr->destination->state.number, TokenSetName(tr->destination));
							sNum++;
						}
					}
				}
			}
			if (mState == NULL) {
				if (prntd) prntd = false;
				else fputs("		  ", f);
                PrintStateTransition(f, -1, -1, targetLanguage->nullValue);
				sNum++;
			} else {
				if (tCount != 0) {
					for (i = 0; i != CharSetSize(); i++) {
						if (!cSet[i]) {
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

static void regexpcopy(char *copy, char *regexp) {
	regexp++;
	while (*regexp != '\0') {
		if (*regexp == '"') {
			if (regexp[1] != '\0') {
				yyerror("quote error in regexp\n");
				errorOccurred = true;
			}
			*copy = '\0';
			return;
		} else if (*regexp == '\\') {
			if (regexp[1] == '\0') {
				yyerror("escape error in regexp\n");
				errorOccurred = true;
				regexp++;
			} else if (regexp[1] == '"') {
				*copy++ = '"';
				regexp += 2;
			} else if (regexp[1] == '\\') {
				*copy++ = '\\';
				*copy++ = '\\';
				regexp += 2;
			} else {
				*copy++ = *regexp++;
			}
		} else {
			*copy++ = *regexp++;
		}
	}
}

static void VerifyTokenOverlap(ESet oTokenSet) {
	int nrTokens = 0;
	ESet tokenSet = oTokenSet;
	ESet token;

	while (tokenSet != NULL) {
        int tokenIndex = (int) ((Answer) tokenSet->element)->info;
        if (tokenIndex != -1) {
            /* Ignore overlap with endOfInput/IGNORE, since endOfInput can't
               happen, and IGNORE is skipped during parsing when the overlapping
               token is not in the expected set. */
            nrTokens++;
        }
		tokenSet = tokenSet->next;
	}
	if (nrTokens > 1) {
		fprintf(stderr, "Overlap between tokens:\n");
		for (tokenSet = oTokenSet; tokenSet != NULL; tokenSet = tokenSet->next) {
			int tokenIndex = (int) ((Answer) tokenSet->element)->info;
			Name name = TokenIndexName(tokenIndex, true);
			TREE node = InsertNodeSingle(&overlap, name, NULL, (TreeCmp) NameCmp);
			TREE nOverlap = GetInfo(node);
			fprintf(stderr, "%s:%d: token %d: %s\n", inputFileName, name->lineNr, tokenIndex, name->name);
			for (token = oTokenSet; token != NULL; token = token->next) {
				InsertNodeSingle(&nOverlap, TokenIndexName((int) ((Answer) token->element)->info, true), NULL, (TreeCmp) NameCmp);
			}
			SetInfo(node, nOverlap);
		}
	}
}

static void CheckOverlap(RegExp dfa) {
	Transition tr, tr2;
	ESet transitions;
	int mCount = CharSetSize(), count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;
	RegExp regExp;

	for (regExp = dfa; regExp != NULL; regExp = regExp->next) {
		if (regExp->transitions != NULL) {
			mState = NULL;
			transitions = NULL;
			tCount = CharSetSize();
			for (i = 0; i != tCount; i++) cSet[i] = false;
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (!cSet[tr->transitionChar]) {
					mCount--;
					cSet[tr->transitionChar] = true;
					tCount--;
				}
			}
			/* mCount is nr. of fails, mState == NULL, the fail state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (tr->destination != mState) /* not the most efficient check, but ok */
				{
					for (count = 0, tr2 = tr; tr2 != NULL; tr2 = tr2->next)
						if (tr2->destination == tr->destination) count++;
					if (count > mCount) {
						mCount = count;
						mState = tr->destination;
					}
				}
			}
			/* mCount is max. nr. of transitions to the same state, mState is that state */
			for (tr = regExp->transitions; tr != NULL; tr = tr->next) {
				if (tr->destination != mState && !SetMember(tr->destination, transitions)) {
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next) {
						if (tr2->destination == tr->destination) {
							TREE node = FindNode(tokenSets, tr->destination->info, TokenSetCmp);
							if (node != NULL) {
								VerifyTokenOverlap(GetKey(node));
							}
						}
					}
				}
			}
			if (mState != NULL) {
				TREE node = FindNode(tokenSets, mState->info, TokenSetCmp);
				if (node != NULL) {
					VerifyTokenOverlap(GetKey(node));
				}
			}
			RemoveSet(transitions);
		}
	}
}

static void CompileScanner(void) {
	TREE sym;
	Name name;
	RegExp nfa = NULL;
	int res;
	char regexpbuf[256];

	if (!HasProperty("escregexp")) {
		viMode = true;
	}
	push_localadm();
	nfa = InitRegExp();
	SetSingleAnswer(false);
	if (HasProperty("extcharset")) {
		SetCharSetSize(256);
	} else {
		SetCharSetSize(128);
	}

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
		name = GetKey(sym);
		if (name->nameType == string && name->index != 0 && name->isTerminal) {
			regexpcopy(regexpbuf, name->name);
			if ((res = AddRegExp(nfa, regexpbuf, (void *) name->index)) != 0) {
				fprintf(stderr, "%s:%d: %s\n", inputFileName, name->lineNr, regExpError[res]);
				errorOccurred = true;
			}
		}
	}
	dfa = N2DFA(nfa);
	MinimizeDFA(dfa);
}

void PrintRule2(FILE *f, Node rule) {
	NodeList rules;

	while (rule != NULL) {
		switch (rule->nodeType) {
			case single:
				switch (rule->element.name->nameType) {
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
				for (rules = rule->element.alternatives; rules != NULL; rules = rules->rest) {
					fputs("(", f);
					PrintRule2(f, rules->first);
					fputs(")", f);
					if (rules->rest != NULL) {
						fputs(";", f);
					}
				}
				fputs(")", f);
				break;
			case breakToken:
				fprintf(f, "ON %s BREAK", rule->element.name->name);
				break;
			case errorMessage:
				fprintf(f, "ON %s BREAK %s", rule->element.sub->element.name->name, rule->element.sub->next->element.name->name);
				break;
			default:
				fprintf(stderr, "DetermineFollowRule: unknown nodetype %d", rule->nodeType);
				return;
		}
		rule = rule->next;
		if (rule != NULL) {
			fputs(",", f);
		}
	}
}

void PrintRule(FILE *f, Name lhs, Node rule) {
	fprintf(f, "%s:", lhs->name);
	PrintRule2(f, rule);
	fputs(".\n", f);
}

static void PrintTree(FILE *f, TREE set) {
	set = GetFirstNode(set);
	while (set != NULL) {
		fprintf(f, "%s", ((Name) GetKey(set))->name);
		set = GetNextNode(set);
		if (set != NULL) {
			fputs(", ", f);
		}
	}
}

static int tabWidth = 4;

static void PrintCode(char *str, NameType prev, NameType next, int indent) {
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

static int TokenSetCmp2(void *tSet1, void *tSet2) {
	TREE t1 = GetFirstNode(tSet1);
	TREE t2 = GetFirstNode(tSet2);

	while (t1 != NULL && t2 != NULL) {
		Name token1 = (Name) GetKey(t1);
		Name token2 = (Name) GetKey(t2);
		int res = strcmp(token1->name, token2->name);
		if (res != 0) {
			return (res);
		}
		t1 = GetNextNode(t1);
		t2 = GetNextNode(t2);
	}
	return (t1 != NULL? 1: t2 != NULL? -1: 0);
}

static void TokenSetName2(TREE tokenSet) {
	TREE node = FindNode(tokenSets2, tokenSet, TokenSetCmp2);

	if (node == NULL) {
		fputs("cannot find token set: ", stderr);
		PrintTree(stderr, tokenSet);
		fputc('\n', stderr);
	}
    targetLanguage->pointerToArray(outputFile, "llTokenSet", (long int) GetInfo(node));
}

static void CreateTokenSet2(FILE *f, TREE tokenSet) {
	TREE node = FindNode(tokenSets2, tokenSet, TokenSetCmp2);

	if (node == NULL) {
		tokenSetNr++;
		InsertNodeSingle(&tokenSets2, CopyTree(tokenSet, NULL, NULL), (void *) tokenSetNr, TokenSetCmp2);
        targetLanguage->arrayDeclaration(f, "static", targetLanguage->tokenSetType,
                                         "llTokenSet", tokenSetNr, (nodeNumber + 1) / nrBitsInElement + 1, true);
		printTokenSet(f, tokenSet);
	}
}

static void CreateGuard(Boolean guard, Boolean generateCode, Boolean empty, int indent, TREE firsts, Boolean firstOfRule, Boolean *usesLA) {
	if (guard) {
		if (generateCode) {
			doIndent(indent);
			fputs(targetLanguage->owner, outputFile);
			targetLanguage->startFunctionCall(outputFile, "waitForToken", NULL, NULL);
			targetLanguage->nextCallParameter(outputFile);
			if (firsts == NULL) {
				fputs("follow", outputFile);
			} else if (empty) {
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
			targetLanguage->nextCallParameter(outputFile);
			targetLanguage->nextCallParameterSymbol(outputFile, "follow");
			targetLanguage->endFunctionCall(outputFile);
            fputs(targetLanguage->terminateStatement, outputFile);
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
			if (firsts != NULL && empty) {
				*usesLA = true;
			}
			CreateTokenSet2(outputFile, firsts);
		}
	}
}

static void PrintFirstSymbols(TREE ofirsts, void (*cond)(Boolean, int), int indent, Boolean positive, Boolean generateCode, TREE *symbols, int lineNr, Boolean emptyFollow, Boolean* usesLA) {
	TREE common = ofirsts; // !!! Intersection2(ofirsts, oneOf); !!! See KillTree at end and emptyFollow

	if (generateCode) {
        cond(true, indent);
        fprintf(outputFile, "%stokenInCommon(%scurrSymbol, ", (positive? "": "!"), targetLanguage->owner);
		// !!! if (emptyFollow) {
		// 	fputs("uniteTokenSets(", outputFile);
        //     if (targetLanguage->requiresLocalLTSetStorage) {
        //         targetLanguage->pointerToArray(outputFile, "ltSet", -1);
        //         fputs(", ", outputFile);
        //     }
        //     fputs("follow, ", outputFile);
		// }
        TokenSetName2(common);
		// !!! if (emptyFollow) {
        //     fputs(")", outputFile);
        // }
        fputs(")", outputFile);
        cond(false, indent);
		if (common == NULL || (symbols != NULL && NotEmptyIntersection(*symbols, common))) {
			TREE intersection = Intersection(*symbols, common);
			fputs(" /* Not an LL(1)-grammar: first symbols ", outputFile);
			PrintTree(outputFile, intersection);
			fputs(" */", outputFile);
			KillTree(intersection, NULL);
			fprintf(stderr, "%s:%d: not an LL(1)-grammar (first symbols)\n", inputFileName, lineNr);
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
		// !!! if (emptyFollow) {
		// 	*usesLA = true;
		// }
	}
	// !!! KillTree(common, NULL);
}

static void GenerateSingle(Node rule, NameType previous, int indent, Boolean generateCode) {
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

/**
 * rule: sub-rule to generate
 * emptyToEndOfRule: true when there is an empty path from rule to the end of the top production rule.
 * follow: the set of symbols that can follow rule
 */
static void GenerateRule(Name ruleName, Node rule, Boolean emptyToEndOfRule, TREE follow, int indent, Boolean generateCode, Boolean guard, TREE firstNext, Boolean *usesLA) {
	NameType previous = string;
	TREE chainElt1Followers;
	TREE chainElt2Followers;
	TREE directFollowers;
	TREE overlap;
	Boolean empty1, empty2;

    if (generateCode && HasProperty("debugFF")) {
        doIndent(indent);
        fputs("/* follow: ", outputFile); PrintTree(outputFile, follow);
        fputs(", firstNext: ", outputFile); PrintTree(outputFile, firstNext);
		fprintf(outputFile, ", emptyToEndOfRule: %s", emptyToEndOfRule? "true": "false");
        fputs("*/", outputFile);
    }
	while (rule != NULL) {
		TREE firsts = NULL;
		TREE followers = NULL;
		TREE lFirstNext = NULL;
		TREE common = NULL;
        TREE sequenceFollowers = NULL;
		(void) collectFirst(&followers, follow, rule->next);
		empty2 = collectFirst(&lFirstNext, firstNext, rule->next);
		if (generateCode && HasProperty("debugFF")) {
			doIndent(indent);
			fputs("/* followers: ", outputFile); PrintTree(outputFile, followers);
			fputs(", lFirstNext: ", outputFile); PrintTree(outputFile, lFirstNext);
			fprintf(outputFile, ", empty2: %s", empty2? "true": "false");
			fputs("*/", outputFile);
		}
		switch (rule->nodeType) {
			case single:
				if (rule->element.name->nameType == identifier) {
					if (generateCode) {
						RuleResult res = rule->element.name->ruleResult;
						Name returnType = res == NULL? NULL: res->returnType;
						Name assignTo = rule->assignTo;
						Boolean assignToRuleOutput = assignTo != NULL && ruleName->ruleResult != NULL && ruleName->ruleResult->returnVariable != NULL && strcmp(assignTo->name, ruleName->ruleResult->returnVariable->name) == 0;
						doIndent(indent);
						int nrParameters = openFunctionCall(outputFile, rule->element.name->name, rule->arglist, assignToRuleOutput? NULL: returnType, rule->assignTo, ruleName);
						if (nrParameters != rule->element.name->nrParameters) {
							fprintf(stderr, "%s:%d: error: wrong number of parameters\n", inputFileName, rule->lineNr);
						}
                        targetLanguage->nextCallParameter(outputFile);
						if (empty2 && emptyToEndOfRule) {
							if (lFirstNext == NULL) {
								targetLanguage->nextCallParameterSymbol(outputFile, "follow");
							} else {
								fputs("uniteTokenSets(", outputFile);
                                if (targetLanguage->requiresLocalLTSetStorage) {
                                    targetLanguage->pointerToArray(outputFile, "ltSet", -1);
                                    fputs(", ", outputFile);
                                }
                                fputs("follow, ", outputFile);
                                TokenSetName2(lFirstNext);
                                fputs(")", outputFile);
							}
						} else {
							TokenSetName2(lFirstNext);
						}
						closeFunctionCall(outputFile);
						fputs(targetLanguage->terminateStatement, outputFile);
					} else {
						if (empty2 && emptyToEndOfRule && lFirstNext != NULL) {
							*usesLA = true;
						}
						CreateTokenSet2(outputFile, lFirstNext);
					}
				} else if (rule->element.name->nameType == token || rule->element.name->nameType == string) {
					if (generateCode) {
						doIndent(indent);
					    fputs(targetLanguage->owner, outputFile);
						targetLanguage->startFunctionCall(outputFile, "getToken", NULL, NULL);
						targetLanguage->nextCallParameter(outputFile);
                        targetLanguage->tokenEnum(outputFile, rule->element.name->index, rule->element.name->name);
						targetLanguage->nextCallParameter(outputFile);
                        TokenSetName2(lFirstNext);
						targetLanguage->nextCallParameter(outputFile);
						if (empty2 && emptyToEndOfRule) {
							if (lFirstNext == NULL) {
								targetLanguage->nextCallParameterSymbol(outputFile, "follow");
							} else {
								fputs("uniteTokenSets(", outputFile);
                                if (targetLanguage->requiresLocalLTSetStorage) {
                                    targetLanguage->pointerToArray(outputFile, "ltSet", -1);
                                    fputs(", ", outputFile);
                                }
                                fputs("follow, ", outputFile);
                                TokenSetName2(lFirstNext);
                                fputs(")", outputFile);
							}
						} else {
							TokenSetName2(lFirstNext);
						}
						targetLanguage->nextCallParameter(outputFile);
						targetLanguage->nextCallParameterSymbol(outputFile, empty2 && emptyToEndOfRule? "true": "false");
						targetLanguage->endFunctionCall(outputFile);
						fputs(targetLanguage->terminateStatement, outputFile);
						if (rule->assignTo != NULL) {
							Boolean noDecl = rule->assignTo != NULL && ruleName->ruleResult != NULL && ruleName->ruleResult->returnVariable != NULL && strcmp(rule->assignTo->name, ruleName->ruleResult->returnVariable->name) == 0;
							targetLanguage->assignLastSymbolTo(outputFile, indent, rule->assignTo, noDecl);
						}
					} else {
						if (empty2 && emptyToEndOfRule && lFirstNext != NULL) {
							*usesLA = true;
						}
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
				empty1 = collectFirst(&firsts, NULL, rule->element.sub);
				CreateGuard(guard, generateCode, true, indent, firsts, false, usesLA);
				// !!! if (empty1) Unite(&firsts, followers);
				common = NULL; // !!! Intersection(firsts, oneOf);
				if (generateCode) {
					doIndent(indent);
				}
				PrintFirstSymbols(firsts, ifCondition, indent, true, generateCode, NULL,
								  rule->lineNr, empty1, usesLA);
				if (generateCode) {
					startBlock(indent);
					overlap = Intersection2(firsts, followers);
					if (overlap != NULL) {
						if (!rule->element.sub->preferShift) {
							doIndent(indent + 1);
							fputs("/* Not an LL(1)-grammar (first/followers of option overlaps) */\n", outputFile);
							fprintf(stderr, "%s:%d: not an LL(1)-grammar (first/followers of option overlap): ", inputFileName, rule->lineNr);
							PrintTree(stderr, overlap);
							fputc('\n', stderr);
							notLLError = true;
						}
						KillTree(overlap, NULL);
					}
				}
				GenerateRule(ruleName, rule->element.sub, empty2 && emptyToEndOfRule, followers,
							 indent + 1, generateCode, false, lFirstNext, usesLA);
				if (generateCode) {
					terminateBlock(indent);
				}
				break;
			case sequence:
				empty1 = collectFirst(&firsts, NULL, rule->element.sub);
				Unite(&lFirstNext, firsts); /* a sequence can be followed by itself */
				CreateGuard(guard, generateCode, empty1, indent, firsts, false, usesLA);
				if (empty1) {
                    Unite(&firsts, followers);
                    sequenceFollowers = firsts;
                } else {
                    Unite(&sequenceFollowers, firsts);
                    Unite(&sequenceFollowers, followers);
                }
				Unite(&common, firsts);
				if (generateCode) {
					startDoWhileLoop(indent);
				}
				GenerateRule(ruleName, rule->element.sub, empty2 && emptyToEndOfRule, sequenceFollowers,
							 indent + 1, generateCode, false, lFirstNext, usesLA);
				CreateGuard(true, generateCode, empty2, indent + 1, lFirstNext, false, usesLA);
				if (generateCode) {
					overlap = Intersection2(firsts, followers);
					if (overlap != NULL) {
						if (!rule->preferShift) {
							doIndent(indent);
							fputs("/* Not an LL(1)-grammar (first/followers of sequence overlap) */\n", outputFile);
							fprintf(stderr, "%s:%d: not an LL(1)-grammar (first/followers of sequence overlap): ", inputFileName, rule->lineNr);
							PrintTree(stderr, overlap);
							fputc('\n', stderr);
							notLLError = true;
						}
						KillTree(overlap, NULL);
					}
				}
				PrintFirstSymbols(firsts, endDoWhileLoop, indent, true, generateCode, NULL,
								  rule->lineNr, false, usesLA);
                if (sequenceFollowers != firsts) {
                    KillTree(sequenceFollowers, NULL);
                }
				break;
			case chain:
				if (generateCode) {
					startUnconditionalLoop(indent);
				}
				chainElt1Followers = NULL;
				chainElt2Followers = NULL;
				Node chainElt1 = rule->element.sub->element.sub;
				Node chainElt2 = rule->element.sub->next->element.sub;
				Boolean emptyChain1 = collectFirst(&chainElt2Followers, followers, chainElt1);
				collectFirst(&chainElt1Followers, chainElt2Followers, chainElt2);
				(void) Unite(&chainElt1Followers, followers);
				directFollowers = NULL;
				if (collectFirst(&directFollowers, NULL, chainElt2)) {
					(void) collectFirst(&directFollowers, NULL, chainElt1);
				}
				(void) Unite(&directFollowers, lFirstNext);
				GenerateRule(ruleName, chainElt1, empty2 && emptyToEndOfRule, chainElt1Followers,
							 indent + 1, generateCode, true, directFollowers, usesLA);
				KillTree(directFollowers, NULL);
				KillTree(chainElt1Followers, NULL);
				KillTree(chainElt2Followers, NULL);
				chainElt1Followers = NULL;
				(void) collectFirst(&chainElt1Followers, followers, chainElt1);
				(void) collectFirst(&firsts, chainElt1Followers, chainElt2);
				KillTree(chainElt1Followers, NULL);
				if (generateCode) {
					overlap = Intersection2(firsts, followers);
					if (overlap != NULL) {
						if (!rule->preferShift) {
							doIndent(indent + 1);
							fputs("/* Not an LL(1)-grammar (first/followers of chain overlap) */\n", outputFile);
							fprintf(stderr, "%s:%d: not an LL(1)-grammar (first/followers of chain overlap): ", inputFileName, rule->lineNr);
							PrintTree(stderr, overlap);
							fputc('\n', stderr);
							notLLError = true;
						}
						KillTree(overlap, NULL);
					}
					doIndent(indent + 1);
				}
				PrintFirstSymbols(firsts, ifCondition, indent, false, generateCode, NULL, rule->lineNr, false, usesLA);
				if (generateCode) {
                    startBlock(indent + 1);
					breakStatement(indent + 2);
					terminateBlock(indent + 1);
				}
				chainElt1Followers = NULL;
				chainElt2Followers = NULL;
				collectFirst(&chainElt2Followers, NULL, chainElt2);
				(void) Unite(&chainElt2Followers, followers);
				(void) collectFirst(&chainElt1Followers, chainElt2Followers, chainElt1);
				directFollowers = NULL;
				if (collectFirst(&directFollowers, NULL, chainElt1)) {
					(void) collectFirst(&directFollowers, NULL, chainElt2);
					Unite(&directFollowers, lFirstNext);
				}
				GenerateRule(ruleName, chainElt2, emptyChain1 && empty2 && emptyToEndOfRule,
						     chainElt1Followers, indent + 1, generateCode, false, directFollowers, usesLA);
				KillTree(directFollowers, NULL);
				KillTree(chainElt1Followers, NULL);
				KillTree(chainElt2Followers, NULL);
				if (generateCode) {
                    endUnconditionalLoop(indent);
				}
				break;
			case alternative:
				PrintAlternatives(ruleName, rule->element.alternatives, emptyToEndOfRule, followers, indent,
								  targetLanguage->errorMsg, targetLanguage->errorArgs, generateCode, lFirstNext, usesLA, rule->lineNr);
				break;
			case breakToken:
				collectFirstOn(&firsts, rule);
				if (generateCode) {
					doIndent(indent);
				}
				PrintFirstSymbols(firsts, ifCondition, indent, true, generateCode,
								  NULL, rule->lineNr, false, usesLA);
				if (generateCode) {
					startBlock(indent);
					breakStatement(indent + 1);
					terminateBlock(indent);
				}
				break;
			case errorMessage:
				collectFirstOn(&firsts, rule);
				if (generateCode) {
					doIndent(indent);
				}
				PrintFirstSymbols(firsts, ifCondition, indent, true, generateCode,
								  NULL, rule->lineNr, false, usesLA);
				if (generateCode) {
					startBlock(indent);
					doIndent(indent + 1);
					targetLanguage->startFunctionCall(outputFile, "llerror", NULL, NULL);
					targetLanguage->nextCallParameter(outputFile);
					targetLanguage->nextCallParameterSymbol(outputFile, rule->element.sub->next->element.name->name);
					targetLanguage->endFunctionCall(outputFile);
					fputs(targetLanguage->terminateStatement, outputFile);
					terminateBlock(indent);
				}
				break;
		}
		rule = rule->next;
		KillTree(followers, NULL);
		KillTree(firsts, NULL);
		KillTree(common, NULL);
		KillTree(lFirstNext, NULL);
	}
}

static void PrintAlternatives(Name ruleName, NodeList pRules, Boolean emptyToEndOfRule, TREE follow, int indent, const char *errormsg, const char* errorargs, Boolean generateCode, TREE firstNext, Boolean *usesLA, int startLineNr) {
	int cnt = 0;
	TREE firsts = NULL;
	TREE common;
	TREE symbols = NULL;
	NodeList rules;
	Node rule;
	Boolean printError = true;

	if (pRules == NULL) {
		yyerror("error in grammar: %s not defined!\n", errormsg);
		return;
	}
	rule = pRules->first;
	if (pRules != NULL && pRules->rest == NULL) {
		GenerateRule(ruleName, rule, emptyToEndOfRule, follow, indent, generateCode, false, firstNext, usesLA);
	} else {
		Boolean empty = false, empty1;
		for (rules = pRules; rules != NULL; rules = rules->rest) {
			firsts = NULL;
			if (rules->rest != NULL || !(rules->first == NULL || rules->first->empty)) {
				empty1 = collectFirst(&firsts, follow, rules->first);
				if (empty1) {
					if (empty) {
						if (generateCode) {
							fputs("/* Not an LL(1)-grammar (more than one empty) */\n", outputFile);
						} else if (rules->first == NULL) {
							fprintf(stderr, "%s:%d: error: not an LL(1)-grammar:more than one empty\n", inputFileName, pRules->first->lineNr);
						} else {
							fprintf(stderr, "%s:%d: error: not an LL(1)-grammar: more than one empty\n", inputFileName, rules->first->lineNr);
						}
						notLLError = true;
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
				PrintFirstSymbols(firsts, (cnt == 0? ifCondition: elseIfCondition), indent, true, generateCode, &symbols, (rules->first == NULL? startLineNr: rules->first->lineNr), empty1, usesLA);
				if (generateCode) {
					startBlock(indent);
				}
				common = NULL; // !!! Intersection(firsts, oneOf);
				GenerateRule(ruleName, rules->first, emptyToEndOfRule, follow, indent + 1,
							 generateCode, false, firstNext, usesLA);
				KillTree(firsts, NULL);
				KillTree(common, NULL);
				cnt++;
				if (generateCode) {
					terminateBlock(indent);
				}
			} else {
				// Don't generate a condition, but always take this path; if there
				// is a wrong symbol, it will be skipped by the elements.
				printError = false;
				if (generateCode) {
					if (cnt != 0) {
						if (bracesOnNewLine) {
							doIndent(indent);
						} else {
							fputs(" ", outputFile);
						}
						fputs("else", outputFile);
						startBlock(indent);
					}
					GenerateRule(ruleName, rules->first, emptyToEndOfRule, follow, (cnt == 0? indent: indent + 1), generateCode, false, firstNext, usesLA);
					if (cnt != 0) {
						terminateBlock(indent);
					}
				}
			}
		}
		if (generateCode && printError) {
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

static void CreateTokenSets(Name name, void *dummy) {
#pragma unused(dummy)

	if (name->nameType == identifier) {
		PrintAlternatives(name, name->rules, true, name->follow, 1, name->name, "", false, NULL, &name->usesLA, name->lineNr);
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
            PrintLocalDeclarationWithInitialization(outputFile, returnType, returnVariable, initialValue);
		}
		if (name->usesLA && targetLanguage->requiresLocalLTSetStorage) {
            doIndent(globalIndent + 1);
            PrintLocalDeclaration(outputFile, llTokenSetTypeString, "ltSet");
		}
		if (HasProperty("debug")) {
            doIndent(globalIndent + 1);
			targetLanguage->startFunctionCall(outputFile, "debugTraceStart", NULL, NULL);
			targetLanguage->nextCallParameter(outputFile);
			targetLanguage->nextCallParameterSymbol(outputFile, "\"");
			targetLanguage->nextCallParameterSymbol(outputFile, name->name);
			targetLanguage->nextCallParameterSymbol(outputFile, "\"");
			targetLanguage->endFunctionCall(outputFile);
		}
		PrintAlternatives(name, name->rules, true, name->follow, globalIndent + 1, name->name, "", true, NULL, &dummyLA, name->lineNr);
		if (returnType != NULL && returnVariable != NULL) {
            doIndent(globalIndent + 1);
            PrintReturnValue(outputFile, returnVariable->name);
		}
		if (HasProperty("debug")) {
            doIndent(globalIndent + 1);
			targetLanguage->startFunctionCall(outputFile, "debugTraceEnd", NULL, NULL);
			targetLanguage->nextCallParameter(outputFile);
			targetLanguage->nextCallParameterSymbol(outputFile, "\"");
			targetLanguage->nextCallParameterSymbol(outputFile, name->name);
			targetLanguage->nextCallParameterSymbol(outputFile, "\"");
			targetLanguage->endFunctionCall(outputFile);
		}
        doIndent(globalIndent);
		fputs("}\n\n", outputFile);
	}
}

static void ShowAllRules(void) {
	TREE sym;
	Name name;
	NodeList rules;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
		name = GetKey(sym);
		if (name->nameType == identifier) {
			for (rules = name->rules; rules != NULL; rules = rules->rest) {
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

static void DefineEnums(void) {
	TREE sym;
	Name name;
	Boolean prntd = false;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
		name = GetKey(sym);
		if (name->nameType == token && name->index > 0) {
			if (prntd) fputs(",\n", outputFile);
			fprintf(outputFile, "	%s = %ld", name->name, name->index);
			prntd = true;
		}
	}
	if (prntd) fputs("\n", outputFile);
}

static Boolean HasEnums(void) {
	TREE sym;
	Name name;

	for (sym = GetFirstNode(idTree); sym != NULL; sym = GetNextNode(sym)) {
		name = GetKey(sym);
		if (name->nameType == token && name->index > 0) {
			return (true);
		}
	}
	return (false);
}

static void CreateTokenNames(void) {
	Name name;
	int i;

    PrintArrayDeclaration(outputFile, "static", targetLanguage->stringType, "tokenName", -1, nodeNumber + 2);
	fputs("\n\t\"IGNORE\",\n", outputFile);
	for (i = 1; i <= nodeNumber; i++) {
		name = TokenIndexName(i, true);
		if (name != NULL && name->nameType == token) {
			fprintf(outputFile, "	\"%s\",\n", name->name);
		} else if (name != NULL && name->nameType == string) {
			fprintf(outputFile, "	%s,\n", name->name);
		} else {
			fputs("	\"@&^@#&^\",\n", outputFile);
		}
	}
	fputs("	\"EOF\",\n", outputFile);
	fprintf(outputFile, "%s%s\n", targetLanguage->endOfArrayLiteral, targetLanguage->terminateDeclaration);
}

static long int CountNonEmpty(void *key, void *info) {
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
			if (name == NULL) {
				fputs("	{NULL, 0},\n", outputFile);
			} else {
				fprintf(outputFile, "	{%sFunctions, %ld},\n", name->name, CountTree(name->functions, CountNonEmpty));
			}
		}
		fputs("};\n", outputFile);
	}
}

static void DefineKeywordList(void) {
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
