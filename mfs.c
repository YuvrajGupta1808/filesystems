#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mfs.h"
#include "fsDir.h"
#include "fsLow.h"
#include "fsPath.h"
#include "fsBitmap.h"

char* strCWD;

int fs_setcwd(char *pathname){
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    char *pathCpy = malloc(strlen(pathname)+1);
    strcpy(pathCpy,pathname);

    int retVal = parsePath(pathCpy, ppi);
    // check for errors
    if(ppi->posInParent < 0 && ppi->posInParent != -2){
        free(ppi);
        free(pathCpy);
        return -1;
        
    }
    // check for errors
    if(retVal < 0){
        free(ppi);
        free(pathCpy);
        printf("fs_setcwd:ERROR IN PARSE PATH: %d\n", retVal);
        return -1;
    }

    DirEntry* entry = ppi->parent;
    // make sure last value is a valid directory
    if(!entryIsDir(entry, ppi->posInParent)){
        printf("%s\n", pathname);
        free(ppi);
        free(pathCpy);
        printf("ERROR IS NOT A DIR: %d\n", retVal);
        return -1;
    }

    // load the directory to memory
    DirEntry* cwd = loadDir(entry, ppi->posInParent);
    // set current directory
    setCWD(cwd);
    // update str value of CWD
    pathCleaner(pathname);
    free(ppi);
    free(pathCpy);
    return 0;
}

int fs_mkdir(const char *pathname, mode_t mode){
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    char* pathCpy = malloc(strlen(pathname)+1); 
    strcpy(pathCpy, pathname);
    int retVal = parsePath(pathCpy, ppi);

    // check for errors from parsePath
    if(retVal < 0){
        free(ppi);
        free(pathCpy);
        printf("ERROR in PARSE PATH: %d\n", retVal);
        return -1;
    }

    // check for errors from parsePath
     if(ppi->posInParent != -1){
       printf("Dir already exists: %d\n", retVal);
        free(ppi);
        free(pathCpy);
        return 0;
    }

    // find free location in parent
    int loc = findUnusedDE(ppi->parent);

    if(loc == -1){
        free(ppi);
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
    free(ppi);
    free(pathCpy);
    return 0;
}

char* fs_getcwd(char *pathname, size_t size){
    char* cwdSTR = getCWDStr();
    char* retVal =  strncpy(pathname, cwdSTR,size);
    free(cwdSTR);
    return retVal;
}

int fs_isFile(char * filename){
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    char *pathCpy = malloc(strlen(filename)+1);
    strcpy(pathCpy,filename);
    int retVal = parsePath(pathCpy, ppi);

    // check for errors (does not exist)
    if(ppi->posInParent == -1){
        free(pathCpy);
        free(ppi);
        return 0;
    }
    // check for errors
    if(retVal < 0){
        free(pathCpy);
        free(ppi);
        printf("fs_isFile: ERROR IN PARSE PATH: %d\n", retVal);
        return 0;
    }
    
    // check whether is not dir
    retVal = !entryIsDir(ppi->parent, ppi->posInParent);
    free(ppi);
    free(pathCpy);
    return retVal;
}

int fs_isDir(char * filename){
    // allocate memory for return value of parsePath
    ppinfo* ppi = malloc(sizeof(ppinfo));
    char *pathCpy = malloc(strlen(filename)+1);
    strcpy(pathCpy,filename);
    int retVal = parsePath(pathCpy, ppi);
    // check for errors (does not exist)
    if(ppi->posInParent == -1){
        free(pathCpy);
        free(ppi);
        return 0;
    }
    // check for errors
    if(retVal < 0){
        free(pathCpy);
        free(ppi);
        printf("fs_isFile: ERROR IN PARSE PATH: %d\n", retVal);
        return 0;
    }
    // check whether is dir
    retVal = entryIsDir(ppi->parent, ppi->posInParent);
    free(ppi);
    free(pathCpy);
    return retVal;
}

int fs_delete(char* filename){
    printf("fs_delete\n");
	//allocate memory for storing parsePath info
	ppinfo* ppi = malloc(sizeof(ppinfo));
	if(ppi == NULL){
		perror("Memory allocation failed");
		return -1;
	}
	//allocate memory to make a copy of filename
	char *pathCpy = malloc(strlen(filename)+1);
	if(pathCpy == NULL){
	    perror("Memory allocation failed");
	    free(ppi);
		return -1;
	}
	
	strcpy(pathCpy, filename);
	int retVal = parsePath(pathCpy, ppi);

	// Check if the file exists in the parent directory
	if(ppi -> posInParent == -1){
		printf("fs_delete: FILE DOESN'T EXIST.\n");
		free(ppi);
		free(pathCpy);
		return -1;
	}

	//check for errors
	if(retVal < 0){
		free(pathCpy);
        free(ppi);
        printf("fs_delete: ERROR IN PARSE PATH: %d\n", retVal);
        return -1;
	}
	
	DirEntry* parentDir = ppi -> parent;
	int fileIndex = ppi -> posInParent;


	parentDir[fileIndex].modificationTime = time(NULL); //current time
    printf("Bits to be clear: loc: %d, size: %d, indes: %d \n",parentDir[fileIndex].location, parentDir[fileIndex].size,fileIndex);
    for(int i = parentDir[fileIndex].location; i < parentDir[fileIndex].location+parentDir[fileIndex].size; i++){
        clearBit(i);
    }
    writeBits();
    parentDir[fileIndex].size = 0;
	parentDir[fileIndex].location = 0;
    printf("Pre fs_delete free\n");
    parentDir[fileIndex].name[0] = '\0';
    printf("Post fs_delete free\n");
	LBAwrite(parentDir, parentDir->size, parentDir->location);
	
	free(ppi);
	free(pathCpy);
	
	return 0; // success	
}
// helper to check if directory is empty
int isDirEmpty(DirEntry* dirEntry){
    for(int i = 2; i < dirEntry->size; i++){
        if(dirEntry[i].location != 0){
            return 0;
        }
    }
    return 1;
}

int fs_rmdir(const char *pathname){
    printf("fs_rmdir\n");
    //allocate memory for storing parsePath info
	ppinfo* ppi = malloc(sizeof(ppinfo));
	if(ppi == NULL){
		perror("Memory allocation failed");
		return -1;
	}
	
	char* pathCpy = malloc(strlen(pathname)+1);
	if(pathCpy == NULL){
	    perror("Memory allocation failed");
	    free(ppi);
		return -1;
	}
	
	strcpy(pathCpy, pathname);
	
	int retVal = parsePath(pathCpy, ppi);
	
	if(ppi -> posInParent == -1){
	    printf("Directory doesn't exist\n");
	    free(ppi);
	    free(pathCpy);
	    return -1;    
	}
	
	//check for errors
	if(retVal < 0){
		free(pathCpy);
        freeIfNotNeeded(ppi->parent);
        free(ppi);
        printf("fs_rmdir: ERROR IN PARSE PATH: %d\n", retVal);
        return -1;
	}
	
	DirEntry* parentDir = ppi->parent;
	int dirIndex = ppi -> posInParent;
    DirEntry* childDE = loadDir(parentDir, dirIndex);
	
    if(parentDir[dirIndex].location == getRoot()->location){
    printf("Cant remove root\n");
    return -1;
    }

	//Check if directory is empty -- we cannot delete if directory isn't empty
	if(!isDirEmpty(childDE)){
	    printf("cannot remove. Directory isn't empty.\n");
        freeIfNotNeeded(ppi->parent);
	    free(ppi);
	    free(pathCpy);
        free(childDE);
	    return -1;
	}

    free(childDE);
	
	// clear the directory entry
    printf("S: %d L: %d\n",parentDir[dirIndex].size, parentDir[dirIndex].location);
    for(int i = parentDir[dirIndex].location; i < parentDir[dirIndex].location+parentDir[dirIndex].size; i++){
        clearBit(i);
    }

	memset(&parentDir[dirIndex], 0, sizeof(DirEntry));
	parentDir -> modificationTime = time(NULL); //current time

    writeBits();
	LBAwrite(parentDir, parentDir -> size, parentDir -> location);
	
    freeIfNotNeeded(ppi->parent);
	free(ppi);
	free(pathCpy);
	
	return 0; //success
}


