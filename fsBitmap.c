/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: fsBitmap.c
*
* Description:: This file includes the implementation of the methods needed for
* the free space is represented as a bitmap.
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "fsBitmap.h"
#include "fsLow.h"

fsMap freeSpaceMap;

int initBitMap(int numBytes, int numBlocks){
    // populate golbal struct and allocate memory and pupulate fields
    freeSpaceMap.map =  malloc(numBytes);
    if(freeSpaceMap.map == NULL){
        return -1;
    }
    //populate in memory struct with helpful values
    freeSpaceMap.fsSize = numBytes;
    freeSpaceMap.numBlocks = numBlocks;
    return 0;
}

int getBit(int bitNum){
    int byteIndex = bitNum / 8;
    int bitPosition = bitNum % 8;
    int mask = (1 << bitPosition);
    // check state of bit
    int value = (freeSpaceMap.map[byteIndex] & mask) != 0;
    return value;
}
void setBit(int bitNum){
    // get the bits index + position from bitNum
    int byteIndex = bitNum / 8;
    int bitPosition = bitNum % 8;
    
    // bassed on position of bit create a mask
    int mask = (1 << bitPosition);
    
    // perform bitwise or to set the bit to 1
    freeSpaceMap.map[byteIndex] |= mask;
}

int reInitBitMap(int numBytes, int numBlocks){
    // initilizes memory to be filled by blocks from volume
    int good = initBitMap(numBytes, numBlocks);
    if(good == -1){
        return -1;
    }
    // save freespace map from memory to volume
    LBAread(freeSpaceMap.map,freeSpaceMap.numBlocks,1);
    return 0;
}

void writeBits(){
    // save freespace map from memory to volume
    LBAwrite(freeSpaceMap.map, freeSpaceMap.numBlocks,1);
}

int clearBit(int bitNum){
    // get the bits index + position from bitNum
    int byteIndex = bitNum / 8;
    int bitPosition = bitNum % 8;

    // based on the position of the bit create a mask
    int mask = ~(1 << bitPosition);

    // perform bitwise 'and' with the mask to set it to 0
    freeSpaceMap.map[byteIndex] &= mask;
    return 0;
}

int nextFree(int blocksNeeded){
    int count = 0;
    int startIndex = -1;
    // loop over all blocks in free space map represented by blocks
    for (int i = 0; i < freeSpaceMap.fsSize; i++) {
        int value = getBit(i);
        if (!value) {
            count++;
            if(count == blocksNeeded){
                return startIndex;
            }
        }else{
            //set start to next block
            count = 0;
            startIndex = i+1;
        }
    }
    return -1;
}

void printBitMap(){
    // prints the first 100 bits
    // function is used for testing
    for (int i = 0; i < 100; i++) {
        int byteIndex = i / 8;
        int bitPosition = i % 8;
        int mask = (1 << bitPosition);
        printf("%d", (freeSpaceMap.map[byteIndex] & mask) != 0);
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

