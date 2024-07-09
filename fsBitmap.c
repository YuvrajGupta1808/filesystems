#include <stdio.h>

#include "fsBitmap.h"
#include <stdlib.h>
#include "fsLow.h"

fsMap freeSpaceMap;

int initBitMap(int numBytes,int fsSize){
    freeSpaceMap.map =  malloc(numBytes);
    freeSpaceMap.fsSize = fsSize;
}

int getBit(int bitNum){
    return 0;
}
int setBit(int bitNum){
    int byteIndex = bitNum / 8; 
    int bitPosition = bitNum % 8; 
    freeSpaceMap.map[byteIndex] |= (1 << bitPosition);
    return 0;
}

int reInitBitMap(int numBytes, int fsSize){
    initBitMap(numBytes, fsSize);
    LBAread(freeSpaceMap.map,freeSpaceMap.fsSize,1);
}

int writeBits(){
    LBAwrite(freeSpaceMap.map, freeSpaceMap.fsSize,1);
}

int clearBit(int bitNum){
    return 0;
}

void printBitMap(){
    for (int i = 0; i < 100; ++i) {
        int byte_index = i / 8;
        int bit_position = i % 8;
        printf("%d", (freeSpaceMap.map[byte_index] & (1 << bit_position)) != 0);
        if ((i + 1) % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}
