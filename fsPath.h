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