%initcode

type LLTokenSet = number[];

type KeyWordList = {[keyword:string]: number}|undefined;

%%const endOfInput = %N;
%%const nrTokens = %N;
%%const endOfInputSet = [%E]; 
%{regexpcontext
%%const inputFirsts: LLTokenSet = %F
%}regexpcontext
const llTokenSetSize = Math.floor((nrTokens + 30) / 31);
let llLineNumber = 1;
let llLinePosition = 1;
let errorOccurred = false;
let currSymbol: LLTokenSet = [];
interface localstate {
    state: number|undefined;
}

export function sprintf(args: any): string {
    let format = args[0];
    let argIndex = 1;
    let output = "";

    for (let i = 0; i < format.length; i++) {
        let ch = format[i];
        if (ch == '%') {
            let argm = format.slice(i).match(/%([0-9.*]*)([a-zA-Z])/);
            if (argm !== undefined) {
                let argarg = argm[1];
                let argtype = argm[2];
                i += argm[0].length - 1;
                if (argarg == ".*" || argarg == "*") {
                    let maxlen = Number(args[argIndex++]);
                    let argt = args[argIndex++];
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

%{llerror
function llerror(... args: any[]): void {
    let msg = sprintf(args);

    console.log("llerror " + lastSymbolPos.line + ":" + lastSymbolPos.position + ": " + msg);
}

%}llerror
export var inputFileName: string;
export function setInputFileName(fn: string): void {
    inputFileName = fn;
}
let scanBuffer = "";
%{ignorebuffer
export let ignoreBuffer = "";
%}ignorebuffer
let lastSymbol = "";
export let lastSymbolPos = {line: 0, position: 0};
let bufferEnd = 0;
let bufferFill = 0;
let atEOF = false;
%tokensets
const scanTab = [
%scantable
];
%keywordlist

function nextState(state: localstate, ch: string|undefined): LLTokenSet|undefined {
    let tab: any = state.state !== undefined? scanTab[state.state]: undefined;

    if (tab === undefined) {
        state.state = undefined;
        return undefined;
    }
    let transition = ch !== undefined && ch in tab? tab[ch]: tab[''];
    state.state = transition.destination;
    return transition.accept;
}

function uniteTokenSets(b: LLTokenSet, c: LLTokenSet): LLTokenSet {
    let a: LLTokenSet = [];

    for (let i = 0; i < llTokenSetSize; i++) {
        a[i] = b[i] | c[i];
    }
    return a;
}

function tokenInCommon(a: LLTokenSet, b: LLTokenSet): boolean {
    for (let i = 0; i < llTokenSetSize; i++) {
        if ((a[i] & b[i]) !== 0) {
            return true;
        }
    }
    return false;
}

function waitForToken(set: LLTokenSet, follow: LLTokenSet): void {
    let ltSet: LLTokenSet = uniteTokenSets(set, follow);

%{debug
    console.log(debugIndent + "waitForToken", tokenSetToString(set), tokenSetToString(follow));
%}debug
    while (currSymbol !== endOfInputSet && !tokenInCommon(currSymbol, ltSet)) {
%{regexpcontext
        nextSymbol(set, follow);
%}regexpcontext
%{!regexpcontext
        nextSymbol();
%}!regexpcontext
        llerror("token skipped: %s", lastSymbol);
    }
}

function memberTokenSet(token: number, set: LLTokenSet): boolean {
    return (set[Math.floor(token / 31)] & (1 << (token % 31))) !== 0;
}

%{regexpcontext
function makeTokenSet(token: number): LLTokenSet {
    let set: LLTokenSet = [];

    for (let i = 0; i < llTokenSetSize; i++) {
        set[i] = 0;
    }
    set[Math.floor(token / 31)] = 1 << (token % 31);
    return set;
}

function subtract(a: LLTokenSet, b: LLTokenSet): LLTokenSet {
    let set: LLTokenSet = [];

    for (let i = 0; i < llTokenSetSize; i++) {
        set[i] = a[i] & ~b[i];
    }
    return set;
}

%}regexpcontext
function notEmpty(tSet: LLTokenSet): boolean {
    if (tSet[0] > 1)
        return true;
    for (let i = 1; i < tSet.length; i++)
        if (tSet[i] > 0)
            return true;
    return false;
}

%{keywords
function lLKeyWord(tokenSet: LLTokenSet): LLTokenSet {
    let keywordText = scanBuffer.slice(0, bufferEnd);

    for (let i = 0; i != nrTokens; i++) {
        let kwi = keywordList[i];
        if (kwi != undefined && memberTokenSet(i, tokenSet) &&
              kwi.hasOwnProperty(keywordText)) {
            let keyword = kwi[keywordText];
            if (keyword !== undefined) {
                let llKeyWordSet: LLTokenSet = [];
                llKeyWordSet[Math.floor(keyword / 31)] = 1 << (keyword % 31);
                return llKeyWordSet;
            }
        }
    }
    return tokenSet;
}

%}keywords
%{regexpcontext
function nextSymbol(first: LLTokenSet, follow: LLTokenSet): void
%}regexpcontext
%{!regexpcontext
function nextSymbol(): void
%}!regexpcontext
{
    let bufferPos: number;
    let state: localstate = { state: 0 };
    let token: LLTokenSet|undefined;
    let ch: string|undefined;
    let lastNlPos = 0, nlPos = 0;
%{regexpcontext
    let recognizedTokens: LLTokenSet[] = [];
    let bufferEnds: number[] = [0, 0];
    let firstOrFollow = uniteTokenSets(first, follow);
%}regexpcontext
%{!regexpcontext
    let recognizedToken: LLTokenSet|undefined = undefined;
%}!regexpcontext

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
%{regexpcontext
            let prio = tokenInCommon(token, firstOrFollow)? 0: 1;
            recognizedTokens[prio] = token;
            bufferEnds[prio] = bufferPos;
%}regexpcontext
%{!regexpcontext
            recognizedToken = token;
%}!regexpcontext
            bufferEnd = bufferPos;
        }
        if (state.state === undefined) {
            if (atEOF && bufferFill == 0) {
                currSymbol = endOfInputSet;
%{debug
                console.log(debugIndent + "nextSymbol: EOF");
%}debug
                return;
            }
%{regexpcontext
            if (recognizedTokens[0] === undefined && recognizedTokens[1] === undefined) {
                llerror("Illegal character: '%c'\n", scanBuffer[0]);
                bufferEnd = 1;
            } else if (recognizedTokens[0] !== undefined && notEmpty(recognizedTokens[0])) {
%{keywords
                currSymbol = lLKeyWord(recognizedTokens[0]);
%}keywords
%{!keywords
                currSymbol = recognizedTokens[0];
%}!keywords
                bufferEnd = bufferEnds[0];
%{debug
                console.log(debugIndent + "nextSymbol prio 0:", '"' + debugPrintable(scanBuffer.slice(0, bufferEnd)) + '"', tokenSetToString(currSymbol));
%}debug
                return;
            } else if (recognizedTokens[1] !== undefined && notEmpty(recognizedTokens[1])) {
%{keywords
                currSymbol = lLKeyWord(recognizedTokens[1]);
%}keywords
%{!keywords
                currSymbol = recognizedTokens[1];
%}!keywords
                bufferEnd = bufferEnds[1];
%{debug
                console.log(debugIndent + "nextSymbol prio 1:", '"' + debugPrintable(scanBuffer.slice(0, bufferEnd)) + '"', tokenSetToString(currSymbol));
%}debug
                return;
            }
%}regexpcontext
%{!regexpcontext
            if (recognizedToken === undefined) {
                llerror("Illegal character: '%c'\n", scanBuffer[0]);
                bufferEnd = 1;
            } else if (notEmpty(recognizedToken)) {
%{keywords
                currSymbol = lLKeyWord(recognizedToken);
%}keywords
%{!keywords
                currSymbol = recognizedToken;
%}!keywords
%{debug
                console.log(debugIndent + "nextSymbol:", '"' + debugPrintable(scanBuffer.slice(0, bufferEnd)) + '"', tokenSetToString(currSymbol));
%}debug
                return;
            }
%}!regexpcontext
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
%{regexpcontext
            recognizedTokens = [];
            bufferEnds = [0, 0];
%}regexpcontext
%{!regexpcontext
            recognizedToken = undefined;
%}!regexpcontext
            state.state = 0;
            bufferEnd = 0;
            bufferPos = 0;
        }
    }
    currSymbol = endOfInputSet;
%{debug
    console.log(debugIndent + "nextSymbol: EOF");
%}debug
}

function getToken(token: number, firstNext: LLTokenSet, follow: LLTokenSet, firstNextEmpty: boolean): void {
    let ltSet: LLTokenSet = firstNextEmpty? uniteTokenSets(firstNext, follow): firstNext;
%{regexpcontext
    let tokenSet: LLTokenSet = makeTokenSet(token);
%}regexpcontext

%{debug
    console.log(debugIndent + "getToken", tokenName[token], tokenSetToString(firstNext), tokenSetToString(follow), firstNextEmpty, llLineNumber + ":" + llLinePosition);
%}debug
    while (currSymbol != endOfInputSet && !memberTokenSet(token, currSymbol) &&
           !tokenInCommon(currSymbol, ltSet)) {
%{regexpcontext
        nextSymbol(tokenSet, ltSet);
%}regexpcontext
%{!regexpcontext
        nextSymbol();
%}!regexpcontext
        if (!memberTokenSet(0, currSymbol)) {
            llerror("token skipped: %s", lastSymbol);
        }
    }
    if (!memberTokenSet(token, currSymbol)) {
        llerror("token expected: %s", tokenName[token]);
    } else {
%{regexpcontext
        nextSymbol(ltSet, follow);
%}regexpcontext
%{!regexpcontext
        nextSymbol();
%}!regexpcontext
    }
}

function toSymbolList(set: LLTokenSet): string[] {
    let list: string[] = [];

    for (let i = 0; i < nrTokens; i++) {
        if (memberTokenSet(i, set)) {
            list.push(tokenName[i]);
        }
    }
    return list;
}

export function tokenSetToString(set: LLTokenSet): string {
    return "{" + toSymbolList(set).join(",") + "}";
}

%{debug
let debugIndent = "";
function debugTraceStart(symbol: string): void {
    console.log(debugIndent + "[ " + symbol);
    debugIndent += "  ";
}

function debugTraceEnd(symbol: string): void {
    debugIndent = debugIndent.slice(0, -2);
    console.log(debugIndent + "] " + symbol);
}

function debugPrintable(str: string): string {
    return str.replace(/\\/g, "\\\\").replace(/\n/g, "\\n").replace(/\r/g, "\\r");
}

%}debug
%parsefunctions

%exitcode

var inputString: string, inputPosition: number;

function getNextCharacter() {
    return inputPosition < inputString.length?
           inputString[inputPosition++]: undefined;
}

%%export function parse(str: string, *S): void {
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
%{regexpcontext
    nextSymbol(inputFirsts, endOfInputSet);
%}regexpcontext
%{!regexpcontext
    nextSymbol();
%}!regexpcontext
%%    %S(#S);
}

%{main
declare function require(pkg: string): any;
declare var process: any;
var fs = require("fs");
parse(fs.readFileSync(process.argv[2]).toString());
%}main
