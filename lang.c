#include <string.h>
#include <stdio.h>
#include "bool.h"
#include "avl3.h"
#include "llgen.h"
#include "lang.h"

static int nrFunctionParameters;
static const char* lastParameterName;
static int lastParameterNameSpaces;
static int firstTypeNameSpaces;
static const char* returnType;
static Boolean inParamType;

static void spaces(FILE* f, int nrSpaces, int minNrSpaces) {
    if (nrSpaces < minNrSpaces) {
        nrSpaces = minNrSpaces;
    }
    for (int i = 0; i < nrSpaces; i++) {
        fputc(' ', f);
    }
}

static void cLanguageInit() {
	nrBitsInElement = 8;
}

static void cLanguageStartDoWhileLoop(FILE* outputFile, int indent) {
    fputs("do", outputFile);
    startBlock(indent);
}

static void cLanguageEndDoWhileLoop(FILE* outputFile, Boolean open, int indent) {
    if (open) {
        terminateBlock(indent);
        if (bracesOnNewLine) {
           doIndent(indent);
        } else {
            fputc(' ', outputFile);
        }
        fprintf(outputFile, "while (");
    } else {
        fprintf(outputFile, ");");
    }
}

static void cLanguageArrayDeclaration(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size, Boolean init) {
    fprintf(f, "%s %s %s", scope, type, name);
    if (suffix >= 0) {
        fprintf(f, "%ld", suffix);
    }
    if (size >= 0) {
        fprintf(f, "[%d]", size);
    } else {
        fprintf(f, "[]");
    }
    if (init) {
        fprintf(f, " = ");
    }
}

static void cLanguageLocalDeclaration(FILE* f, const char* type, const char* name) {
    fprintf(f, "%s %s", type, name);
}

static void cLanguageLocalDeclarationWithInitialization(FILE* f, const char* type, const char* name, const char* initialValue) {
    fprintf(f, "%s %s = %s", type, name, initialValue);
}

static void cLanguageReturnValue(FILE* f, const char* returnValue) {
    fprintf(f, "return %s", returnValue);
}

static void cLanguageStateTransitionStart(FILE* f, long int nr) {
    fprintf(f, "/* %3ld */ ", nr);
}

static void cLanguageStateTransition(FILE* f, int ch, long int destination, const char* accept) {
    fprintf(f, "{ ");
    if (ch == -1) {
        fprintf(f, "-1");
    } else if (ch == '\'' || ch == '\\') {
        fprintf(f, "'\\%c'", ch);
    } else if (' ' <= ch && ch <= '~') {
        fprintf(f, "'%c'", ch);
    } else {
        fprintf(f, "'\\x%02x'", ch);
    }
    fprintf(f, ", %ld, %s },\n", destination, accept);
}

static void cLanguageEmptyStateTransition(FILE* f, long int nr) {
}

static void cLanguageStateTransitionEnd(FILE* f) {
}

static void cLanguagePointerToArray(FILE* f, const char* name, long int nr) {
    fputs(name, f);
    if (nr >= 0) {
        fprintf(f, "%ld", nr);
    }
}

static void cLanguageTokenEnum(FILE* f, long int index, const char* name) {
    fputs(name, f);
}

static void cLanguageStartFunctionDeclaration(FILE* f, const char* functionName, const char* type) {
    fprintf(f, "static %s %s(", type, functionName);
    nrFunctionParameters = 0;
}

static void cLanguageParameterDeclarationName(FILE* f, const char* str, int nrSpaces) {
    if (nrFunctionParameters > 0) {
        spaces(f, firstTypeNameSpaces, 1);
        fprintf(f, " %s,", lastParameterName);
    }
    lastParameterName = str;
    lastParameterNameSpaces = nrSpaces;
    inParamType = false;
    nrFunctionParameters++;
}

static void cLanguageNextDeclarationParameterSymbol(FILE* f, const char* str, int nrSpaces) {
    if (!inParamType) {
        spaces(f, lastParameterNameSpaces, nrFunctionParameters > 0? 1: 0);
        firstTypeNameSpaces = nrSpaces;
        inParamType = true;
    } else {
        spaces(f, nrSpaces, 0);
    }
    fputs(str, f);
}

static void cLanguageEndFunctionDeclaration(FILE* f) {
    if (nrFunctionParameters > 0) {
        spaces(f, lastParameterNameSpaces, 1);
        fprintf(f, " %s", lastParameterName);
    }
    fprintf(f, ")");
}

static void cLanguageStartFunctionCall(FILE* f, const char*functionName, const char* returnType, const char* assignTo) {
    if (returnType != NULL && assignTo != NULL) {
        fprintf(f, "%s %s = ", returnType, assignTo);
    }
    fprintf(f, "%s(", functionName);
    nrFunctionParameters = 0;
}

static void cLanguageNextCallParameter(FILE* f) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ", ");
    }
    nrFunctionParameters++;
}

static void cLanguageNextCallParameterSymbol(FILE* f, const char* str) {
    fputs(str, f);
    nrFunctionParameters++;
}

static void cLanguageEndFunctionCall(FILE* f) {
    fprintf(f, ")");
}

static  void cLanguageResetFunctionCall(FILE* f) {
    nrFunctionParameters = 0;
}

static  void cLanguageEmitParameterDeclaration(FILE* f) {
    fprintf(f, "%s", lastParameterName);
}

Language cLanguage = {
	"c",
	cLanguageInit,
    true,
    "llerror",
    "",
    "syntax error after %s %.*s",
    ", lastSymbol, bufferEnd, scanBuffer",
    "",
    "NULL",
    "char*",
    "unsigned char",
    "KeyWordList",
    "{NULL, 0}",
    "{",
    "}",
    ";",
    ";",
    "if (",
    ")",
    "else",
    "for (;;)",
    "break",
    cLanguageStartDoWhileLoop,
    cLanguageEndDoWhileLoop,
    cLanguageArrayDeclaration,
    cLanguageLocalDeclaration,
    cLanguageLocalDeclarationWithInitialization,
    cLanguageReturnValue,
    cLanguageStateTransitionStart,
    cLanguageStateTransition,
    cLanguageEmptyStateTransition,
    cLanguageStateTransitionEnd,
    cLanguagePointerToArray,
    cLanguageTokenEnum,
    cLanguageStartFunctionDeclaration,
    cLanguageParameterDeclarationName,
    cLanguageNextDeclarationParameterSymbol,
    cLanguageEndFunctionDeclaration,
    cLanguageStartFunctionCall,
    cLanguageNextCallParameter,
    cLanguageNextCallParameterSymbol,
    cLanguageEndFunctionCall,
    cLanguageResetFunctionCall,
    cLanguageEmitParameterDeclaration
};

static void cppLanguageInit() {
	nrBitsInElement = 8;
}

Language cppLanguage = {
	"cpp",
	cppLanguageInit,
    true,
    "llerror",
    "",
    "syntax error after %s %.*s",
    ", lastSymbol, bufferEnd, scanBuffer",
    "",
    "NULL",
    "const char*",
    "unsigned char",
    "KeyWordList",
    "{NULL, 0}",
    "{",
    "}",
    ";",
    ";",
    "if (",
    ")",
    "else",
    "for (;;)",
    "break",
    cLanguageStartDoWhileLoop,
    cLanguageEndDoWhileLoop,
    cLanguageArrayDeclaration,
    cLanguageLocalDeclaration,
    cLanguageLocalDeclarationWithInitialization,
    cLanguageReturnValue,
    cLanguageStateTransitionStart,
    cLanguageStateTransition,
    cLanguageEmptyStateTransition,
    cLanguageStateTransitionEnd,
    cLanguagePointerToArray,
    cLanguageTokenEnum,
    cLanguageStartFunctionDeclaration,
    cLanguageParameterDeclarationName,
    cLanguageNextDeclarationParameterSymbol,
    cLanguageEndFunctionDeclaration,
    cLanguageStartFunctionCall,
    cLanguageNextCallParameter,
    cLanguageNextCallParameterSymbol,
    cLanguageEndFunctionCall,
    cLanguageResetFunctionCall,
    cLanguageEmitParameterDeclaration
};

static void jsLanguageInit() {
	nrBitsInElement = 31;
    AddProperty("jsts");
}

static void jsLanguageArrayDeclaration(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size, Boolean init) {
    fprintf(f, "var %s", name);
    if (suffix >= 0) {
        fprintf(f, "%ld", suffix);
    }
    if (init) {
        fprintf(f, " = [");
    }
}

static void jsLanguageLocalDeclaration(FILE* f, const char* type, const char* name) {
    fprintf(f, "var %s = []", name);
}

static void jsLanguageLocalDeclarationWithInitialization(FILE* f, const char* type, const char* name, const char* initialValue) {
    fprintf(f, "var %s = %s", name, initialValue);
}

static void jsLanguageReturnValue(FILE* f, const char* returnValue) {
    fprintf(f, "return %s", returnValue);
}

static void jsLanguageStateTransitionStart(FILE* f, long int nr) {
    fprintf(f, "/* %3ld */ {\n          ", nr);
}

static void jsLanguageStateTransition(FILE* f, int ch, long int destination, const char* accept) {
    fprintf(f, "  ");
    if (ch == -1) {
        fprintf(f, "''");
    } else if (ch == '\'' || ch == '\\') {
        fprintf(f, "'\\%c'", ch);
    } else if (' ' <= ch && ch <= '~') {
        fprintf(f, "'%c'", ch);
    } else {
        fprintf(f, "'\\x%02x'", ch);
    }
    fprintf(f, ": {");
    if (destination != -1) {
        fprintf(f, "destination:%ld", destination);
    }
    if (strcmp(accept, "undefined") != 0) {
        if (destination != -1) {
            fprintf(f, ",");
        }
        fprintf(f, "accept:%s", accept);
    }
    fprintf(f, "},\n");
}

static void jsLanguageEmptyStateTransition(FILE* f, long int nr) {
    fprintf(f, "/* %3ld */ undefined,\n", nr);
}

static void jsLanguageStateTransitionEnd(FILE* f) {
    fprintf(f, "          },\n");
}

static void jsLanguagePointerToArray(FILE* f, const char* name, long int nr) {
    fputs(name, f);
    if (nr >= 0) {
        fprintf(f, "%ld", nr);
    }
}

static void jsLanguageTokenEnum(FILE* f, long int index, const char* name) {
    fprintf(f, "%ld/*%s*/", index, name);
}

static void jsLanguageStartFunctionDeclaration(FILE* f, const char* functionName, const char* type) {
    fprintf(f, "function %s(", functionName);
    nrFunctionParameters = 0;
}

static void jsLanguageParameterDeclarationName(FILE* f, const char* str, int nrSpaces) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ", ");
    }
    fputs(str, f);
    nrFunctionParameters++;
}

static void jsLanguageNextDeclarationParameterSymbol(FILE* f, const char* str, int nrSpaces) {
}

static void jsLanguageEndFunctionDeclaration(FILE* f) {
    fputs(")", f);
}

static void jsLanguageStartFunctionCall(FILE* f, const char*functionName, const char* returnType, const char* assignTo) {
    if (returnType != NULL && assignTo != NULL) {
        fprintf(f, "var %s = ", assignTo);
    }
    fprintf(f, "%s(", functionName);
    nrFunctionParameters = 0;
}

static void jsLanguageNextCallParameter(FILE* f) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ", ");
    }
    nrFunctionParameters++;
}

static void jsLanguageNextCallParameterSymbol(FILE* f, const char* str) {
    fputs(str, f);
    nrFunctionParameters++;
}

static void jsLanguageEndFunctionCall(FILE* f) {
    fputs(")", f);
}

static void jsLanguageResetFunctionCall(FILE* f) {
    nrFunctionParameters = 0;
}

static void jsLanguageEmitParameterDeclaration(FILE* f) {
}

Language jsLanguage = {
	"js",
	jsLanguageInit,
    false,
    "llerror",
    "",
    "syntax error after %s %.*s",
    ", lastSymbol, bufferEnd, scanBuffer",
    "",
    "undefined",
    "string",
    NULL,
    "KeyWordList",
    "undefined",
    "[",
    "]",
    ";",
    ";",
    "if (",
    ")",
    "else",
    "for (;;)",
    "break",
    cLanguageStartDoWhileLoop,
    cLanguageEndDoWhileLoop,
    jsLanguageArrayDeclaration,
    jsLanguageLocalDeclaration,
    jsLanguageLocalDeclarationWithInitialization,
    jsLanguageReturnValue,
    jsLanguageStateTransitionStart,
    jsLanguageStateTransition,
    jsLanguageEmptyStateTransition,
    jsLanguageStateTransitionEnd,
    jsLanguagePointerToArray,
    jsLanguageTokenEnum,
    jsLanguageStartFunctionDeclaration,
    jsLanguageParameterDeclarationName,
    jsLanguageNextDeclarationParameterSymbol,
    jsLanguageEndFunctionDeclaration,
    jsLanguageStartFunctionCall,
    jsLanguageNextCallParameter,
    jsLanguageNextCallParameterSymbol,
    jsLanguageEndFunctionCall,
    jsLanguageResetFunctionCall,
    jsLanguageEmitParameterDeclaration
};

static void tsLanguageInit() {
    AddProperty("jsts");
	nrBitsInElement = 31;
}

static void tsLanguageArrayDeclaration(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size, Boolean init) {
    fprintf(f, "let %s", name);
    if (suffix >= 0) {
        fprintf(f, "%ld", suffix);
    }
    if (init) {
        fprintf(f, ": %s[] = [", type);
    }
}

static void tsLanguageLocalDeclaration(FILE* f, const char* type, const char* name) {
    fprintf(f, "let %s: %s = %s", name, type,
            strcmp(type, llTokenSetTypeString) == 0? "[]": "undefined");
}

static void tsLanguageLocalDeclarationWithInitialization(FILE* f, const char* type, const char* name, const char* initialValue) {
    fprintf(f, "var %s: %s = %s", name, type, initialValue);
}

static void tsLanguageStartFunctionDeclaration(FILE* f, const char* functionName, const char* type) {
    fprintf(f, "function %s(", functionName);
    returnType = type;
    nrFunctionParameters = 0;
}

static void tsLanguageParameterDeclarationName(FILE* f, const char* str, int nrSpaces) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ",");
    }
    spaces(f, nrSpaces, nrFunctionParameters > 0? 1: 0);
    fputs(str, f);
    inParamType = false;
    nrFunctionParameters++;
}

static void tsLanguageNextDeclarationParameterSymbol(FILE* f, const char* str, int nrSpaces) {
    if (!inParamType) {
        fputs(":", f);
        inParamType = true;
    }
    spaces(f, nrSpaces, 0);
    fputs(str, f);
}

static void tsLanguageEndFunctionDeclaration(FILE* f) {
    fprintf(f, "): %s", returnType);
}

static void tsLanguageStartFunctionCall(FILE* f, const char*functionName, const char* returnType, const char* assignTo) {
    if (returnType != NULL && assignTo != NULL) {
        fprintf(f, "let %s: %s = ", assignTo, returnType);
    }
    fprintf(f, "%s(", functionName);
    nrFunctionParameters = 0;
}

static void tsLanguageResetFunctionCall(FILE* f) {
    nrFunctionParameters = 0;
}

static void tsLanguageEmitParameterDeclaration(FILE* f) {
}

Language tsLanguage = {
	"ts",
	tsLanguageInit,
    false,
    "llerror",
    "",
    "syntax error after %s %.*s",
    ", lastSymbol, bufferEnd, scanBuffer",
    "",
    "undefined",
    "string",
    "number",
    "KeyWordList",
    "undefined",
    "[",
    "]",
    ";",
    ";",
    "if (",
    ")",
    "else",
    "for (;;)",
    "break",
    cLanguageStartDoWhileLoop,
    cLanguageEndDoWhileLoop,
    tsLanguageArrayDeclaration,
    tsLanguageLocalDeclaration,
    tsLanguageLocalDeclarationWithInitialization,
    jsLanguageReturnValue,
    jsLanguageStateTransitionStart,
    jsLanguageStateTransition,
    jsLanguageEmptyStateTransition,
    jsLanguageStateTransitionEnd,
    jsLanguagePointerToArray,
    jsLanguageTokenEnum,
    tsLanguageStartFunctionDeclaration,
    tsLanguageParameterDeclarationName,
    tsLanguageNextDeclarationParameterSymbol,
    tsLanguageEndFunctionDeclaration,
    tsLanguageStartFunctionCall,
    jsLanguageNextCallParameter,
    jsLanguageNextCallParameterSymbol,
    jsLanguageEndFunctionCall,
    tsLanguageResetFunctionCall,
    tsLanguageEmitParameterDeclaration
};

static void goLanguageInit() {
	nrBitsInElement = 32;
    bracesOnNewLine = false;
}

static void goLanguageStartDoWhileLoop(FILE* outputFile, int indent) {
    fputs("for", outputFile);
    startBlock(indent);
}

static void goLanguageEndDoWhileLoop(FILE* outputFile, Boolean open, int indent) {
    if (open) {
        doIndent(indent + 1);
        fprintf(outputFile, "if !");
    } else {
        startBlock(indent + 1);
        doIndent(indent + 2);
        fputs("break", outputFile);
        terminateBlock(indent + 1);
        terminateBlock(indent);
    }
}

static void goLanguageArrayDeclaration(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size, Boolean init) {
    fprintf(f, "var %s", name);
    if (suffix >= 0) {
        fprintf(f, "%ld", suffix);
    }
    if (init) {
        fprintf(f, " =");
    }
    if (size >= 0) {
        fprintf(f, " [%d]%s", size, type);
    } else {
        fprintf(f, " []%s", type);
    }
    if (init) {
        fprintf(f, "{");
    }
}

static void goLanguageLocalDeclaration(FILE* f, const char* type, const char* name) {
    fprintf(f, "var %s %s", name,
            (strcmp(type, llTokenSetTypeString) == 0? "[llTokenSetSize]uint32": type));
}

static void goLanguageLocalDeclarationWithInitialization(FILE* f, const char* type, const char* name, const char* initialValue) {
    fprintf(f, "var %s %s = %s", name, type, initialValue);
}

static void goLanguageReturnValue(FILE* f, const char* returnValue) {
    fprintf(f, "return %s", returnValue);
}

static void goLanguageStateTransitionStart(FILE* f, long int nr) {
    fprintf(f, "/* %3ld */ ", nr);
}

static void goLanguageStateTransition(FILE* f, int ch, long int destination, const char* accept) {
    fprintf(f, "{");
    if (ch == -1) {
        fprintf(f, "-1");
    } else if (ch == '\'' || ch == '\\') {
        fprintf(f, "'\\%c'", ch);
    } else if (' ' <= ch && ch <= '~') {
        fprintf(f, "'%c'", ch);
    } else {
        fprintf(f, "'\\x%02x'", ch);
    }
    if (strcmp(accept, "nil") == 0) {
        fprintf(f, ", %ld, %s},\n", destination, accept);
    } else {
        fprintf(f, ", %ld, &%s},\n", destination, accept);
    }
}

static void goLanguageEmptyStateTransition(FILE* f, long int nr) {
}

static void goLanguageStateTransitionEnd(FILE* f) {
}

static void goLanguagePointerToArray(FILE* f, const char* name, long int nr) {
    fprintf(f, "&%s", name);
    if (nr >= 0) {
        fprintf(f, "%ld", nr);
    }
}

static void goLanguageTokenEnum(FILE* f, long int index, const char* name) {
    fprintf(f, "%ld/*%s*/", index, name);
}

static void goLanguageStartFunctionDeclaration(FILE* f, const char* functionName, const char* type) {
    fprintf(f, "func %s(", functionName);
    returnType = type;
    nrFunctionParameters = 0;
}

static void goLanguageParameterDeclarationName(FILE* f, const char* str, int nrSpaces) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ",");
    }
    spaces(f, nrSpaces, nrFunctionParameters > 0? 1: 0);
    fputs(str, f);
    nrFunctionParameters++;
}

static void goLanguageNextDeclarationParameterSymbol(FILE* f, const char* str, int nrSpaces) {
    spaces(f, nrSpaces, 0);
    fputs(str, f);
}

static void goLanguageEndFunctionDeclaration(FILE* f) {
    fprintf(f, ")");
    if (strcmp(returnType, "void") != 0) {
        fprintf(f, " %s", returnType);
    }
}

static void goLanguageStartFunctionCall(FILE* f, const char*functionName, const char* returnType, const char* assignTo) {
    if (returnType != NULL && assignTo != NULL) {
        fprintf(f, "var %s %s = ", assignTo, returnType);
    }
    fprintf(f, "%s(", functionName);
    nrFunctionParameters = 0;
}

static void goLanguageNextCallParameter(FILE* f) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ", ");
    }
    nrFunctionParameters++;
}

static void goLanguageNextCallParameterSymbol(FILE* f, const char* str) {
    fputs(str, f);
    nrFunctionParameters++;
}

static void goLanguageEndFunctionCall(FILE* f) {
    fputs(")", f);
}

static  void goLanguageResetFunctionCall(FILE* f) {
    nrFunctionParameters = 0;
}

static  void goLanguageEmitParameterDeclaration(FILE* f) {
}

Language goLanguage = {
	"go",
	goLanguageInit,
    false,
    "llerror",
    "",
    "syntax error after %s %.*s",
    ", lastSymbol, bufferEnd, scanBuffer",
    "",
    "nil",
    "string",
    "uint32",
    "keyWordList",
    "nil",
    "{",
    "}",
    "",
    "",
    "if ",
    "",
    "else",
    "for",
    "break",
    goLanguageStartDoWhileLoop,
    goLanguageEndDoWhileLoop,
    goLanguageArrayDeclaration,
    goLanguageLocalDeclaration,
    goLanguageLocalDeclarationWithInitialization,
    goLanguageReturnValue,
    goLanguageStateTransitionStart,
    goLanguageStateTransition,
    goLanguageEmptyStateTransition,
    goLanguageStateTransitionEnd,
    goLanguagePointerToArray,
    goLanguageTokenEnum,
    goLanguageStartFunctionDeclaration,
    goLanguageParameterDeclarationName,
    goLanguageNextDeclarationParameterSymbol,
    goLanguageEndFunctionDeclaration,
    goLanguageStartFunctionCall,
    goLanguageNextCallParameter,
    goLanguageNextCallParameterSymbol,
    goLanguageEndFunctionCall,
    goLanguageResetFunctionCall,
    goLanguageEmitParameterDeclaration
};

static void rustLanguageInit() {
    nrBitsInElement = 32;
    bracesOnNewLine = false;
}

static void rustLanguageStartDoWhileLoop(FILE* outputFile, int indent) {
    fputs("loop", outputFile);
    startBlock(indent);
}

static void rustLanguageEndDoWhileLoop(FILE* outputFile, Boolean open, int indent) {
    if (open) {
        doIndent(indent + 1);
        fprintf(outputFile, "if !");
    } else {
        startBlock(indent + 1);
        doIndent(indent + 2);
        fputs("break", outputFile);
        terminateBlock(indent + 1);
        terminateBlock(indent);
    }
}

static void rustLanguageArrayDeclaration(FILE* f, const char* scope, const char* type, const char* name, long int suffix, int size, Boolean init) {
    fprintf(f, "const %s", name);
    if (suffix >= 0) {
        fprintf(f, "%ld", suffix);
    }
    if (size >= 0) {
        fprintf(f, ": [%s; %d]", type, size);
    } else {
        fprintf(f, ": [%s]", type);
    }
    if (init) {
        fprintf(f, " = [");
    }
}

static void rustLanguageLocalDeclaration(FILE* f, const char* type, const char* name) {
    fprintf(f, "let %s: %s", name,
            (type == llTokenSetTypeString == 0? "[u32; llTokenSetSize]": type));
}

static void rustLanguageLocalDeclarationWithInitialization(FILE* f, const char* type, const char* name, const char* initialValue) {
    fprintf(f, "let %s: %s = %s", name, type, initialValue);
}

static void rustLanguageReturnValue(FILE* f, const char* returnValue) {
    fprintf(f, "return %s", returnValue);
}

static void rustLanguageStateTransitionStart(FILE* f, long int nr) {
    fprintf(f, "/* %3ld */ ", nr);
}

static void rustLanguageStateTransition(FILE* f, int ch, long int destination, const char* accept) {
    fprintf(f, "DFAState {ch: ");
    if (ch == -1) {
        fputs("0xff", f);
    } else {
        fprintf(f, "0x%02x", (unsigned) ch);
        switch (ch) {
            case '/': case '!':
                fprintf(f, "/* %c */", ch);
                break;
            case '\t':
                fputs("/*\\t*/", f);
                break;
            case '\n':
                fputs("/*\\n*/", f);
                break;
            case '\r':
                fputs("/*\\r*/", f);
                break;
            default:
                if (' ' <= ch && ch < 127) {
                    fprintf(f, "/*%c*/", ch);
                }
        }
    }
    if (destination == -1) {
        fprintf(f, ", next: 0xffffffff");
    } else {
        fprintf(f, ", next: %ld", destination);
    }
    if (strcmp(accept, "nil") == 0) {
        fprintf(f, ", llTokenSet: None},\n");
    } else {
        fprintf(f, ", llTokenSet: Some(%s)},\n", accept);
    }
}

static void rustLanguageEmptyStateTransition(FILE* f, long int nr) {
}

static void rustLanguageStateTransitionEnd(FILE* f) {
}

static void rustLanguagePointerToArray(FILE* f, const char* name, long int nr) {
    fprintf(f, "%s", name);
    if (nr >= 0) {
        fprintf(f, "%ld", nr);
    }
}

static void rustLanguageTokenEnum(FILE* f, long int index, const char* name) {
    fprintf(f, "%ld/*%s*/", index, name);
}

static void rustLanguageStartFunctionDeclaration(FILE* f, const char* functionName, const char* type) {
    if (globalIndent == 0) {
        fprintf(f, "    ");
        globalIndent = 1;
    }
    fprintf(f, "fn %s(&mut self", functionName);
    returnType = type;
    nrFunctionParameters = 1;
}

static void rustLanguageParameterDeclarationName(FILE* f, const char* str, int nrSpaces) {
    if (nrFunctionParameters > 0) {
        fputc(',', f);
    }
    spaces(f, nrSpaces, nrFunctionParameters > 0? 1: 0);
    fputs(str, f);
    fputc(':', f);
    nrFunctionParameters++;
}

static void rustLanguageNextDeclarationParameterSymbol(FILE* f, const char* str, int nrSpaces) {
    spaces(f, nrSpaces, 0);
    fputs(str, f);
}

static void rustLanguageEndFunctionDeclaration(FILE* f) {
    fprintf(f, ")");
    if (strcmp(returnType, "void") != 0) {
        fprintf(f, " -> %s", returnType);
    }
}

static void rustLanguageStartFunctionCall(FILE* f, const char*functionName, const char* returnType, const char* assignTo) {
    if (returnType != NULL && assignTo != NULL) {
        fprintf(f, "let %s: %s = ", assignTo, returnType);
    }
    fprintf(f, "%s(", functionName);
    nrFunctionParameters = 0;
}

static void rustLanguageNextCallParameter(FILE* f) {
    if (nrFunctionParameters > 0) {
        fprintf(f, ", ");
    }
    nrFunctionParameters++;
}

static void rustLanguageNextCallParameterSymbol(FILE* f, const char* str) {
    fputs(str, f);
    nrFunctionParameters++;
}

static void rustLanguageEndFunctionCall(FILE* f) {
    fputs(")", f);
}

static void rustLanguageResetFunctionCall(FILE* f) {
    nrFunctionParameters = 0;
}

static void rustLanguageEmitParameterDeclaration(FILE* f) {
}

Language rustLanguage = {
    "rs",
    rustLanguageInit,
    false,
    "llerror!",
    "self, ",
    "syntax error",
    "",
    "self.",
    "nil",
    "&'static str",
    "u32",
    "KeywordList",
    "nil",
    "[",
    "]",
    ";",
    ";",
    "if ",
    "",
    "else",
    "loop",
    "break",
    rustLanguageStartDoWhileLoop,
    rustLanguageEndDoWhileLoop,
    rustLanguageArrayDeclaration,
    rustLanguageLocalDeclaration,
    rustLanguageLocalDeclarationWithInitialization,
    rustLanguageReturnValue,
    rustLanguageStateTransitionStart,
    rustLanguageStateTransition,
    rustLanguageEmptyStateTransition,
    rustLanguageStateTransitionEnd,
    rustLanguagePointerToArray,
    rustLanguageTokenEnum,
    rustLanguageStartFunctionDeclaration,
    rustLanguageParameterDeclarationName,
    rustLanguageNextDeclarationParameterSymbol,
    rustLanguageEndFunctionDeclaration,
    rustLanguageStartFunctionCall,
    rustLanguageNextCallParameter,
    rustLanguageNextCallParameterSymbol,
    rustLanguageEndFunctionCall,
    rustLanguageResetFunctionCall,
    rustLanguageEmitParameterDeclaration
};
