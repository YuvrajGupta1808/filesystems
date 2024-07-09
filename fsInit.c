/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name:: Satisfaction
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
#include "fsDir.h"


typedef struct VCB
{
	int totalBlocks;
 	int blockSize;
 	int locRootDir;
	int freeSpace;
	int signature;
} VCB;


int initDir(int minNum, DirEntry* parent);

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n",
	 numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	// number that will be checked against from volume
	int magicNum = -1;
	VCB *VCBP = (VCB*) malloc(blockSize);
	// get value from volume(SampleVolume) casted to VCB struct at VCB location
	// 0;
	LBAread(VCBP,1,0);
	// retreive the value at the signature
	int magicNum2 = VCBP->signature;
	printf("MAGIC NUM1: %d\n",magicNum2);
	if(magicNum != magicNum2){
		printf("NOT INITILIZED |\n");
		VCBP->signature = magicNum;
		// get the number of bytes that are required to represent the free space
		int freeSpaceBytes = (numberOfBlocks+7)/8;

		// get the number of blocks that the free space will take up
		int freeSpaceBlocks = (freeSpaceBytes + (blockSize-1))/blockSize;

		// initilize free space bassed on this
		VCBP->freeSpace = freeSpaceBlocks;
		// initilize bit map for free space
		initBitMap(blockSize*numberOfBlocks, freeSpaceBlocks);

		// set the total number of used blocks 1 for VCB + freeSpaceBlocks
		for(int i = 0; i <= freeSpaceBlocks; i++){
			setBit(i);
		}
		writeBits();

		//initilize and get position of root directory
		VCBP->locRootDir = initDir(50, NULL);
		LBAwrite(VCBP, 1, 0);



	}else{
		printf("already initilized\n");
		printf("Key: %d\nFreeSpace: %d\n",VCBP->signature,VCBP->freeSpace);
		// restore data to memory from VCB
		reInitBitMap(blockSize*numberOfBlocks, VCBP->freeSpace);
	}

	return 0;
	}

	


	
	
	void exitFileSystem ()
	{
	printf ("System exiting\n");
	}