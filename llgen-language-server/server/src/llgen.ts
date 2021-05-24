

import {
    SymbolType
} from './symbols';

let fileBaseName: string;
export function setFileBaseName(fn: string|undefined) {
    fileBaseName = fn === undefined? "unknown": fn;
}

let defineFun: (type: SymbolType, lastSymbol: string, llLineNumber: number, llLinePosition: number, docComment: string|undefined) => boolean;
export function setDefineFun(fn: (type: SymbolType, lastSymbol: string, llLineNumber: number, llLinePosition: number, docComment: string|undefined) => boolean) {
    defineFun = fn;
}

let usageFun: (type: SymbolType, lastSymbol: string, llLineNumber: number, llLinePosition: number) => void;
export function setUsageFun(fn: (type: SymbolType, lastSymbol: string, llLineNumber: number, llLinePosition: number) => void) {
    usageFun = fn;
}

let messageFun: (type: string, subType: string, message: string, llLineNumber: number, llLinePosition: number, length: number) => void;
export function setMessageFun(fn: (type: string, subType: string, message: string, llLineNumber: number, llLinePosition: number, length: number) => void) {
    messageFun = fn;
}

function llerror(...args: any[]): void {
    messageFun("error", "syntax", sprintf(args), lastSymbolPos.line, lastSymbolPos.position, lastSymbol.length);
}

function llwarn(warnType: string, ...args: any[]): void {
    let msg = sprintf(args);
    messageFun("warning", warnType, msg, lastSymbolPos.line, lastSymbolPos.position, lastSymbol.length);
}

let stylisticWarnings = {
    something: true
};

export function updateStylisticWarnings(obj: any): void {
    if (obj instanceof Object) {
        if ("something" in obj) {
            stylisticWarnings.something = obj.something;
        }
    }
}

export let nonTerminalParameters: {[name: string]: string[]|undefined} = {};
let currentNonTerminal: string = "";
let docComment: string|undefined = undefined;

function initialize(): void {
    nonTerminalParameters = {};
}

function enterNonTerminal(): void {
    currentNonTerminal = lastSymbol;
    if (!defineFun(SymbolType.nonTerminal, currentNonTerminal, lastSymbolPos.line, lastSymbolPos.position, docComment)) {
        llerror("symbol redefined");
    }
    nonTerminalParameters[currentNonTerminal] = [];
}


type LLTokenSet = number[];

type KeyWordList = {[keyword:string]: number}|undefined;

const endOfInput = 32;
const nrTokens = 32;
const endOfInputSet = [0, 0x00000002]; 
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

export var inputFileName: string;
export function setInputFileName(fn: string): void {
    inputFileName = fn;
}
let scanBuffer = "";
let lastSymbol = "";
export let lastSymbolPos = {line: 0, position: 0};
let bufferEnd = 0;
let bufferFill = 0;
let atEOF = false;
let llTokenSet1: number[] = [0x00008000, 0x00000000]; /* string */
let llTokenSet2: number[] = [0x00002000, 0x00000000]; /* feature_def */
let llTokenSet3: number[] = [0x10000000, 0x00000000]; /* single_token */
let llTokenSet4: number[] = [0x00010000, 0x00000000]; /* output */
let llTokenSet5: number[] = [0x00000800, 0x00000000]; /* assign */
let llTokenSet6: number[] = [0x00004000, 0x00000000]; /* number */
let llTokenSet7: number[] = [0x00000001, 0x00000000]; /* */
let llTokenSet8: number[] = [0x00020000, 0x00000000]; /* identifier */
let llTokenSet9: number[] = [0x00000040, 0x00000000]; /* right_brace */
let llTokenSet10: number[] = [0x00000020, 0x00000000]; /* left_brace */
let llTokenSet11: number[] = [0x00000010, 0x00000000]; /* right_bracket */
let llTokenSet12: number[] = [0x00000008, 0x00000000]; /* left_bracket */
let llTokenSet13: number[] = [0x00001000, 0x00000000]; /* is */
let llTokenSet14: number[] = [0x00000400, 0x00000000]; /* semicolon */
let llTokenSet15: number[] = [0x00000200, 0x00000000]; /* colon */
let llTokenSet16: number[] = [0x00000100, 0x00000000]; /* period */
let llTokenSet17: number[] = [0x00000080, 0x00000000]; /* comma */
let llTokenSet18: number[] = [0x00000004, 0x00000000]; /* right_parenthesis */
let llTokenSet19: number[] = [0x00000002, 0x00000000]; /* left_parenthesis */
let tokenName: string[] = [
	"IGNORE",
	"left_parenthesis",
	"right_parenthesis",
	"left_bracket",
	"right_bracket",
	"left_brace",
	"right_brace",
	"comma",
	"period",
	"colon",
	"semicolon",
	"assign",
	"is",
	"feature_def",
	"number",
	"string",
	"output",
	"identifier",
	"chain_sym",
	"keyword_sym",
	"sequence_sym",
	"option_sym",
	"subtoken_sym",
	"ignore_sym",
	"shift_sym",
	"on_sym",
	"break_sym",
	"error_sym",
	"single_token",
	"@&^@#&^",
	"@&^@#&^",
	"@&^@#&^",
	"EOF",
];
const scanTab = [
/*   0 */ {
            '~': {destination:6,accept:llTokenSet3},
		    '?': {destination:6,accept:llTokenSet3},
		    '*': {destination:6,accept:llTokenSet3},
		    '!': {destination:6,accept:llTokenSet3},
		    '}': {destination:24,accept:llTokenSet9},
		    '|': {destination:25,accept:llTokenSet3},
		    '{': {destination:26,accept:llTokenSet10},
		    '^': {destination:22,accept:llTokenSet3},
		    '%': {destination:22,accept:llTokenSet3},
		    ']': {destination:27,accept:llTokenSet11},
		    '[': {destination:28,accept:llTokenSet12},
		    '>': {destination:29,accept:llTokenSet3},
		    '=': {destination:30,accept:llTokenSet13},
		    '<': {destination:31,accept:llTokenSet3},
		    ';': {destination:32,accept:llTokenSet14},
		    ':': {destination:33,accept:llTokenSet15},
		    '9': {destination:21,accept:llTokenSet6},
		    '8': {destination:21,accept:llTokenSet6},
		    '7': {destination:21,accept:llTokenSet6},
		    '6': {destination:21,accept:llTokenSet6},
		    '5': {destination:21,accept:llTokenSet6},
		    '4': {destination:21,accept:llTokenSet6},
		    '3': {destination:21,accept:llTokenSet6},
		    '2': {destination:21,accept:llTokenSet6},
		    '1': {destination:21,accept:llTokenSet6},
		    '0': {destination:21,accept:llTokenSet6},
		    '/': {destination:34,accept:llTokenSet3},
		    '.': {destination:35,accept:llTokenSet16},
		    '-': {destination:36,accept:llTokenSet3},
		    ',': {destination:37,accept:llTokenSet17},
		    '+': {destination:38,accept:llTokenSet3},
		    ')': {destination:39,accept:llTokenSet18},
		    '(': {destination:40,accept:llTokenSet19},
		    '\'': {destination:7},
		    '&': {destination:41,accept:llTokenSet3},
		    '#': {destination:42},
		    '"': {destination:1},
		    ' ': {destination:43,accept:llTokenSet7},
		    '\x0d': {destination:43,accept:llTokenSet7},
		    '\x0a': {destination:43,accept:llTokenSet7},
		    '\x09': {destination:43,accept:llTokenSet7},
		    '\x00': {},
		    '\x01': {},
		    '\x02': {},
		    '\x03': {},
		    '\x04': {},
		    '\x05': {},
		    '\x06': {},
		    '\x07': {},
		    '\x08': {},
		    '\x0b': {},
		    '\x0c': {},
		    '\x0e': {},
		    '\x0f': {},
		    '\x10': {},
		    '\x11': {},
		    '\x12': {},
		    '\x13': {},
		    '\x14': {},
		    '\x15': {},
		    '\x16': {},
		    '\x17': {},
		    '\x18': {},
		    '\x19': {},
		    '\x1a': {},
		    '\x1b': {},
		    '\x1c': {},
		    '\x1d': {},
		    '\x1e': {},
		    '\x1f': {},
		    '$': {},
		    '@': {},
		    '\\': {},
		    '`': {},
		    '\x7f': {},
		    '': {destination:23,accept:llTokenSet8},
          },
/*   1 */ {
            '\\': {destination:2},
		    '"': {destination:3,accept:llTokenSet1},
		    '\x0a': {},
		    '\x0d': {},
		    '': {destination:1},
          },
/*   2 */ {
            '\x00': {},
		    '\x0a': {},
		    '\x0d': {},
		    '': {destination:1},
          },
/*   3 */ undefined,
/*   4 */ {
            'z': {destination:4,accept:llTokenSet2},
		    'y': {destination:4,accept:llTokenSet2},
		    'x': {destination:4,accept:llTokenSet2},
		    'w': {destination:4,accept:llTokenSet2},
		    'v': {destination:4,accept:llTokenSet2},
		    'u': {destination:4,accept:llTokenSet2},
		    't': {destination:4,accept:llTokenSet2},
		    's': {destination:4,accept:llTokenSet2},
		    'r': {destination:4,accept:llTokenSet2},
		    'q': {destination:4,accept:llTokenSet2},
		    'p': {destination:4,accept:llTokenSet2},
		    'o': {destination:4,accept:llTokenSet2},
		    'n': {destination:4,accept:llTokenSet2},
		    'm': {destination:4,accept:llTokenSet2},
		    'l': {destination:4,accept:llTokenSet2},
		    'k': {destination:4,accept:llTokenSet2},
		    'j': {destination:4,accept:llTokenSet2},
		    'i': {destination:4,accept:llTokenSet2},
		    'h': {destination:4,accept:llTokenSet2},
		    'g': {destination:4,accept:llTokenSet2},
		    'f': {destination:4,accept:llTokenSet2},
		    'e': {destination:4,accept:llTokenSet2},
		    'd': {destination:4,accept:llTokenSet2},
		    'c': {destination:4,accept:llTokenSet2},
		    'b': {destination:4,accept:llTokenSet2},
		    'a': {destination:4,accept:llTokenSet2},
		    '_': {destination:4,accept:llTokenSet2},
		    'Z': {destination:4,accept:llTokenSet2},
		    'Y': {destination:4,accept:llTokenSet2},
		    'X': {destination:4,accept:llTokenSet2},
		    'W': {destination:4,accept:llTokenSet2},
		    'V': {destination:4,accept:llTokenSet2},
		    'U': {destination:4,accept:llTokenSet2},
		    'T': {destination:4,accept:llTokenSet2},
		    'S': {destination:4,accept:llTokenSet2},
		    'R': {destination:4,accept:llTokenSet2},
		    'Q': {destination:4,accept:llTokenSet2},
		    'P': {destination:4,accept:llTokenSet2},
		    'O': {destination:4,accept:llTokenSet2},
		    'N': {destination:4,accept:llTokenSet2},
		    'M': {destination:4,accept:llTokenSet2},
		    'L': {destination:4,accept:llTokenSet2},
		    'K': {destination:4,accept:llTokenSet2},
		    'J': {destination:4,accept:llTokenSet2},
		    'I': {destination:4,accept:llTokenSet2},
		    'H': {destination:4,accept:llTokenSet2},
		    'G': {destination:4,accept:llTokenSet2},
		    'F': {destination:4,accept:llTokenSet2},
		    'E': {destination:4,accept:llTokenSet2},
		    'D': {destination:4,accept:llTokenSet2},
		    'C': {destination:4,accept:llTokenSet2},
		    'B': {destination:4,accept:llTokenSet2},
		    'A': {destination:4,accept:llTokenSet2},
		    '9': {destination:4,accept:llTokenSet2},
		    '8': {destination:4,accept:llTokenSet2},
		    '7': {destination:4,accept:llTokenSet2},
		    '6': {destination:4,accept:llTokenSet2},
		    '5': {destination:4,accept:llTokenSet2},
		    '4': {destination:4,accept:llTokenSet2},
		    '3': {destination:4,accept:llTokenSet2},
		    '2': {destination:4,accept:llTokenSet2},
		    '1': {destination:4,accept:llTokenSet2},
		    '0': {destination:4,accept:llTokenSet2},
		    '': {},
          },
/*   5 */ {
            'z': {destination:4,accept:llTokenSet2},
		    'y': {destination:4,accept:llTokenSet2},
		    'x': {destination:4,accept:llTokenSet2},
		    'w': {destination:4,accept:llTokenSet2},
		    'v': {destination:4,accept:llTokenSet2},
		    'u': {destination:4,accept:llTokenSet2},
		    't': {destination:4,accept:llTokenSet2},
		    's': {destination:4,accept:llTokenSet2},
		    'r': {destination:4,accept:llTokenSet2},
		    'q': {destination:4,accept:llTokenSet2},
		    'p': {destination:4,accept:llTokenSet2},
		    'o': {destination:4,accept:llTokenSet2},
		    'n': {destination:4,accept:llTokenSet2},
		    'm': {destination:4,accept:llTokenSet2},
		    'l': {destination:4,accept:llTokenSet2},
		    'k': {destination:4,accept:llTokenSet2},
		    'j': {destination:4,accept:llTokenSet2},
		    'i': {destination:4,accept:llTokenSet2},
		    'h': {destination:4,accept:llTokenSet2},
		    'g': {destination:4,accept:llTokenSet2},
		    'f': {destination:4,accept:llTokenSet2},
		    'e': {destination:4,accept:llTokenSet2},
		    'd': {destination:4,accept:llTokenSet2},
		    'c': {destination:4,accept:llTokenSet2},
		    'b': {destination:4,accept:llTokenSet2},
		    'a': {destination:4,accept:llTokenSet2},
		    '_': {destination:4,accept:llTokenSet2},
		    'Z': {destination:4,accept:llTokenSet2},
		    'Y': {destination:4,accept:llTokenSet2},
		    'X': {destination:4,accept:llTokenSet2},
		    'W': {destination:4,accept:llTokenSet2},
		    'V': {destination:4,accept:llTokenSet2},
		    'U': {destination:4,accept:llTokenSet2},
		    'T': {destination:4,accept:llTokenSet2},
		    'S': {destination:4,accept:llTokenSet2},
		    'R': {destination:4,accept:llTokenSet2},
		    'Q': {destination:4,accept:llTokenSet2},
		    'P': {destination:4,accept:llTokenSet2},
		    'O': {destination:4,accept:llTokenSet2},
		    'N': {destination:4,accept:llTokenSet2},
		    'M': {destination:4,accept:llTokenSet2},
		    'L': {destination:4,accept:llTokenSet2},
		    'K': {destination:4,accept:llTokenSet2},
		    'J': {destination:4,accept:llTokenSet2},
		    'I': {destination:4,accept:llTokenSet2},
		    'H': {destination:4,accept:llTokenSet2},
		    'G': {destination:4,accept:llTokenSet2},
		    'F': {destination:4,accept:llTokenSet2},
		    'E': {destination:4,accept:llTokenSet2},
		    'D': {destination:4,accept:llTokenSet2},
		    'C': {destination:4,accept:llTokenSet2},
		    'B': {destination:4,accept:llTokenSet2},
		    'A': {destination:4,accept:llTokenSet2},
		    '': {},
          },
/*   6 */ undefined,
/*   7 */ {
            '\\': {destination:8},
		    '\'': {destination:9,accept:llTokenSet4},
		    '\x0a': {},
		    '\x0d': {},
		    '': {destination:7},
          },
/*   8 */ {
            '\x00': {},
		    '\x0a': {},
		    '\x0d': {},
		    '': {destination:7},
          },
/*   9 */ undefined,
/*  10 */ undefined,
/*  11 */ {
            '9': {destination:12,accept:llTokenSet6},
		    '8': {destination:12,accept:llTokenSet6},
		    '7': {destination:12,accept:llTokenSet6},
		    '6': {destination:12,accept:llTokenSet6},
		    '5': {destination:12,accept:llTokenSet6},
		    '4': {destination:12,accept:llTokenSet6},
		    '3': {destination:12,accept:llTokenSet6},
		    '2': {destination:12,accept:llTokenSet6},
		    '1': {destination:12,accept:llTokenSet6},
		    '0': {destination:12,accept:llTokenSet6},
		    '': {},
          },
/*  12 */ {
            'e': {destination:20},
		    'E': {destination:20},
		    '9': {destination:12,accept:llTokenSet6},
		    '8': {destination:12,accept:llTokenSet6},
		    '7': {destination:12,accept:llTokenSet6},
		    '6': {destination:12,accept:llTokenSet6},
		    '5': {destination:12,accept:llTokenSet6},
		    '4': {destination:12,accept:llTokenSet6},
		    '3': {destination:12,accept:llTokenSet6},
		    '2': {destination:12,accept:llTokenSet6},
		    '1': {destination:12,accept:llTokenSet6},
		    '0': {destination:12,accept:llTokenSet6},
		    '': {},
          },
/*  13 */ {
            '.': {destination:6,accept:llTokenSet3},
		    '': {},
          },
/*  14 */ undefined,
/*  15 */ {
            '*': {destination:16},
		    '': {destination:15},
          },
/*  16 */ {
            '/': {destination:14,accept:llTokenSet7},
		    '': {destination:15},
          },
/*  17 */ {
            '\x00': {},
		    '\x0a': {},
		    '\x0d': {},
		    '': {destination:17,accept:llTokenSet7},
          },
/*  18 */ {
            '9': {destination:18,accept:llTokenSet6},
		    '8': {destination:18,accept:llTokenSet6},
		    '7': {destination:18,accept:llTokenSet6},
		    '6': {destination:18,accept:llTokenSet6},
		    '5': {destination:18,accept:llTokenSet6},
		    '4': {destination:18,accept:llTokenSet6},
		    '3': {destination:18,accept:llTokenSet6},
		    '2': {destination:18,accept:llTokenSet6},
		    '1': {destination:18,accept:llTokenSet6},
		    '0': {destination:18,accept:llTokenSet6},
		    '': {},
          },
/*  19 */ {
            '9': {destination:18,accept:llTokenSet6},
		    '8': {destination:18,accept:llTokenSet6},
		    '7': {destination:18,accept:llTokenSet6},
		    '6': {destination:18,accept:llTokenSet6},
		    '5': {destination:18,accept:llTokenSet6},
		    '4': {destination:18,accept:llTokenSet6},
		    '3': {destination:18,accept:llTokenSet6},
		    '2': {destination:18,accept:llTokenSet6},
		    '1': {destination:18,accept:llTokenSet6},
		    '0': {destination:18,accept:llTokenSet6},
		    '': {},
          },
/*  20 */ {
            '9': {destination:18,accept:llTokenSet6},
		    '8': {destination:18,accept:llTokenSet6},
		    '7': {destination:18,accept:llTokenSet6},
		    '6': {destination:18,accept:llTokenSet6},
		    '5': {destination:18,accept:llTokenSet6},
		    '4': {destination:18,accept:llTokenSet6},
		    '3': {destination:18,accept:llTokenSet6},
		    '2': {destination:18,accept:llTokenSet6},
		    '1': {destination:18,accept:llTokenSet6},
		    '0': {destination:18,accept:llTokenSet6},
		    '\x00': {},
		    '\x01': {},
		    '\x02': {},
		    '\x03': {},
		    '\x04': {},
		    '\x05': {},
		    '\x06': {},
		    '\x07': {},
		    '\x08': {},
		    '\x09': {},
		    '\x0a': {},
		    '\x0b': {},
		    '\x0c': {},
		    '\x0d': {},
		    '\x0e': {},
		    '\x0f': {},
		    '\x10': {},
		    '\x11': {},
		    '\x12': {},
		    '\x13': {},
		    '\x14': {},
		    '\x15': {},
		    '\x16': {},
		    '\x17': {},
		    '\x18': {},
		    '\x19': {},
		    '\x1a': {},
		    '\x1b': {},
		    '\x1c': {},
		    '\x1d': {},
		    '\x1e': {},
		    '\x1f': {},
		    ' ': {},
		    '!': {},
		    '"': {},
		    '#': {},
		    '$': {},
		    '%': {},
		    '&': {},
		    '\'': {},
		    '(': {},
		    ')': {},
		    '*': {},
		    '': {destination:19},
          },
/*  21 */ {
            'e': {destination:20},
		    'E': {destination:20},
		    '9': {destination:21,accept:llTokenSet6},
		    '8': {destination:21,accept:llTokenSet6},
		    '7': {destination:21,accept:llTokenSet6},
		    '6': {destination:21,accept:llTokenSet6},
		    '5': {destination:21,accept:llTokenSet6},
		    '4': {destination:21,accept:llTokenSet6},
		    '3': {destination:21,accept:llTokenSet6},
		    '2': {destination:21,accept:llTokenSet6},
		    '1': {destination:21,accept:llTokenSet6},
		    '0': {destination:21,accept:llTokenSet6},
		    '.': {destination:12,accept:llTokenSet6},
		    '': {},
          },
/*  22 */ {
            '=': {destination:6,accept:llTokenSet3},
		    '': {},
          },
/*  23 */ {
            'z': {destination:23,accept:llTokenSet8},
		    'y': {destination:23,accept:llTokenSet8},
		    'x': {destination:23,accept:llTokenSet8},
		    'w': {destination:23,accept:llTokenSet8},
		    'v': {destination:23,accept:llTokenSet8},
		    'u': {destination:23,accept:llTokenSet8},
		    't': {destination:23,accept:llTokenSet8},
		    's': {destination:23,accept:llTokenSet8},
		    'r': {destination:23,accept:llTokenSet8},
		    'q': {destination:23,accept:llTokenSet8},
		    'p': {destination:23,accept:llTokenSet8},
		    'o': {destination:23,accept:llTokenSet8},
		    'n': {destination:23,accept:llTokenSet8},
		    'm': {destination:23,accept:llTokenSet8},
		    'l': {destination:23,accept:llTokenSet8},
		    'k': {destination:23,accept:llTokenSet8},
		    'j': {destination:23,accept:llTokenSet8},
		    'i': {destination:23,accept:llTokenSet8},
		    'h': {destination:23,accept:llTokenSet8},
		    'g': {destination:23,accept:llTokenSet8},
		    'f': {destination:23,accept:llTokenSet8},
		    'e': {destination:23,accept:llTokenSet8},
		    'd': {destination:23,accept:llTokenSet8},
		    'c': {destination:23,accept:llTokenSet8},
		    'b': {destination:23,accept:llTokenSet8},
		    'a': {destination:23,accept:llTokenSet8},
		    '_': {destination:23,accept:llTokenSet8},
		    'Z': {destination:23,accept:llTokenSet8},
		    'Y': {destination:23,accept:llTokenSet8},
		    'X': {destination:23,accept:llTokenSet8},
		    'W': {destination:23,accept:llTokenSet8},
		    'V': {destination:23,accept:llTokenSet8},
		    'U': {destination:23,accept:llTokenSet8},
		    'T': {destination:23,accept:llTokenSet8},
		    'S': {destination:23,accept:llTokenSet8},
		    'R': {destination:23,accept:llTokenSet8},
		    'Q': {destination:23,accept:llTokenSet8},
		    'P': {destination:23,accept:llTokenSet8},
		    'O': {destination:23,accept:llTokenSet8},
		    'N': {destination:23,accept:llTokenSet8},
		    'M': {destination:23,accept:llTokenSet8},
		    'L': {destination:23,accept:llTokenSet8},
		    'K': {destination:23,accept:llTokenSet8},
		    'J': {destination:23,accept:llTokenSet8},
		    'I': {destination:23,accept:llTokenSet8},
		    'H': {destination:23,accept:llTokenSet8},
		    'G': {destination:23,accept:llTokenSet8},
		    'F': {destination:23,accept:llTokenSet8},
		    'E': {destination:23,accept:llTokenSet8},
		    'D': {destination:23,accept:llTokenSet8},
		    'C': {destination:23,accept:llTokenSet8},
		    'B': {destination:23,accept:llTokenSet8},
		    'A': {destination:23,accept:llTokenSet8},
		    '9': {destination:23,accept:llTokenSet8},
		    '8': {destination:23,accept:llTokenSet8},
		    '7': {destination:23,accept:llTokenSet8},
		    '6': {destination:23,accept:llTokenSet8},
		    '5': {destination:23,accept:llTokenSet8},
		    '4': {destination:23,accept:llTokenSet8},
		    '3': {destination:23,accept:llTokenSet8},
		    '2': {destination:23,accept:llTokenSet8},
		    '1': {destination:23,accept:llTokenSet8},
		    '0': {destination:23,accept:llTokenSet8},
		    '': {},
          },
/*  24 */ undefined,
/*  25 */ {
            '|': {destination:6,accept:llTokenSet3},
		    '=': {destination:6,accept:llTokenSet3},
		    '': {},
          },
/*  26 */ undefined,
/*  27 */ undefined,
/*  28 */ undefined,
/*  29 */ {
            '>': {destination:22,accept:llTokenSet3},
		    '': {},
          },
/*  30 */ undefined,
/*  31 */ {
            '<': {destination:22,accept:llTokenSet3},
		    '': {},
          },
/*  32 */ undefined,
/*  33 */ undefined,
/*  34 */ {
            '=': {destination:6,accept:llTokenSet3},
		    '/': {destination:17,accept:llTokenSet7},
		    '*': {destination:15},
		    '': {},
          },
/*  35 */ {
            '9': {destination:12,accept:llTokenSet6},
		    '8': {destination:12,accept:llTokenSet6},
		    '7': {destination:12,accept:llTokenSet6},
		    '6': {destination:12,accept:llTokenSet6},
		    '5': {destination:12,accept:llTokenSet6},
		    '4': {destination:12,accept:llTokenSet6},
		    '3': {destination:12,accept:llTokenSet6},
		    '2': {destination:12,accept:llTokenSet6},
		    '1': {destination:12,accept:llTokenSet6},
		    '0': {destination:12,accept:llTokenSet6},
		    '.': {destination:13},
		    '': {},
          },
/*  36 */ {
            '>': {destination:10,accept:llTokenSet5},
		    '=': {destination:6,accept:llTokenSet3},
		    '-': {destination:6,accept:llTokenSet3},
		    '9': {destination:21,accept:llTokenSet6},
		    '8': {destination:21,accept:llTokenSet6},
		    '7': {destination:21,accept:llTokenSet6},
		    '6': {destination:21,accept:llTokenSet6},
		    '5': {destination:21,accept:llTokenSet6},
		    '4': {destination:21,accept:llTokenSet6},
		    '3': {destination:21,accept:llTokenSet6},
		    '2': {destination:21,accept:llTokenSet6},
		    '1': {destination:21,accept:llTokenSet6},
		    '0': {destination:21,accept:llTokenSet6},
		    '.': {destination:11},
		    '': {},
          },
/*  37 */ undefined,
/*  38 */ {
            '=': {destination:6,accept:llTokenSet3},
		    '+': {destination:6,accept:llTokenSet3},
		    '9': {destination:21,accept:llTokenSet6},
		    '8': {destination:21,accept:llTokenSet6},
		    '7': {destination:21,accept:llTokenSet6},
		    '6': {destination:21,accept:llTokenSet6},
		    '5': {destination:21,accept:llTokenSet6},
		    '4': {destination:21,accept:llTokenSet6},
		    '3': {destination:21,accept:llTokenSet6},
		    '2': {destination:21,accept:llTokenSet6},
		    '1': {destination:21,accept:llTokenSet6},
		    '0': {destination:21,accept:llTokenSet6},
		    '.': {destination:11},
		    '': {},
          },
/*  39 */ undefined,
/*  40 */ undefined,
/*  41 */ {
            '=': {destination:6,accept:llTokenSet3},
		    '&': {destination:6,accept:llTokenSet3},
		    '': {},
          },
/*  42 */ {
            'z': {destination:4,accept:llTokenSet2},
		    'y': {destination:4,accept:llTokenSet2},
		    'x': {destination:4,accept:llTokenSet2},
		    'w': {destination:4,accept:llTokenSet2},
		    'v': {destination:4,accept:llTokenSet2},
		    'u': {destination:4,accept:llTokenSet2},
		    't': {destination:4,accept:llTokenSet2},
		    's': {destination:4,accept:llTokenSet2},
		    'r': {destination:4,accept:llTokenSet2},
		    'q': {destination:4,accept:llTokenSet2},
		    'p': {destination:4,accept:llTokenSet2},
		    'o': {destination:4,accept:llTokenSet2},
		    'n': {destination:4,accept:llTokenSet2},
		    'm': {destination:4,accept:llTokenSet2},
		    'l': {destination:4,accept:llTokenSet2},
		    'k': {destination:4,accept:llTokenSet2},
		    'j': {destination:4,accept:llTokenSet2},
		    'i': {destination:4,accept:llTokenSet2},
		    'h': {destination:4,accept:llTokenSet2},
		    'g': {destination:4,accept:llTokenSet2},
		    'f': {destination:4,accept:llTokenSet2},
		    'e': {destination:4,accept:llTokenSet2},
		    'd': {destination:4,accept:llTokenSet2},
		    'c': {destination:4,accept:llTokenSet2},
		    'b': {destination:4,accept:llTokenSet2},
		    'a': {destination:4,accept:llTokenSet2},
		    '_': {destination:4,accept:llTokenSet2},
		    'Z': {destination:4,accept:llTokenSet2},
		    'Y': {destination:4,accept:llTokenSet2},
		    'X': {destination:4,accept:llTokenSet2},
		    'W': {destination:4,accept:llTokenSet2},
		    'V': {destination:4,accept:llTokenSet2},
		    'U': {destination:4,accept:llTokenSet2},
		    'T': {destination:4,accept:llTokenSet2},
		    'S': {destination:4,accept:llTokenSet2},
		    'R': {destination:4,accept:llTokenSet2},
		    'Q': {destination:4,accept:llTokenSet2},
		    'P': {destination:4,accept:llTokenSet2},
		    'O': {destination:4,accept:llTokenSet2},
		    'N': {destination:4,accept:llTokenSet2},
		    'M': {destination:4,accept:llTokenSet2},
		    'L': {destination:4,accept:llTokenSet2},
		    'K': {destination:4,accept:llTokenSet2},
		    'J': {destination:4,accept:llTokenSet2},
		    'I': {destination:4,accept:llTokenSet2},
		    'H': {destination:4,accept:llTokenSet2},
		    'G': {destination:4,accept:llTokenSet2},
		    'F': {destination:4,accept:llTokenSet2},
		    'E': {destination:4,accept:llTokenSet2},
		    'D': {destination:4,accept:llTokenSet2},
		    'C': {destination:4,accept:llTokenSet2},
		    'B': {destination:4,accept:llTokenSet2},
		    'A': {destination:4,accept:llTokenSet2},
		    '!': {destination:5},
		    '': {},
          },
/*  43 */ {
            ' ': {destination:43,accept:llTokenSet7},
		    '\x0d': {destination:43,accept:llTokenSet7},
		    '\x0a': {destination:43,accept:llTokenSet7},
		    '\x09': {destination:43,accept:llTokenSet7},
		    '': {},
          },
];
let keywordList: KeyWordList[] = [
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	{"BREAK": 26, "CHAIN": 18, "ERROR": 27, "IGNORE": 23, "KEYWORD": 19, "ON": 25, "OPTION": 21, "SEQUENCE": 20, "SHIFT": 24, "SUBTOKEN": 22},
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
	undefined,
];

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

    while (currSymbol !== endOfInputSet && !tokenInCommon(currSymbol, ltSet)) {
        nextSymbol();
        llerror("token skipped: %s", lastSymbol);
    }
}

function memberTokenSet(token: number, set: LLTokenSet): boolean {
    return (set[Math.floor(token / 31)] & (1 << (token % 31))) !== 0;
}

function notEmpty(tSet: LLTokenSet): boolean {
    if (tSet[0] > 1)
        return true;
    for (let i = 1; i < tSet.length; i++)
        if (tSet[i] > 0)
            return true;
    return false;
}

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

function nextSymbol(): void
{
    let bufferPos: number;
    let state: localstate = { state: 0 };
    let token: LLTokenSet|undefined;
    let ch: string|undefined;
    let lastNlPos = 0, nlPos = 0;
    let recognizedToken: LLTokenSet|undefined = undefined;

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
            } else if (notEmpty(recognizedToken)) {
                currSymbol = lLKeyWord(recognizedToken);
                return;
            }
            /* If nothing recognized, continue; no need to copy buffer */
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

function getToken(token: number, firstNext: LLTokenSet, follow: LLTokenSet, firstNextEmpty: boolean): void {
    let ltSet: LLTokenSet = firstNextEmpty? uniteTokenSets(firstNext, follow): firstNext;

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

let llTokenSet20: number[] = [0x1003DF0A, 0x00000000]; /* assign colon identifier is left_bracket left_parenthesis number output period semicolon single_token string */
let llTokenSet21: number[] = [0x1003DF8E, 0x00000000]; /* assign colon comma identifier is left_bracket left_parenthesis number output period right_parenthesis semicolon single_token string */
let llTokenSet22: number[] = [0x00000080, 0x00000000]; /* comma */
let llTokenSet23: number[] = [0x00000000, 0x00000000]; /* */
let llTokenSet24: number[] = [0x03038022, 0x00000000]; /* identifier left_brace left_parenthesis on_sym output shift_sym string */
let llTokenSet25: number[] = [0x00000400, 0x00000000]; /* semicolon */
let llTokenSet26: number[] = [0x03038422, 0x00000000]; /* identifier left_brace left_parenthesis on_sym output semicolon shift_sym string */
let llTokenSet27: number[] = [0x1003DFEA, 0x00000000]; /* assign colon comma identifier is left_brace left_bracket left_parenthesis number output period right_brace semicolon single_token string */
let llTokenSet28: number[] = [0x1003DFAA, 0x00000000]; /* assign colon comma identifier is left_brace left_bracket left_parenthesis number output period semicolon single_token string */
let llTokenSet29: number[] = [0x1003DF00, 0x00000000]; /* assign colon identifier is number output period semicolon single_token string */
let llTokenSet30: number[] = [0x00000002, 0x00000000]; /* left_parenthesis */
let llTokenSet31: number[] = [0x1003DFAE, 0x00000000]; /* assign colon comma identifier is left_brace left_bracket left_parenthesis number output period right_parenthesis semicolon single_token string */
let llTokenSet32: number[] = [0x00000008, 0x00000000]; /* left_bracket */
let llTokenSet33: number[] = [0x1003DFBA, 0x00000000]; /* assign colon comma identifier is left_brace left_bracket left_parenthesis number output period right_bracket semicolon single_token string */
let llTokenSet34: number[] = [0x00000020, 0x00000000]; /* left_brace */
let llTokenSet35: number[] = [0x03028002, 0x00000000]; /* identifier left_parenthesis on_sym shift_sym string */
let llTokenSet36: number[] = [0x01000000, 0x00000000]; /* shift_sym */
let llTokenSet37: number[] = [0x02028002, 0x00000000]; /* identifier left_parenthesis on_sym string */
let llTokenSet38: number[] = [0x00340000, 0x00000000]; /* chain_sym option_sym sequence_sym */
let llTokenSet39: number[] = [0x00040000, 0x00000000]; /* chain_sym */
let llTokenSet40: number[] = [0x00200000, 0x00000000]; /* option_sym */
let llTokenSet41: number[] = [0x00100000, 0x00000000]; /* sequence_sym */
let llTokenSet42: number[] = [0x00010000, 0x00000000]; /* output */
let llTokenSet43: number[] = [0x00020000, 0x00000000]; /* identifier */
let llTokenSet44: number[] = [0x00001000, 0x00000000]; /* is */
let llTokenSet45: number[] = [0x00820000, 0x00000000]; /* identifier ignore_sym */
let llTokenSet46: number[] = [0x00822020, 0x00000000]; /* feature_def identifier ignore_sym left_brace */
let llTokenSet47: number[] = [0x00002000, 0x00000000]; /* feature_def */
let llTokenSet48: number[] = [0x10000000, 0x00000000]; /* single_token */
let llTokenSet49: number[] = [0x00000200, 0x00000000]; /* colon */
let llTokenSet50: number[] = [0x00000100, 0x00000000]; /* period */
let llTokenSet51: number[] = [0x00008000, 0x00000000]; /* string */
let llTokenSet52: number[] = [0x00004000, 0x00000000]; /* number */
let llTokenSet53: number[] = [0x00000800, 0x00000000]; /* assign */
let llTokenSet54: number[] = [0x00001A02, 0x00000000]; /* assign colon is left_parenthesis */
let llTokenSet55: number[] = [0x00000A02, 0x00000000]; /* assign colon left_parenthesis */
let llTokenSet56: number[] = [0x00000A00, 0x00000000]; /* assign colon */
let llTokenSet57: number[] = [0x03038522, 0x00000000]; /* identifier left_brace left_parenthesis on_sym output period semicolon shift_sym string */
let llTokenSet58: number[] = [0x00800000, 0x00000000]; /* ignore_sym */
let llTokenSet59: number[] = [0x00000802, 0x00000000]; /* assign left_parenthesis */
let llTokenSet60: number[] = [0x03038426, 0x00000000]; /* identifier left_brace left_parenthesis on_sym output right_parenthesis semicolon shift_sym string */
let llTokenSet61: number[] = [0x00000004, 0x00000000]; /* right_parenthesis */
let llTokenSet62: number[] = [0x02000000, 0x00000000]; /* on_sym */
let llTokenSet63: number[] = [0x00028000, 0x00000000]; /* identifier string */
let llTokenSet64: number[] = [0x0C000000, 0x00000000]; /* break_sym error_sym */
let llTokenSet65: number[] = [0x04000000, 0x00000000]; /* break_sym */
let llTokenSet66: number[] = [0x08000000, 0x00000000]; /* error_sym */
let llTokenSet67: number[] = [0x00080000, 0x00000000]; /* keyword_sym */
let llTokenSet68: number[] = [0x1003DF9A, 0x00000000]; /* assign colon comma identifier is left_bracket left_parenthesis number output period right_bracket semicolon single_token string */
let llTokenSet69: number[] = [0x00000010, 0x00000000]; /* right_bracket */
let llTokenSet70: number[] = [0x1003DF8A, 0x00000000]; /* assign colon comma identifier is left_bracket left_parenthesis number output period semicolon single_token string */


function actual_parameters(follow: LLTokenSet): void {
	getToken(1/*left_parenthesis*/, llTokenSet20, llTokenSet20, false);
	for (;;) {
		do {
			wildcard(llTokenSet21);
			waitForToken(uniteTokenSets(llTokenSet21, follow), follow);
		} while (tokenInCommon(currSymbol, llTokenSet20));
		if (!tokenInCommon(currSymbol, llTokenSet22)) {
			break;
		}
		getToken(7/*comma*/, llTokenSet20, llTokenSet20, false);
	}
	getToken(2/*right_parenthesis*/, llTokenSet23, follow, true);
}


function alternative(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet24)) {
		for (;;) {
			element(uniteTokenSets(follow, llTokenSet22));
			if (!tokenInCommon(currSymbol, llTokenSet22)) {
				break;
			}
			getToken(7/*comma*/, llTokenSet24, llTokenSet24, false);
		}
	}
}


function alternatives(follow: LLTokenSet): void {
	for (;;) {
		alternative(uniteTokenSets(follow, llTokenSet25));
		if (!tokenInCommon(currSymbol, llTokenSet25)) {
			break;
		}
		getToken(10/*semicolon*/, llTokenSet26, uniteTokenSets(follow, llTokenSet26), true);
	}
}


function code(follow: LLTokenSet): void {
	getToken(5/*left_brace*/, llTokenSet27, llTokenSet27, false);
	if (tokenInCommon(currSymbol, llTokenSet28)) {
		do {
			code_element(llTokenSet27);
			waitForToken(uniteTokenSets(llTokenSet27, follow), follow);
		} while (tokenInCommon(currSymbol, llTokenSet28));
	}
	getToken(6/*right_brace*/, llTokenSet23, follow, true);
}


function code_element(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet29)) {
		non_bracket_symbol(follow);
	} else if (tokenInCommon(currSymbol, llTokenSet22)) {
		getToken(7/*comma*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet30)) {
		getToken(1/*left_parenthesis*/, llTokenSet31, llTokenSet31, false);
		if (tokenInCommon(currSymbol, llTokenSet28)) {
			do {
				code_element(llTokenSet31);
				waitForToken(uniteTokenSets(llTokenSet31, follow), follow);
			} while (tokenInCommon(currSymbol, llTokenSet28));
		}
		getToken(2/*right_parenthesis*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet32)) {
		getToken(3/*left_bracket*/, llTokenSet33, llTokenSet33, false);
		if (tokenInCommon(currSymbol, llTokenSet28)) {
			do {
				code_element(llTokenSet33);
				waitForToken(uniteTokenSets(llTokenSet33, follow), follow);
			} while (tokenInCommon(currSymbol, llTokenSet28));
		}
		getToken(4/*right_bracket*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet34)) {
		getToken(5/*left_brace*/, llTokenSet27, llTokenSet27, false);
		if (tokenInCommon(currSymbol, llTokenSet28)) {
			do {
				code_element(llTokenSet27);
				waitForToken(uniteTokenSets(llTokenSet27, follow), follow);
			} while (tokenInCommon(currSymbol, llTokenSet28));
		}
		getToken(6/*right_brace*/, llTokenSet23, follow, true);
	} else {
		llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
	}
}


function element(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet35)) {
		if (tokenInCommon(currSymbol, llTokenSet36)) {
			getToken(24/*shift_sym*/, llTokenSet37, llTokenSet37, false);
		}
		single_element(uniteTokenSets(follow, llTokenSet38));
		if (tokenInCommon(currSymbol, llTokenSet38)) {
			if (tokenInCommon(currSymbol, llTokenSet39)) {
				getToken(18/*chain_sym*/, llTokenSet37, llTokenSet37, false);
				single_element(follow);
			} else if (tokenInCommon(currSymbol, llTokenSet40)) {
				getToken(21/*option_sym*/, llTokenSet23, follow, true);
			} else if (tokenInCommon(currSymbol, llTokenSet41)) {
				getToken(20/*sequence_sym*/, llTokenSet40, uniteTokenSets(follow, llTokenSet40), true);
				if (tokenInCommon(currSymbol, llTokenSet40)) {
					getToken(21/*option_sym*/, llTokenSet23, follow, true);
				}
			} else {
				llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
			}
		}
	} else if (tokenInCommon(currSymbol, llTokenSet34)) {
		code(follow);
	} else if (tokenInCommon(currSymbol, llTokenSet42)) {
		getToken(16/*output*/, llTokenSet23, follow, true);
	} else {
		llerror("element");
	}
}


function formal_parameters(follow: LLTokenSet): void {
	getToken(1/*left_parenthesis*/, llTokenSet20, llTokenSet20, false);
	for (;;) {
		do {
			wildcard(llTokenSet21);
			waitForToken(uniteTokenSets(llTokenSet21, follow), follow);
		} while (tokenInCommon(currSymbol, llTokenSet20));
		if (!tokenInCommon(currSymbol, llTokenSet22)) {
			break;
		}
		getToken(7/*comma*/, llTokenSet20, llTokenSet20, false);
	}
	getToken(2/*right_parenthesis*/, llTokenSet23, follow, true);
}


function function_result(follow: LLTokenSet): void {
	getToken(11/*assign*/, llTokenSet43, llTokenSet43, false);
	getToken(17/*identifier*/, llTokenSet23, follow, true);
}


function function_return(follow: LLTokenSet): void {
	getToken(11/*assign*/, llTokenSet43, llTokenSet43, false);
	getToken(17/*identifier*/, llTokenSet43, llTokenSet43, false);
	getToken(17/*identifier*/, llTokenSet44, uniteTokenSets(follow, llTokenSet44), true);
	if (tokenInCommon(currSymbol, llTokenSet44)) {
		getToken(12/*is*/, llTokenSet34, llTokenSet34, false);
		code(follow);
	}
}


function llgen_file(follow: LLTokenSet): void {
	
	initialize();
	do {
		if (tokenInCommon(currSymbol, llTokenSet45)) {
			rule(uniteTokenSets(follow, llTokenSet46));
		} else if (tokenInCommon(currSymbol, llTokenSet34)) {
			code(uniteTokenSets(follow, llTokenSet46));
		} else if (tokenInCommon(currSymbol, llTokenSet47)) {
			getToken(13/*feature_def*/, llTokenSet46, uniteTokenSets(follow, llTokenSet46), true);
		} else {
			llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
		}
		waitForToken(uniteTokenSets(llTokenSet46, follow), follow);
	} while (tokenInCommon(currSymbol, llTokenSet46));
}


function non_bracket_symbol(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet48)) {
		getToken(28/*single_token*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet43)) {
		getToken(17/*identifier*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet49)) {
		getToken(9/*colon*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet25)) {
		getToken(10/*semicolon*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet50)) {
		getToken(8/*period*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet42)) {
		getToken(16/*output*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet51)) {
		getToken(15/*string*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet52)) {
		getToken(14/*number*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet53)) {
		getToken(11/*assign*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet44)) {
		getToken(12/*is*/, llTokenSet23, follow, true);
	} else {
		llerror("non_bracket_symbol");
	}
}


function rule(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet43)) {
		getToken(17/*identifier*/, llTokenSet54, llTokenSet54, false);
		if (tokenInCommon(currSymbol, llTokenSet44)) {
			
			if (!defineFun(SymbolType.terminal, lastSymbol, lastSymbolPos.line, lastSymbolPos.position, docComment)) {
			    llerror("symbol redefined");
			}
			token_declaration(uniteTokenSets(follow, llTokenSet50));
		} else if (tokenInCommon(currSymbol, llTokenSet55)) {
			
			enterNonTerminal();
			if (tokenInCommon(currSymbol, llTokenSet30)) {
				formal_parameters(llTokenSet56);
			}
			if (tokenInCommon(currSymbol, llTokenSet53)) {
				function_return(llTokenSet49);
			}
			getToken(9/*colon*/, llTokenSet57, uniteTokenSets(follow, llTokenSet57), true);
			alternatives(uniteTokenSets(follow, llTokenSet50));
		} else {
			llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
		}
	} else if (tokenInCommon(currSymbol, llTokenSet58)) {
		getToken(23/*ignore_sym*/, llTokenSet51, llTokenSet51, false);
		getToken(15/*string*/, llTokenSet50, uniteTokenSets(follow, llTokenSet50), true);
	} else {
		llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
	}
	getToken(8/*period*/, llTokenSet23, follow, true);
}


function single_element(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet43)) {
		getToken(17/*identifier*/, llTokenSet59, uniteTokenSets(follow, llTokenSet59), true);
		
		usageFun(SymbolType.unknown, lastSymbol, lastSymbolPos.line, lastSymbolPos.position);
		if (tokenInCommon(currSymbol, llTokenSet30)) {
			actual_parameters(uniteTokenSets(follow, llTokenSet53));
		}
		if (tokenInCommon(currSymbol, llTokenSet53)) {
			function_result(follow);
		}
	} else if (tokenInCommon(currSymbol, llTokenSet51)) {
		getToken(15/*string*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet30)) {
		getToken(1/*left_parenthesis*/, llTokenSet60, llTokenSet60, false);
		alternatives(llTokenSet61);
		getToken(2/*right_parenthesis*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet62)) {
		getToken(25/*on_sym*/, llTokenSet63, llTokenSet63, false);
		if (tokenInCommon(currSymbol, llTokenSet43)) {
			getToken(17/*identifier*/, llTokenSet64, uniteTokenSets(follow, llTokenSet64), true);
			
			usageFun(SymbolType.unknown, lastSymbol, lastSymbolPos.line, lastSymbolPos.position);
		} else if (tokenInCommon(currSymbol, llTokenSet51)) {
			getToken(15/*string*/, llTokenSet64, uniteTokenSets(follow, llTokenSet64), true);
		} else {
			llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
		}
		if (tokenInCommon(currSymbol, llTokenSet65)) {
			getToken(26/*break_sym*/, llTokenSet23, follow, true);
		} else if (tokenInCommon(currSymbol, llTokenSet66)) {
			getToken(27/*error_sym*/, llTokenSet51, llTokenSet51, false);
			getToken(15/*string*/, llTokenSet23, follow, true);
		} else {
			llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
		}
	} else {
		llerror("single_element");
	}
}


function token_declaration(follow: LLTokenSet): void {
	getToken(12/*is*/, llTokenSet63, llTokenSet63, false);
	if (tokenInCommon(currSymbol, llTokenSet51)) {
		getToken(15/*string*/, llTokenSet67, uniteTokenSets(follow, llTokenSet67), true);
		if (tokenInCommon(currSymbol, llTokenSet67)) {
			getToken(19/*keyword_sym*/, llTokenSet43, llTokenSet43, false);
			getToken(17/*identifier*/, llTokenSet23, follow, true);
		}
	} else if (tokenInCommon(currSymbol, llTokenSet43)) {
		getToken(17/*identifier*/, llTokenSet30, llTokenSet30, false);
		getToken(1/*left_parenthesis*/, llTokenSet43, llTokenSet43, false);
		getToken(17/*identifier*/, llTokenSet61, llTokenSet61, false);
		getToken(2/*right_parenthesis*/, llTokenSet23, follow, true);
	} else {
		llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
	}
}


function wildcard(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet29)) {
		non_bracket_symbol(follow);
	} else if (tokenInCommon(currSymbol, llTokenSet30)) {
		getToken(1/*left_parenthesis*/, llTokenSet21, llTokenSet21, false);
		wildcards(llTokenSet61);
		getToken(2/*right_parenthesis*/, llTokenSet23, follow, true);
	} else if (tokenInCommon(currSymbol, llTokenSet32)) {
		getToken(3/*left_bracket*/, llTokenSet68, llTokenSet68, false);
		wildcards(llTokenSet69);
		getToken(4/*right_bracket*/, llTokenSet23, follow, true);
	} else {
		llerror("wildcard");
	}
}


function wildcards(follow: LLTokenSet): void {
	if (tokenInCommon(currSymbol, llTokenSet70)) {
		do {
			if (tokenInCommon(currSymbol, llTokenSet20)) {
				wildcard(uniteTokenSets(follow, llTokenSet70));
			} else if (tokenInCommon(currSymbol, llTokenSet22)) {
				getToken(7/*comma*/, llTokenSet70, uniteTokenSets(follow, llTokenSet70), true);
			} else {
				llerror("syntax error after %s %.*s", lastSymbol, bufferEnd, scanBuffer);
			}
			waitForToken(uniteTokenSets(llTokenSet70, follow), follow);
		} while (tokenInCommon(currSymbol, llTokenSet70));
	}
}



var inputString: string, inputPosition: number;

function getNextCharacter() {
    return inputPosition < inputString.length?
           inputString[inputPosition++]: undefined;
}

export function parse(str: string, ): void {
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
    llgen_file(endOfInputSet);
}

