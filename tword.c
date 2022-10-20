#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "shareddefs.h"
#include <pthread.h>
#include <time.h>
#include <sys/time.h>

struct Node {
    struct Node* leftPtr;
    struct Node* rightPtr;
    char* word;
    int count;
};

struct thread_arg {
    int threadCounter;
    char* filename;
};

struct Node* insertWord(struct Node* root, char* word){
    struct Node* newNode;
    if ( root == NULL ) {
        newNode = (struct Node*) malloc(sizeof(struct Node));
        newNode->leftPtr = NULL;
        newNode->rightPtr = NULL;
        newNode->word = word;
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
            free(word);
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
        }
    }
    return root;
}

void printTree(struct Node* root, FILE* outputFile, int* count) {
    if ( root == NULL ) {
        return;
    }
    printTree(root->leftPtr, outputFile, count);
    if ( (*count) == 0 ) {
        fprintf(outputFile, "%s %d", root->word, root->count);
    }
    else {
        fprintf(outputFile, "\n%s %d", root->word, root->count);
    }
    (*count)++;
    printTree(root->rightPtr, outputFile, count);
}

void printTreeConsole(struct Node* root) {
    if ( root == NULL ) {
        return;
    }
    printTreeConsole(root->leftPtr);
    printf("%s %d\n", root->word, root->count);
    printTreeConsole(root->rightPtr);
}

void deallocate(struct Node* root) {
    if ( root == NULL ) {
        return;
    }
    deallocate(root->leftPtr);
    deallocate(root->rightPtr);
    free(root);
}

void deallocateWithWord(struct Node* root) {
    if ( root == NULL ) {
        return;
    }
    deallocateWithWord(root->leftPtr);
    deallocateWithWord(root->rightPtr);
    free(root->word);
    free(root);
}

void* parseFile(void* arguments);
struct Node** head = NULL;
struct Node* parentHead = NULL;

void traverse(struct Node** parentHead, struct Node* root) {
    if ( root == NULL ) {
        return;
    }
    traverse(parentHead, root->leftPtr);
    *parentHead = insertWithCount(*parentHead, root->word, root->count);
    traverse(parentHead, root->rightPtr);
}

int main(int argc, char* argv[]) {

    if ( argc < 4 ) {
        printf("You have entered insufficient number of arguments! Usage: pword <msgsize> <outfile> <N> <infile1> .... <infileN>\n");
        return -1;
    }
    char* outfile = argv[1];

    int n = atoi(argv[2]);
    char* fileNames[n];
    for ( int i = 0; i < n; i++ ) {
        fileNames[i] = argv[i + 3];
    }

    // start the timer
    time_t t;
    srand((unsigned) time(&t));

    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    long before = currTime.tv_usec;

    head = malloc(sizeof(struct Node*) * n);
    struct thread_arg arguments[n];
    pthread_t thread_ids[n];

    for ( int i = 0; i < n; i++ ) {
        arguments[i].threadCounter = i;   
        arguments[i].filename = fileNames[i];
        pthread_create(&thread_ids[i], NULL, &parseFile, (void*) &arguments[i]);
    }

    for ( int i = 0; i < n; i++ ) {
        pthread_join(thread_ids[i], NULL);
    }
    
    for ( int i = 0; i < n; i++ ) {
        traverse(&parentHead, head[i]);
    }

    FILE* outputFile;
    outputFile = fopen(outfile, "w");
    int num = 0;

    printTree(parentHead, outputFile, &num);

    deallocate(parentHead);
    fclose(outputFile);

    for ( int i = 0; i < n; i++ ) {
        deallocateWithWord(head[i]);
    }
    free(head);

    gettimeofday(&currTime, NULL);
    long after= currTime.tv_usec;
    printf("\nThe execution took: %ld ms \n", after - before);
}

void* parseFile(void* arguments) {
    struct thread_arg* args = (struct thread_arg*) arguments;
    char* fileName = (*args).filename;
    int threadCounter = (*args).threadCounter;
    FILE* file;
    char word[64];
    file = fopen(fileName, "r");
    head[threadCounter] = NULL;

    while ( fscanf(file, "%s", word) == 1 ) {

        char* current = strdup(word);
        
        for ( int i = 0; current[i] != '\0'; i++ ) {
            if ( current[i] >= 'a' && current[i] <= 'z' ) {
                current[i] = current[i] - 32;
            }
        }
        
        head[threadCounter] = insertWord(head[threadCounter], current);
    }

    fclose(file);
    pthread_exit(0);
}