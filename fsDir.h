/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name::
* Student IDs::
* GitHub-Name::
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

typedef struct{
 time_t creationTime;
 time_t modificationTime;
 time_t accessTime;
 int location;
 int size;
 int permissions;
 char name[MAX_LEN_FOR_NAME];
} DirEntry;

// initilize a directory
int initDir(int minNum, DirEntry* parent);
// int reInitDir(int startBlock);

#endif