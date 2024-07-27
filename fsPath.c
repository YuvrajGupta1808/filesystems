#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fsDir.h"
#include "fsPath.h"

#define MAXSTACKSIZE 50
char* stack[MAXSTACKSIZE];
char* strPath = NULL;
int size = 0;

// adds new value to top of stack
void push(char* value){
    // allocate memory for str
    stack[size] = malloc(strlen(value) + 1);
    if (stack[size] == NULL) {
        return;
    }
    // fill stack with value
    strcpy(stack[size], value);
    // increment size
    size++;
}

void pop(){
    // decrement size
    size--;
    // free alocated memory
    free(stack[size]);
    // set values to null
    stack[size] = NULL;
}

char* toString(){
    int strLen = 0;
    int offset = 0;
    // itterate over stack and its values to get amount of memory needed
    for(int i = 0; i < size; i++){
        strLen += strlen(stack[i])+1;
    }
    // if strPath is not null free it
    if (strPath != NULL) {
        free(strPath);
        strPath = NULL;
    }
    // allocate the memory
    char *newStrPath = realloc(strPath, strLen + 2);
    if (newStrPath == NULL) {
        return NULL;
    }
    // set strPath to newStrPath
    strPath = newStrPath;
    strPath[0] = '\0';
    
    // iterate over size concatinating string
    for(int i = 0; i < size; i++){
        strcat(strPath, "/");
        strcat(strPath, stack[i]);
    }
    // add final '/'
    strcat(strPath, "/");
    return strPath;
}


void pathCleaner(char* path){
    // if path is root remove all of old path
    if( path[0] == '/' ){
        while(size > 0){
            pop();
        }
    }
    
    char* token1 = strtok(path,"/");
    //tokenize entire string by '/'
    while(token1 != NULL){
        // if toke n is '.' then no need to add to stack
        if(strcmp(token1,".") == 0){
            token1 = strtok(NULL,"/");
            continue;
        }else
            // if token is '..' then we need to remove last index
            if(strcmp(token1,"..") == 0){
                // as long as it is not at the top
                if(size > 0 ){
                    pop();
                }
            }else{
                // if it is not '.' or '..' then just add it
                push(token1);
            }
        token1 = strtok(NULL,"/");
    };
    
    
}

int parsePath(char* path, ppinfo* ppi){
    // if path is not valid return error
    if(path == NULL){
        return -1;
    }
    DirEntry* start;
    if(path[0] == '/'){
        start = getRoot();
        
    }else{
        start = getCWD();
    }
    DirEntry* parent = start;
    start = NULL;
    
    char* token1 = strtok(path,"/");
    // if path is just /
    if(token1 == NULL){
        ppi->parent = parent;
        ppi->lastElement = NULL;
        ppi->posInParent = 0;
        return 0;
    }
    
    char *token2;
    
    do{
        // set position at start of iteration so it is always up to date
        ppi->posInParent = findNameInDir(parent, token1);
        token2 = strtok(NULL,"/");
        // end of path
        if(token2 == NULL){
            // set parent
            ppi->parent = parent;
            // name of the last element in path
            ppi->lastElement = token1;
            return 0;
        }
        // if path is invalid
        if(ppi->posInParent < 0){
            ppi->parent = NULL;
            ppi->lastElement;
            return -2;
        }
        
        // can not continue if path is invalid
        if(!entryIsDir(parent, ppi->posInParent)){
            return -3;
        }
        // because we confimed this entry exists we can load it into mempry
        DirEntry* temp = loadDir(parent,ppi->posInParent);
        freeIfNotNeeded(parent);
        // swap parent to be the new temp, and token
        parent = temp;
        token1 = token2;
    } while(token2 != NULL);
    
    
}

void setCWDStr(char *pathCpy){
    pathCleaner(pathCpy);
}

char* getCWDStr(){
    return toString();
}

void freePPI(ppinfo* ppi){
    freeIfNotNeeded(ppi->parent);
    free(ppi);
}

void freeSTRCWD(){
    
    while(size > 0 ){
        pop();
    }
    
    if(strPath != NULL){
        free(strPath);
    }
}


