%{streams
#include <iostream>
%}streams
%{!streams
#include <cstdio>
%}!streams
#include <cstdlib>
#include <cstring>
%{llerror
#include <cstdarg>
%}llerror
%{unicode
#include <cstddef>
typedef wchar_t LLChar;
typedef wchar_t ULLChar;
%}unicode
%{!unicode
typedef char LLChar;
typedef unsigned char ULLChar;
%}!unicode
%{!llerror
%{!sllerror
extern void llerror(const char*, ...);
%}!sllerror
%}!llerror
%{nextsymbol
static void LocalnextSymbol(void);
static void nextSymbol(void);
%}nextsymbol

%initcode

struct DFAState
{
    short int ch;
    int next;
    unsigned char *llTokenSet;
};
typedef struct DFAState DFATable[];

%%\#define endOfInput %N
%%\#define nrTokens %N
#define llTokenSetSize ((nrTokens + 7) / 8)
typedef unsigned char LLTokenSet[llTokenSetSize];

%{functions
typedef struct
{
    int (*test)(const LLChar*);
    int tokenValue;
} Function;

typedef struct FunctionList
{
    Function *array;
    int length;
} FunctionList;

%}functions
%{keywords
typedef struct KeyWord
{
    char *keywordString;
    int tokenValue;
} KeyWord;

typedef struct KeyWordList
{
    KeyWord *array;
    int length;
} KeyWordList;

%}keywords
%{enumid
enum {
%enum
};
%}enumid

static unsigned char* currSymbol;
%{!noinput
%{stringinput
static const LLChar *inputString;
%}stringinput
%{!stringinput
%{!streams
static FILE* inputFile;
%}!streams
%{streams
static istream* inputFile;
%}streams
%}!stringinput
%}!noinput
%{noinput
static void* inputPtr;
%}noinput
%%static LLTokenSet endOfInputSet = {%E}; 
%{llerror
%{!noinput
%{!stringinput
const char* inputFileName = "(stdin)";
%}!stringinput
%}!noinput
static int llLineNumber = 1;
static int llLinePosition = 1;
int errorOccurred = 0;
%}llerror
%{!llerror
extern int llLineNumber;
extern int llLinePosition;
%}!llerror
%{functions
%functionlist
%{!keywords
static LLTokenSet llKeyWordSet;
%}!keywords
%}functions
%{keywords
static LLTokenSet llKeyWordSet;
%keywordlist
%}keywords

%prototypes
%{!main
%{!noinput
%{stringinput
%%void parse_%S(const LLChar \*ipStr*, *S);
%}stringinput
%{!stringinput
%%void parse_%S(FILE \*ipFile*, *S);
%}!stringinput
%}!noinput
%{noinput
%%void parse_%S(void \*ipPtr*, *S);
%}noinput

%}!main

%{llerror
%{!sllerror

#include <cstdio>

static void llerror(const char* format, ...)
{
    va_list args;

%{!noinput
%{!stringinput
    fprintf(stderr, "%s:%d:%d: ", inputFileName, llLineNumber, llLinePosition);
%}!stringinput
%{stringinput
    fprintf(stderr, "%d:%d: ", llLineNumber, llLinePosition);
%}stringinput
%}!noinput
%{noinput
    fprintf(stderr, "%d:%d: ", llLineNumber, llLinePosition);
%}noinput
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    errorOccurred = 1;
%{exitonerror
    exit(2);
%}exitonerror
}

%}!sllerror
%}llerror
#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 1024
#endif

static unsigned char *uniteTokenSets(LLTokenSet a, LLTokenSet b, LLTokenSet c)
{
    int i;

    for (i = 0; i != llTokenSetSize; i++)
    {
        a[i] = b[i] | c[i];
    }
    return (a);
}

static int memberTokenSet(int token, LLTokenSet set)
{
    return (set[token / 8] & (1 << (token % 8)));
}

static void SetTokenSet(LLTokenSet set, int token, int value)
{
    if (value == 0) set[token / 8] &= ~(1 << (token % 8));
    else set[token / 8] |= (1 << (token % 8));
}

static int tokenInCommon(LLTokenSet set1, LLTokenSet set2)
{
    int i;

    for (i = 0; i != llTokenSetSize; i++)
    {
        if (*set1 & *set2) return (1);
        set1++;
        set2++;
    }
    return (0);
}

static LLChar scanBuffer[MAX_BUFFER_SIZE];
static LLChar lastSymbol[MAX_BUFFER_SIZE];
static int bufferEnd = 0;
static int bufferFill = 0;
static int atEOF = 0;
%tokensets
static struct DFAState scanTab[] = {
%scantable
};

static unsigned char *nextState(int* state, short int ch)
{
    int i = *state;

    while (scanTab[i].ch != ch && scanTab[i].ch != -1)
    {
        i++;
    }
    *state = scanTab[i].next;
    return (scanTab[i].llTokenSet);
}

%{keywords
static int KeywordCmp(const void *k1, const void *k2)
{
    LLChar* s1 = (LLChar*) k1;
    char* s2 = ((KeyWord *) k2)->keywordString;

    while (*s1 != 0 && *s2 != 0)
    {
	if (*s1 != *s2)
	{
	    break;
	}
	s1++; s2++;
    }
    return (*s1 - *s2);
}

static unsigned char *lLKeyWord(LLTokenSet tokenSet)
{
    int i;

    for (i = 0; i != nrTokens; i++)
    {
        if (memberTokenSet(i, tokenSet) && keywordList[i].array != NULL)
        {
            KeyWord *keyword;
            LLChar fChar = scanBuffer[bufferEnd];
            scanBuffer[bufferEnd] = '\0';
            keyword = (KeyWord*) bsearch(scanBuffer, keywordList[i].array, keywordList[i].length, sizeof(KeyWord), KeywordCmp);
            scanBuffer[bufferEnd] = fChar;
            if (keyword != NULL)
            {
                memset(llKeyWordSet, 0, sizeof(LLTokenSet));
                llKeyWordSet[keyword->tokenValue / 8] = 1 << (keyword->tokenValue % 8);
                return (llKeyWordSet);
            }
        }
    }
    return (tokenSet);
}

%}keywords
%{functions
static int LLFunction(LLTokenSet tokenSet, unsigned char** replacement)
{
    int i, j;
    int nrRecognized = 0;
    LLChar fChar = scanBuffer[bufferEnd];

    scanBuffer[bufferEnd] = '\0';
    memset(llKeyWordSet, 0, sizeof(LLTokenSet));
    for (i = 0; i != nrTokens; i++)
    {
        if (memberTokenSet(i, tokenSet) && functionList[i].array != NULL)
        {
            for (j = 0; j != functionList[i].length; j++)
            {
                if ((*functionList[i].array[j].test)(scanBuffer))
                {
                    scanBuffer[bufferEnd] = fChar;
                    llKeyWordSet[functionList[i].array[j].tokenValue / 8] |= 1 << (functionList[i].array[j].tokenValue % 8);
                    nrRecognized++;
                }
            }
        }
    }
    scanBuffer[bufferEnd] = fChar;
    if (nrRecognized != 0)
    {
        *replacement = llKeyWordSet;
        return (1);
    }
    return (0);
}

%}functions
static int NotEmpty(unsigned char *tSet)
{
    int i = llTokenSetSize - 1;

    if (*tSet > 1) return (1);
    tSet++;
    while (i != 0)
    {
        if (*tSet++ != 0) return (1);
        i--;
    }
    return (0);
}

%{!noinput
int llNrCharactersRead = 0;
%{unicode
%{stringinput
static int nrBytesRead = 0;
static void GetOffset(void)
{
}
%}stringinput
%{!stringinput
%{!streams
static fpos_t offset;
static void GetOffset(void)
{
    offset = ftell(inputFile);
}
%}!streams
%{streams
static streamoff offset;
static void GetOffset(void)
{
    offset = inputFile->tellg();
}
%}streams
%}!stringinput

static int GetNextByte(unsigned char* ch)
{
%{stringinput
    if (*inputString == '\0')
    {
	return (0);
    }
    else
    {
	nrBytesRead++;
	*ch = *inputString++;
	return (1);
    }
%}stringinput
%{!stringinput
%{!streams
    return ((*ch = fgetc(inputFile)) != EOF);
%}!streams
%{streams
    return (!inputFile->bad() && inputFile->get(*(char*)ch));
%}streams
%}!stringinput
}
%}unicode
%{!unicode
static int getNextCharacter(LLChar* ch)
{
%{stringinput
    if (*inputString == '\0')
    {
	return (0);
    }
    else
    {
	*ch = *inputString++;
	llNrCharactersRead++;
	return (1);
    }
%}stringinput
%{!stringinput
%{!streams
    if ((*ch = fgetc(inputFile)) != EOF)
%}!streams
%{streams
    if (!inputFile->bad() && inputFile->get(*(char*)ch))
%}streams
    {
	llNrCharactersRead++;
	return (1);
    }
    return (0);
%}!stringinput
}
%}!unicode

%{unicode
typedef enum UnicodeFileEncodingType
{
    ucs4_be,
    ucs4_le,
    utf16_be,
    utf16_le,
    utf8,
    ebcdic,
    isolatin1
} UnicodeFileEncodingType;

static UnicodeFileEncodingType encoding;
static int validStream = 0; 

static void ResetToBeginningOfFile(void)
{
%{stringinput
    inputString -= nrBytesRead;
    nrBytesRead = 0;
%}stringinput
%{!stringinput
%{!streams
    fseek(inputFile, 0, SEEK_SET);
%}!streams
%{streams
    inputFile->seekg(0, ios::beg);
%}streams
%}!stringinput
}
 
static void ResetToOffsetPlus(int n)
{
%{stringinput
    inputString -= nrBytesRead - n;
    nrBytesRead = n;
%}stringinput
%{!stringinput
%{!streams
    fsetpos(inputFile, &offset);
    fseek(inputFile, n, SEEK_CUR);
%}!streams
%{streams
    inputFile->seekg(offset + n, ios::beg);
%}streams
%}!stringinput
}
 
static int Eof(void)
{
%{stringinput
    return (*inputString == 0); 
%}stringinput
%{!stringinput
%{!streams
    return (feof(inputFile));
%}!streams
%{streams
    return (inputFile->eof());
%}streams
%}!stringinput
}
 
static void DetermineFileType(void)
{
    unsigned char ch0, ch1, ch2, ch3;

    validStream = 1;
    /* The Unicode tags are at the beginning of the file, but we may be a little bit further ... */
    GetOffset();
    ResetToBeginningOfFile();
    if (!GetNextByte(&ch0) || !GetNextByte(&ch1))
    {
	encoding = isolatin1;
	ResetToOffsetPlus(0);
    }
    else if (ch0 == 0xFE && ch1 == 0xFF)
    {
	encoding = utf16_be;
	ResetToOffsetPlus(2);
    }
    else if (ch0 == 0xFF && ch1 == 0xFE)
    {
	encoding = utf16_le;
	ResetToOffsetPlus(2);
    }
    else if (ch0 == 0357 && ch1 == 0273 && ch2 == 0277)
    {
	encoding = utf8;
	ResetToOffsetPlus(2);
    }
    else
    {
	if (!GetNextByte(&ch2) || !GetNextByte(&ch3))
	{
	    encoding = isolatin1;
	    ResetToOffsetPlus(0);
	}
	else if (ch0 == 0x00 && ch1 == 0x00 && ch2 == 0xFE && ch3 == 0xFF)
	{
	    encoding = ucs4_be;
	    ResetToOffsetPlus(4);
	}
	else if (ch0 == 0xFF && ch1 == 0xFE && ch2 == 0x00 && ch3 == 0x00)
	{
	    encoding = ucs4_le;
	    ResetToOffsetPlus(4);
	}
	else if (ch0 == 0x00 && ch1 == 0x3C && ch2 == 0x00 && ch3 == 0x3F)
	{
	    encoding = utf16_be;
	    ResetToOffsetPlus(4);
	}
	else if (ch0 == 0x3C && ch1 == 0x00 && ch2 == 0x3F && ch3 == 0x00)
	{
	    encoding = utf16_le;
	    ResetToOffsetPlus(4);
	}
	else if (ch0 == 0x3C && ch1 == 0x3F && ch2 == 0x78 && ch3 == 0x6D)
	{
	    encoding = utf8;
	    ResetToOffsetPlus(4);
	}
	else if (ch0 == 0x4C && ch1 == 0x6F && ch2 == 0xA7 && ch3 == 0x94)
	{
	    encoding = ebcdic;
	    ResetToOffsetPlus(4);
	}
	else if (ch0 == 0 && ch1 == 0 && ch2 == 0xFE && ch3 == 0xFF)
	{
	    encoding = ucs4_be;
	    ResetToOffsetPlus(4);
	}
	else
	{
	    encoding = isolatin1;
	    ResetToOffsetPlus(0);
	}
    }
}
 
static int getUCS4BE(LLChar* ch)
{
    unsigned char ch0, ch1, ch2, ch3;

    if (!GetNextByte(&ch0) || !GetNextByte(&ch1) || !GetNextByte(&ch2) || !GetNextByte(&ch3))
    {
	return (0);
    }
    *ch = (ch3 << 24) | (ch2 << 16) | (ch1 << 8) | ch0;
    return (1);
}

static int getUCS4LE(LLChar* ch)
{
    unsigned char ch0, ch1, ch2, ch3;

    if (!GetNextByte(&ch0) || !GetNextByte(&ch1) || !GetNextByte(&ch2) || !GetNextByte(&ch3))
    {
	return (0);
    }
    *ch = (ch0 << 24) | (ch1 << 16) | (ch2 << 8) | ch3;
    return (1);
}

#define halfBase		0x0010000UL
#define halfShift		10
#define UNI_SUR_HIGH_START	0xD800
#define UNI_SUR_HIGH_END	0xDBFF
#define UNI_SUR_LOW_START	0xDC00
#define UNI_SUR_LOW_END		0xDFFF

static int getUTF16BE(LLChar* ch)
{
    unsigned char ch0, ch1;

    if (!GetNextByte(&ch0) || !GetNextByte(&ch1))
    {
	return (0);
    }
    *ch = (ch0 << 8) | ch1;
    if (*ch >= UNI_SUR_HIGH_START && *ch <= UNI_SUR_HIGH_END)
    {
	if (!GetNextByte(&ch0) || !GetNextByte(&ch1))
	{
	    return (0);
	}
	LLChar ch2 = (ch0 << 8) | ch1;
	if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
	{
	    *ch = ((*ch - UNI_SUR_HIGH_START) << halfShift)
		+ (ch2 - UNI_SUR_LOW_START) + halfBase;
	}
	else
	{
	    return (0);
	}
    }
    return (1);
}

static int getUTF16LE(LLChar* ch)
{
    unsigned char ch0, ch1;

    if (!GetNextByte(&ch0) || !GetNextByte(&ch1))
    {
	return (0);
    }
    *ch = (ch1 << 8) | ch0;
    if (*ch >= UNI_SUR_HIGH_START && *ch <= UNI_SUR_HIGH_END)
    {
	if (!GetNextByte(&ch0) || !GetNextByte(&ch1))
	{
	    return (0);
	}
	LLChar ch2 = (ch1 << 8) | ch0;
	if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END)
	{
	    *ch = ((*ch - UNI_SUR_HIGH_START) << halfShift)
		+ (ch2 - UNI_SUR_LOW_START) + halfBase;
	}
	else
	{
	    return (0);
	}
    }
    return (1);
}

static int getNextCharacter(LLChar* ch)
{
    static int unicodeInitialized = 0;

    if (!unicodeInitialized)
    {
	DetermineFileType();
	unicodeInitialized = 1;
    }
    if (!validStream || Eof())
    {
	return (0);
    }
    switch (encoding)
    {
	case ucs4_be:
	    return (getUCS4BE(ch));
	case ucs4_le:
	    return (getUCS4LE(ch));
	case utf16_be:
	    return (getUTF16BE(ch));
	case utf16_le:
	    return (getUTF16LE(ch));
	case isolatin1:
	    {
		unsigned char byte;
		int ret = GetNextByte(&byte);
		*ch = (LLChar) byte;
		return (ret);
	    }
	default:
	    validStream = 0;
	    return (0);
    }
}
%}unicode
%}!noinput
%{noinput
static int getNextCharacter(LLChar*);
%}noinput

%{nextsymbol
static void LocalnextSymbol(void)
%}nextsymbol
%{!nextsymbol
static void nextSymbol(void)
%}!nextsymbol
{
    int bufferPos;
    int state = 0;
    unsigned char *recognizedToken = NULL;
    unsigned char *token;
    LLChar ch;

    /* Copy last recognized symbol into buffer and adjust positions */
    for (bufferPos = 0; bufferPos != bufferEnd; bufferPos++)
    {
            if (scanBuffer[bufferPos] == '\n')
            {
                llLineNumber++;
                llLinePosition = 1;
            }
            else
            {
                llLinePosition++;
            }
            lastSymbol[bufferPos] = scanBuffer[bufferPos];
    }
    lastSymbol[bufferEnd] = '\0';
    bufferFill -= bufferEnd; /* move remains of scanBuffer to beginning */
    memmove(scanBuffer, scanBuffer + bufferEnd, (size_t) bufferFill * sizeof(LLChar)); /* expensive for larger buffers; should use round robin? */
    bufferPos = 0;
    bufferEnd = 0;
    while (bufferPos != bufferFill || !atEOF)
    {
        if (bufferPos != bufferFill)
        {
            ch = scanBuffer[bufferPos++];
        }
    	else if (atEOF || !getNextCharacter(&ch))
        {
            atEOF = 1;
        }
        else
        {
            if (bufferPos < MAX_BUFFER_SIZE - 1) {  
                scanBuffer[bufferPos++] = ch;
            } else {
                bufferPos++;
            }
            bufferFill++;
        }
        if (atEOF)
        {
            state = -1;
        }
        else if ((token = nextState(&state, (ULLChar) ch)) != NULL)
        {
            recognizedToken = token;
            bufferEnd = bufferPos;
        }
        if (state == -1)
        {
            if (atEOF && bufferFill == 0)
            {
                currSymbol = endOfInputSet;
                return;
            }
            if (recognizedToken == NULL)
            {
                llerror("Illegal character: %c (%02x)\n", scanBuffer[0], scanBuffer[0]);
                bufferEnd = 1;
            }
            else if (NotEmpty(recognizedToken))
            {
%{functions
                if (LLFunction(recognizedToken, &currSymbol))
                {
                	return;
                }
%}functions
%{keywords
                currSymbol = lLKeyWord(recognizedToken);
%}keywords
%{!keywords
                currSymbol = recognizedToken;
%}!keywords
                return;
            }
            /* If nothing recognized, continue; no need to copy buffer */
            for (bufferPos = 0; bufferPos != bufferEnd; bufferPos++)
            {
                if (scanBuffer[bufferPos] == '\n')
                {
                    llLineNumber++;
                    llLinePosition = 1;
                }
                else
                {
                    llLinePosition++;
                }
            }
            bufferFill -= bufferEnd; /* move remains of scanBuffer to beginning */
            memmove(scanBuffer, scanBuffer + bufferEnd, (size_t) bufferFill * sizeof(LLChar)); /* too expensive for larger buffers; use round robin in that case */
            recognizedToken = 0;
            state = 0;
            bufferEnd = 0;
            bufferPos = 0;
        }
    }
    currSymbol = endOfInputSet;
}

static int CurrentSymbolLength(void)
{
    return (bufferEnd);
}

static void CurrentSymbol(LLChar *buf)
{
    memcpy(buf, scanBuffer, (size_t) bufferEnd * sizeof(LLChar));
    buf[bufferEnd] = '\0';
}

static void waitForToken(LLTokenSet set, LLTokenSet follow)
{
    LLTokenSet ltSet;

    (void) uniteTokenSets(ltSet, set, follow);
    while (currSymbol != endOfInputSet && !tokenInCommon(currSymbol, ltSet))
    {
            nextSymbol();
        llerror("token skipped: %s", lastSymbol);
    }
}

static void getToken(int token, LLTokenSet set, LLTokenSet follow)
{
    LLTokenSet ltSet;

    (void) uniteTokenSets(ltSet, set, follow);
    while (currSymbol != endOfInputSet && !memberTokenSet(token, currSymbol) && !tokenInCommon(currSymbol, ltSet))
    {
        nextSymbol();
        if (!memberTokenSet(0, currSymbol)) {
            llerror("token skipped: %s", lastSymbol);
        }
    }
    if (!memberTokenSet(token, currSymbol))
    {
        llerror("token expected: %s", tokenName[token]);
    }
    else
    {
        nextSymbol();
    }
}

%parsefunctions
%{main
%exitcode

int main(int argc, char *argv[])
{
#ifdef __MWERKS__
    argc = ccommand(&argv);
#endif
%{llerror
%{argv0
    argv0 = argv[0];
%}argv0
%}llerror
    if (argc > 1)
    {
%{!stringinput
        inputFileName = argv[1];
%}!stringinput
        if (freopen(argv[1], "r", stdin) == NULL)
        {
            perror(argv[1]);
            exit(1);
        }
    }
    inputFile = stdin;
    nextSymbol();
%%    %S(#S);
    if (!memberTokenSet(endOfInput, currSymbol))
    {
        llerror("end of input expected");
    }
    return (0);
}
%}main
%{!main
%{!noinput
%{stringinput
%%void parse_%S(const LLChar \*ipStr*, *S)
{
    inputString = ipStr;
%}stringinput
%{!stringinput
%{!streams
%%void parse_%S(FILE \*ipFile*, *S)
%}!streams
%{streams
%%void parse_%S(istream \*ipFile*, *S)
%}streams
{
    inputFile = ipFile;
%}!stringinput
%}!noinput
%{noinput
%%void parse_%S(void\* ipPtr*, *S)
{
    inputPtr = ipPtr;
%}noinput
    bufferEnd = 0;
    bufferFill = 0;
    atEOF = 0;
    llLineNumber = 1;
    llLinePosition = 1;
    nextSymbol();
%%    %S(#S);
    if (!memberTokenSet(endOfInput, currSymbol))
    {
        llerror("end of input expected");
    }
}

%exitcode
%}!main
