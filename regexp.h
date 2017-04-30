#include "bool.h"

typedef struct RegExp *RegExp;
typedef struct FRegExp *FRegExp;

typedef struct Answer
{
	int number;
	void *info;
} *Answer;
	
typedef struct ESet *ESet;
struct ESet
{
    void *element;
    ESet next;
};

typedef struct Transition *Transition;
struct Transition
{
	int transitionChar;
	RegExp destination;
	Transition next;
};

struct RegExp
{
	Transition transitions;
	Transition eTransitions;
	ESet info;
	RegExp next;
	long int lastVisit;
	union
	{
		long int number;
		ESet set;
		FRegExp fRegExp;
	} state;
};

struct FRegExp
{
	ESet info;
	FRegExp next;
	FRegExp destination[256];
};

RegExp MakeRegExp(Transition transitions, Transition eTransitions, ESet info);
void ELink(RegExp from, RegExp to);
Answer MakeAnswer(int number, void *info);
Transition MakeTransition(int transitionChar, RegExp destination, Transition next);
void DumpRegExp(RegExp fa);
void CountNrStates(RegExp regExp);
RegExp InitRegExp(void);
int AddRegExp(RegExp nfa, char *pattern, void *info);
ESet MatchNFA(RegExp nfa, char *line);
void RemoveNFA(RegExp nfa);
RegExp N2DFA(RegExp nfa);
void MinimizeDFA(RegExp dfa);
ESet MatchDFA(RegExp nfa, char *line);
void RemoveDFA(RegExp dfa);
void PrepareMatchNFA2(RegExp regExp);
ESet MatchNFA2(RegExp regExp, char *line);
void FinalizeMatchNFA2(void);
void PrintDFA(RegExp dfa);
void DisassembleDFA(RegExp dfa);
int CompileDFA(RegExp dfa, int prevLen, unsigned char *code, Boolean generate);
FRegExp CompileFDFA(RegExp dfa);
void SetCharSetSize(int cSize);
int CharSetSize(void);
void SetSingleAnswer(Boolean sAnswer);
Boolean SingleAnswer(void);
int MatchFDFA(FRegExp fDfa, char *line, ESet *answer);
Boolean SetMember(void *element, ESet set);
Boolean SubSet(ESet s1, ESet s2);
Boolean SetEqual(ESet s1, ESet s2);
ESet AddElement(void *element, ESet next);
void RemoveSet(ESet set);
void push_localadm(void);
void pop_localadm(void);

extern Boolean viMode;
extern Boolean caseSens;
extern RegExp currentAutomaton;
