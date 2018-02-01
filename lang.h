#include <stdio.h>

/* Implementation interface for the code generator. Each language
   has to implement its own set.
 */
typedef struct Language {
    const char* extension; // Default extension for output file
    void (*initialize)(); // Should initialize llgen's settings
    Boolean requiresLocalLTSetStorage; // When true, a local ltSet is created for the union of token sets
    const char* errorReporter; // Function called for error handling
    const char* errorReporterArg; // First argument(s) to error reporter
    const char* errorMsg; // Error message when first of rule fails
    const char* errorArgs; // Arguments to error message
    const char* owner; // Expression owning currSymbol, getToken, etc.
    const char* nullValue; // String representing NULL
    const char* stringType; // Type for strings
    const char* tokenSetType; // Type for token sets
    const char* keyWordListType; // Type for keyword lists
    const char* emptyKeyWordListEntry; // etc.
    const char* startOfArrayLiteral;
    const char* endOfArrayLiteral;
    const char* terminateDeclaration;
    const char* terminateStatement;
    const char* openIf;
    const char* closeIf;
    const char* elseStr;
    const char* unconditionalLoop;
    const char* breakStatement;
    void (*startDoWhileLoop)(FILE* outputFile, int indent);
    void (*endDoWhileLoop)(FILE* outputFile, Boolean open, int indent);
    void (*arrayDeclaration)(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size, Boolean init);
    void (*localDeclaration)(FILE* f, const char* type, const char* name);
    void (*localDeclarationWithInitialization)(FILE* f, const char* type, const char* name, const char* initialValue);
    void (*returnValue)(FILE* f, const char* returnValue);
    void (*stateTransitionStart)(FILE* f, long int nr);
    void (*stateTransition)(FILE* f, int ch, long int destination, const char* accept);
    void (*emptyStateTransition)(FILE* f, long int nr);
    void (*stateTransitionEnd)(FILE* f);
    void (*pointerToArray)(FILE* f, const char* name, long int nr);
    void (*tokenEnum)(FILE* f, long int index, const char* name);
    void (*startFunctionDeclaration)(FILE* f, const char* functionName, const char* type);
    void (*parameterDeclarationName)(FILE* f, const char* str, int nrSpaces);
    void (*nextDeclarationParameterSymbol)(FILE* f, const char* str, int nrSpaces);
    void (*endFunctionDeclaration)(FILE* f);
    void (*startFunctionCall)(FILE* f, const char*functionName, const char* returnType, const char* assignTo);
    void (*nextCallParameter)(FILE* f);
    void (*nextCallParameterSymbol)(FILE* f, const char* str);
    void (*endFunctionCall)(FILE* f);
    void (*resetFunctionCall)(FILE* f);
    void (*emitParameterDeclaration)(FILE* f);
    void (*assignLastSymbolTo)(FILE* f, int indent, Name name);
} Language;

extern Language cLanguage;
extern Language cppLanguage;
extern Language jsLanguage;
extern Language tsLanguage;
extern Language goLanguage;
extern Language rustLanguage;
