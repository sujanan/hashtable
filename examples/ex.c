#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "../hashtable.h"

#define ARRAYLEN(a) (sizeof(a)/sizeof(a[0]))

static inline int eqstrkey(void *s1, void *s2) {
    return !strcmp((const char *)s1, (const char *)s2);
}

static unsigned int djb2(unsigned char *s) {
    unsigned long h = 5381;
    int c;
    for (int i = 0; c = s[i]; i++)
        h = ((h << 5) + h) + c;
    return h & INT_MAX;
}

static void randstr(char *s, int size) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    s[size - 1] = '\0';
    for (int i = 0; i < size - 1; i++)
        s[i] = charset[(int)((double)rand() / RAND_MAX * (sizeof(charset) - 1))]; 
}

static void printtab(struct hashtable *h) {
    struct hashtableIter iter;
    hashtableIterInit(&iter, h);
    struct hashtableEntry *e;
    for (int i = 0; e = hashtableIterNext(&iter); i++)
        printf("%d: {%s, %s}\n", i, (char *)e->key.data, (char *)e->val);
}

int main(int argc, char **argv) {
    struct hashtable h;
    hashtableInit(&h, 11, 0.75, eqstrkey);
    char keys[10][8];
    char vals[10][4];
    for (int i = 0; i < ARRAYLEN(keys); i++) {
        randstr(keys[i], ARRAYLEN(keys[i]));
        randstr(vals[i], ARRAYLEN(vals[i]));
        struct hashtableKey k = {.data = keys[i], .hash = djb2(keys[i])};
        hashtablePut(&h, &k, vals[i]);
    }
    for (int i = 0; i < ARRAYLEN(keys)/2; i++) {
        struct hashtableKey k = {.data = keys[i], .hash = djb2(keys[i])};
        hashtableRemove(&h, &k);
    }
    printtab(&h);
    hashtableFree(&h);
    return 0;
}
