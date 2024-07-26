/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
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

// nextFree returns the first location of a sequence of blocks of
// requested size 
int nextFree(int blocksNeeded);
// initBitmap alocates memory for the bitmap
int initBitMap(int numBytes, int fsSize);
// reInitBitMap alocates memory for the bitmap and populates it with data from 
// the volume
int reInitBitMap(int numBytes, int fsSize);
// getBit returns wether or not a block is used or not
int getBit(int bitNum);
// setBit sets a bit to update the block
void setBit(int bitNum);
// clearBit clears the bit so teh block is unused
int clearBit(int bitNum);
// writeBits updates the freespace in the volume to match the version in memory
void writeBits();
// printBitMap prints the first 100 bits
void printBitMap();
// freeBitmap frees the memory
void freeBitMap();

#endif
