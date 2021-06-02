#include <stdio.h>
#include <stdlib.h>

typedef struct elem {
    int key;
    struct elem *next;
} elem;

int funhash() {
    return 5;
}

int main(void) {

    int size = 100;

    elem **hashTable = (elem **) malloc (sizeof(2 * size * sizeof(elem *)));
    for (int i = 0; i < 2 * size; i++) {
        hashTable[i] = NULL;
    }

    int x = 5;

    int risprov = funhash();
    if (hashTable[risprov] == NULL) {
        elem * new = (elem *) malloc(sizeof(elem));
        new->key = x;
        new->next = NULL;
        hashTable[risprov] = new;
    } else {
        elem * new = (elem *) malloc(sizeof(elem));
        new->key = x;
        new->next = hashTable[risprov];
        hashTable[risprov] = new;
    }

    return 0;
}