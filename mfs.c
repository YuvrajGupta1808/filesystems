/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: mfs.c
*
* Description:: 
*	This is the file system interface.
*	This is the interface needed by the driver to interact with
*	your filesystem.
*
**************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mfs.h"
#include "fsDir.h"
#include "fsLow.h"
#include "fsPath.h"
#include "fsBitmap.h"

char* strCWD;

// helper to check if directory is empty
int isDirEmpty(DirEntry* dirEntry){
    for(int i = 2; i < dirEntry->size; i++){
        if(dirEntry[i].location != 0){
            return 0;
        }
    }
    return 1;
}

//Method to set the current working directory. 
int fs_setcwd(char *pathname){
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    char *pathCpy = strdup(pathname);
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    // check for errors
    if(ppi->posInParent < 0){
        freePPI(ppi);
        return -1;
        
    }
    // check for errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_setcwd:ERROR IN PARSE PATH: %d\n", retVal);
        return retVal;
    }
    
    DirEntry* entry = ppi->parent;
    // make sure last value is a valid directory
    if(!entryIsDir(entry, ppi->posInParent)){
        printf("fs_setcwd: is not a valid path\n");
        freePPI(ppi);
        return -1;
    }
    // load the directory to memory
    DirEntry* cwd = loadDir(entry, ppi->posInParent);
    // set current directory
    freePPI(ppi);
    setCWD(cwd);
    // update str value of CWD
    pathCleaner(pathname);
    return 0;
}

//Fucntion to create a new directory. Used by the md command in shell. 
int fs_mkdir(const char *pathname, mode_t mode){
    // allocate memory for return value of parsePath
    
    ppinfo* ppi = malloc(sizeof(ppinfo));
    // must wait unitl after write to disk to free
    char* pathCpy = strdup(pathname);
    int retVal = parsePath(pathCpy, ppi);
    // check for errors from parsePath
    if(retVal < 0){
        freePPI(ppi);
        printf("Invalid Path: %s\n", pathname);
        free(pathCpy);
        return -1;
    }
    
    // check for errors from parsePath
    if(ppi->posInParent != -1){
        printf("%s already exists\n", pathname);
        freePPI(ppi);
        free(pathCpy);
        return 0;
    }
    
    // find free location in parent
    int loc = findUnusedDE(ppi->parent);
    if(loc == -1){
        freePPI(ppi);
        free(pathCpy);
        return -2;
    }
    
    // initilize new directory and set it to memory
    int newDirLoc = initDir(50,ppi->parent,getBlockSize());
    DirEntry* newDir = DirToMem(newDirLoc);
    // set permisions to passed in values
    newDir->permissions |= mode;
    DirEntry* val = ppi->parent;
    //update values of parent to newly created directory
    memcpy(&val[loc], newDir, sizeof(DirEntry));
    strcpy(val[loc].name,ppi->lastElement);
    //save to disk
    LBAwrite(val, val->size,val->location);
    freeIfNotNeeded(newDir);
    freePPI(ppi);    
    free(pathCpy);
    return 0;
}

//Method to get the current working directory. 
char* fs_getcwd(char *pathBuf, size_t size){
    char* retVal =  strncpy(pathBuf, getCWDStr() ,size);
    return retVal;
}

// fs_isFile returns 1 if it is a file, 0 if it not
int fs_isFile(const char * filename){
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    if (ppi == NULL) {
        return -1;
    }
    
    char *pathCpy = strdup(filename);
    if (pathCpy == NULL) {
        return -1;
    }
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    // check for errors (does not exist)
    if(ppi->posInParent == -1){
        freePPI(ppi);
        return 0;
    }
    // check for errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_isFile: ERROR IN PARSE PATH: %d\n", retVal);
        return 0;
    }
    
    // check whether is not dir
    retVal = !entryIsDir(ppi->parent, ppi->posInParent);
    freePPI(ppi);
    return retVal;
}

//Function that returns if the directory is file or directory. 
int fs_isDir(char * filename){
    
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    if (ppi == NULL) {
        return -1;
    }
    
    char *pathCpy = strdup(filename);
    if (pathCpy == NULL) {
        freePPI(ppi);
        return -1;
    }
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    // check for errors (does not exist)
    if(ppi->posInParent == -1){
        freePPI(ppi);
        return 0;
    }
    
    // check for errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_isFile: ERROR IN PARSE PATH: %d\n", retVal);
        return 0;
    }
    
    // check whether is dir
    retVal = entryIsDir(ppi->parent, ppi->posInParent);
    freePPI(ppi);
    return retVal;
}

//Deletes the directory 
int fs_delete(char* filename){
    //allocate memory for storing parsePath info
    ppinfo* ppi = malloc(sizeof(ppinfo));
    if(ppi == NULL){
        return -1;
    }
    
    //allocate memory to make a copy of filename
    char *pathCpy = strdup(filename);
    if(pathCpy == NULL){
        free(ppi);
        return -1;
    }
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    // Check if the file exists in the parent directory
    if(ppi->posInParent == -1){
        printf("fs_delete: FILE DOESN'T EXIST.\n");
        freePPI(ppi);
        return -1;
    }
    
    //check for errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_delete: ERROR IN PARSE PATH: %d\n", retVal);
        return -1;
    }
    
    DirEntry* parentDir = ppi->parent;
    int fileIndex = ppi->posInParent;
    
    
    parentDir[fileIndex].modificationTime = time(NULL); //current time
    for(int i = parentDir[fileIndex].location; i < parentDir[fileIndex].location+parentDir[fileIndex].size; i++){
        clearBit(i);
    }
    writeBits();
    
    parentDir[fileIndex].size = 0;
    parentDir[fileIndex].location = 0;
    parentDir[fileIndex].name[0] = '\0';
    LBAwrite(parentDir, parentDir->size, parentDir->location);
    
    freePPI(ppi);
    
    return 0;
}


//Method to remove the given directory. 
int fs_rmdir(const char *pathname){
    //allocate memory for storing parsePath info
    ppinfo* ppi = malloc(sizeof(ppinfo));
    if(ppi == NULL){
        return -1;
    }
    
    char* pathCpy = strdup(pathname);
    if(pathCpy == NULL){
        free(ppi);
        return -1;
    }
    
    int retVal = parsePath(pathCpy, ppi);
    free(pathCpy);
    
    if(ppi -> posInParent == -1){
        printf("Directory doesn't exist\n");
        freePPI(ppi);
        return -1;
    }
    
    //check for errors
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_rmdir: ERROR IN PARSE PATH: %d\n", retVal);
        return -1;
    }
    
    DirEntry* parentDir = ppi->parent;
    int dirIndex = ppi -> posInParent;
    DirEntry* childDE = loadDir(parentDir, dirIndex);
    
    if(parentDir[dirIndex].location == getRoot()->location){
        printf("Cant remove root\n");
        freePPI(ppi);
        freeIfNotNeeded(childDE);
        return -1;
    }

    if(parentDir[dirIndex].location == getCWD()->location){
        printf("Cant remove CWD\n");
        freePPI(ppi);
        freeIfNotNeeded(childDE);
        return -1;
    }
    
    //Check if directory is empty
    if(!isDirEmpty(childDE)){
        printf("cannot remove. Directory isn't empty.\n");
        freePPI(ppi);
        freeIfNotNeeded(childDE);
        return -1;
    }
    
    freeIfNotNeeded(childDE);
    
    // clear the directory entry
    for(int i = parentDir[dirIndex].location; i < parentDir[dirIndex].location+parentDir[dirIndex].size; i++){
        clearBit(i);
    }
    writeBits();
    
    memset(&parentDir[dirIndex], 0, sizeof(DirEntry));
    parentDir -> modificationTime = time(NULL);
    
    LBAwrite(parentDir, parentDir -> size, parentDir -> location);
    
    freePPI(ppi);
    
    return 0;
}


