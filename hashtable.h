#ifndef HASHTABLE_H
#define HASHTABLE_H

typedef int (*keyeqFn)(void *k1, void *k2);

struct hashtableKey {
    void *data;
    unsigned int hash;
};

struct hashtableEntry {
    void *val;
    struct hashtableKey key;
    struct hashtableEntry *next;
};

struct hashtable {
    struct hashtableEntry **tab;
    int cap;
    int count;
    float loadfac;
    int threshold;
    keyeqFn keyeq;
};

struct hashtableIter {
    int pos;
    struct hashtableEntry **tab;
    struct hashtableEntry *curr;
};

static inline void hashtableIterInit(struct hashtableIter *iter,
                                     struct hashtable *h) {
    iter->tab = h->tab; 
    iter->pos = h->cap; 
    iter->curr = NULL;
}
struct hashtableEntry *hashtableIterNext(struct hashtableIter *iter);

void hashtableInit(struct hashtable *h,
        int cap, float loadfac, keyeqFn keyeq);
void hashtableFree(struct hashtable *h);
void *hashtablePut(struct hashtable *h,
        struct hashtableKey *key, void *val);
void *hashtableGet(struct hashtable *h, struct hashtableKey *key);
void *hashtableRemove(struct hashtable *h, struct hashtableKey *key);
 
#endif
