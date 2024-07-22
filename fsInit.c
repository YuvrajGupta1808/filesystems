/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
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

// the VCB stucture holds data required to initilize/reinitilize
// the filesystem
typedef struct VCB
{
	int totalBlocks; // total blocks availible to use
 	int blockSize;   // # of bytes per block
 	int locRootDir;  // block # for start of root dir
	int freeSpace;   // # of blocks that represent free space bit map
	int signature;   // used to verify if volume has been formated/used
} VCB;


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n",
	 numberOfBlocks, blockSize);
	// number that will be checked against from volume
	int accMagicNum = 0b10101010101010101010110011011011;
	VCB *VCBP = (VCB*) malloc(blockSize);

	// value from volume(SampleVolume) casted to VCB struct at VCB location 0 
	LBAread(VCBP,1,0);

	// retreive the value at the signature
	int VBCMagicNum = VCBP->signature;

	// check if volume has been initilized before
	if(accMagicNum != VBCMagicNum){

		// fill VCB with signature so the VCB will be recognized as initilized
		VCBP->signature = accMagicNum;

		// get number of bytes that are required to represent the free space
		int freeSpaceBytes = (numberOfBlocks+7)/8;

		// fill VCB with information that will help reinililize the volume
		VCBP->totalBlocks = numberOfBlocks;
		VCBP->blockSize = blockSize;
		setBlockSize(VCBP->blockSize);
		// get the number of blocks that the free space will take up
		int freeSpaceBlocks = (freeSpaceBytes + (blockSize-1))/blockSize;

		// initilize free space bassed on this
		VCBP->freeSpace = freeSpaceBlocks;

		// initilize bit map for free space
		int retVal = initBitMap(blockSize*numberOfBlocks, freeSpaceBlocks);
		if(retVal < 0){
			free(VCBP);
			return -1;
		}

		// set the total number of used blocks 1 for VCB + freeSpaceBlocks
		for(int i = 0; i <= freeSpaceBlocks; i++){
			// changes bits to bitmap in memory
			setBit(i);
		}

		// add changes from memory bitmap to disk
		writeBits();

		//initilize and get position of root directory
		VCBP->locRootDir = initDir(50, NULL, blockSize);
		if(VCBP->locRootDir < 0){
			free(VCBP);
			return -2;
		}

		// set root directory to memory
		setRoot(DirToMem(VCBP->locRootDir));
		
		// write newly filled VCB struct to disk at block position 0
		LBAwrite(VCBP, 1, 0);

	}else{
		// restore data from VCB to reinitilize BitMap and root directory
		setBlockSize(VCBP->blockSize);
		reInitBitMap(blockSize*numberOfBlocks, VCBP->freeSpace);
		setRoot(DirToMem(VCBP->locRootDir));
		
	}
	// once data has been written and/or added to memory VCBP is no longer
	// needed
	free(VCBP);
	return 0;
	}

	


	
	
	void exitFileSystem ()
	{
	
	printf ("System exiting\n");
	}