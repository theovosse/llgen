#include "bool.h"

#if YYLSP_NEEDED
#define yyoverflow(str, a, b, c, d, e, f, g) yyerror(str)
#else
#define yyoverflow(str, a, b, c, d, e) yyerror(str)
#endif

typedef enum NameType {
    identifier,
    string,
    output,
    code,
    token,
    function
} NameType;

typedef enum NodeType {
    single,
    option,
    sequence,
    chain,
    alternative
} NodeType;

typedef struct Name *Name;
typedef struct Node *Node;
typedef struct NodeList *NodeList;
typedef struct RuleResult *RuleResult;

struct Node {
    short int lineNr;
    NodeType nodeType;
    Node arglist;
    Name assignTo;
    union {
        Name name;
        Node sub;
        NodeList alternatives;
    } element;
    Boolean preferShift;
    Boolean empty;
    Node next;
};

struct NodeList {
    Node first;
    NodeList rest;
    Boolean empty;
};

struct RuleResult {
    Name returnVariable;
    Name returnType;
    Name initialValue;
};

struct Name {
    char *name;
    TREE first;
    TREE follow;
    NodeList rules;
    int lineNr;
    long int index;
    Boolean empty;
    Boolean usesLA; ///< when true, function calls uniteTokenSets, so C needs a
                    ///< declaration of an auxiliary variable.
    Boolean isTerminal; ///< When true, scanner token is generated
    NameType nameType;
    Node arglist;
    int nrParameters;
    RuleResult ruleResult;
    TREE keywords;
    TREE functions;
};

extern int yylex(void);
extern int yyparse(void);
extern void yyerror(char *fmt, ...);

extern Name MakeName(char *name, int index, NameType nameType, short int lineNr);
extern Name InsertSymbol(char *str, NodeList rules, Boolean lhs, NameType nameType);
extern Name InsertString(char *str);
extern void AssignString(Name name, Name str);
extern void KeywordString(Name name, Name str, Name ident);
extern void Function(Name name, Name fun, Name ident);
extern Node MakeNode(short int lineNr, NodeType nodeType, void *element, Node next);
extern NodeList MakeNodeList(Node first, NodeList rest);
extern int NameCmp(Name str1, Name str2);
extern int NameCmpStr(Name str1, char *str2);
extern char *MakeString(char *);
extern void AddProperty(char *property);
extern void RetractProperty(char *property);
extern Node AppendNode(Node node1, Node node2);
extern void doIndent(int indent);
extern void startBlock(int indent);
extern void terminateBlock(int indent);
extern RuleResult MakeRuleResult(Name, Name, Name);
extern NodeList appendNodeList(NodeList list, NodeList app);
extern Boolean containsGrammarSymbol(NodeList list);
extern int nrParameters(Node argList);

/* interface with grammar/scanner */
extern char *yytext;
extern int lineNumber;
extern int cLineNumber;
extern int nrBlanks;
extern FILE *yyin;
extern FILE *outputFile;
extern Name startSymbol;
extern Name endOfInput;
extern char codeBlock[];
extern Node preCodeBlocks, postCodeBlocks;

/* interface with lang.c */
extern int nrBitsInElement;
extern Boolean bracesOnNewLine;
extern char llTokenSetTypeString[];
extern int globalIndent;
