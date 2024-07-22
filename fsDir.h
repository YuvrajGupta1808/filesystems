/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: fsDir.h
*
* Description:: This file contains the definitions of the functions for
* the directory
*
**************************************************************/

#ifndef _FSDIR_H
#define _FSDIR_H

#define MAX_LEN_FOR_NAME 255
#define DIRECTORY 0x80000000

typedef struct DirEntry{
 time_t creationTime;
 time_t modificationTime;
 time_t accessTime;
 int location;
 int size;
 int permissions;
 char name[MAX_LEN_FOR_NAME];
} DirEntry;

// initDir initilizes a directory, updates freespace, sets '.' and '..', all 
// others are empty
int initDir(int minNum, DirEntry* parent, int blockSize);
// DirToMem sets the directory at a block location creates it in memory and
// returns a pointer to the directory in memory
DirEntry* DirToMem(int startBlock);
// findNameInDir returns the location of a DE name within a parent
int findNameInDir(DirEntry* parent, char* name);
// findNameInDir returns the location of an available DE
int findUnusedDE(DirEntry* parent);
// entryIsDir returns 1 if dir is entry 0 otherwise
int entryIsDir(DirEntry* parent, int index);
// freeIfNotNeeded will free the parent if it not the cwd or root
void freeIfNotNeeded(DirEntry* parent);
// loadDir is built on top of DirToMem but checks if the directory is 
// root or cwd, before allocating memory
DirEntry* loadDir(DirEntry* parent, int pos);
// getRoot return pointer to root DE
DirEntry* getRoot();
// setRoot sets root directory
void setRoot(DirEntry* root);
// getCWD returns pointer to CWD DE
DirEntry* getCWD();
// getBlockSize returns block size
int getBlockSize();
// setBlockSize sets block size
void setBlockSize(int blockSize);
// setCWD frees old CWD if needed and sets passed in cwd as new
void setCWD(DirEntry* newCWD);
#endif