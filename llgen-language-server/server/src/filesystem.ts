import * as fs from 'fs';
import * as url from 'url';

export function getBaseDirectoryFromURI(uri: string): string|undefined {
    let components = url.parse(uri);

    if (components.protocol === "file:" && components.host !== undefined && components.path !== undefined) {
        let fileName = decodeURI(components.host + components.path);
        let lastSlash: number = fileName.lastIndexOf('/');
        return lastSlash >= 0? fileName.slice(0, lastSlash): undefined; 
    }
    return undefined;
}

// Returns the concatenation of two paths, and shortens the path when p2 starts
// with ../
export function concatenatePaths(p1: string, p2: string): string {
    let p1Pos: number = p1.length;
    let p2Pos: number = 0;

    if (p2.charAt(0) === '/') {
        return p2;
    }
    if (p1.charAt(p1Pos - 1) === '/') {
        p1Pos--;
    }
    while (true) {
        if (p2.startsWith("../", p2Pos)) {
            p2Pos += 3;
            p1Pos = p1.lastIndexOf("/", p1Pos - 1);
            if (p1Pos === -1) {
                p2Pos = 0;
                break;
            }
        } else if (p2.startsWith("..", p2Pos) && p2.length === p2Pos + 2) {
            p2Pos += 2;
            p1Pos = p1.lastIndexOf("/", p1Pos - 1);
            if (p1Pos === -1) {
                p2Pos = 0;
                break;
            }
        } else if (p2.startsWith("./", p2Pos)) {
            p2Pos += 2;
        } else if (p2.startsWith(".", p2Pos) && p2.length === p2Pos + 1) {
            p2Pos += 1;
        } else {
            break;
        }
    }
    return (p1Pos === p1.length? p1 + "/": p1.slice(0, p1Pos + 1)) +
           (p2Pos === 0? p2: p2.slice(p2Pos));
}

export function makeRelativePathName(uri: string, name: string): string[] {
    let baseDirectory = getBaseDirectoryFromURI(uri);

    return baseDirectory !== undefined? [concatenatePaths(baseDirectory, name)]: []; 
}

/** The directories under which files included with <...> can be found */
export let includeFilePaths: string[] = [];

export function initIncludeFilePaths(uri: string): void {
    // Not needed
}

export function makeAbsolutePathName(name: string): string[] {
    return includeFilePaths.map(function(inclFilePath: string): string {
        return concatenatePaths(inclFilePath, name);
    });
}

export function makeURI(fileName: string): string {
    return encodeURI("file://" + fileName);
}

export let nrOpenReadRequests: number = 0;

export function readFile(uri: string, cb: (err: any, data: string) => void) {
    nrOpenReadRequests++;
    fs.readFile(uri, "utf8", function(err: any, data: string): void {
        nrOpenReadRequests--;
        cb(err, data);
    });
}

export function clearIncludes(): void {
    includeFilePaths = [];
}

export function addIncludePath(path: string): void {
    includeFilePaths.push(path);
}

export function fileExists(fileName: string): boolean {
    return fs.existsSync(fileName);
}

/**
 * name of the debug log file; not that if the debug log dir hasn't been set,
 * it's created in the server process' directory.
 */
let debugLogFileName: string = "debug.log";
let writeModeAppend: boolean = false;

/**
 * Sets the directory for the debug log file. The name consists of "debug.log."
 * followed by the date of creation (in ISO format). 
 * 
 * @export
 * @param {string} dir base directory where the log file is placed
 */
export function setDebugLogDir(dir: string|null, append: boolean): void {
    if (dir !== null) {
        debugLogFileName = dir + "/debug.log." + (new Date()).toISOString();
        writeModeAppend = append;
    }
}

/**
 * Writes a log message to the log file. If the directory has not been set
 * using {@link setDebugLogDir}, writes to "debug.log" in some unspecified
 * place.
 * 
 * @export
 * @param {string} str log message
 */
export function debugLog(str: string): void {
    fs.writeFileSync(debugLogFileName, str + "\n", writeModeAppend? {flag: "a"}: undefined);
    writeModeAppend = true;
}
