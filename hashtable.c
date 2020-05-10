#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "hashtable.h"

#define MIN(a, b) (((a) < (b)) ? a : b)

#define TABLE_MAX_CAP INT_MAX

#define keyEq(k1, k2, eq) \
    ((k1)->hash == (k2)->hash && eq((k1)->data, (k2)->data))

#define indexFor(hash, len) \
    (((hash) & TABLE_MAX_CAP) % len)

#define thresholdFor(cap, loadfac) \
    MIN((int)((cap) * (loadfac)), TABLE_MAX_CAP)

static inline int eqkeyDefault(void *k1, void *k2) {
    return k1 == k2;
}

struct hashtableEntry *hashtableIterNext(struct hashtableIter *iter) {
    while (!iter->curr && iter->pos > 0) 
        iter->curr = iter->tab[--iter->pos];
    struct hashtableEntry *e = iter->curr;
    if (iter->curr)
        iter->curr = iter->curr->next;
    return e;
}

static void _hashtableRehash(struct hashtable *h) {
    int newcap;
    if (h->cap == TABLE_MAX_CAP) 
        return;
    if (h->cap > (TABLE_MAX_CAP >> 1))
        newcap = TABLE_MAX_CAP;
    else
        newcap = h->cap << 1;
    
    struct hashtableEntry **newtab =
        malloc(sizeof(struct hashtableEntry *) * newcap);
    if (!newtab)
        return;
    memset(newtab, 0, sizeof(struct hashtableEntry *) * newcap);

    for (int i = h->cap - 1; i >= 0; i--) {
        struct hashtableEntry *old = h->tab[i];
        while (old) {
            struct hashtableEntry *e = old;
            old = old->next;
            int index = indexFor(e->key.hash, newcap);
            e->next = newtab[index];
            newtab[index] = e;
        }
    }
    free(h->tab);
    h->tab = newtab;
    h->cap = newcap;
    h->threshold = thresholdFor(h->cap, h->loadfac);
}

static struct hashtableEntry *_hashtableEntryNew(struct hashtableKey *key,
                                                 void *val,
                                                 struct hashtableEntry *next) {
    struct hashtableEntry *e = malloc(sizeof(struct hashtableEntry));
    if (!e)
        return NULL;
    e->key = *key;
    e->val = val;
    e->next = next;
    return e;
}

void hashtableInit(struct hashtable *h,
                   int cap,
                   float loadfac,
                   keyeqFn keyeq) {
    h->count = 0;
    h->cap = MIN(cap, TABLE_MAX_CAP);
    h->threshold = thresholdFor(h->cap, loadfac);
    h->loadfac = loadfac;
    h->keyeq = (!keyeq) ? eqkeyDefault : keyeq;

    h->tab = malloc(sizeof(struct hashtableEntry *) * h->cap);
    if (!h->tab)
        return;
    memset(h->tab, 0, sizeof(struct hashtableEntry *) * h->cap);
}

void hashtableFree(struct hashtable *h) {
    if (!h->tab)  
        return;
    struct hashtableIter iter;
    hashtableIterInit(&iter, h);
    struct hashtableEntry *e = hashtableIterNext(&iter);
    while (e) {
        struct hashtableEntry *del = e;
        e = hashtableIterNext(&iter);
        free(del);
    }
    free(h->tab);
    h->cap = 0;
    h->count = 0;
}

void *hashtablePut(struct hashtable *h, struct hashtableKey *key, void *val) {
    int index = indexFor(key->hash, h->cap);
    struct hashtableEntry *e = h->tab[index];
    while (e) {
        if (keyEq(&e->key, key, h->keyeq)) {
            void *old = e->val;
            e->val = val;
            return old;
        }
        e = e->next;
    }

    if (h->count >= h->threshold)
        _hashtableRehash(h);
    h->tab[index] = _hashtableEntryNew(key, val, h->tab[index]);
    if (h->tab[index])
        h->count++;
    return NULL;
}

void *hashtableGet(struct hashtable *h, struct hashtableKey *key) {
    struct hashtableEntry *e = h->tab[indexFor(key->hash, h->cap)];
    while (e) {
        if (keyEq(&e->key, key, h->keyeq))
            return e->val;
        e = e->next;
    }
    return NULL;
}

void *hashtableRemove(struct hashtable *h, struct hashtableKey *key) {
    int index = indexFor(key->hash, h->cap);
    struct hashtableEntry *prev = NULL;
    struct hashtableEntry *curr = h->tab[index];
    while (curr) {
        if (keyEq(&curr->key, key, h->keyeq)) {
            if (prev)
                prev->next = curr->next;
            else
                h->tab[index] = curr->next;
            h->count--;
            void *old = curr->val;
            free(curr);
            return old;
        }
        prev = curr;
        curr = curr->next; 
    }
    return NULL; 
}

