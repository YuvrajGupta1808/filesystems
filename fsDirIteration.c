/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: fsDirIteration.c
*
* Description:: This file implements a basic directory management system fro a 
* file system, it includes functions to open, read, and close directory streams,
* as well as retrieve file status information. 
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfs.h"
#include "fsPath.h"

#define MAXFDIR 10 
fdDir fdDirArray[MAXFDIR];
int hasInit = 0;

// initilize fdDirArray to contain null values
void init (){
    // Check if already initialized
    if (hasInit){
        return;
    }
    // iterate over array initilize set diritem to NULL
    for (int i = 0; i < MAXFDIR; i++){
        fdDirArray[i].di = NULL; 
    }

    // Store that the function has already ran.
    hasInit = 1;
}

//Get the first available fiel descriptor directory.
int getFdDir (){
    for (int i = 0; i < MAXFDIR; i++){
        //return first free file descriptor directory
        if (fdDirArray[i].di == NULL){
            fdDirArray[i].di = (struct fs_diriteminfo *)-2;
            return i;
        }
    }
    return (-1);
}

//Open directory stream.
fdDir * fs_opendir(const char *pathname){
    // initilize if needed
    if(hasInit == 0){
        init();
    }

    // get next availible dir fd
    int currentDir = getFdDir();
    if(currentDir == -1){
        return NULL;
    }
    // copy path to avoid changing original
    char *pathCpy = malloc(strlen(pathname)+1);
    strcpy(pathCpy, pathname);
  
    ppinfo* ppi = malloc(sizeof(ppinfo));
    int retVal = parsePath(pathCpy, ppi);
    

    if(retVal < 0){
        freePPI(ppi);
        printf("fs_opendir: ERROR IN PARSE PATH: %d\n", retVal);
        return NULL;
    }

    int pos = ppi->posInParent;
    if(ppi->posInParent == -1){
        return NULL;
    }
    
    // load if not needed
     fdDirArray[currentDir].directory = loadDir(ppi->parent, ppi->posInParent);
    
    // fill struct values
    fdDirArray[currentDir].d_reclen = (ppi->parent[pos].size);
    fdDirArray[currentDir].dirEntryPosition = 0;
    fdDirArray[currentDir].currentDir = currentDir;
    fdDirArray[currentDir].di = malloc(sizeof(struct fs_diriteminfo));
    freePPI(ppi);
    free(pathCpy);

    return &fdDirArray[currentDir];
}

//Read a directory entry.
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    
    //Skip entries with location 0
    while(dirp->directory[dirp->dirEntryPosition].location == 0 
            && dirp->dirEntryPosition != dirp->directory->size){
        dirp->dirEntryPosition+=1;
    }

    // reached end of directorys size
    if(dirp->dirEntryPosition == dirp->directory->size){
        return NULL;
    }

    //use dirEntryPositioncurrent dir value to fill struct
    DirEntry* cur = &dirp->directory[dirp->dirEntryPosition];
    strncpy(fdDirArray[dirp->currentDir].di->d_name,cur->name,255);

    fdDirArray[dirp->currentDir].di->d_reclen = cur->size*getBlockSize();
    fdDirArray[dirp->currentDir].di->fileType = cur->permissions;
    
    // update current dir value
    dirp->dirEntryPosition+=1;
    return fdDirArray[dirp->currentDir].di;
}

//Close a directory stream
int fs_closedir(fdDir *dirp){
    // free unneeded memory
    free(fdDirArray[dirp->currentDir].di);
    freeIfNotNeeded(dirp->directory);
    // set value dir info to null
    fdDirArray[dirp->currentDir].di = NULL;
    return dirp->currentDir;
}

//Get file status. 
int fs_stat(const char *path, struct fs_stat *buf){
    char *pathCpy = strdup(path);
    ppinfo* ppi = malloc(sizeof(ppinfo));
    int retVal = parsePath(pathCpy, ppi);

    free(pathCpy);
    if(retVal < 0){
        freePPI(ppi);
        printf("fs_stat: ERROR IN PARSE PATH: %d\n", retVal);
        return -1;
    }
    int byte = ppi->parent[ppi->posInParent].gg;
    // fill passed in buffer with values from parsePath
    buf->st_accesstime = ppi->parent[ppi->posInParent].accessTime;
    buf->st_blocks = ppi->parent[ppi->posInParent].size;
    buf->st_createtime = ppi->parent[ppi->posInParent].creationTime;
    buf->st_modtime = ppi->parent[ppi->posInParent].modificationTime;
    buf->st_size = fs_isFile(path)?byte:ppi->parent[ppi->posInParent].size;

    freePPI(ppi);
    return 0;
}