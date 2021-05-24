%initcode

%%const endOfInput = %N;
%%const nrTokens = %N;
%%const endOfInputSet = [%E]; 
const llTokenSetSize = Math.floor((nrTokens + 30) / 31);
var llLineNumber = 1;
var llLinePosition = 1;
var errorOccurred = false;
var inputFileName;
var currSymbol = [];

function sprintf(args) {
    var format = args[0];
    var argIndex = 1;
    var output = "";

    for (var i = 0; i < format.length; i++) {
        var ch = format[i];
        if (ch == '%') {
            var argm = format.slice(i).match(/%([0-9.*]*)([a-zA-Z])/);
            if (argm !== undefined) {
                var argarg = argm[1];
                var argtype = argm[2];
                i += argm[0].length - 1;
                if (argarg == ".*" || argarg == "*") {
                    var maxlen = Number(args[argIndex++]);
                    var argt = args[argIndex++];
                    output += argt.slice(0, maxlen);
                } else {
                    output += args[argIndex++];
                }
            }
        } else {
            output += ch;
        }
    }
    return output;
}
exports.sprintf = sprintf;

%{llerror
function llerror() {
    var msg = sprintf(arguments);

    console.log("llerror " + lastSymbolPos.line + ":" + lastSymbolPos.position + ": " + msg);
}

%}llerror
var scanBuffer = "";
%{ignorebuffer
let ignoreBuffer = "";
%}ignorebuffer
var lastSymbol = "";
var lastSymbolPos = {line: 0, position: 0};
var bufferEnd = 0;
var bufferFill = 0;
var atEOF = false;
%tokensets
const scanTab = [
%scantable
];
%keywordlist

function nextState(state, ch)
{
    var tab = scanTab[state.state];

    if (tab === undefined) {
        state.state = undefined;
        return undefined;
    }
    var transition = ch in tab? tab[ch]: tab[''];
    state.state = transition.destination;
    return transition.accept;
}

function uniteTokenSets(a, b, c)
{
    for (var i = 0; i < b.length || i < c.length; i++) {
        a[i] = b[i] | c[i];
    }
    return a;
}

function tokenInCommon(a, b)
{
    for (var i = 0; i < a.length && i < b.length; i++) {
        if ((a[i] & b[i]) !== 0) {
            return true;
        }
    }
    return false;
}

function waitForToken(set, follow)
{
    var ltSet = [];

    uniteTokenSets(ltSet, set, follow);
    while (currSymbol !== endOfInputSet && !tokenInCommon(currSymbol, ltSet)) {
        nextSymbol();
        llerror("token skipped: %s", lastSymbol);
    }
}

function memberTokenSet(token, set)
{
    return (set[Math.floor(token / 31)] & (1 << (token % 31))) !== 0;
}

function NotEmpty(tSet)
{
    if (tSet[0] > 1)
        return true;
    for (var i = 1; i < tSet.length; i++)
        if (tSet[i] > 0)
            return true;
    return false;
}

%{keywords
function lLKeyWord(tokenSet)
{
    var keywordText = scanBuffer.slice(0, bufferEnd);

    for (var i = 0; i != nrTokens; i++) {
        if (memberTokenSet(i, tokenSet) && keywordList[i] != undefined) {
            var keyword = keywordList[i][keywordText];
            if (keyword !== undefined) {
                var llKeyWordSet = [];
                llKeyWordSet[Math.floor(keyword / 31)] = 1 << (keyword % 31);
                return llKeyWordSet;
            }
        }
    }
    return tokenSet;
}

%}keywords
function nextSymbol()
{
    var bufferPos;
    var state = { state: 0 };
    var recognizedToken = undefined;
    var token;
    var ch;
    var lastNlPos = 0, nlPos = 0;

%{ignorebuffer
%{!keepignorebuffer
    ignoreBuffer = "";
%}!keepignorebuffer
%}ignorebuffer
    /* Copy last recognized symbol into buffer and adjust positions */
    lastSymbol = scanBuffer.slice(0, bufferEnd);
    lastSymbolPos.line = llLineNumber;
    lastSymbolPos.position = llLinePosition;
    bufferFill -= bufferEnd; /* move remains of scanBuffer to beginning */
    while ((nlPos = scanBuffer.indexOf('\n', nlPos)) != -1 && nlPos < bufferEnd) {
        llLineNumber++;
        lastNlPos = nlPos;
        llLinePosition = 0;
        nlPos++;
    }
    llLinePosition += bufferEnd - lastNlPos;
    scanBuffer = scanBuffer.slice(bufferEnd); /* expensive for larger buffers; should use round robin? repeated below */
    bufferPos = 0;
    bufferEnd = 0;
    while (bufferPos !== bufferFill || !atEOF) {
        if (bufferPos !== bufferFill) {
            ch = scanBuffer[bufferPos++];
        } else if (atEOF || !(ch = getNextCharacter())) {
            atEOF = true;
        } else {
            scanBuffer += ch;
            bufferPos++;
            bufferFill++;
        }
        if (atEOF) {
            state.state = undefined;
        } else if ((token = nextState(state, ch)) !== undefined) {
            recognizedToken = token;
            bufferEnd = bufferPos;
        }
        if (state.state === undefined) {
            if (atEOF && bufferFill == 0) {
                currSymbol = endOfInputSet;
                return;
            }
            if (recognizedToken === undefined) {
                llerror("Illegal character: '%c'\n", scanBuffer[0]);
                bufferEnd = 1;
            } else if (NotEmpty(recognizedToken)) {
%{keywords
                currSymbol = lLKeyWord(recognizedToken);
%}keywords
%{!keywords
                currSymbol = recognizedToken;
%}!keywords
                return;
            }
%{ignorebuffer
            ignoreBuffer += scanBuffer.substr(0, bufferEnd);
%}ignorebuffer
%{!ignorebuffer
            /* If nothing recognized, continue; no need to copy buffer */
%}!ignorebuffer
            lastNlPos = nlPos = 0;
            while ((nlPos = scanBuffer.indexOf('\n', nlPos)) != -1 && nlPos < bufferEnd) {
                llLineNumber++;
                lastNlPos = nlPos;
                llLinePosition = 0;
                nlPos++;
            }
            llLinePosition += bufferEnd - lastNlPos;
            bufferFill -= bufferEnd;
            scanBuffer = scanBuffer.slice(bufferEnd);
            recognizedToken = undefined;
            state.state = 0;
            bufferEnd = 0;
            bufferPos = 0;
        }
    }
    currSymbol = endOfInputSet;
}

function getToken(token, set, follow)
{
    var ltSet = [];

    uniteTokenSets(ltSet, set, follow);
    while (currSymbol != endOfInputSet && !memberTokenSet(token, currSymbol) &&
           !tokenInCommon(currSymbol, ltSet)) {
        nextSymbol();
        if (!memberTokenSet(0, currSymbol)) {
            llerror("token skipped: %s", lastSymbol);
        }
    }
    if (!memberTokenSet(token, currSymbol)) {
        llerror("token expected: %s", tokenName[token]);
    } else {
        nextSymbol();
    }
}

function toSymbolList(set)
{
    var list = [];

    for (var i = 0; i < nrTokens; i++) {
        if (memberTokenSet(i, set)) {
            list.push(tokenName[i]);
        }
    }
    return list;
}

%parsefunctions

%exitcode

var inputString, inputPosition;

function getNextCharacter() {
    return inputPosition < inputString.length?
           inputString[inputPosition++]: undefined;
}

%%exports.parse = function(str, *S) {
    inputString = str;
    inputPosition = 0;
    llLineNumber = 1;
    llLinePosition = 1;
    errorOccurred = false;
    currSymbol = [];
    scanBuffer = "";
    lastSymbol = "";
    lastSymbolPos = {line: 0, position: 0};
    bufferEnd = 0;
    bufferFill = 0;
    atEOF = false;
    nextSymbol();
%%    %S(#S);
};
