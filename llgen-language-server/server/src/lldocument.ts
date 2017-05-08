import {
    EMap, Equal
} from './util';

import {
    Diagnostic, Position, DiagnosticSeverity, Range,
    IConnection
} from 'vscode-languageserver';

import {
    PathPositionTree, positionCompare, PathPositionNode,
    findPathPositionNode, createPathPositionNode
} from './path';

import {
    SymbolUsage,
    SymbolDefinition
} from './symbols';

/**
 * Contains the information obtained from parsing one .ll file.
 * 
 * @export
 * @class LLGenDocument
 */
export class LLDocument {
    
    /** Location and unique id of the document */
    uri: string;

    /** all symbols defined in this document indexed by type */
    definedSymbols: Map<string, SymbolDefinition[]>;

    /** a list of all used symbols per document, sorted by occurrence (line, column) */
    usedSymbols: SymbolUsage[];

    /** Position tree representing the symbols and their sub expressions in this document */
    positionTree: PathPositionTree;

    /**
     * List of errors and warnings from the parsing phase. This list doesn't
     * change during the analysis phase. 
     */
    private parseDiagnostics: Diagnostic[] = [];

    /** List of errors and warnings from later analysis */
    private analysisDiagnostics: Diagnostic[] = [];

    /**
     * When true, the errors have changed since they were last sent to the work
     * space, and need to be sent again.
     */
    private errorsChanged: boolean = false;

    constructor(uri: string) {
        this.uri = uri;
        this.init();
    }

    init(): void {
        this.definedSymbols = new Map();
        this.usedSymbols = [];
        this.positionTree = [];
    }

    addSymbolDefinition(symbol: string, symbolDefinition: SymbolDefinition): boolean {
        let map = this.definedSymbols;
        let firstDefinition = true;

        if (!map.has(symbol)) {
            map.set(symbol, []);
        } else {
            firstDefinition = false;
        }
        map.get(symbol)!.push(symbolDefinition);
        return firstDefinition;
    }

    addSymbolUsage(usage: SymbolUsage): void {
        this.usedSymbols.push(usage);
    }

    /** Removes the information a document has added, initializing it for
     *  (re)parsing. If the same document is parsed again, the state afterwards
     *  is identical to the state before calling this function. Tracing changes
     *  to inheritanceMapClosure is the caller's responsibility.
     */
    clear(): void {
        this.init();
        this.resetParseDiagnostics();
    }

    /** Like clearDocument, but the document won't be parsed any time soon.
     *  Sets inheritanceClosureOutdated in case there is a possible change.
     */
    close(connection: IConnection): boolean {
        connection.sendDiagnostics({
            uri: this.uri,
            diagnostics: []
        });
        return false;
    }

    addParseDiagnostic(d: Diagnostic): void {
        if (this.parseDiagnostics.length === 0 ||
              positionCompare(
                  this.parseDiagnostics[this.parseDiagnostics.length - 1].range.start,
                  d.range.start
              ) !== 0) {
            this.parseDiagnostics.push(d);
            this.errorsChanged = true;
        }
    }

    resetParseDiagnostics(): void {
        this.errorsChanged = true;
        this.parseDiagnostics.length = 0;
    }

    addAnalysisDiagnostic(d: Diagnostic): void {
        this.analysisDiagnostics.push(d);
        this.errorsChanged = true;
    }

    resetAnalysisDiagnostics(): void {
        if (this.analysisDiagnostics.length !== 0) { 
            this.errorsChanged = true;
        }
        this.analysisDiagnostics.length = 0;
    }

    sendDiagnostics(connection: IConnection, maxNumberOfProblems: number): void {
        if (this.errorsChanged) {
            let diagnostics = this.parseDiagnostics.concat(this.analysisDiagnostics);
            if (diagnostics.length > maxNumberOfProblems) {
                diagnostics = diagnostics.
                    sort(function(a: Diagnostic, b: Diagnostic): number {
                            return a.severity! - b.severity!;
                        }).
                    slice(0, maxNumberOfProblems).
                    sort(function(a: Diagnostic, b: Diagnostic): number {
                        return positionCompare(a.range.start, b.range.start);
                    });
            }
            connection.sendDiagnostics({
                uri: this.uri,
                diagnostics: diagnostics
            });
            this.errorsChanged = false;
        }
    }

    getShortName(): string {
        return this.uri.slice(this.uri.lastIndexOf('/') + 1);
    }

    postParseValidation(): void {
        for (var i = 0; i < this.usedSymbols.length; i++) {
            var sym = this.usedSymbols[i];
            if (!this.definedSymbols.has(sym.symbol)) {
                this.addParseDiagnostic({
                    severity: DiagnosticSeverity.Error,
                    range: sym.getRange(),
                    message: "symbol not defined"
                });
            }
        }
    }
}
