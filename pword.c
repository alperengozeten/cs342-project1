#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "shareddefs.h"
#include <mqueue.h>


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
        }
    }
    return root;
}

void printTree(struct Node* root) {
    if ( root == NULL ) {
        return;
    }
    printTree(root->leftPtr);
    printf("%s %d\n", root->word, root->count);
    printTree(root->rightPtr);
}

struct item* traverse(struct Node* root, mqd_t* mqPtr, int msg_size, struct item* itemPtr, int* remainingSpace, int* pairCount) {
    if ( root == NULL ) {
        return itemPtr;
    }
    traverse(root->leftPtr, mqPtr, msg_size, itemPtr, remainingSpace, pairCount);
    printf("%s %d\n", root->word, root->count);

    char* word = root->word;
    int freq = root->count;

    if ( strlen(word) + 5 <= *remainingSpace ) {
        char* ptr = &(*itemPtr).astr[msg_size - *remainingSpace];

        for ( int i = 0; word[i] != '\0'; i++ ) {
            *ptr = word[i];
            ptr++;
        }

        // put mark at the end
        *ptr = '\0';
        ptr++;

        // write int
        int* pointer = (int*) ptr;
        *pointer = freq;

        *remainingSpace = *remainingSpace- strlen(word) - 5;
        *pairCount = *pairCount + 1;
        return traverse(root->rightPtr, mqPtr, msg_size, itemPtr, remainingSpace, pairCount);
    }
    else {
        int* ptr = (int*) &(*itemPtr).astr[0];
        *ptr = *pairCount;

        // send a message
        int n;
        n = mq_send(*mqPtr, (char*) itemPtr, sizeof(struct item) + sizeof(char[msg_size]), 0);

        if ( n == -1 ) {
            printf("mq_send failed\n");
        }
        else {
            printf("mq_send success\n");
        }

        free(itemPtr);

        //struct item item;
        struct item* newItemPtr = malloc(sizeof(struct item) + sizeof(char[msg_size]));
        newItemPtr->msg_size = msg_size;

        *remainingSpace = msg_size - strlen(word) - 9;
        *pairCount = 1;
        return traverse(root->rightPtr, mqPtr, msg_size, newItemPtr, remainingSpace, pairCount);
    }
}
void parseFile(char* fileName, int msg_size);
struct Node* parentHead = NULL;

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

    // create a message queue
    mqd_t mq; 
    mq = mq_open(MQNAME, O_RDWR | O_CREAT, 0666, NULL);

    if ( mq == -1 ) {
        printf("can not create message queue\n");
        exit(1);
    }

    
    pid_t f;
    for ( int i = 0; i < n; i++ ) {
        
        f = fork();
        if ( f == 0 ) {
            parseFile(fileNames[i], msg_size);
            exit(0);
        }
    }

    struct mq_attr mq_attr;
    struct item* itemptr;
    int num;
    int buflen;
    char* bufptr;
    int terminalCount = 0;

    mq_getattr(mq, &mq_attr);
    buflen = mq_attr.mq_msgsize;
    bufptr = (char*) malloc(buflen);

    while ( terminalCount != n ) {
        
        num = mq_receive(mq, (char*) bufptr, buflen, NULL);
        if ( num == - 1 ) {
            printf("mq_receive failed\n");
        }
        else {
            printf("mq_receive success");
            itemptr = (struct item*) bufptr;   

            int* pairCountPtr = (int*) &(*itemptr).astr[0];
            printf("%d\n", *pairCountPtr);
            printf("%c\n", (*itemptr).astr[4]);
            if ( *pairCountPtr == -1 ) {
                terminalCount++;
            }
            else {
                int pairCount = *pairCountPtr;
                char* current = (char *) (pairCountPtr + 1);

                for ( int i = 0; i < pairCount; i++ ) {
                    int* numPtr = (int*) (current + strlen(current) + 1);
                    printf("%s", current);
                    printf("%d\n", *numPtr);

                    char* saveWord = strdup(current);
                    parentHead = insertWord(parentHead, saveWord);

                    if ( i != (pairCount - 1) ) {
                        current = (char *) (numPtr + 1);
                    }
                }
            }
        }
    }

    printTree(parentHead);

    free(bufptr);
    mq_close(mq);
    return 0;
}

struct Node* head = NULL;
void parseFile(char* fileName, int msg_size) {
    FILE* file;
    char word[64];
    file = fopen(fileName, "r");

    mqd_t mq;
    struct item item;
    mq = mq_open(MQNAME, O_RDWR);

    if ( mq == -1 ) {
        printf("opening mq failed\n");
    }

    while ( fscanf(file, "%s", word) == 1 ) {

        // might leak memory
        char* current = strdup(word);
        
        for ( int i = 0; current[i] != '\0'; i++ ) {
            if ( current[i] >= 'a' && current[i] <= 'z' ) {
                current[i] = current[i] - 32;
            }
        }
        
        head = insertWord(head, current);
    }

    struct item* itemPtr = malloc(sizeof(item) + sizeof(char[msg_size]));
    itemPtr->msg_size = msg_size;
    int pairCount = 0;
    int remainingSpace = msg_size - 4;
    itemPtr = traverse(head, &mq, msg_size, itemPtr, &remainingSpace, &pairCount);

    int* ptr = (int*) &(*itemPtr).astr[0];
    *ptr = pairCount;

    // send a message
    int n;
    n = mq_send(mq, (char*) itemPtr, sizeof(struct item) + sizeof(char[msg_size]), 0);

    if ( n == -1 ) {
        printf("mq_send failed\n");
    }
    else {
        printf("mq_send success\n");
    }   

    free(itemPtr);

    // send terminal message for this process
    struct item* terminalItemPtr = malloc(sizeof(item) + sizeof(char[msg_size]));
    terminalItemPtr->msg_size = msg_size;
    int* pointer = (int*) &(*terminalItemPtr).astr[0];
    *pointer = -1;

    // send the terminal message
    n = mq_send(mq, (char*) terminalItemPtr, sizeof(struct item) + sizeof(char[msg_size]), 0);

    if ( n == -1 ) {
        printf("mq_send failed\n");
    }
    else {
        printf("mq_send success\n");
    }   

    free(terminalItemPtr);

    printf("%c", (*itemPtr).astr[20]);
    printf("%d", (*itemPtr).astr[16]);
    printf("%d", remainingSpace);
    int* p = (int*) &(*itemPtr).astr[0];
    printf("%d", *p);
    
    
    fclose(file);
    mq_close(mq);
}
