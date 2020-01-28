//
//  swordx2
//  by Giacomo Menchi
//

#ifndef bst_h
#define bst_h

#include <wchar.h>

typedef struct node {
    wchar_t occurrence[50];
    int wordcounter;
    struct node *left;
    struct node *right;
} Node;

void findFirstOccurrence(FILE *fileout, Node *root, short alpha);
Node* getMostFrequent(void);
Node* insert(Node *root, wchar_t occurrence[50]);
Node* search(Node *root, wchar_t occurrence[50]);
void visitInorder(FILE *fileout, Node *root, short alpha);

#endif /* bst_h */
