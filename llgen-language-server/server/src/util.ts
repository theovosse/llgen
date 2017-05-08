export interface Equal {
    isEqual(v: Equal): boolean;
}

export class EArray<T extends Equal> extends Array<T> implements Equal {
    isEqual(v: EArray<T>): boolean {
        if (v === undefined) {
            return this.length === 0;
        }
        if (this.length === v.length) {
            for (var i = 0; i < this.length; i++) {
                if (!this[i].isEqual(v[i])) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }
}

export class ESet<T> extends Set<T> implements Equal {
    isEqual(v: ESet<T>): boolean {
        if (v === undefined) {
            return this.size === 0;
        }
        if (this.size === v.size) {
            for (let e of this) {
                if (!v.has(e)) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }
}

export class EMap<K, V extends Equal> extends Map<K, V> implements Equal {
    isEqual(p: EMap<K, V>): boolean {
        if (p === undefined) {
            return this.size === 0;
        }
        if (this.size === p.size) {
            for (let [k, v] of this) {
                if (!p.has(k) || !v.isEqual(p.get(k)!)) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    hasEqualKeys(p: Map<K, V>): boolean {
        if (p === undefined) {
            return this.size === 0;
        }
        if (this.size === p.size) {
            for (let k of this.keys()) {
                if (!p.has(k)) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }

    getKeys(): K[] {
        var keys: K[] = [];

        for (let key of this.keys()) {
            keys.push(key);
        }
        return keys;
    }
}

/** Like EMap, but identical when keys are identical */
export class EKMap<K, V> extends Map<K, V> implements Equal {
    isEqual(p: EKMap<K, V>): boolean {
        if (p === undefined) {
            return this.size === 0;
        }
        if (this.size === p.size) {
            for (let k of this.keys()) {
                if (!p.has(k)) {
                    return false;
                }
            }
            return true;
        } else {
            return false;
        }
    }
}
