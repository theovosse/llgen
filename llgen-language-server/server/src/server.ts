'use strict';

import {
    IPCMessageReader, IPCMessageWriter, createConnection, IConnection, TextDocuments,
    DiagnosticSeverity, InitializeParams, InitializeResult, CompletionItem,
    CompletionItemKind, Location, Range, Position, Definition, Hover,
    SymbolInformation, SymbolKind, TextDocumentIdentifier, ReferenceParams,
    WorkspaceSymbolParams, TextDocumentPositionParams, TextDocument,
    DocumentSymbolParams, DidCloseTextDocumentParams
} from 'vscode-languageserver';

import {
    parse, setFileBaseName, setDefineFun, setMessageFun, setUsageFun,
    updateStylisticWarnings, lastSymbolPos, nonTerminalParameters
} from "./llgen";

import  {
    PathPositionNode, PathPositionTree, finishPositionTree
} from './path';

import {
    LLDocument
} from './lldocument';

import {
    SymbolDefinition,
    SymbolUsage,
    SymbolType,
    getRange,
    getSymbolDefineType,
    isSymbolDefineType,
    removeSymbolDefineType
} from './symbols';

import {
    readFile, makeURI, makeAbsolutePathName, makeRelativePathName,
    fileExists, nrOpenReadRequests
} from './filesystem';

import {
    setDebugLogDir, debugLog
} from './filesystem';

function processConfigFile(dir: string): void {
    let configFileName: string = dir + "/llconfig.json";

    haltProcessingQueue();
    readFile(configFileName, function (err: any, data: string): void {
        allowProcessingQueue();
        if (!err) {
            // Note: this document is not retained
            let configDoc: LLDocument = new LLDocument(makeURI(configFileName));
            addToFrontOfProcessingQueue(configDoc, data, ProcessQueueItemType.config, false);
        }
    });
}

// PROCESSING QUEUE

enum ProcessQueueItemType {
    ll,
    config,
}

/** Item to process */
class ProcessQueueItem {
    /** The document holder */
    doc: LLDocument;
    /** The textual contents of the document */
    text: string;
    /** The type of document */
    type: ProcessQueueItemType;
    /** When true, the file is open in the editor */
    openInEditor: boolean;
}

let processingAllowed: boolean = true;

let processingQueue: ProcessQueueItem[] = [];

function haltProcessingQueue(): void {
    processingAllowed = false;
    // debugLog("halt queue"); // !!!
}

function allowProcessingQueue(): void {
    processingAllowed = true;
    // debugLog("resume queue " + processingQueue.map(function (pqi): string {
    //     return pqi.doc.getShortName() + " (" + ProcessQueueItemType[pqi.type] + ")";
    // }).join(", "));  // !!!
}

function addToFrontOfProcessingQueue(doc: LLDocument, text: string, type: ProcessQueueItemType, openInEditor: boolean): void {
    // debugLog("add to front of queue " + doc.getShortName() + " " + ProcessQueueItemType[type]); // !!!
    processingQueue.unshift({doc: doc, text: text, type: type, openInEditor: openInEditor});
    processNextQueueItem();
}

function addToEndOfProcessingQueue(doc: LLDocument, text: string, type: ProcessQueueItemType, openInEditor: boolean): void {
    // debugLog("add to end of queue " + doc.getShortName() + " " + ProcessQueueItemType[type]); // !!!
    processingQueue.push({doc: doc, text: text, type: type, openInEditor: openInEditor});
    processNextQueueItem();
}

function processNextQueueItem(): void {
    while (nrOpenReadRequests === 0 && processingAllowed && processingQueue.length !== 0) {
        let queueItem: ProcessQueueItem = processingQueue.shift()!;
        // debugLog("process " + queueItem.doc.uri + " " + ProcessQueueItemType[queueItem.type]); // !!!
        switch (queueItem.type) {
          case ProcessQueueItemType.config:
            processConfiguration(queueItem.doc, queueItem.text);
            break;
          case ProcessQueueItemType.ll:
            validateTextDocument(queueItem.doc, queueItem.text, queueItem.openInEditor);
            break;
        }
    }
    if (nrOpenReadRequests === 0 && processingAllowed) {
        finishUpProcessing();
    }
}

function sendDiagnostics(): void {
    for (let doc of documentMap.values()) {
        doc.sendDiagnostics(connection, config.maxNumberOfProblems);
    }
}

function finishUpProcessing(): void {
    sendDiagnostics();
} 

// LANGUAGE DEFINITION AND USAGE

function processConfiguration(doc: LLDocument, configText: string): void {
    // no config read
}

// CALLBACK FROM THE PARSER

// Adds a message; ignores specific warnings
function messageFun(type: string, subType: string, msg: string, position: Position, length: number, doc: LLDocument): void {
    function ignoreThisMessage(): boolean {
        return type === "warning" &&
               ((subType === "comma" && !config.stylisticWarnings.missingSemicolon) ||
                (subType === "future" && !config.stylisticWarnings.gllMode));
    }
    if (!ignoreThisMessage()) {
        doc.addParseDiagnostic({
            severity: type === "warning"? DiagnosticSeverity.Warning: DiagnosticSeverity.Error,
            range: getRange(position, length),
            message: msg
        });
    }
}

/** Maps docURI -> LLGenDocument */
let documentMap: Map<string, LLDocument> = new Map();

function addSymbolDefinition(type: SymbolType, doc: LLDocument, symbol: string, position: Position, docComment: string): boolean {
    let symbolDefinition: SymbolDefinition = new SymbolDefinition(
        doc, position, type, docComment, symbol);
    let firstDefinition =  doc.addSymbolDefinition(symbol, symbolDefinition);
    let defineType = getSymbolDefineType(type);

    if (defineType !== undefined) {
        addSymbolUsage(defineType, doc, symbol, position);
    }
    return firstDefinition;
}

// Store symbol usage
function addSymbolUsage(type: SymbolType, doc: LLDocument, symbol: string, position: Position): void {
    let usage = new SymbolUsage(symbol, type, position);
    
    doc.addSymbolUsage(usage);
}

// DOCUMENT HANDLING

interface LLGenLanguageServerSettings {
    /** Maximum size of list sent back */
    maxNumberOfProblems?: number;
    gll?: boolean;
    stylisticWarnings?: {
        /** When true, warns for superfluous commas */
        missingSemicolon?: boolean;
        /** When true, warns that var <filename> should be var classes */
        gllMode?: boolean;
    };
    llPath?: string[];
}

// Default config
let config = {
	maxNumberOfProblems: 100,
    gll: true,
    stylisticWarnings: {
        missingSemicolon: false,
        gllMode: true
    }
};

function updateConfiguration(settings: LLGenLanguageServerSettings|undefined): boolean {
    let reparse: boolean = false;

    if (settings === undefined) {
        return false;
    }
    if (settings.maxNumberOfProblems !== config.maxNumberOfProblems) {
          config.maxNumberOfProblems = settings.maxNumberOfProblems || 100;
        reparse = true;
    }
    if (settings.stylisticWarnings instanceof Object) {
        if (settings.stylisticWarnings.missingSemicolon !== undefined &&
              settings.stylisticWarnings.missingSemicolon !== config.stylisticWarnings.missingSemicolon) {
            config.stylisticWarnings.missingSemicolon = !!settings.stylisticWarnings.missingSemicolon;
            reparse = true;
        }
        if (settings.stylisticWarnings.gllMode !== undefined &&
              settings.stylisticWarnings.gllMode !== config.stylisticWarnings.gllMode) {
            config.stylisticWarnings.gllMode = !!settings.stylisticWarnings.gllMode;
            reparse = true;
        }
        updateStylisticWarnings(config.stylisticWarnings);
    }
    return reparse;
}

// let count: number = 0; // debugging

function validateTextDocument(doc: LLDocument, text: string, openInEditor: boolean): void {
    let baseName: string = doc.uri.replace(/^(.*\/)?([^\/]*)\..*$/, "$2");
    
    // Set up handlers
    setFileBaseName(baseName.match(/Constants$/)? undefined: baseName);
    setDefineFun(function(type: SymbolType, symbol: string, line: number, position: number, docComment: string): boolean {
        return addSymbolDefinition(type, doc, symbol, {line: line - 1, character: position - 1}, docComment);
    });
    setUsageFun(function(type: SymbolType, symbol: string, line: number, position: number): void {
        // connection.console.log(`message ${type} ${inherit} ${symbol}`); // !!!
        addSymbolUsage(type, doc, symbol, {line: line - 1, character: position - 1});
    });
    setMessageFun(function(type: string, subType: string, msg: string, line: number, position: number, length: number): void {
        // connection.console.log(`message ${type} ${subType} ${msg}`); // !!!
        messageFun(type, subType, msg, {line: line - 1, character: position - 1}, length, doc);
    });

	validateText(doc, text);

    doc.postParseValidation();
}

/** This should be 1 at most; anything higher and one call to validateText
    interrupts another.
**/
let parseLevel: number = 0;

function validateText(doc: LLDocument, text: string): void {
	if (parseLevel !== 0) {
		connection.console.log("parseLevel = " + parseLevel);
	}
	parseLevel++;

    // Clear information from previous parse
    doc.clear();

    // Parse the text; messages are collected via the above handlers
    try {
        parse(text);
    } catch (ex) {
        connection.console.error(ex);
        messageFun("error", "exception", "parser crash", {line: lastSymbolPos.line - 1, character: lastSymbolPos.position - 1}, 100, doc);
    }

    finishPositionTree(doc.positionTree);
	parseLevel--;
}

function findSymbolForPosition(textDocumentPosition: TextDocumentPositionParams): {doc: LLDocument, usage: SymbolUsage}|undefined {
    let docURI = textDocumentPosition.textDocument.uri;
    let doc = documentMap.get(docURI);
    let pos = textDocumentPosition.position;

    function binsearch(arr: SymbolUsage[], val: Position): SymbolUsage|undefined {
        let from: number = 0;
        let to: number = arr.length - 1;

        function compare(a: SymbolUsage, b: Position): number {
            return a.position.line !== b.line? a.position.line - b.line:
                   b.character === a.position.character && a.symbol.length === 0? 0:
                   b.character > a.position.character + a.symbol.length? -1:
                   b.character < a.position.character? 1:
                   0;
        }

        if (from > to) {
            return undefined;
        }
        while (from < to) {
            let i: number = Math.floor((to + from) / 2);
            let res: number = compare(arr[i], val);
            if (res < 0) {
                from = i + 1;
            } else if (res > 0) {
                to = i - 1;
            } else {
                return arr[i];
            }
        }
        return compare(arr[from], val) === 0? arr[from]: undefined;
    }

    if (doc === undefined) {
        connection.console.log(`cannot find position`);
        return undefined;
    } else {
        let usage = binsearch(doc.usedSymbols, pos);
        if (usage !== undefined && isSymbolDefineType(usage.type)) {
            connection.console.log(`found define ${usage.symbol}`);
            return {
                doc: doc,
                usage: new SymbolUsage(usage.symbol, removeSymbolDefineType(usage.type), usage.position)
            };
        } else {
            return usage === undefined? undefined: {
                doc: doc,
                usage: usage
            };
        }
    }
}

function dumpPositionTree(tree: PathPositionTree, indent: string = ""): string {
    let txt: string = "";
    let nextIndent: string = indent + "  ";

    function formatpos(p: Position|undefined): string {
        return p !== undefined? p.line + ":" + p.character: "?";
    }

    for (var i = 0; i < tree.length; i++) {
        var node: PathPositionNode = tree[i];
        txt += indent + node.attribute + " [" + formatpos(node.start) + "," + formatpos(node.avStart) + "," + formatpos(node.end) + "]" + "\n";
        if (node.next !== undefined) {
            txt += dumpPositionTree(node.next, nextIndent);
        }
    }
    return txt;
}

function builtInCompletions(textDocumentPosition: TextDocumentPositionParams): CompletionItem[] {
    return ["SHIFT", "KEYWORD", "SEQUENCE", "CHAIN", "OPTION", "IGNORE"].map(
        (keyword) => {
                return {
                label: keyword,
                kind: SymbolKind.Property,
                data: {
                    symbol: keyword
                }
            }
        }
    );
}

/**
 * Strips the leading comment symbols and spaces from dc
 * 
 * @param {string} dc 
 * @returns {string} 
 */
let docCommentStart = /^##[ \t]*/;
function leftAlign(docComment: string|undefined): string|undefined {
    if (docComment === undefined) {
        return undefined;
    }

    let lines = docComment.split("\n");
    let minLength = lines.map((line) => {
        let matches = line.match(docCommentStart);
        return matches === null? 2: matches[0].length;
    }).reduce((min: number, len: number) => len < min? len: min, 2);

    return lines.map((line: string): string => line.substr(minLength)).join("\n");
}

// LANGUAGE FEATURES

// Finds symbol to be completed and returns all symbols with a corresponding
// definition.
function llCompletionHandler(textDocumentPosition: TextDocumentPositionParams): CompletionItem[] {
    let usage = findSymbolForPosition(textDocumentPosition);
    let compl = builtInCompletions(textDocumentPosition);
    let ci: CompletionItem[] = [];

    if (usage !== undefined) {
        let usageType: SymbolType = usage.usage.type;
        let doc = usage.doc;
        if (doc.definedSymbols !== undefined) {
            doc.definedSymbols.forEach(function(definitions: SymbolDefinition[], symbol: string): void {
                if (symbol !== undefined /*&& symbol.startsWith(usage!.symbol)*/) {
                    for (let i = 0; i < definitions.length; i++) {
                        let def = definitions[i];
                        ci.push({
                            label: symbol,
                            kind: def.type === SymbolType.nonTerminal? SymbolKind.Method: SymbolKind.Constant,
                            data: {
                                symbol: symbol,
                                docComment: leftAlign(def.docComment)
                            }
                        });
                    }
                }
            });
        }
    }
    return ci.concat(compl);
}

function llHoverProvider(textDocumentPosition: TextDocumentPositionParams): Hover {
    let usage = findSymbolForPosition(textDocumentPosition);
    let doc = documentMap.get(textDocumentPosition.textDocument.uri);

	if (doc === undefined || usage === undefined) {
        return { contents: [] };
    }
    let hoverTexts: string[] = [];
    let usageType: SymbolType = usage.usage.type;
    let defMap: Map<string, SymbolDefinition[]> = doc.definedSymbols;
    if (defMap !== undefined && defMap.has(usage.usage.symbol)) {
        let definitions = defMap.get(usage.usage.symbol)!;
        for (let i = 0; i < definitions.length; i++) {
            let def = definitions[i];
            let text: string|undefined;
            switch (def.type) {
                case SymbolType.terminal:
                text = "terminal";
                break;
                case SymbolType.nonTerminal:
                text = "non-terminal";
                break;
            }
            if (def.docComment !== undefined) {
                if (text !== undefined) {
                    text += "\n" + leftAlign(def.docComment);
                } else {
                    text = leftAlign(def.docComment);
                }
            }
            if (text !== undefined) {
                hoverTexts.push(text);
            }
        }
    }
	return { contents: hoverTexts };
}

function llDefinitionProvider(textDocumentPosition: TextDocumentPositionParams): Definition {
    let usage = findSymbolForPosition(textDocumentPosition);
    let doc = documentMap.get(textDocumentPosition.textDocument.uri);

    if (doc === undefined || usage === undefined) {
        return [];
    }
    let symbol: string = usage.usage.symbol;
    let definitionDistance: {definition: SymbolDefinition;}[] = [];
    let defMap: Map<string, SymbolDefinition[]> = doc.definedSymbols;
    if (defMap !== undefined && defMap.has(symbol)) {
        let definitions = defMap.get(usage.usage.symbol)!;
        for (let i = 0; i < definitions.length; i++) {
            let def = definitions[i];
            definitionDistance.push({definition: def});
        }
    }
    return definitionDistance.map((distDef): Location => {
                let def = distDef.definition;
                return Location.create(def.document.uri,
                                       getRange(def.position, symbol.length));
            });
}

function llListAllSymbolsInFile(params: DocumentSymbolParams/*textDocumentIdentifier: TextDocumentIdentifier*/): SymbolInformation[] {
    let doc = documentMap.get(params.textDocument.uri);

    if (doc === undefined) {
        return [];
    }
    let si: SymbolInformation[] = [];
    doc.definedSymbols.forEach(function(defs: SymbolDefinition[], symbol: string): void {
        if (symbol !== undefined && defs !== undefined && defs.length > 0) {
            let def: SymbolDefinition = defs[0];
            si.push(SymbolInformation.create(
                symbol,
                def.type === SymbolType.terminal? SymbolKind.Constant: SymbolKind.Method,
                getRange(def.position, symbol.length)));
        }
    });
    return si;
}

function llReferenceProvider(rp: ReferenceParams): Location[] {
    let includeDeclaration = rp.context.includeDeclaration;
    let usage = findSymbolForPosition(rp);

    if (usage === undefined) {
        return [];
    }
    let doc = usage.doc;
    let locs: Location[] = [];
    if (includeDeclaration) {
        let defMap: Map<string, SymbolDefinition[]> = doc.definedSymbols;
        if (defMap.has(usage!.usage.symbol)) {
            let defs = defMap.get(usage!.usage.symbol)!;
            for (let i = 0; i < defs.length; i++) {
                locs.push(Location.create(doc.uri, defs[i].getRange()));
            }
        }
    }
    let symbolUsage = doc.usedSymbols;
    for (let i = 0; i < symbolUsage.length; i++) {
        let docUsage = symbolUsage[i];
        if (docUsage.symbol === usage.usage.symbol) {
            locs.push(Location.create(doc.uri, docUsage.getRange()));
        }
    }
    return locs;
}

// Fake reference for the source that includes open documents
let editorURI: string = "editor://";
let editorDocument: LLDocument = new LLDocument(editorURI);

function closeDocURI(params: DidCloseTextDocumentParams): void {
    let doc = documentMap.get(params.textDocument.uri);

    if (doc !== undefined) {
        doc.close(connection);
        documentMap.delete(params.textDocument.uri);
    }
}

// INTERFACE TO VSCODE

// Create a connection for the server. The connection uses Node's IPC as a transport
let connection: IConnection = createConnection(new IPCMessageReader(process), new IPCMessageWriter(process));

// After the server has started the client sends an initilize request. The server receives
// in the passed params the rootPath of the workspace plus the client capabilites. 
let workspaceRoot: string|null;

// Create a simple text document manager. The text document manager
// supports full document sync only
let documents: TextDocuments = new TextDocuments();
// Make the text document manager listen on the connection
// for open, change and close text document events
documents.listen(connection);

connection.onInitialize((params: InitializeParams): InitializeResult => {
    // connection.console.log("Initializing server");
    workspaceRoot = params.rootUri;
    setDebugLogDir(workspaceRoot, false);
    // debugLog("starting " + workspaceRoot);
    connection.console.log(`server starting: ${process.pid}`); // !!!
    if (workspaceRoot !== null) {
        processConfigFile(workspaceRoot);
    }
    // debugLog("read config");
    connection.console.log("server read config"); // !!!
    return {
        capabilities: {
            // Tell the client that the server works in FULL text document sync mode
            textDocumentSync: documents.syncKind,
            // Tell the client that the server support code complete
            completionProvider: {
                resolveProvider: true
            },
            definitionProvider: true,
            hoverProvider: true,
            documentSymbolProvider: true,
            referencesProvider: true,
            workspaceSymbolProvider: true
            // signatureHelpProvider?: SignatureHelpOptions;
            // documentHighlightProvider?: boolean;
            // codeActionProvider?: boolean;
            // codeLensProvider?: CodeLensOptions;
            // documentFormattingProvider?: boolean;
            // documentRangeFormattingProvider?: boolean;
            // documentOnTypeFormattingProvider?: DocumentOnTypeFormattingOptions;
            // renameProvider?: boolean;
        }
    }
});

// The content of a text document has changed. This event is emitted
// when the text document first opened or when its content has changed.
documents.onDidChangeContent((change) => {
    let doc = documentMap.get(change.document.uri);
    let type: ProcessQueueItemType = ProcessQueueItemType.ll;

    if (doc === undefined) {
        doc = new LLDocument(change.document.uri);
        documentMap.set(change.document.uri, doc);
    }
    addToEndOfProcessingQueue(doc, change.document.getText(), type, true);
});

// The settings interface describe the server relevant settings part
interface Settings {
    llLanguageServer: LLGenLanguageServerSettings;
}

// The settings have changed. Is send on server activation as well.
connection.onDidChangeConfiguration((change) => {
    if (change === undefined) {
        return;
    }
    let settings = <Settings> change.settings;
    if (updateConfiguration(settings.llLanguageServer)) {
        // Revalidate any open text documents immediately
        documents.all().forEach(function(document: TextDocument): void {
            let doc = documentMap.get(document.uri);
            let type: ProcessQueueItemType = ProcessQueueItemType.ll;
            if (doc !== undefined) {
                addToEndOfProcessingQueue(doc, document.getText(), type, true);
            }
        });
    }
});

/*
connection.onDidOpenTextDocument((params) => {
    // A text document got opened in VSCode.
    // params.uri uniquely identifies the document. For documents store on disk this is a file URI.
    // params.text the initial full content of the document.
    connection.console.log(`${params.uri} opened.`);
});

connection.onDidChangeTextDocument((params) => {
    // The content of a text document did change in VSCode.
    // params.uri uniquely identifies the document.
    // params.contentChanges describe the content changes to the document.
    connection.console.log(`${params.uri} changed: ${JSON.stringify(params.contentChanges)}`);
});

connection.onDidChangeWatchedFiles((change) => {
    // Monitored files have change in VSCode
    connection.console.log('We received an file change event');
});
*/

// This handler provides the initial list of the completion items.
connection.onCompletion(llCompletionHandler);

// This handler resolve additional information for the item selected in
// the completion list.
connection.onCompletionResolve((item: CompletionItem): CompletionItem => {
    item.detail = item.data.label;
    item.documentation = item.data.docComment;
    return item;
});

// List of all definitions (including docComment) of the symbol at the given position
connection.onHover(llHoverProvider);

// List of locations of the symbol at the given position sorted by inheritance
// distance.
connection.onDefinition(llDefinitionProvider);

// List of all definitions in a file
connection.onDocumentSymbol(llListAllSymbolsInFile);

connection.onReferences(llReferenceProvider);

connection.onDidCloseTextDocument(closeDocURI);

// Listen on the connection
connection.listen();
