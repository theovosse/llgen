import {
    Equal
} from './util';

import {
    Position, Range
} from 'vscode-languageserver';

import {
    LLDocument
} from './lldocument';

export function positionOffset(position: Position, nrChars: number): Position {
    return { line: position.line, character: position.character + nrChars };
}

export function getRange(position: Position, nrChars: number): Range {
    return { start: position, end: positionOffset(position, nrChars) };
}

export enum SymbolType {
    unknown,
    terminal,
    nonTerminal,
    defineTerminal,
    defineNonTerminal
}

export function getSymbolDefineType(t: SymbolType): SymbolType {
    switch (t) {
      case SymbolType.terminal: return SymbolType.defineTerminal;
      case SymbolType.nonTerminal: return SymbolType.defineNonTerminal;
    }
    return t;
}

export function removeSymbolDefineType(t: SymbolType): SymbolType {
    switch (t) {
      case SymbolType.defineTerminal: return SymbolType.terminal;
      case SymbolType.defineNonTerminal: return SymbolType.nonTerminal;
    }
    return t;
}

export function isSymbolDefineType(t: SymbolType): boolean {
    return t >= SymbolType.defineTerminal;
}

/** Information about the definition of a symbol
*/
export class SymbolDefinition implements Equal {
    /** the symbol itself; when undefined, it represents screenArea */
    private symbol: string;

    constructor(
        /// file in which it is defined
        public document: LLDocument,
        /// symbol position start
        public position: Position,
        public type: SymbolType,
        public docComment: string,
        symbol: string
    ) {
        this.symbol = symbol;
    }

    getRange(): Range {
        return getRange(this.position, this.symbol === undefined? 10: this.symbol.length);
    }

    isEqual(p: SymbolDefinition): boolean {
        return this.symbol === p.symbol && this.document === p.document &&
            this.position === p.position && this.type === p.type &&
            this.docComment === p.docComment;
    }

    getSymbol(): string {
        return this.symbol;
    }
}

/** Information about a single symbol usage occurrence in a file
*/
export class SymbolUsage implements Equal {
    constructor(
        /// The symbol itself
        public symbol: string,
        public type: SymbolType,
        /// symbol position start
        public position: Position
    ) {
    }

    isEqual(p: SymbolUsage): boolean {
        return this.symbol === p.symbol && this.type === p.type &&
               this.position === p.position;
    }

    getRange(): Range {
        return getRange(this.position, this.symbol === undefined? 10: this.symbol.length);
    }
}
