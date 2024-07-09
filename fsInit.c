/**************************************************************
* Class::  CSC-415-0# Spring 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsBitmap.h"


typedef struct VCB
{
	int totalBlocks;
 	int blockSize;
 	int locRootDir;
	int freeSpace;
	int signature;
} VCB;



int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n",
	 numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	int magicNum = 1234567890;
	VCB *VCBP = (VCB*) malloc(blockSize);
	LBAread(VCBP,1,0);
	int magicNum2 = VCBP->signature;
	printf("MAGIC NUM1: %d\n",magicNum2);
	if(1234567890 != magicNum2){
		printf("NOT INITILIZED |\n");
		VCBP->signature = magicNum;
		
		
		// get the number of bytes that are required to represent the free space
		int freeSpaceBytes = (numberOfBlocks+7)/8;

		// get the number of blocks that the free space will take up
		int freeSpaceBlocks = (freeSpaceBytes + (blockSize-1))/blockSize;

		// initilize free space bassed on this
		initBitMap(blockSize*numberOfBlocks, freeSpaceBlocks);

		// initilize bit map
		VCBP->freeSpace = freeSpaceBlocks;

		// set first 6 to used +1 for VCB
		for(int i = 0; i <= freeSpaceBlocks; i++){
			setBit(i);
		}
		writeBits();
		LBAwrite(VCBP, 1, 0);



	}else{
		printf("already initilized\n");
		printf("Key: %d\nFreeSpace: %d\n",VCBP->signature,VCBP->freeSpace);
		// initilize free space bassed on this
		reInitBitMap(blockSize*numberOfBlocks, VCBP->freeSpace);
		printBitMap();
	}

	return 0;
	}


	
	
	void exitFileSystem ()
	{
	printf ("System exiting\n");
	}