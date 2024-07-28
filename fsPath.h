/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: fsPath.h
*
* Description:: This file is a header file for a stack-based path management system. 
* It includes functions to manage the stack, clean and parse paths, and 
* convert the stack to a string representation of the path.
*
**************************************************************/


#ifndef _FSPATH_H
#define _FSPATH_H

#include "fsDir.h"

typedef struct ppinfo {
    DirEntry* parent;
    char*     lastElement;
    int       posInParent;
} ppinfo;

// pathCleaner takes in a path and uses a stack to manage CWD string 
void pathCleaner(char* path);
// parse path takes in a path and a struct, the struct will be filled with that 
// relate to the passed in path.
int parsePath(char* path, ppinfo* ppi);
// getCWDStr returns the string value of the stack
char* getCWDStr();

void freePPI(ppinfo* ppi);

void freeSTRCWD();

#endif