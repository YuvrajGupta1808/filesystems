/**************************************************************
* Class::  CSC-415-0# Spring 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: fsBitmap.h
*
* Description:: Main driver for file system assignment.
*
* This file is where the implementation for the bitmap(free space alocation)
* is stored.
*
**************************************************************/

#ifndef _BITMAP_H
#define _BITMAP_H

typedef struct fsMap {
    int blockSize;
    int numBlocks;
    int fsSize;
    char *map;
} fsMap;

int initBitMap(int numBytes, int fsSize);
int reInitBitMap(int numBytes, int fsSize);
int getBit(int bitNum);
int setBit(int bitNum);
int clearBit(int bitNum);
int writeBits();
void printBitMap();

#endif
