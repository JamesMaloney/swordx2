//
//  swordx2
//  by Giacomo Menchi
//

#ifndef bst_h
#define bst_h

#include <stdbool.h>
#include <wchar.h>

typedef struct node {
    wchar_t occurrence[50];
    int wordcounter;
    struct node *left;
    struct node *right;
} Node;

void findFirstOccurrence(FILE *fileout, Node *root, bool alpha);
Node* getmostfrequent(void);
Node* insert(Node *root, wchar_t occurrence[50]);
Node* search(Node *root, wchar_t occurrence[50]);
void visitInorder(FILE *fileout, Node *root, bool alpha);

#endif /* bst_h */
