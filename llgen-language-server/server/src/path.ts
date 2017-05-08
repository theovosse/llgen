import {
    Position
} from 'vscode-languageserver';

/** Describes which character range a certain attribute spans */
export class PathPositionNode {
    /** The dominating attribute at this range of the document */
    attribute: string;
    /** Start of the range under attribute */
    start: Position;
    /** End of the range under attribute; when undefined, the tree has not been closed (yet) */
    end: Position|undefined;
    /** Position where the AV starts; if undefined, there is no lower path */
    avStart: Position|undefined;
    /** The next level of attributes; undefined when there is none; [] when it's empty */
    next: PathPositionTree|undefined;
}

/** A tree is a series of non-overlapping nodes, ordered by start position */
export type PathPositionTree = PathPositionNode[];

/**
 * Create a node in the path tree, assuming everything is added to the final
 * node.
 * 
 * @param {PathPositionTree} tree root of the document's position tree
 * @param {string[]} path path to the current node
 * @returns {PathPositionNode} the added node or undefined in case of error
 */
export function createPathPositionNode(tree: PathPositionTree, path: string[], startPosition: Position): PathPositionNode {
    let ptr: PathPositionTree = tree;
    let node: PathPositionNode;

    for (var i = 0; i < path.length - 1; i++) {
        if (ptr.length === 0) {
            // Happens when adding a path whose prefix hasn't been added
            node = {
                attribute: path[i],
                start: startPosition,
                end: undefined,
                avStart: undefined,
                next: []
            };
            ptr.push(node);
            ptr = node.next!; // Has just been set to a defined value
        } else {
            node = ptr[ptr.length - 1];
            if (node.next === undefined) {
                node.next = [];
            } 
            ptr = ptr[ptr.length - 1].next!; // Has just been set to a defined value
        }
    }
    if (ptr.length !== 0 && ptr[ptr.length - 1].end === undefined) {
        return ptr[ptr.length - 1];
    } else {
        node = {
            attribute: path[path.length - 1],
            start: startPosition,
            end: undefined,
            avStart: undefined,
            next: undefined
        };
        ptr.push(node);
        return node;
    }
}

/** Find last node in the path tree at the depth specified by the path */
export function findPathPositionNode(tree: PathPositionTree|undefined, path: string[]): PathPositionNode|undefined {
    let ptr: PathPositionTree|undefined = tree;

    for (var i = 0; ptr !== undefined && ptr.length !== 0 && i < path.length - 1; i++) {
        ptr = ptr[ptr.length - 1].next;
    }
    return ptr !== undefined && ptr.length !== 0? ptr[ptr.length - 1]: undefined;
}

/** Close unclosed top nodes (happens because of the artificial adding of an extra
    level under screenArea). Cannot close terminal attributes, but they cannot
    be left open anyway. */
export function finishPositionTree(tree: PathPositionTree): void {
    for (var i = 0; i < tree.length; i++) {
        var node = tree[i];
        if (node.next !== undefined) {
            if (node.start === undefined || node.end === undefined) {
                finishPositionTree(node.next);
            }
            if (node.start === undefined && node.next.length > 0) {
                node.start = node.next[0].start;
            }
            if (node.end === undefined && node.next.length > 0) {
                node.end = node.next[node.next.length - 1].end;
            }
        }
    }
}

export function positionCompare(a: Position, b: Position|undefined): number {
    return b === undefined? -1:
           a.line !== b.line? a.line - b.line:
           a.character - b.character; 
}

function inRange(pos: Position, start: Position, end: Position|undefined): boolean {
    return positionCompare(start, pos) <= 0 && positionCompare(pos, end) <= 0;
}

export function findPathToPosition(tree: PathPositionTree|undefined, position: Position, parent: PathPositionNode): [PathPositionNode|undefined, string[]|undefined, PathPositionNode|undefined] {
    if (tree !== undefined) {
        for (var i = 0; i < tree.length; i++) {
            var node = tree[i];
            if (inRange(position, node.start, node.end)) {
                let [lowestNode, restPath, parentOfLowestNode] = findPathToPosition(node.next, position, node);
                return restPath === undefined? [node, [node.attribute], parent]:
                       [lowestNode, [node.attribute].concat(restPath), parentOfLowestNode];
            }
        }
    }
    return [undefined, undefined, undefined];
}
