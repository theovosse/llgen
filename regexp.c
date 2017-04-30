#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "storage.h"
#include "regexp.h"

struct localadm
{
	RegExp oldRegExps;
	Transition oldTransitions;
	ESet oldSets;
	long int totalMalloc, nrTransition, nrSet, nrRegExp;
	long int totalFree, nrOldRegExps, nrOldSets, nrOldTransitions;
	struct localadm *next;
};

/* GLOBAL VARIABLES
 */

static int errorInExpression;
static int stateNumber = 0;
static long int lastVisit = 0;
RegExp currentAutomaton;
static FRegExp currentFDfa;
long int totalMalloc = 0, nrTransition = 0, nrSet = 0, nrRegExp = 0;
Boolean viMode = false;
Boolean caseSens = true;
long int totalFree = 0, nrOldRegExps = 0, nrOldSets = 0, nrOldTransitions = 0;
static RegExp oldRegExps = NULL;
static Transition oldTransitions = NULL;
static ESet oldSets = NULL;
static struct localadm *las = NULL;
static int charSetSize = 256;
static Boolean singleAnswer = false;
static Boolean isInDot[256] =
{
	false,true, true, true, true, true, true, true, true, true, false,true, true, false,true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, 
	true, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true
};

/* FORWARD DECLARATIONS
 */

static char *choice(RegExp start, char *pattern, RegExp *end);

/* INTERNAL FUNCTIONS
 */

static void OutOfMemory(char *func)
{
	(void) fprintf(stderr, "Out of memory in %s\n", func);
	(void) fprintf(stderr, "%ld bytes allocated\n", totalMalloc);
	(void) fprintf(stderr, "%ld transitions\n", nrTransition);
	(void) fprintf(stderr, "%ld sets\n", nrSet);
	(void) fprintf(stderr, "%ld regexps\n", nrRegExp);
}

void push_localadm(void)
{
	struct localadm *la;

	push_tempmalloc();
	la = (struct localadm *) temp_malloc(sizeof(struct localadm));
	if (la == NULL)
	{
		OutOfMemory("push_localadm");
		exit(1);
	}
	la->oldRegExps = oldRegExps;
	la->oldTransitions = oldTransitions;
	la->oldSets = oldSets;
	la->totalMalloc = totalMalloc;
	la->totalFree = totalFree;
	la->nrTransition = nrTransition;
	la->nrSet = nrSet;
	la->nrRegExp = nrRegExp;
	la->nrOldTransitions = nrOldTransitions;
	la->nrOldSets = nrOldSets;
	la->nrOldRegExps = nrOldRegExps;
	la->next = las;
	las = la;
	oldRegExps = NULL;
	oldTransitions = NULL;
	oldSets = NULL;
	totalMalloc += sizeof(struct localadm);
}

void pop_localadm(void)
{
	if (las == NULL)
	{
		(void) fprintf(stderr, "pop_localadm: las == NULL\n");
		exit(1);
	}
	oldRegExps = las->oldRegExps;
	oldTransitions = las->oldTransitions;
	oldSets = las->oldSets;
	totalMalloc = las->totalMalloc;
	nrTransition = las->nrTransition;
	nrSet = las->nrSet;
	nrRegExp = las->nrRegExp;
	totalFree = las->totalFree;
	nrOldTransitions = las->nrOldTransitions;
	nrOldSets = las->nrOldSets;
	nrOldRegExps = las->nrOldRegExps;
	las = las->next;
	pop_tempmalloc();
}

void SetCharSetSize(int cSize)
{
	charSetSize = cSize;
}

int CharSetSize(void)
{
	return (charSetSize);
}

void SetSingleAnswer(Boolean sAnswer)
{
	singleAnswer = sAnswer;
}

Boolean SingleAnswer(void)
{
	return (singleAnswer);
}

RegExp MakeRegExp(Transition transitions, Transition eTransitions, ESet info)
{
	RegExp regExp;

	if (oldRegExps == NULL)
	{
		regExp = (RegExp) temp_malloc(sizeof(struct RegExp));
		totalMalloc += sizeof(struct RegExp); nrRegExp++;
		if (regExp == NULL)
		{ 
			OutOfMemory("MakeRegExp");
			exit(1);
		}
	}
	else
	{
		regExp = oldRegExps;
		oldRegExps = oldRegExps->next;
		nrOldRegExps--;
		totalFree -= sizeof(struct RegExp);
	}
	regExp->transitions = transitions;
	regExp->eTransitions = eTransitions;
	regExp->info = info;
	regExp->state.number = stateNumber++;
	regExp->lastVisit = 0;
	regExp->next = currentAutomaton->next;
	currentAutomaton->next = regExp;
	return (regExp);
}

static void FreeRegExp(RegExp re)
{
	re->next = oldRegExps;
	oldRegExps = re;
	nrOldRegExps++;
	totalFree += sizeof(struct RegExp);
}

static FRegExp MakeFRegExp(ESet info)
{
	FRegExp fRegExp = (FRegExp) temp_malloc(sizeof(struct FRegExp));

	if (fRegExp == NULL)
	{ 
		OutOfMemory("MakeFRegExp");
		exit(1);
	}
	totalMalloc += sizeof(struct FRegExp);
	fRegExp->info = info;
	fRegExp->next = currentFDfa->next;
	currentFDfa->next = fRegExp;
	return (fRegExp);
}

static FRegExp InitFRegExp(ESet info)
{
	FRegExp fRegExp = (FRegExp) temp_malloc(sizeof(struct FRegExp));

	if (fRegExp == NULL)
	{ 
		OutOfMemory("MakeFRegExp");
		exit(1);
	}
	totalMalloc += sizeof(struct FRegExp);
	fRegExp->info = info;
	fRegExp->next = NULL;
	currentFDfa = fRegExp;
	return (fRegExp);
}

Transition MakeTransition(int transitionChar, RegExp destination, Transition next)
{
	Transition transition;

	if (oldTransitions == NULL)
	{
		transition = (Transition) temp_malloc(sizeof(struct Transition));
		totalMalloc += sizeof(struct Transition); nrTransition++;
		if (transition == NULL)
		{
			OutOfMemory("MakeTransition");
			exit(1);
		}
	}
	else
	{
		transition = oldTransitions;
		oldTransitions = oldTransitions->next;
		nrOldTransitions--;
		totalFree -= sizeof(struct Transition);
	}
	transition->transitionChar = transitionChar;
	transition->destination = destination;
	transition->next = next;
	return (transition);
}

static void FreeTransition(Transition tr)
{
	tr->next = oldTransitions;
	oldTransitions = tr;
	nrOldTransitions++;
	totalFree += sizeof(struct Transition);
}

ESet AddElement(void *element, ESet next)
{
	ESet sPtr;

	if (oldSets == NULL)
	{
		sPtr = (ESet) temp_malloc(sizeof(struct ESet));
		totalMalloc += sizeof(struct ESet); nrSet++;
		if (sPtr == NULL)
		{
			OutOfMemory("AddElement");
			exit(1);
		}
	}
	else
	{
		sPtr = oldSets;
		oldSets = oldSets->next;
		nrOldSets--;
		totalFree -= sizeof(struct ESet);
	}
	sPtr->element = element;
	sPtr->next = next;
	return (sPtr);
}

static void FreeSet(ESet set)
{
	set->next = oldSets;
	oldSets = set;
	nrOldSets++;
	totalFree += sizeof(struct ESet);
}

void RemoveSet(ESet set)
{
	ESet aux;

	while (set != NULL)
	{
		aux = set->next;
		FreeSet(set);
		set = aux;
	}
}

void ELink(RegExp from, RegExp to)
{
	from->eTransitions = MakeTransition('\0', to, from->eTransitions);
}

/* chardenot: anything but \; "\\\\"; */
static char chardenot(char **rest, Boolean *special, Boolean callFromCharList)
{
	char ch;

	if (**rest == '\0')
	{
		*special = false;
		return ('\0');
	}
	if (**rest == '\\')
	{
		(*rest)++;
		switch (**rest)
		{
			case '0':
				*special = false;
				ch = '\0';
				break;
			case 't':
				*special = false;
				ch = '\t';
				break;
			case 'n':
				*special = false;
				ch = '\n';
				break;
			case 'r':
				*special = false;
				ch = '\r';
				break;
			default:
				ch = **rest;
				if (viMode)
				{
					*special = false;
				}
				else
				{
					*special = ch != '\\';
				}
				break;
		}
		(*rest)++;
		return (ch);
	}
	ch = *(*rest)++;
	if (viMode)
	{
		*special = (!callFromCharList && strchr("()[|*+?.", ch) != NULL) || (callFromCharList && strchr("^-]", ch) != NULL);
	}
	else
	{
		*special = false;
	}
	return (ch);
}

/* charlist: chardenot; chardenot, "\\-", chardenot; "\\-", chardenot; chardenot, "\\-" */
static char *charlist(RegExp start, char *pattern, RegExp *end)
{
	Boolean chars[256];
	int i;
	char *rest = pattern;
	Boolean flag = true;
	unsigned char low;
	unsigned char high;
	Boolean special;
	Boolean range;
	char ch;

	*end = MakeRegExp(NULL, NULL, NULL);
	ch = chardenot(&rest, &special, true);
	if (special && ch == '^')
	{
		flag = false;
		ch = chardenot(&rest, &special, true);
	}
	for (i = 0; i != charSetSize; i++)
	{
		chars[i] = false;
	}
	while (errorInExpression == 0 && (special || *rest != '\0' || ch != '\0') && !(special && ch == ']'))
	{
		range = false;
		if (special)
		{
			if (ch == '-')
			{
				low = '\0';
				range = true;
			}
			else
			{
				errorInExpression = 7; /* "\\-" expected */
			}
		}
		else
		{
			low = ch;
		}
		ch = chardenot(&rest, &special, true);
		if (special)
		{
			if (ch == '-' && !range)
			{
				high = chardenot(&rest, &special, true);
				if (special && high == ']')
				{
					ch = high;
					high = '\377';
				}
				else
				{
					if (high <= low || special)
					{
						errorInExpression = 8; /* error in range */
					}
					ch = chardenot(&rest, &special, true);
				}
			}
			else if (ch == ']')
			{
				high = low;
			}
			else
			{
				errorInExpression = 7; /* "\\-" expected */
			}
		}
		else
		{
			if (range)
			{
				high = ch;
				ch = chardenot(&rest, &special, true);
			}
			else
			{
				high = low;
			}
		}
		for (i = low; i <= high; i++)
		{
			chars[i] = true;
		}
	}
	if (*rest == '\0' && ch == '\0')
	{
		if (errorInExpression == 0) errorInExpression = 5; /* "\\]" expected */
	}
	for (i = 0; i != charSetSize; i++)
	{
		if (chars[i] == flag)
		{
			start->transitions = MakeTransition(i, *end, start->transitions);
			if (!caseSens)
			{
				if ('a' <= i && i <= 'z' && !chars[i - 'a' + 'A'])
				{
					start->transitions = MakeTransition(i - 'a' + 'A', *end, start->transitions);
				}
				else if ('A' <= i && i <= 'Z' && !chars[i - 'A' + 'a'])
				{
					start->transitions = MakeTransition(i - 'A' + 'a', *end, start->transitions);
				}
			}
		}
	}
	return (rest);
}

/* character: char; "\\[ntr]"; "\\."; "\\(", choice, "\\)"; "\\[", "\\^" OPTION, charlist, "\\]" */
static char *character(RegExp start, char *pattern, RegExp *end)
{
	char *rest = pattern;
	int i;
	char ch;
	Boolean special;

	ch = chardenot(&rest, &special, false);
	if (special)
	{
		if (ch == '.')
		{
			/* Beetje inefficient */
			*end = MakeRegExp(NULL, NULL, NULL);
			for (i = 0; i != charSetSize; i++)
			{
				if (isInDot[i])
				{
					start->transitions = MakeTransition(i, *end, start->transitions);
				}
			}
		}
		else if (ch == '[')
		{
			rest = charlist(start, rest, end);
		}
		else if (ch == '(')
		{
			rest = choice(start, rest, end);
			ch = chardenot(&rest, &special, false);
			if (!special || ch != ')')
			{
				if (errorInExpression == 0) errorInExpression = 3; /* "\\)" expected */
			}
		}
		else
		{
			*end = MakeRegExp(NULL, NULL, NULL);
			start->transitions = MakeTransition(ch, *end, start->transitions);
			errorInExpression = 1; /* Illegal escape character */
		}
	}
	else if (*pattern == '\0')
	{
		if (errorInExpression == 0) errorInExpression = 2; /* Unexpected end of expression */
	}
	else
	{
		*end = MakeRegExp(NULL, NULL, NULL);
		start->transitions = MakeTransition(ch, *end, start->transitions);
		if (!caseSens)
		{
			if ('a' <= ch && ch <= 'z')
			{
				start->transitions = MakeTransition(ch - 'a' + 'A', *end, start->transitions);
			}
			else if ('A' <= *pattern && *pattern <= 'Z')
			{
				start->transitions = MakeTransition(ch - 'A' + 'a', *end, start->transitions);
			}
		}
	}
	return (rest);
}

/* factor: character, (\\"+"; "\\*"; "\\?") OPTION */
static char *factor(RegExp start, char *pattern, RegExp *end)
{
	char *rest = pattern;
	char *rest2 = rest;
	char nextCh;
	Boolean special;

	rest2 = rest = character(start, pattern, end);
	nextCh = chardenot(&rest2, &special, false);
	if (special)
	{
		if (nextCh == '+')
		{
			rest = rest2;
			ELink(*end, start);
		}
		else if (nextCh == '*')
		{
			rest = rest2;
			ELink(*end, start);
			*end = start;
		}
		else if (nextCh == '?')
		{
			rest = rest2;
			ELink(start, *end);
		}
		else if (nextCh == '\0' || strchr("\\(|)[].0tnr", nextCh) == NULL)
		{
			if (errorInExpression == 0) errorInExpression = 1; /* Illegal escape character */
		}
	}
	return (rest);
}

/* sequence: factor SEQUENCE */
static char *sequence(RegExp start, char *pattern, RegExp *end)
{
	RegExp startFactor = start;
	RegExp endFactor;
	char *rest = pattern;
	char *rest2;
	char nextCh;
	Boolean special;

	do
	{
		rest2 = rest = factor(startFactor, rest, &endFactor);
		startFactor = endFactor;
		nextCh = chardenot(&rest2, &special, false);
	}
	while (errorInExpression == 0 && *rest != '\0' && !(special && (nextCh == '|' || nextCh == ')')));
	*end = endFactor;
	return (rest);
}

/* choice: sequence SEQ "\\|" */
static char *choice(RegExp start, char *pattern, RegExp *end)
{
	char *rest;
	char nextCh;
	Boolean special;
	char *rest2;
	RegExp startSequence = MakeRegExp(NULL, NULL, NULL);

	ELink(start, startSequence);
	rest2 = rest = sequence(startSequence, pattern, end);
	nextCh = chardenot(&rest2, &special, false);
	if (errorInExpression == 0 && nextCh == '|' && special)
	{
		RegExp endSequence = MakeRegExp(NULL, NULL, NULL);
		ELink(*end, endSequence);
		*end = endSequence;
		do
		{
			startSequence = MakeRegExp(NULL, NULL, NULL);
			ELink(start, startSequence);
			rest2 = rest = sequence(startSequence, rest2, &endSequence);
			nextCh = chardenot(&rest2, &special, false);
			ELink(endSequence, *end);
		}
		while (errorInExpression == 0 && nextCh == '|' && special);
	}
	return (rest);
}

static ESet EClosure(ESet stateSet)
{
	ESet *tail;
	ESet element;
	Transition tr;

	lastVisit++;
	for (element = stateSet; element->next != NULL; element = element->next)
	{
		((RegExp) element->element)->lastVisit = lastVisit;
	}
	((RegExp) element->element)->lastVisit = lastVisit;
	tail = &element->next;
	element = stateSet;
	while (element != NULL)
	{
		for (tr = ((RegExp) element->element)->eTransitions; tr != NULL; tr = tr->next)
		{
			if (tr->destination->lastVisit != lastVisit)
			{
				*tail = AddElement(tr->destination, NULL);
				tr->destination->lastVisit = lastVisit;
				tail = &((*tail)->next);
			}
		}
		element = element->next;
	}
	return (stateSet);
}

Boolean SetMember(void *element, ESet set)
{
	while (set != NULL)
	{
		if (set->element == element)
		{
			return (true);
		}
		set = set->next;
	}
	return (false);
}

Boolean SubSet(ESet s1, ESet s2)
{
	ESet s;

	while (s1 != NULL)
	{
		for (s = s2; s != NULL; s = s->next) if (s->element == s1->element) break;
		if (s == NULL) return (false);
		s1 = s1->next;
	}
	return (true);
}

Boolean SetEqual(ESet s1, ESet s2)
{
	return (SubSet(s1, s2) && SubSet(s2, s1));
}

static ESet CollectTransitions(ESet stateSet, int ch)
{
	ESet destSet = NULL;
	Transition tr;

	while (stateSet != NULL)
	{
		for (tr = ((RegExp) stateSet->element)->transitions; tr != NULL; tr = tr->next)
		{
			if (tr->transitionChar == ch && !SetMember(tr->destination, destSet))
			{
				destSet = AddElement(tr->destination, destSet);
			}
		}
		stateSet = stateSet->next;
	}
	return (destSet);
}

static RegExp Member(ESet stateSet, RegExp fa)
{
	while (fa != NULL)
	{
		if (SetEqual(stateSet, fa->state.set))
		{
			return (fa);
		}
		fa = fa->next;
	}
	return (NULL);
}

static ESet CollectInfo(ESet stateSet)
{
	ESet info = NULL;

	while (stateSet != NULL)
	{
		if (((RegExp) stateSet->element)->info != NULL)
		{
			info = AddElement(((RegExp) stateSet->element)->info->element, info);
		}
		stateSet = stateSet->next;
	}
	return (info);
}

Answer MakeAnswer(int number, void *info)
{
	Answer answer = (Answer) temp_malloc(sizeof(struct Answer));

	if (answer == NULL)
	{
		OutOfMemory("MakeAnswer");
		exit(1);
	}
	totalMalloc += sizeof(struct Answer);
	answer->number = number;
	answer->info = info;
	return (answer);
}

static int globStateNr;
static RegExp *state;
static Boolean *stateState;
static Boolean *oStates;

void CountNrStates(RegExp regExp)
{
	Transition tr;

	if (regExp == NULL || regExp->lastVisit == lastVisit)
	{
		return;
	}
	regExp->lastVisit = lastVisit;
	if (state != NULL)
	{
		state[regExp->state.number] = regExp;
	}
	else
	{
		regExp->state.number = globStateNr++;
	}
	for (tr = regExp->eTransitions; tr != NULL; tr = tr->next)
	{
		CountNrStates(tr->destination);
	}
	for (tr = regExp->transitions; tr != NULL; tr = tr->next)
	{
		CountNrStates(tr->destination);
	}
}

static void DumpRegExp2(RegExp regExp)
{
	Transition tr, tr2;
	ESet transitions;
	int mCount = charSetSize, count;
	RegExp mState = NULL;
	Boolean cSet[256];
	int i, tCount;

	if (regExp->lastVisit != lastVisit)
	{
		regExp->lastVisit = lastVisit;
		printf("%lx: ", regExp->state.number);
		if (regExp->transitions != NULL)
		{
			transitions = NULL;
			tCount = charSetSize;
			for (i = 0; i != charSetSize; i++) cSet[i] = false;
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
				/* Suppresses \0...\037 en \177...\377 */
				if (' ' <= tr->transitionChar && tr->transitionChar <= '~')
				{
					if (tr->destination != mState && !SetMember(tr->destination, transitions))
					{
						transitions = AddElement(tr->destination, transitions);
						printf("{");
						for (tr2 = tr; tr2 != NULL; tr2 = tr2->next)
							if (tr2->destination == tr->destination && tr2->transitionChar >= ' ')
								fputc(tr2->transitionChar, stdout);
						printf("}->%lx ", tr->destination->state.number);
					}
				}
			}
			if (mState == NULL)
			{
				printf("otherwise fail ");
			}
			else
			{
				if (tCount != 0)
				{
					printf("{");
					for (i = 0; i != charSetSize; i++)
					{
						if (!cSet[i]) fputc(i, stdout);
					}
					printf("}->fail ");
				}
				printf("otherwise->%lx ", mState->state.number);
			}
			RemoveSet(transitions);
		}
		if (regExp->eTransitions != NULL)
		{
			printf("Eps:{");
			for (tr = regExp->eTransitions; tr != NULL; tr = tr->next)
			{
				printf("%lx", tr->destination->state.number);
				if (tr->next != NULL) putchar(',');
			}
			printf("}");
		}
		if (regExp->info != NULL)
		{
			ESet set = regExp->info;
			Answer first = set->element;
			while (set != NULL)
			{
				if (singleAnswer)
				{
					if (first->number > ((Answer) set->element)->number)
					{
						first = set->element;
					}
				}
				else
				{
					printf(" res='%d.%s'", ((Answer) set->element)->number, (const char*) ((Answer) set->element)->info);
				}
				set = set->next;
			}
			if (singleAnswer)
			{
				printf(" res='%s'", (const char*) first->info);
			}
		}
		putchar('\n');
		for (tr = regExp->transitions; tr != NULL; tr = tr->next)
		{
			DumpRegExp2(tr->destination);
		}
		for (tr = regExp->eTransitions; tr != NULL; tr = tr->next)
		{
			DumpRegExp2(tr->destination);
		}
	}
}

static void MarkDFA(RegExp dfa)
{
	Transition tr;

	if (dfa->lastVisit != lastVisit)
	{
		dfa->lastVisit = lastVisit;
		for (tr = dfa->transitions; tr != NULL; tr = tr->next)
		{
			MarkDFA(tr->destination);
		}
	}
}

static void RenumberDFA(RegExp dfa)
{
	long int dfaStateNumber = 0;
	RegExp state;

	for (state = dfa; state != NULL; state = state->next)
	{
		state->state.number = dfaStateNumber++;
		if (state->eTransitions != NULL)
		{
			fprintf(stderr, "epsilon transitions in dfa?\n");
		}
	}
}

static void RemoveUnreachableStates(RegExp dfa)
{
	RegExp state, auxState, prev = dfa;
	Transition tr, auxTr;

	/* Never remove the first state */
	for (state = dfa->next; state != NULL; state = auxState)
	{
		auxState = state->next;
		if (state->lastVisit == lastVisit)
		{
			prev->next = state;
			prev = state;
		}
		else
		{
			for (tr = state->transitions; tr != NULL; tr = auxTr)
			{
				auxTr = tr->next;
				FreeTransition(tr);
			}
			FreeRegExp(state);
		}
	}
	prev->next = NULL;
}

static Boolean AllDestinationsEqual(RegExp state1, RegExp state2)
{
	int i;
	Transition tr;
	ESet d1[256], d2[256];

	for (i = 0; i != charSetSize; i++) d1[i] = d2[i] = NULL;
	for (tr = state1->transitions; tr != NULL; tr = tr->next)
	{
		d1[tr->transitionChar] = tr->destination->state.set;
	}
	for (tr = state2->transitions; tr != NULL; tr = tr->next)
	{
		d2[tr->transitionChar] = tr->destination->state.set;
	}
	for (i = 0; i != charSetSize; i++)
	{
		if (d1[i] != d2[i])
		{
			return (false);
		}
	}
	return (true);
}

static Boolean FirstAnswerIdentical(ESet info1, ESet info2)
{
	int first1;
	int first2;

	if (info1 == NULL || info2 == NULL)
	{
		return (info1 == info2);
	}
	for (first1 = ((Answer) info1->element)->number, info1 = info1->next; info1 != NULL; info1 = info1->next)
	{
		if (((Answer) info1->element)->number < first1)
		{
			first1 = ((Answer) info1->element)->number;
		}
	}
	for (first2 = ((Answer) info2->element)->number, info2 = info2->next; info2 != NULL; info2 = info2->next)
	{
		if (((Answer) info2->element)->number < first2)
		{
			first2 = ((Answer) info2->element)->number;
		}
	}
	return (first1 == first2);
}

/* EXPORTED FUNCTIONS
 */

RegExp InitRegExp(void)
{
	RegExp regExp = (RegExp) temp_malloc(sizeof(struct RegExp));

	totalMalloc += sizeof(struct RegExp); nrRegExp++;
	if (regExp == NULL)
	{
		OutOfMemory("InitRegExp");
		exit(1);
	}
	regExp->transitions = NULL;
	regExp->eTransitions = NULL;
	regExp->info = NULL;
	regExp->state.number = stateNumber++;
	regExp->lastVisit = 0;
	regExp->next = NULL;
	return (regExp);
}

int AddRegExp(RegExp regExp, char *pattern, void *info)
{
	RegExp end = NULL;
	char *rest;
	RegExp newRe;
	Answer answer;
	static int number = 0;

	currentAutomaton = regExp;
	answer = MakeAnswer(number++, info);
	newRe = MakeRegExp(NULL, NULL, NULL);
	ELink(regExp, newRe);
	errorInExpression = 0;
	rest = choice(newRe, pattern, &end);
    if (errorInExpression != 0) {
        return errorInExpression;
    }
	end->info = AddElement(answer, NULL); /* An NFA has 1 info per final state */
	if (*rest != '\0')
	{
		if (errorInExpression == 0) errorInExpression = 4; /* End of expression expected */
	}
	return (errorInExpression);
}

ESet MatchNFA(RegExp regExp, char *line)
{
	ESet stateSet;
	ESet sPtr;
	ESet *tail;
	Transition tr;
	ESet oldStateSet;
	ESet answers = NULL;
	int currChar;

	stateSet = AddElement(regExp, NULL);
	/* Take eTransitions */
	lastVisit++;
	regExp->lastVisit = lastVisit;
	for (sPtr = stateSet, tail = &(stateSet->next); sPtr != NULL; sPtr = sPtr->next)
	{
		for (tr = ((RegExp) (sPtr->element))->eTransitions; tr != NULL; tr = tr->next)
		{
			if (tr->destination->lastVisit != lastVisit)
			{
				*tail = AddElement(tr->destination, NULL);
				tr->destination->lastVisit = lastVisit;
				tail = &((*tail)->next);
			}
		}
	}
	for (currChar = (int) *line++ & 0xFF; currChar != '\0' && stateSet != NULL; currChar = *line++)
	{
		lastVisit++;
		tail = NULL;
		/* Take transitions */
		oldStateSet = stateSet;
		stateSet = NULL;
		for (sPtr = oldStateSet; sPtr != NULL; sPtr = sPtr->next)
		{
			for (tr = ((RegExp) (sPtr->element))->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->transitionChar == currChar)
				{
					stateSet = AddElement(tr->destination, stateSet);
					tr->destination->lastVisit = lastVisit;
					if (tail == NULL)
					{
						tail = &(stateSet->next);
					}
				}
			}
		}
		RemoveSet(oldStateSet);
		/* Take eTransitions */
		for (sPtr = stateSet; sPtr != NULL; sPtr = sPtr->next)
		{
			for (tr = ((RegExp) (sPtr->element))->eTransitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination->lastVisit != lastVisit)
				{
					*tail = AddElement(tr->destination, NULL);
					tr->destination->lastVisit = lastVisit;
					tail = &((*tail)->next);
				}
			}
		}
	}
	for (sPtr = stateSet; sPtr != NULL; sPtr = sPtr->next)
	{
		if (((RegExp) (sPtr->element))->info != NULL)
		{
			((ESet) (((RegExp) sPtr->element)->info))->next = answers;
			answers = (ESet) (((RegExp) (sPtr->element))->info);
		}
	}
	RemoveSet(stateSet);
	return (answers);
}

void PrepareMatchNFA2(RegExp regExp)
{
	lastVisit++; globStateNr = 0;
	state = NULL;
	CountNrStates(regExp);
	push_localadm();
	state = (RegExp *) temp_malloc(globStateNr * sizeof(RegExp));
	stateState = (Boolean *) temp_malloc(globStateNr * sizeof(Boolean));
	oStates = (Boolean *) temp_malloc(globStateNr * sizeof(Boolean));
	if (state == NULL || stateState == NULL || oStates == NULL)
	{
		OutOfMemory("MatchNFA");
		return;
	}
	lastVisit++;
	CountNrStates(regExp);
}

ESet MatchNFA2(RegExp regExp, char *line)
{
	Boolean *sptr;
	Boolean *optr;
	RegExp *stptr;
	ESet answers = NULL;
	Transition tr;
	int i;
	Boolean changes;

	for (sptr = stateState, i = 0; i != globStateNr; i++)
	{
		*sptr++ = false;
	}
	/* Take eTransitions */
	memcpy(oStates, stateState, globStateNr * sizeof(Boolean));
	stateState[0] = true;
	do
	{
		changes = false;
		for (i = 0, sptr = stateState, optr = oStates, stptr = state; i != globStateNr; i++, sptr++, optr++, stptr++)
		{
			if (*sptr && !*optr)
			{
				*optr = true;
				for (tr = (*stptr)->eTransitions; tr != NULL; tr = tr->next)
				{
					if (!stateState[tr->destination->state.number])
					{
						changes = true;
						stateState[tr->destination->state.number] = true;
					}
				}
			}
		}
	}
	while (changes);
	while (*line != '\0')
	{
		/* Take transitions */
		changes = false;
		memcpy(oStates, stateState, globStateNr * sizeof(Boolean));
		for (sptr = stateState, i = 0; i != globStateNr; i++) *sptr++ = false;
		for (i = 0, optr = oStates, stptr = state; i != globStateNr; i++, optr++, stptr++)
		{
			if (*optr)
			{
				for (tr = (*stptr)->transitions; tr != NULL; tr = tr->next)
				{
					if (tr->transitionChar == ((int) *line & 0xFF))
					{
						changes = true;
						stateState[tr->destination->state.number] = true;
					}
				}
			}
		}
		if (!changes)
		{
			return (NULL);
		}
		/* Take eTransitions */
		memcpy(oStates, stateState, globStateNr * sizeof(Boolean));
		stateState[0] = true;
		do
		{
			changes = false;
			for (i = 0, sptr = stateState, optr = oStates, stptr = state; i != globStateNr; i++, sptr++, optr++, stptr++)
			{
				if (*sptr && !*optr)
				{
					*optr = true;
					for (tr = (*stptr)->eTransitions; tr != NULL; tr = tr->next)
					{
						if (!stateState[tr->destination->state.number])
						{
							changes = true;
							stateState[tr->destination->state.number] = true;
						}
					}
				}
			}
		}
		while (changes);
		line++;
	}
	for (sptr = stateState, stptr = state, i = 0; i != globStateNr; i++, stptr++)
	{
		if (*sptr++ && (*stptr)->info != NULL)
		{
			((ESet) ((*stptr)->info))->next = answers;
			answers = (ESet) ((*stptr)->info);
		}
	}
	return (answers);
}

void FinalizeMatchNFA2(void)
{
	pop_localadm();
}

ESet MatchDFA(RegExp regExp, char *line)
{
	RegExp currState = regExp;
	Transition tr;
	int currChar;

	for (currChar = (int) *line++ & 0xFF; currChar != '\0' && currState != NULL; currChar = *line++)
	{
		for (tr = currState->transitions, currState = NULL; tr != NULL && currState == NULL; tr = tr->next)
		{
			if (tr->transitionChar == currChar)
			{
				currState = tr->destination;
			}
		}
	}
	if (currState != NULL)
	{
		return ((ESet) currState->info);
	}
	else
	{
		return (NULL);
	}
}

void DumpRegExp(RegExp regExp)
{
	lastVisit++;
	DumpRegExp2(regExp);
}

void RemoveNFA(RegExp nfa)
{
	RegExp aux;
	Transition tr;
	Transition auxTr;

	while (nfa != NULL)
	{
		aux = nfa->next;
		for (tr = nfa->transitions; tr != NULL; tr = auxTr)
		{
			auxTr = tr->next;
			FreeTransition(tr);
		}
		for (tr = nfa->eTransitions; tr != NULL; tr = auxTr)
		{
			auxTr = tr->next;
			FreeTransition(tr);
		}
		if (nfa->info != NULL)
		{
			((ESet) nfa->info)->next = NULL;
			RemoveSet((ESet) nfa->info);
		}
		FreeRegExp(nfa);
		nfa = aux;
	}
}

RegExp N2DFA(RegExp nfa)
{
	long int mark = ++lastVisit;
	RegExp dfa;
	RegExp currState;
	ESet stateSet;
	RegExp newState;
	int ch;
	Boolean allMarked = false;
	
	dfa = InitRegExp();
	currentAutomaton = dfa;
	dfa->state.set = EClosure(AddElement(nfa, NULL));
	while (!allMarked)
	{
		 allMarked = true;
		 currState = dfa;
		 while (allMarked && currState != NULL)
		 {
			 if (currState->lastVisit != mark)
			 {
				 allMarked = false;
			 }
			 else
			 {
				 currState = currState->next;
			 }
		 }
		 if (currState != NULL)
		 {
			 currState->lastVisit = mark;
			 for (ch = 0; ch != charSetSize; ch++)
			 {
				 if ((stateSet = CollectTransitions(currState->state.set, ch)) != NULL)
				 {
					 stateSet = EClosure(stateSet);
					 if ((newState = Member(stateSet, dfa)) == NULL)
					 {
						 newState = InitRegExp();
						 newState->state.set = stateSet;
						 newState->info = CollectInfo(stateSet);
						 newState->next = dfa->next;
						 dfa->next = newState;
					 }
					 else
					 {
						 RemoveSet(stateSet);
					 }
					 currState->transitions = MakeTransition(ch, newState, currState->transitions);
				 }
			 }
		 }
	}
	for (currState = dfa; currState != NULL; currState = currState->next)
	{
		RemoveSet(currState->state.set);
		currState->state.set = NULL;
	}
	RenumberDFA(dfa);
	return (dfa);
}

void RemoveDFA(RegExp dfa)
{
	RegExp aux;
	Transition tr, auxTr;

	while (dfa != NULL)
	{
		aux = dfa->next;
		for (tr = dfa->transitions; tr != NULL; tr = auxTr)
		{
			auxTr = tr->next;
			FreeTransition(tr);
		}
		RemoveSet((ESet) dfa->info);
		FreeRegExp(dfa);
		dfa = aux;
	}
}

void MinimizeDFA(RegExp dfa)
{
	RegExp state;
	ESet partition = NULL, nPartition;
	ESet element, element2;
	ESet nSet1, nSet2;
	Boolean changes;
	Transition tr;

	currentAutomaton = dfa;
	push_localadm();
	/* Partitioning: one class for each type final state, plus one for the rest */
	for (state = dfa; state != NULL; state = state->next)
	{
		for (element = partition; element != NULL; element = element->next)
		{
			ESet stateSet = element->element;
			if ((singleAnswer? FirstAnswerIdentical(state->info, ((RegExp) (stateSet->element))->info):
							   SetEqual(state->info, ((RegExp) (stateSet->element))->info)))
			{
				element->element = AddElement(state, element->element);
				state->state.set = element;
				break;
			}
		}
		if (element == NULL)
		{
			ESet stateSet = AddElement(state, NULL);
			partition = AddElement(stateSet, partition);
			state->state.set = partition;
		}
	}
	/* Refine partitioning */
	do
	{
		changes = false;
		nPartition = NULL;
		for (element = partition; element != NULL; element = element->next)
		{
			element2 = element->element;
			nSet1 = AddElement(element2->element, NULL);
			nSet2 = NULL;
			element2 = element2->next;
			/* Check if all other members of this partition have the same destinations as the first element */
			while (element2 != NULL)
			{
				if (AllDestinationsEqual(nSet1->element, element2->element))
				{
					nSet1 = AddElement(element2->element, nSet1);
				}
				else
				{
					nSet2 = AddElement(element2->element, nSet2);
				}
				element2 = element2->next;
			}
			nPartition = AddElement(nSet1, nPartition);
			for (element2 = nSet1; element2 != NULL; element2 = element2->next)
			{
				((RegExp) (element2->element))->state.set = nPartition;
			}
			if (nSet2 != NULL)
			{
				nPartition = AddElement(nSet2, nPartition);
				changes = true;
				for (element2 = nSet2; element2 != NULL; element2 = element2->next)
				{
					((RegExp) (element2->element))->state.set = nPartition;
				}
			}
		}
		RemoveSet(partition);
		partition = nPartition;
	}
	while (changes);
	/* Ensure that the first state stays the same when mapping... */
	for (element = partition; element != NULL; element = element->next)
	{
		if (SetMember(dfa, element->element))
		{
			element->element = AddElement(dfa, element->element);
			break;
		}
	}
	/* Map states and transitions: pick first element of each partition */
	for (state = dfa; state != NULL; state = state->next)
	{
		for (tr = state->transitions; tr != NULL; tr = tr->next)
		{
			tr->destination = ((ESet) (tr->destination->state.set->element))->element;
		}
	}
	/* Clean up adm */
	pop_localadm();
	/* Remove unreachable states */
	lastVisit++;
	MarkDFA(dfa);
	RemoveUnreachableStates(dfa);
	RenumberDFA(dfa);
}

void PrintDFA(RegExp dfa)
{
	RegExp state;
	Transition tr, tr2;
	ESet transitions;
	int mCount = charSetSize, count;
	RegExp mState = NULL;
	ESet elt;
	int i, tCount;
	Boolean cSet[256];

	printf("int PMatch(char *str)\n{\n    int pos = 0, lLen = -1;\n\n");
	for (state = dfa; state != NULL; state = state->next)
	{
		tCount = charSetSize;
		for (i = 0; i != charSetSize; i++) cSet[i] = false;
		for (tr = state->transitions; tr != NULL; tr = tr->next)
		{
			cSet[tr->transitionChar] = true;
			mCount--;
			tCount--;
		}
		/* mCount is nr. of fails, mState == NULL, the fail state */
		for (tr = state->transitions; tr != NULL; tr = tr->next)
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
		printf("  l%ld:\n", state->state.number);
		if (state->info != NULL)
		{
			Answer first;
			printf("    lLen = pos;\n");
			for (elt = state->info, first = elt->element; elt != NULL; elt = elt->next)
			{
				if (singleAnswer)
				{
					if (first->number > ((Answer) elt->element)->number)
					{
						first = elt->element;
					}
				}
				else
				{
					printf("    Accept(%d, \"%s\");\n", ((Answer) elt->element)->number, (const char*) ((Answer) elt->element)->info);
				}
			}
			if (singleAnswer)
			{
				printf("    Accept(\"%s\");\n", (const char*) first->info);
			}
		}
		if (state->transitions == NULL)
		{
			printf("    goto fail;\n");
		}
		else
		{
			printf("    if (*str == '\\0') goto fail;\n");
			printf("    pos++;\n    switch (*str++)\n    {\n");
			transitions = NULL;
			for (tr = state->transitions; tr != NULL; tr = tr->next)
			{
				if (tr->destination != mState && !SetMember(tr->destination, transitions))
				{
					transitions = AddElement(tr->destination, transitions);
					for (tr2 = tr; tr2 != NULL; tr2 = tr2->next)
					{
						if (tr2->destination == tr->destination)
						{
							printf("        case ");
							if (tr2->transitionChar < ' ' || tr2->transitionChar > '~')
							{
								printf("\\%03x:\n", tr2->transitionChar);
							}
							else
							{
								printf("'%c':\n", tr2->transitionChar);
							}
						}
					}
					printf("            goto l%ld;\n", tr->destination->state.number);
				}
			}
			if (mState == NULL)
			{
				printf("        default:\n            goto fail;\n");
			}
			else
			{
				if (tCount != 0)
				{
					for (i = 0; i != charSetSize; i++)
					{
						if (!cSet[i])
						{
							printf("        case ");
							if (i < ' ' || i > '~')
							{
								printf("\\%03x:\n", i);
							}
							else
							{
								printf("'%c':\n", i);
							}
						}
					}
					printf("            goto fail;\n");
				}
				printf("        default:\n            goto l%ld;\n", mState->state.number);
			}
			RemoveSet(transitions);
			printf("    }\n");
		}
	}
	printf("  fail:\n    return (lLen);\n}\n");
}

void DisassembleDFA(RegExp dfa)
{
	RegExp state;
	Transition tr, tr2;
	int mCount = charSetSize, count;
	RegExp mState = NULL;
	ESet elt;
	int i, tCount;
	RegExp cSet[256];
	int tempLabel = 1;
	Boolean justPrintedLabel = false;
	int nrStates = 0;

	for (state = dfa; state != NULL; state = state->next) nrStates++;
	tempLabel = nrStates + 1;
	printf(";\tNull-terminated string in A0.\n;\tResult in D0,A0\n");
	printf("\tMOVE.L A0,A1\n\tMOVE.W #$FFFF,D0\n\tCLR.W  D1\n");
	for (state = dfa; state != NULL; state = state->next)
	{
		tCount = charSetSize;
		for (i = 0; i != charSetSize; i++) cSet[i] = NULL;
		for (tr = state->transitions; tr != NULL; tr = tr->next)
		{
			cSet[tr->transitionChar] = tr->destination;
			mCount--;
			tCount--;
		}
		/* mCount is nr. of fails, mState == NULL, the fail state */
		for (tr = state->transitions; tr != NULL; tr = tr->next)
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
		if (justPrintedLabel) printf("\n");
		printf("@%ld:", state->state.number);
		justPrintedLabel = true;
		if (state->info != NULL)
		{
			Answer first;
			printf("\tMOVE.W D1,D0\n");
			for (elt = state->info, first = elt->element; elt != NULL; elt = elt->next)
			{
				if (first->number > ((Answer) elt->element)->number)
				{
					first = elt->element;
				}
			}
			printf("\tMOVE.L #$%lx,A0\n", (long) first->info);
			justPrintedLabel = false;
		}
		if (state->transitions == NULL)
		{
			if (state->next != NULL)
			{
				printf("\tJMP    @%d\n", nrStates);
				justPrintedLabel = false;
			}
		}
		else
		{
			printf("\tADDQ.W #1,D1\n");
			printf("\tMOVE.B (A1)+,D2\n");
		 /* printf("\tTST.B  D2\n\tBEQ    @%d\n", nrStates); */
			justPrintedLabel = false;
			i = 0;
			while (i != charSetSize)
			{
				if (cSet[i] != mState)
				{
					if (i < charSetSize - 2 && cSet[i] == cSet[i + 1] && cSet[i] == cSet[i + 2])
					{
						if (i != 0)
						{
							printf("\tCMP.B  #$%02x,D2\t; '", i);
							if (' ' <= i && i <= '~') printf("%c'\n", i);
							else printf("\\%03o'\n", i);
							printf("\tBLT    @%d\n", tempLabel);
							justPrintedLabel = false;
						}
						do
						{
							i++;
						}
						while (i != charSetSize && cSet[i] == cSet[i - 1]);
						i--;
						if (i == charSetSize - 1)
						{
							if (cSet[i] == NULL)
							{
								printf("\tJMP    @%d\n", nrStates);
							}
							else if (cSet[i] == state && state->info != NULL)
							{
								printf("\tJMP    @%ld+8\n", cSet[i]->state.number);
							}
							else
							{
								printf("\tJMP    @%ld\n", cSet[i]->state.number);
							}
						}
						else
						{
							printf("\tCMP.B  #$%02x,D2\t; '", i);
							if (' ' <= i && i <= '~') printf("%c'\n", i);
							else printf("\\%03o'\n", i);
							if (cSet[i] == NULL)
							{
								printf("\tBLE    @%d\n", nrStates);
							}
							else if (cSet[i] == state && state->info != NULL)
							{
								printf("\tBLE    @%ld+8\n", cSet[i]->state.number);
							}
							else
							{
								printf("\tBLE    @%ld\n", cSet[i]->state.number);
							}
						}
						printf("@%d:", tempLabel++);
						justPrintedLabel = true;
					}
					else
					{
						printf("\tCMP.B  #$%02x,D2\t; '", i);
						if (' ' <= i && i <= '~') printf("%c'\n", i);
						else printf("\\%03o'\n", i);
						if (cSet[i] == NULL)
						{
							printf("\tBEQ    @%d\n", nrStates);
						}
						else if (cSet[i] == state && state->info != NULL)
						{
							printf("\tBEQ    @%ld+8\n", cSet[i]->state.number);
						}
						else
						{
							printf("\tBEQ    @%ld\n", cSet[i]->state.number);
						}
						justPrintedLabel = false;
					}
				}
				i++;
			}
			if (mState == NULL)
			{
				if (state->next != NULL)
				{
					printf("\tJMP    @%d\n", nrStates);
				}
			}
			else if (mState == state && state->info != NULL)
			{
				printf("\tJMP    @%ld+8\n", mState->state.number);
			}
			else 
			{
				printf("\tJMP    @%ld\n", mState->state.number);
			}
			justPrintedLabel = false;
		}
	}
	if (justPrintedLabel) printf("\n");
	printf("@%d:\tRTS\n", nrStates);
}

/* Generated code expects null-terminated string in A0 and leaves result in D0 (length), A0 (info) */
int CompileDFA(RegExp dfa, int prevLen, unsigned char *code, Boolean generate)
{
	RegExp state;
	Transition tr;
	int mCount = charSetSize, count;
	RegExp mState = NULL;
	ESet elt;
	int i, j;
	RegExp cSet[256];
	int nrBytes = 0;
	int tmpJump;

	/* MOVE.L A0,A1
	   MOVE.W #$FFFF,D0
	   CLR.W D1 */
	if (generate)
	{
		*code++ = 0x22;
		*code++ = 0x48;
		*code++ = 0x30;
		*code++ = 0x3C;
		*code++ = 0xFF;
		*code++ = 0xFF;
		*code++ = 0x42;
		*code++ = 0x41;
	}
	nrBytes += 8;
	for (state = dfa; state != NULL; state = state->next)
	{
		for (i = 0; i != charSetSize; i++) cSet[i] = NULL;
		for (tr = state->transitions; tr != NULL; tr = tr->next)
		{
			cSet[tr->transitionChar] = tr->destination;
		}
		mCount = -1; mState = NULL;
		for (i = 0; i != charSetSize; i++)
		{
			if (cSet[i] != mState) /* not the most efficient check, but ok */
			{
				for (count = 0, j = i; j != charSetSize; j++)
					if (cSet[j] == cSet[i] && (j == i || cSet[j] != cSet[j - 1]))
						count++;
				if (count > mCount)
				{
					mCount = count;
					mState = cSet[i];
				}
			}
		}
		/* mCount is max. nr. of transitions to the same state, mState is that state */
		state->state.number = nrBytes;
		if (state->info != NULL)
		{
			Answer first;
			for (elt = state->info, first = elt->element; elt != NULL; elt = elt->next)
			{
				if (first->number > ((Answer) elt->element)->number)
				{
					first = elt->element;
				}
			}
			/* MOVEA.L #first->info,A0 */
			if (generate)
			{
				*code++ = 0x20;
				*code++ = 0x7C;
				*code++ = ((long int) first->info) >> 24;
				*code++ = (((long int) first->info) >> 16) & 0xFF;
				*code++ = (((long int) first->info) >> 8) & 0xFF;
				*code++ = ((long int) first->info) & 0xFF;
			}
			nrBytes += 6;
			/* MOVE.W D1,D0 */
			if (generate)
			{
				*code++ = 0x30;
				*code++ = 0x01;
			}
			nrBytes += 2;
		}
		/* MOVE.B (A1)+,D2
		   ADDQ.W #1,D1 */
		if (generate)
		{
			*code++ = 0x14;
			*code++ = 0x19;
			*code++ = 0x52;
			*code++ = 0x41;
		}
		nrBytes += 4;
		i = 0;
		while (i != charSetSize)
		{
			if (cSet[i] != mState)
			{
				if (i < charSetSize - 2 && cSet[i] == cSet[i + 1] && cSet[i] == cSet[i + 2])
				{
					if (i == 0)
					{
						tmpJump = -1;
					}
					else
					{
						/* CMP.B  #i,D2 */
						if (generate)
						{
							*code++ = 0x0C;
							*code++ = 0x02;
							*code++ = 0x00;
							*code++ = i & 0xFF;
						}
						nrBytes += 4;
						/* BLT    tempLabel */
						if (generate)
						{
							*code++ = 0x6D;
							*code++ = 0x08;
						}
						nrBytes += 2;
						tmpJump = nrBytes; /* wijzig code[tmpJump - nrBytes - 1] */
					}
					do
					{
						i++;
					}
					while (i != charSetSize && cSet[i] == cSet[i - 1]);
					i--;
					if (i == charSetSize - 1)
					{
						/* JMP */
						if (generate)
						{
							*code++ = 0x4E;
							*code++ = 0xFA;
						}
						nrBytes += 2;
						if (cSet[i] == NULL)
						{
							/* JMP    FAIL */
							if (generate)
							{
								*code++ = ((prevLen - nrBytes) >> 8) & 0xFF;
								*code++ = (prevLen - nrBytes) & 0xFF;
							}
							nrBytes += 2;
						}
						else if (cSet[i] == state && state->info != NULL)
						{
							/* JMP @cSet[i]->state.number + 6 */
							if (generate)
							{
								*code++ = ((cSet[i]->state.number + 6 - nrBytes) >> 8) & 0xFF;
								*code++ = (cSet[i]->state.number + 6 - nrBytes) & 0xFF;
							}
							nrBytes += 2;
						}
						else
						{
							/* JMP @cSet[i]->state.number */
							if (generate)
							{
								*code++ = ((cSet[i]->state.number - nrBytes) >> 8) & 0xFF;
								*code++ = (cSet[i]->state.number - nrBytes) & 0xFF;
							}
							nrBytes += 2;
						}
					}
					else
					{
						/* CMP.B  #$i,D2 */
						if (generate)
						{
							*code++ = 0x0C;
							*code++ = 0x02;
							*code++ = 0x00;
							*code++ = i & 0xFF;
						}
						nrBytes += 4;
						/* BLE */
						if (generate)
						{
							*code++ = 0x6F;
							*code++ = 0x00;
						}
						nrBytes += 2;
						if (cSet[i] == NULL)
						{
							/* BLE    FAIL */
							if (generate)
							{
								*code++ = ((prevLen - nrBytes) >> 8) & 0xFF;
								*code++ = (prevLen - nrBytes) & 0xFF;
							}
							nrBytes += 2;
						}
						else if (cSet[i] == state && state->info != NULL)
						{
							/* BLE    @cSet[i]->state.number + 6 */
							if (generate)
							{
								*code++ = ((cSet[i]->state.number + 6 - nrBytes) >> 8) & 0xFF;
								*code++ = (cSet[i]->state.number + 6 - nrBytes) & 0xFF;
							}
							nrBytes += 2;
						}
						else
						{
							/* BLE    @cSet[i]->state.number */
							if (generate)
							{
								*code++ = ((cSet[i]->state.number - nrBytes) >> 8) & 0xFF;
								*code++ = (cSet[i]->state.number - nrBytes) & 0xFF;
							}
							nrBytes += 2;
						}
					}
					/* tempLabel */
					if (generate && tmpJump != -1)
					{
						code[tmpJump - nrBytes - 1] = nrBytes - tmpJump;
					}
				}
				else
				{
					if (i == 0)
					{
						/* TST.B  D2 */
						if (generate)
						{
							*code++ = 0x4A;
							*code++ = 0x02;
						}
						nrBytes += 2;
					}
					else
					{
						/* CMP.B  #i,D2 */
						if (generate)
						{
							*code++ = 0x0C;
							*code++ = 0x02;
							*code++ = 0x00;
							*code++ = i & 0xFF;
						}
						nrBytes += 4;
					}
					/* BEQ */
					if (generate)
					{
						*code++ = 0x67;
						*code++ = 0x00;
					}
					nrBytes += 2;
					if (cSet[i] == NULL)
					{
						/* BEQ    FAIL */
						if (generate)
						{
							*code++ = ((prevLen - nrBytes) >> 8) & 0xFF;
							*code++ = (prevLen - nrBytes) & 0xFF;
						}
						nrBytes += 2;
					}
					else if (cSet[i] == state && state->info != NULL)
					{
						/* BEQ    @cSet[i]->state.number + 6 */
						if (generate)
						{
							*code++ = ((cSet[i]->state.number + 6 - nrBytes) >> 8) & 0xFF;
							*code++ = (cSet[i]->state.number + 6 - nrBytes) & 0xFF;
						}
						nrBytes += 2;
					}
					else
					{
						/* BEQ    @cSet[i]->state.number */
						if (generate)
						{
							*code++ = ((cSet[i]->state.number - nrBytes) >> 8) & 0xFF;
							*code++ = (cSet[i]->state.number - nrBytes) & 0xFF;
						}
						nrBytes += 2;
					}
				}
			}
			i++;
		}
		if (mState == NULL)
		{
			if (state->next != NULL)
			{
				/* JMP    FAIL */
				if (generate)
				{
					*code++ = 0x4E;
					*code++ = 0xFA;
					*code++ = ((prevLen - (nrBytes + 2)) >> 8) & 0xFF;
					*code++ = (prevLen - (nrBytes + 2)) & 0xFF;
				}
				nrBytes += 4;
			}
		}
		else
		{
			if (mState == state && state->info != NULL)
			{
				/* JMP    @mState->state.number + 6 */
				if (generate)
				{
					*code++ = 0x4E;
					*code++ = 0xFA;
					*code++ = ((mState->state.number + 6 - (nrBytes + 2)) >> 8) & 0xFF;
					*code++ = (mState->state.number + 6 - (nrBytes + 2)) & 0xFF;
				}
				nrBytes += 4;
			}
			else
			{
				/* JMP    @mState->state.number */
				if (generate)
				{
					*code++ = 0x4E;
					*code++ = 0xFA;
					*code++ = ((mState->state.number - (nrBytes + 2)) >> 8) & 0xFF;
					*code++ = (mState->state.number - (nrBytes + 2)) & 0xFF;
				}
				nrBytes += 4;
			}
		}
	}
	/* RTS
	 */
 /* if (generate)
	{
		RenumberDFA(dfa);
	} */
	return (nrBytes);
}

FRegExp CompileFDFA(RegExp dfa)
{
	RegExp state;
	Transition tr;
	FRegExp fDfa = InitFRegExp(dfa->info);
	FRegExp fState;
	int i;

	dfa->state.fRegExp = fDfa;
	for (state = dfa->next; state != NULL; state = state->next)
	{
		fState = MakeFRegExp(state->info);
		state->state.fRegExp = fState;
	}
	for (state = dfa; state != NULL; state = state->next)
	{
		fState = state->state.fRegExp;
		for (i = 0; i != charSetSize; i++)
		{
			fState->destination[i] = NULL;
		}
		for (tr = state->transitions; tr != NULL; tr = tr->next)
		{
			fState->destination[tr->transitionChar] = tr->destination->state.fRegExp;
		}
	}
	return (fDfa);
}

int MatchFDFA(FRegExp fDfa, char *line, ESet *answer)
{
	register FRegExp fState = fDfa;
	register unsigned char *ptr = (unsigned char *) line;
	ESet info = NULL;
	int rLen = -1;

	while (fState != NULL)
	{
		if (fState->info != NULL)
		{
			info = fState->info;
			rLen = ptr - (unsigned char *) line;
		}
		fState = fState->destination[*ptr++];
	}
	*answer = info;
	return (rLen);
}
