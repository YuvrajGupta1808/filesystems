/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: fsDir.c
*
* Description:: this file contains the implementation of the directory
* functions
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h> 
#include <string.h>
#include "fsBitmap.h"
#include "fsDir.h"
#include "fsLow.h"

int initDir(int minNum, DirEntry* parent){
        // get the number of bytes -> blocks, so we can allocate memory
		int numBytes = minNum * sizeof(DirEntry);
		int numBlocks = (numBytes + 511)/512;
		int bytesToAlloc = numBlocks * 512;
		DirEntry* DEP = malloc(bytesToAlloc);
        // get total number of directory
		int accDe = bytesToAlloc/sizeof(DirEntry);
		printf("TOTAL DIRS: %d\n", accDe);
		int newLoc = nextFree(numBlocks);
		if(newLoc ==-1){
			//handle error (compact data or quit)
		}

		printf("Blocks found at: %d\n", newLoc);
		printf("Blocks Needed: %d\n", numBlocks);
		printf("DE Size: %ld\n", sizeof(DirEntry));
        // update freespace bitmap
		for(int i = 0; i < numBlocks; i++){
			setBit(newLoc+i);
		}
		writeBits();
        
        // set all other directory entries to empty
		for(int i = 2; i < accDe; i++){
			DEP[i].size = 0;
			DEP[i].location = 0;
			DEP[i].name[0] = '\0';
		}

		// add '.' and '..' direcotrys
		time_t curTime = time(NULL);
		DEP[0].location = newLoc;
		DEP[0].creationTime= curTime;
 		DEP[0].modificationTime = curTime;
		DEP[0].size = numBlocks;
 		DEP[0].accessTime = curTime;
		DEP[0].permissions |= DIRECTORY;
        printf("Value: %d\n",DEP[0].permissions);
		strcpy(DEP[0].name,".");


		DirEntry* dotdot = parent;
        // check for root direcotry
		if(dotdot == NULL){
			dotdot = DEP;
		}
    
		memcpy(&DEP[1], dotdot, sizeof(DirEntry));
		strcpy(DEP[1].name,"..");
		LBAwrite(DEP,numBlocks,newLoc);
		return newLoc;
	}

// int reInitDir(int startBlock){
//
// }