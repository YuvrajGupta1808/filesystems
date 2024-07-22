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

DirEntry* root;
DirEntry* currentDir;
int gBlockSize = -1;


int initDir(int minNum, DirEntry* parent, int blockSize){
		// sets a global variable to callers blockSize to be used by other
		// functions
		if(gBlockSize == -1){
			gBlockSize = blockSize;
		}

        // convert the number of bytes -> blocks, so we can allocate memory
		int numBytes = minNum * sizeof(DirEntry);
		int numBlocks = (numBytes + (blockSize-1))/blockSize;
		int bytesToAlloc = numBlocks * blockSize;
		
		DirEntry* DEP = malloc(bytesToAlloc);
		if(DEP == NULL){
			return -1;
		}

        // get total number of directorys that will fit in allocated memory
		int accDe = bytesToAlloc/sizeof(DirEntry);
		int newLoc = nextFree(numBlocks);
		if(newLoc ==-1){
			//handle error (compact data or quit)
			return -2;
		}

        // update freespace bitmap in memory
		for(int i = 0; i < numBlocks; i++){
			setBit(newLoc+i);
		}

		// update freespace bitmap on disk
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
		strcpy(DEP[0].name,".");

		DirEntry* dotdot = parent;

        // check for root direcotry
		if(dotdot == NULL){
			dotdot = DEP;
		}
    
		// copy parents info to '..' DE
		memcpy(&DEP[1], dotdot, sizeof(DirEntry));
		strcpy(DEP[1].name,"..");

		// save directory to disk
		LBAwrite(DEP,numBlocks,newLoc);

		//DEP memory is no longer needed
		free(DEP);
		return newLoc;
	}

DirEntry* DirToMem(int startBlock){
	int blockSize = getBlockSize();
	// malloc enough memory for '.' directory which is a refrence to the values
	// of itself, we use these valeus to get the size and location of the
	// entire directory into memory
	DirEntry* temp = malloc(blockSize);
	if(temp == NULL){
		return NULL;
	}

	LBAread(temp,1,startBlock);
	int size = temp->size;
	int location = temp->location;
	// free memory used to temporaraly store '.'
	free(temp);
	// use info from '.' to allocate an needed amount of memory
	temp = malloc(size*blockSize);
	if(temp == NULL){
		return NULL;
	}
	// fill temp with data for the entire directory
	LBAread(temp, size, location);
	return temp;
}

int findNameInDir(DirEntry* parent, char* value){
	// the total number of DE's in parent directory, used for itteration 
	int numDE = (parent[0].size * gBlockSize)/sizeof(DirEntry);
	for(int i = 0; i < numDE; i++){
		if(strcmp(parent[i].name, value) == 0){
			return i;
		}
	}
	
	return -1;
}

int findUnusedDE(DirEntry* parent){
	// the total number of DE's in parent directory, used for itteration 
	int numDE = (parent[0].size * gBlockSize)/sizeof(DirEntry);
	for(int i = 0; i < numDE; i++){
		// location 0 is reserved for VCB, so it is always invalid for DE
		if(parent[i].location == 0){
			return i;
		}
	}
	return -1;
}

int entryIsDir(DirEntry* parent, int index){
	// checks if bit repersenting directory is 0
	return ((parent[index].permissions & 1<<31) != 0);
}

DirEntry* getRoot(){
	return root;
}

DirEntry* loadDir(DirEntry* parent, int pos){
	// check if dir is root or CWD
	if(parent[pos].location == root->location){
		return root;
	}

	if(currentDir->location == parent[pos].location){
		return currentDir;
	}
	return DirToMem(parent[pos].location);
}

void freeIfNotNeeded(DirEntry* parent){
	// not root
	if(parent->location == root->location){
		return;
	}
	// not cwd
	if(parent->location == currentDir->location){
		return; 
	}
	free(parent);
}

DirEntry* getCWD(){
	return currentDir;
}

void setCWD(DirEntry* value){
	// checks if cwd is root
	if(currentDir->location != root->location){
		// if not free is okay
		free(currentDir);
	}
	currentDir = value;
}

int getBlockSize(){
	return gBlockSize;
}

void setBlockSize(int blockSize){
	gBlockSize = blockSize;
}

void setRoot(DirEntry* setRoot){
	currentDir = setRoot;
 	root = setRoot;
}
