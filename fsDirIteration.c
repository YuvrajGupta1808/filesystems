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
    // already ran, return
    if (hasInit){
        return;
    }
    // iterate over array initilize set diritem to NULL
    for (int i = 0; i < MAXFDIR; i++){
        fdDirArray[i].di = NULL; 
    }

    // record that function has run
    hasInit = 1;
}

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
    char *pathCpy = malloc(strlen(pathname)+1);
    strcpy(pathCpy, pathname);
        
    ppinfo* ppi = malloc(sizeof(ppinfo));
    int retVal = parsePath(pathCpy, ppi);
    

    if(retVal < 0){
        freeIfNotNeeded(ppi->parent);
        free(ppi);
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

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    
    while(dirp->directory[dirp->currentDir].location == 0 && dirp->currentDir != dirp->directory->size){
        dirp->currentDir+=1;
    }

        // reached end of directorys size
    if(dirp->currentDir == dirp->directory->size){
        return NULL;
    }
    //use current dir value to fill struct
    DirEntry* cur = &dirp->directory[dirp->currentDir];
    strncpy(fdDirArray[dirp->dirEntryPosition].di->d_name,cur->name,255);

    fdDirArray[dirp->dirEntryPosition].di->d_reclen = cur->size*getBlockSize();
    fdDirArray[dirp->dirEntryPosition].di->fileType = cur->permissions;
    
    // update current dir value
    dirp->currentDir+=1;
    return fdDirArray[dirp->dirEntryPosition].di;
}

int fs_closedir(fdDir *dirp){
    // free unneeded memory
    free(fdDirArray[dirp->dirEntryPosition].di);
    freeIfNotNeeded(dirp->directory);
    // set value dir info to null
    fdDirArray[dirp->dirEntryPosition].di = NULL;
    return dirp->dirEntryPosition;
}

int fs_stat(const char *path, struct fs_stat *buf){
    char *pathCpy = malloc(strlen(path)+1);
    strcpy(pathCpy,path);
    ppinfo* ppi = malloc(sizeof(ppinfo));
    int retVal = parsePath(pathCpy, ppi);
    if(ppi->posInParent == -1){
    freeIfNotNeeded(ppi->parent);
    retVal = parsePath("/", ppi);
    ppi->posInParent = findNameInDir(ppi->parent, path);
    }

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
    buf->st_size = byte>0?byte:ppi->parent[ppi->posInParent].size;

    freePPI(ppi);
    return 0;
}