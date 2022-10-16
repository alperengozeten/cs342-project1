#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "shareddefs.h"
#include <pthread.h>

struct Node {
    struct Node* leftPtr;
    struct Node* rightPtr;
    char* word;
    int count;
};

struct Node* insertWord(struct Node* root, char* word){
    struct Node* newNode;
    if ( root == NULL ) {
        newNode = (struct Node*) malloc(sizeof(struct Node));
        newNode->leftPtr = NULL;
        newNode->rightPtr = NULL;
        newNode->word = word;
        printf("%s", word);
        newNode->count = 1;

        return newNode;
    }
    else {
        if ( strcmp(word, root->word) < 0 ) {
            root->leftPtr = insertWord(root->leftPtr, word);
        }
        else if ( strcmp(word, root->word) > 0) {
            root->rightPtr = insertWord(root->rightPtr, word);
        }
        else {
            root->count = root->count + 1;
        }
    }
    return root;
}

struct Node* insertWithCount(struct Node* root, char* word, int count){
    struct Node* newNode;
    if ( root == NULL ) {
        newNode = (struct Node*) malloc(sizeof(struct Node));
        newNode->leftPtr = NULL;
        newNode->rightPtr = NULL;
        newNode->word = word;
        printf("%s", word);
        newNode->count = count;

        return newNode;
    }
    else {
        if ( strcmp(word, root->word) < 0 ) {
            root->leftPtr = insertWithCount(root->leftPtr, word, count);
        }
        else if ( strcmp(word, root->word) > 0) {
            root->rightPtr = insertWithCount(root->rightPtr, word, count);
        }
        else {
            root->count = root->count + count;
            free(word);
        }
    }
    return root;
}

void printTree(struct Node* root, FILE* outputFile, int* count) {
    if ( root == NULL ) {
        return;
    }
    *count++;
    printTree(root->leftPtr, outputFile, count);
    if ( *count == 1 ) {
        fprintf(outputFile, "%s %d", root->word, root->count);
    }
    else {
        fprintf(outputFile, "\n%s %d", root->word, root->count);
    }
    printTree(root->rightPtr, outputFile, count);
}

void deallocate(struct Node* root) {
    if ( root == NULL ) {
        return;
    }
    deallocate(root->leftPtr);
    deallocate(root->rightPtr);
    free(root->word);
    free(root);
}

void parseFile(char* fileName, int threadCounter);

int main(int argc, char* argv[]) {

    int pos_msg_size[] = {128, 256, 512, 1024, 2048, 4096};
    if ( argc < 5 ) {
        printf("You have entered insufficient number of arguments! Usage: pword <msgsize> <outfile> <N> <infile1> .... <infileN>\n");
        return -1;
    }
    int msg_size = atoi(argv[1]);
    char* outfile = argv[2];

    // message size should be appropriate
    int found = 0;
    for ( int i = 0; i < 6; i++ ) {
        if ( pos_msg_size[i] == msg_size ) {
            found = 1;
            break;
        }
    }
    if ( found == 0 ) {
        printf("You have entered invalid message size in bytes. Please enter one of the numbers 128, 256, 512, 1024, 2048, 4096.\n");
        return -1;
    }

    int n = atoi(argv[3]);
    char* fileNames[n];
    for ( int i = 0; i < n; i++ ) {
        fileNames[i] = argv[i + 4];
    }




}

struct Node** head = NULL;


void parseFile(char* fileName, int threadCounter) {
    FILE* file;
    char word[64];
    file = fopen(fileName, "r");
    struct Node* currentHead = head[threadCounter];

    while ( fscanf(file, "%s", word) == 1 ) {

        char* current = strdup(word);
        
        for ( int i = 0; current[i] != '\0'; i++ ) {
            if ( current[i] >= 'a' && current[i] <= 'z' ) {
                current[i] = current[i] - 32;
            }
        }
        
        head = insertWord(head, current);
    }
}