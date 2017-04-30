%=TlLTokenSet
%%package %P

import (
	"bufio"
	"fmt"
	"os"
)
%initcode

var ErrorOccurred = false
var inputFileName string

func llerror(format string, v ...interface{}) {
	fmt.Printf("%s:%d:%d %s\n", inputFileName, llLineNumber, llLinePosition, fmt.Sprintf(format, v...))
	ErrorOccurred = true
}

%%const endOfInput = %N
const nrTokens = endOfInput + 1
const llTokenSetSize = ((nrTokens + 31) / 32)

type lLTokenSet *[llTokenSetSize]uint32

%%var endOfInputSet = [llTokenSetSize]uint32{%E}

var currSymbol lLTokenSet
var inputFile *bufio.Reader
var llLineNumber int = 1
var llLinePosition int = 1
var llNrCharactersRead uint32

type dFAState struct {
	ch         rune
	next	   int
	llTokenSet lLTokenSet
}

type keyWordList map[string]uint32

%keywordlist

func uniteTokenSets(a, b, c lLTokenSet) lLTokenSet {
	for i := 0; i != llTokenSetSize; i++ {
		a[i] = b[i] | c[i]
	}
	return a
}

func tokenInCommon(a, b lLTokenSet) bool {
	for i := 0; i != llTokenSetSize; i++ {
		if a[i]&b[i] != 0 {
			return true
		}
	}
	return false
}

func waitForToken(set, follow lLTokenSet) {
	var ltSet [llTokenSetSize]uint32

	uniteTokenSets(&ltSet, set, follow)
	for currSymbol != &endOfInputSet && !tokenInCommon(currSymbol, &ltSet) {
		nextSymbol()
		llerror("token skipped: %s", lastSymbol)
	}
}

func memberTokenSet(token uint, set lLTokenSet) bool {
	return set[token/32]&(1<<(token%32)) != 0
}

%tokensets
var scanTab = []dFAState{
%scantable
}

func nextState(state *int, ch rune) lLTokenSet {
	var i = *state

	for scanTab[i].ch != ch && scanTab[i].ch != -1 {
		i++
	}
	*state = scanTab[i].next
	return scanTab[i].llTokenSet
}

%{keywords
func llKeyWord(tokenSet lLTokenSet) lLTokenSet {
	keywordText := string(scanBuffer[0:bufferEnd])
	for i := 0; i != nrTokens-1; i++ {
		if memberTokenSet(uint(i), tokenSet) && keywordList[i] != nil {
			keyword, present := keywordList[i][keywordText]
			if present {
				var llKeyWordSet [llTokenSetSize]uint32
				llKeyWordSet[keyword/32] = 1 << (keyword % 32)
				return &llKeyWordSet
			}
		}
	}
	return tokenSet
}
%}keywords

func notEmpty(tSet lLTokenSet) bool {
	if tSet[0] > 1 {
		return true
	}
	for i := 1; i < llTokenSetSize; i++ {
		if tSet[i] != 0 {
			return true
		}
	}
	return false
}

func getNextCharacter(ch *rune) bool {
	var err error

	tch, _, err := inputFile.ReadRune()
	if err == nil {
		llNrCharactersRead++
		*ch = tch
		return true
	}
	return false
}

type symbolPos struct {
	line	 int
	position int
}

var scanBuffer []rune
var lastSymbol string
var lastSymbolPos symbolPos
var bufferEnd int
var bufferFill int
var atEOF bool

func findRune(s []rune, ch rune, offset int) (pos int, charFound bool) {
	l := len(s)
	for i := offset; i < l; i++ {
		if s[i] == ch {
			pos = i
			charFound = true
			return
		}
	}
	charFound = false
	return
}

func nextSymbol() {
	var bufferPos int
	var state int
	var recognizedToken lLTokenSet
	var token lLTokenSet
	var lastNlPos int
	var nlPos int
	var ch rune
	var chFound bool

	/* Copy last recognized symbol into buffer and adjust positions */
	lastSymbol = string(scanBuffer[0:bufferEnd])
	lastSymbolPos = symbolPos{llLineNumber, llLinePosition}
	bufferFill -= bufferEnd /* move remains of scanBuffer to beginning */
	nlPos, chFound = findRune(scanBuffer, '\n', 0)
	for chFound && nlPos < bufferEnd {
		llLineNumber++
		lastNlPos = nlPos
		llLinePosition = 0
		nlPos, chFound = findRune(scanBuffer, '\n', nlPos+1)
	}
	llLinePosition += bufferEnd - lastNlPos
	scanBuffer = scanBuffer[bufferEnd:] // expensive for larger buffers; should use round robin? repeated below
	bufferPos = 0
	bufferEnd = 0
	for bufferPos != bufferFill || !atEOF {
		if bufferPos != bufferFill {
			ch = scanBuffer[bufferPos]
			bufferPos++
		} else if atEOF || !getNextCharacter(&ch) {
			atEOF = true
		} else {
			scanBuffer = append(scanBuffer, ch)
			bufferPos++
			bufferFill++
		}
		if atEOF {
			state = -1
		} else {
			token = nextState(&state, ch)
			if token != nil {
				recognizedToken = token
				bufferEnd = bufferPos
			}
		}
		if state == -1 {
			if atEOF && bufferFill == 0 {
				currSymbol = &endOfInputSet
				return
			}
			if recognizedToken == nil {
				llerror("Illegal character: '%c'", scanBuffer[0])
				bufferEnd = 1
			} else if notEmpty(recognizedToken) {
				currSymbol = llKeyWord(recognizedToken)
				return
			}
			/* If nothing recognized, continue; no need to copy buffer */
			lastNlPos = 0
			nlPos, chFound = findRune(scanBuffer, '\n', 0)
			for chFound && nlPos < bufferEnd {
				llLineNumber++
				lastNlPos = nlPos
				llLinePosition = 0
				nlPos, chFound = findRune(scanBuffer, '\n', nlPos+1)
			}
			llLinePosition += bufferEnd - lastNlPos
			bufferFill -= bufferEnd
			scanBuffer = scanBuffer[bufferEnd:]
			recognizedToken = nil
			state = 0
			bufferEnd = 0
			bufferPos = 0
		}
	}
	currSymbol = &endOfInputSet
}

func getToken(token uint, set, follow lLTokenSet) {
	var ltSet [llTokenSetSize]uint32

	uniteTokenSets(&ltSet, set, follow)
	for currSymbol != &endOfInputSet && !memberTokenSet(token, currSymbol) && !tokenInCommon(currSymbol, &ltSet) {
		nextSymbol()
		if !memberTokenSet(0, currSymbol) {
			llerror("token skipped: %s", lastSymbol)
		}
	}
	if !memberTokenSet(token, currSymbol) {
		llerror("token expected: %s", tokenName[token])
	} else {
		nextSymbol()
	}
}

%parsefunctions

%exitcode

%%func Parse(f \*os.File, fName string, *S) {
	inputFile = bufio.NewReader(f)
	inputFileName = fName
	llLineNumber = 1
	llLinePosition = 1
	ErrorOccurred = false
	llNrCharactersRead = 0
	scanBuffer = make([]rune, 0, 64)
	lastSymbol = ""
	lastSymbolPos = symbolPos{0, 0}
	bufferEnd = 0
	bufferFill = 0
	atEOF = false
	nextSymbol()
%%    %S(#S);
}

%{main
func main() {
	f, err := os.Open(inputFileName)
	if err != nil {
		fmt.Printf("%s", err)
		os.Exit(1)
	}
	parse(f, os.Args[1])
}
%}main
