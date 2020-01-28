//
//  swordx2
//  by Giacomo Menchi
//

#include <stdlib.h>
#include "bst.h"
#include "swordx2.h"

Node *mostfrequent = NULL;

Node* insert(Node *root, wchar_t occurrence[50]) {
    if(!root) {
        root = malloc(sizeof(Node));
        wcscpy(root->occurrence, occurrence);
        root->wordcounter = 1;
        addRegisteredWord();
        root->left = NULL;
        root->right = NULL;
        return root;
    }
    if(wcscasecmp(root->occurrence, occurrence) > 0) {
        root->left = insert(root->left, occurrence);
    } else if(wcscasecmp(root->occurrence, occurrence) < 0) {
        root->right = insert(root->right, occurrence);
    } else {
        root->wordcounter++;
    }
    return root;
}

Node* search(Node *root, wchar_t occurrence[50]) {
    if(!root || wcscasecmp(root->occurrence, occurrence) == 0) {
        return root;
    } else if(wcscasecmp(root->occurrence, occurrence) > 0) {
        return search(root->left, occurrence);
    }
    return search(root->right, occurrence);
}

void visitInorder(FILE *fileout, Node *root, short alpha) {
    if(!root) {
        return;
    }
    visitInorder(fileout, root->left, alpha);
    if((!alpha) || (alpha && isAlphaOnly(root->occurrence))) {
        fprintf(fileout, "%ls %d\n", root->occurrence, root->wordcounter);
    }
    visitInorder(fileout, root->right, alpha);
}

void findFirstOccurrence(FILE *fileout, Node *root, short alpha) {
    if(mostfrequent) {
        if(mostfrequent->wordcounter == -1) {
            mostfrequent = NULL;
        }
    }
    if(root) {
        if(root->wordcounter != -1) {
            if((!alpha) || (alpha && isAlphaOnly(root->occurrence))) {
                if(!mostfrequent) {
                    mostfrequent = root;
                }
                if((mostfrequent->wordcounter < root->wordcounter) || ((mostfrequent->wordcounter == root->wordcounter) && (wcscasecmp(mostfrequent->occurrence, root->occurrence) > 0))) {
                    mostfrequent = root;
                }
            }
        }
        findFirstOccurrence(fileout, root->left, alpha);
        findFirstOccurrence(fileout, root->right, alpha);
    }
}

Node* getMostFrequent(void) {
    return mostfrequent;
}
