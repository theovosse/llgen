#!llerror

/* Generate by: ~/work/llgen/llgen +ts -main -llerror llgen.ll */

{

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

}

left_parenthesis = "\(".
right_parenthesis = "\)".
left_bracket = "\[".
right_bracket = "\]".
left_brace = "{".
right_brace = "}".
comma = ",".
period = "\.".
colon = ":".
semicolon = ";".
assign = "->".
is = "=".
feature_def = "#!?[a-zA-Z_][a-zA-Z0-9_]*".
number = "[+\-]?((([0-9]+(\.[0-9]*)?)|([0-9]*\.[0-9]+))([Ee][+-]?[0-9]+)?)".
string = "\"([^\\\r\n\"]|\\.)*\"".
output = "'([^\\\r\n']|\\.)*'".
identifier = "[a-zA-Z_][a-zA-Z_0-9]*".
chain_sym = "CHAIN" KEYWORD identifier.
keyword_sym = "KEYWORD" KEYWORD identifier.
sequence_sym = "SEQUENCE" KEYWORD identifier.
option_sym = "OPTION" KEYWORD identifier.
subtoken_sym = "SUBTOKEN" KEYWORD identifier.
ignore_sym = "IGNORE" KEYWORD identifier.
shift_sym = "SHIFT" KEYWORD identifier.
on_sym = "ON" KEYWORD identifier.
break_sym = "BREAK" KEYWORD identifier.
error_sym = "ERROR" KEYWORD identifier.
single_token = "[+\-*/%&|\^\<\>~!?]|\+\+|--|\<\<|\>\>|&&|\|\||\.\.\.|((\<\<|\>\>|[+\-/%&|\^])=)".
IGNORE "[ \t\n\r]+".
IGNORE "//.*".
IGNORE "/\*([^*]|\*[^/])*\*/".

llgen_file:
    {
        initialize();
    },
    (
        rule;
        code;
        feature_def
    ) SEQUENCE.

rule:
    (
        identifier,
        (
            {
                if (!defineFun(SymbolType.terminal, lastSymbol, lastSymbolPos.line, lastSymbolPos.position, docComment)) {
                    llerror("symbol redefined");
                }
            },
            token_declaration;

            {
                enterNonTerminal();
            },
            formal_parameters OPTION,
            function_return OPTION,
            colon,
            alternatives
        );
        ignore_sym, string
    ),
    period.

token_declaration:
    is,
    (
        string, (keyword_sym, identifier) OPTION;
        identifier, left_parenthesis, identifier, right_parenthesis
    ).

formal_parameters:
    left_parenthesis,
    (wildcard SEQUENCE) CHAIN comma,
    right_parenthesis.

function_return:
    assign, identifier, identifier, (is, code) OPTION.

alternatives:
    alternative CHAIN semicolon.

alternative:
    (element CHAIN comma) OPTION.

element:
    shift_sym OPTION,
    single_element,
    (
        chain_sym, single_element;
        option_sym;
        sequence_sym, option_sym OPTION
    ) OPTION;
	code;
	output.

single_element:
	identifier, {
        usageFun(SymbolType.unknown, lastSymbol, lastSymbolPos.line, lastSymbolPos.position);
    },
    actual_parameters OPTION, function_result OPTION;
	string;
	left_parenthesis, alternatives, right_parenthesis;
    on_sym, (
        identifier, {
            usageFun(SymbolType.unknown, lastSymbol, lastSymbolPos.line, lastSymbolPos.position);
        };
        string
    ), (
        break_sym;
        error_sym, string
    ).

actual_parameters:
    left_parenthesis,
    (wildcard SEQUENCE) CHAIN comma,
    right_parenthesis.

function_result:
    assign, identifier.

wildcard:
    non_bracket_symbol;
	left_parenthesis, wildcards, right_parenthesis;
	left_bracket, wildcards, right_bracket.

wildcards:
    (wildcard; comma) SEQUENCE OPTION.

non_bracket_symbol:
	single_token;
	identifier;
	colon;
	semicolon;
	period;
    output;
	string;
	number;
    assign;
	is.

code:
    left_brace, code_element SEQUENCE OPTION, right_brace.

code_element:
    (
        non_bracket_symbol;
        comma;
        left_parenthesis, code_element SEQUENCE OPTION, right_parenthesis;
        left_bracket, code_element SEQUENCE OPTION, right_bracket;
        left_brace, code_element SEQUENCE OPTION, right_brace
    ).
