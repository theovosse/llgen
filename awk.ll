#!llerror

/* Generate by: ~/work/llgen/llgen +ts -main -llerror awk.ll */

{

import {
    SymbolType
} from './symbols';

let fileBaseName: string;
export function setFileBaseName(fn: string|undefined) {
    fileBaseName = fn === undefined? "unknown": fn;
}

let defineFun: (type: SymbolType, lastSymbol: string, llLineNumber: number, llLinePosition: number, docComment: string|undefined) => void;
export function setDefineFun(fn: (type: SymbolType, lastSymbol: string, llLineNumber: number, llLinePosition: number, docComment: string|undefined) => void) {
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

let includeFun: (include: string, relative: boolean, line: number, position: number, length: number) => void;
export function setIncludeFun(fn: (file: string, relative: boolean, line: number, position: number, length: number) => void) {
    includeFun = fn;
}

function llerror(...args: any[]): void {
    messageFun("error", "syntax", sprintf(args), lastSymbolPos.line, lastSymbolPos.position, lastSymbol.length);
}

function llwarn(warnType: string, ...args: any[]): void {
    let msg = sprintf(args);
    messageFun("warning", warnType, msg, lastSymbolPos.line, lastSymbolPos.position, lastSymbol.length);
}

let stylisticWarnings = {
    superfluousComma: true,
    fileNameIsClassName: true
};

export function updateStylisticWarnings(obj: any): void {
    if (obj instanceof Object) {
        if ("superfluousComma" in obj) {
            stylisticWarnings.superfluousComma = obj.superfluousComma;
        }
        if ("fileNameIsClassName" in obj) {
            stylisticWarnings.fileNameIsClassName = obj.fileNameIsClassName;
        }
    }
}

class ExprTree {
    head: string|undefined;
    type: string;
    operands?: ExprTreeArray;
}

type ExprTreeOrUndefined = ExprTree|undefined;
type ExprTreeArray = ExprTreeOrUndefined[];

let docComment: string|undefined = undefined;

function for_in_expr(expr: ExprTree): boolean {
    return expr.type === "binary" && expr.head === "in" && 
           expr.operands !== undefined && expr.operands[0] !== undefined &&
           expr.operands[0]!.type === "identifier";
}

}

include_sym = "@include".
left_parenthesis = "\(".
right_parenthesis = "\)".
left_bracket = "\[".
right_bracket = "\]".
left_brace = "{".
right_brace = "}".
comma = ",".
dot = "\.".
question_mark = "\?".
colon = ":".
semicolon = ";".
assign = "(\+|-|\*|/|%|\*\*|\^)?=".
and_operator = "\&\&".
dollar = "\$".
or_operator = "\|\|".
increment = "\+\+|--".
number = "[+\-]?((([0-9]+(\.[0-9]*)?)|([0-9]*\.[0-9]+))([Ee][+-]?[0-9]+)?)".
string = "\"([^\\\"]|\\.)*\"".
regexp = "/([^/\r\n]|\\/)*/".
identifier = "[a-zA-Z_][a-zA-Z_0-9]*".
function_call_identifier = "[a-zA-Z_][a-zA-Z_0-9]*\(".
function_sym = "function" KEYWORD identifier.
delete_sym = "delete" KEYWORD identifier.
print_sym = "print" KEYWORD identifier.
if_sym = "if" KEYWORD identifier.
else_sym = "else" KEYWORD identifier.
for_sym = "for" KEYWORD identifier.
while_sym = "while" KEYWORD identifier.
do_sym = "do" KEYWORD identifier.
next_sym = "next" KEYWORD identifier.
exit_sym = "exit" KEYWORD identifier.
return_sym = "return" KEYWORD identifier.
switch_sym = "switch" KEYWORD identifier.
case_sym = "case" KEYWORD identifier.
default_sym = "default" KEYWORD identifier.
break_sym = "break" KEYWORD identifier.
continue_sym = "continue" KEYWORD identifier.
in_sym = "in" KEYWORD identifier.
if_par_sym = "if\(" KEYWORD function_call_identifier.
for_par_sym = "for\(" KEYWORD function_call_identifier.
while_par_sym = "while\(" KEYWORD function_call_identifier.
switch_par_sym = "switch\(" KEYWORD function_call_identifier.
binary_operator = "\+|\*|/|%|[!=]=|[<>]=?|!?~|\*\*|\^|\|".
unary_binary_operator = "-".
unary_operator = "!".
doc_comment = "///.*[^/\r\n].*".
nl = "[\r\n]+".
IGNORE "[ \t]+".
IGNORE "#.*".
IGNORE "\\(\r\n?|\n\r?)".

awk_file:
    (
        doc_comment, {
            docComment = docComment === undefined? lastSymbol: docComment + "\n" + lastSymbol;
        };
        include_line, {
            docComment = undefined;
        };
        line_block, {
            docComment = undefined;
        };
        function_definition, {
            docComment = undefined;
        };
        nl
    ) SEQUENCE.

nl_opt: SHIFT nl OPTION.

include_line:
    include_sym, string.

line_block:
    expression OPTION, /* Can include regexps, BEGIN, etc */
    block(true).

function_definition:
    function_sym,
    (   identifier, {
            defineFun(SymbolType.func, lastSymbol, lastSymbolPos.line, lastSymbolPos.position, docComment);
        },
        left_parenthesis;
        function_call_identifier, {
            defineFun(SymbolType.func, lastSymbol.slice(0, -1), lastSymbolPos.line, lastSymbolPos.position, docComment);
        }
    ),
    formal_parameters,
    nl_opt,
    block(false).

formal_parameters:
    (identifier CHAIN (comma, nl_opt)) OPTION,
    right_parenthesis.

block(is_line_block boolean):
    left_brace,
    statement_sequence(is_line_block) OPTION,
    right_brace,
    nl_opt.

statement_sequence(is_line_block boolean):
    (
        (
            assign_statement;
            delete_statement;
            print_statement;
            break_sym;
            continue_sym;
            return_statement(is_line_block);
            exit_statement(is_line_block)
        ),
        (
            statement_sep, statement_sequence(is_line_block) OPTION;
            /* end of block */
        )
    );
    if_statement(is_line_block), statement_sequence(is_line_block) OPTION;
    for_statement(is_line_block), statement_sequence(is_line_block) OPTION;
    while_statement(is_line_block), statement_sequence(is_line_block) OPTION;
    do_while_statement(is_line_block), statement_sequence(is_line_block) OPTION;
    next_statement(is_line_block), statement_sequence(is_line_block) OPTION;
    switch_statement(is_line_block), statement_sequence(is_line_block) OPTION;
    statement_sep, statement_sequence(is_line_block) OPTION.

statement(is_line_block boolean):
    assign_statement;
    delete_statement;
    print_statement;
    if_statement(is_line_block);
    for_statement(is_line_block);
    while_statement(is_line_block);
    do_while_statement(is_line_block);
    next_statement(is_line_block);
    return_statement(is_line_block);
    exit_statement(is_line_block);
    switch_statement(is_line_block);
    break_sym;
    continue_sym;
    /* empty statement */ semicolon.

statement_sep:
    semicolon; nl.

assign_statement -> expr_list ExprTreeArray = { [] }:
    (expression -> expr, { expr_list.push(expr); }) CHAIN (comma, nl_opt).

rvalue:
    identifier, {
        usageFun(SymbolType.variable, lastSymbol, lastSymbolPos.line, lastSymbolPos.position);
    },
    array_index SEQUENCE OPTION;
    field.

expression -> expr ExprTreeOrUndefined = { undefined }:
    (unary_operator; unary_binary_operator),
    { let op = lastSymbol; },
    expression -> sub_expr, {
        expr = { head: op, type: "unary", operands: [sub_expr] };
    };
    single_term -> sub_expr1,
    (
        { let op: string|undefined = undefined; },
        (
            binary_operator, { op = lastSymbol; }, nl_opt;
            and_operator, { op = lastSymbol; }, nl_opt;
            or_operator, { op = lastSymbol; }, nl_opt;
            in_sym, { op = lastSymbol; }, nl_opt;
            /* Empty operand, no nl continuation! */ { op = "concat"; }
        ),
        expression -> sub_expr2, {
            expr = { head: op, type: "binary", operands: [sub_expr1, sub_expr2] };
        };
        question_mark, expression -> sub_expr2, colon, expression -> sub_expr3, {
            expr = { head: "?:", type: "ternary", operands: [sub_expr1, sub_expr2, sub_expr3] };
        };
        /* end of the expression */ { expr = sub_expr1; } 
    ).

single_term -> expr ExprTreeOrUndefined = { undefined }:
    left_parenthesis, expression -> sub_expr, right_parenthesis, {
        expr = { head: "group", type: "unary", operands: [sub_expr] };
    };
    number, { expr = { head: lastSymbol, type: "number" } };
    string, { expr = { head: lastSymbol, type: "string" } };
    field -> fld_expr, { expr = fld_expr };
    regexp, { expr = { head: lastSymbol, type: "regexp" } };
    function_call_identifier, {
        let funcId = lastSymbol.slice(0, -1);
        usageFun(SymbolType.func, funcId, lastSymbolPos.line, lastSymbolPos.position);
    },
    actual_parameters -> parameters, {
        expr = { head: funcId, type: "call", operands: parameters };
    };
    { let pre_increment: string|undefined = undefined; },
    (increment, { pre_increment = lastSymbol; }) OPTION,
    identifier, {
        let id = lastSymbol;
        expr = { head: id, type: "identifier" };
        usageFun(SymbolType.variable, id, lastSymbolPos.line, lastSymbolPos.position);
    },
    (
        (
            array_index -> index, {
                expr = { head: "sub", type: "binary", operands: [expr, index] };
            }
         ) SEQUENCE;
        /* just an identifier */
    ), {
        if (pre_increment !== undefined) {
            expr = { head: pre_increment, type: "pre", operands: [expr] };
        }
    }, (
        SHIFT increment, { expr = { head: lastSymbol, type: "post", operands: [expr] }; }; /* This is ambiguous, just like in C: i+++++i */
        assign, { let assign_op = lastSymbol; },
        expression -> lvalue, {
            expr = { head: assign_op, type: "assign", operands: [expr, lvalue] };
        };
        /* only a value */
    ).

field -> expr ExprTreeOrUndefined = { undefined }:
    dollar, expression -> field_expr, {
        expr = { head: "$", type: "field", operands: [field_expr] };
    }.

array_index -> index ExprTreeOrUndefined = { undefined }:
    { let indices: ExprTreeArray = []; },
    left_bracket,
    (expression -> expr, { indices.push(expr); }) CHAIN (comma, nl_opt),
    right_bracket, {
        index = indices.length === 1? indices[0]:
                { head: "join", type: "call", operands: indices };
    }.

actual_parameters -> parameters ExprTreeArray = { [] }:
    ((expression -> parameter, { parameters.push(parameter); }) CHAIN (comma, nl_opt)) OPTION,
    right_parenthesis.

delete_statement:
    delete_sym,
    identifier, {
        usageFun(SymbolType.variable, lastSymbol, lastSymbolPos.line, lastSymbolPos.position);
    },
    array_index OPTION.

print_statement:
    print_sym,
    SHIFT (expression CHAIN (comma, nl_opt)) OPTION.

if_statement(is_line_block boolean):
    (if_sym, left_parenthesis; if_par_sym),
    expression,
    right_parenthesis, nl_opt,
    (
        block(is_line_block);
        statement(is_line_block), SHIFT statement_sep OPTION
    ),
    SHIFT nl SEQUENCE OPTION,
    SHIFT (
        else_sym, nl_opt,
        (
            block(is_line_block);
            statement(is_line_block)
        )
    ) OPTION.

for_statement(is_line_block boolean):
    (for_sym, left_parenthesis; for_par_sym),
    { let assign_expr: ExprTreeOrUndefined[]|undefined = undefined; },
    (
        assign_statement -> assign_expr2, { assign_expr = assign_expr2; };
    ),
    (
        {
            if (assign_expr !== undefined && assign_expr.length === 1 && assign_expr[0] !== undefined) {
                if (for_in_expr(assign_expr[0]!)) {
                    llerror("wrong for loop");
                }
            }
        },
        semicolon,
        expression OPTION,
        semicolon,
        assign_statement OPTION;
        /* this branch covers for (var in expr) */ {
            if (assign_expr !== undefined && assign_expr.length === 1 && assign_expr[0] !== undefined) {
                if (!for_in_expr(assign_expr[0]!)) {
                    llerror("wrong for loop");
                }
            }
        }
    ),
    right_parenthesis, nl_opt,
    (
        block(is_line_block);
        statement(is_line_block)
    ).

while_statement(is_line_block boolean):
    (while_sym, left_parenthesis; while_par_sym),
    expression,
    right_parenthesis, nl_opt,
    (
        block(is_line_block);
        statement(is_line_block)
    ).

do_while_statement(is_line_block boolean):
    do_sym, nl_opt,
    block(is_line_block), nl_opt,
    (while_sym, left_parenthesis; while_par_sym),
    expression,
    right_parenthesis.

switch_statement(is_line_block boolean):
    (switch_sym, left_parenthesis; switch_par_sym),
    expression,
    right_parenthesis, nl_opt,
    left_brace,
    (
        (
            case_sym, expression;
            default_sym
        ),
        colon, nl_opt,
        statement(is_line_block) SEQUENCE OPTION
    ) SEQUENCE,
    right_brace,
    nl_opt.

next_statement(is_line_block boolean):
    next_sym, {
        if (!is_line_block) {
            llwarn("info", "next statement not inside line block");
        }
    }.

return_statement(is_line_block boolean):
    return_sym, {
        if (is_line_block) {
            llwarn("info", "return statement inside line block");
        }
    },
    SHIFT expression OPTION.

exit_statement(is_line_block boolean):
    exit_sym,
    SHIFT expression OPTION.
