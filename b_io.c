/**************************************************************
* Class::  CSC-415-01 Summer 2024
* Name:: Yuvraj Gupta, Fasika Abera, Sulav Jung Hamal, Miguel Maurer
* Student IDs:: 922933190, 923038932, 923075813, 922097199
* GitHub-Name:: YuvrajGupta1808, Fasikaabera, Sulavjung, miguelCmaurer
* Group-Name:: Satisfaction
* Project:: Basic File System
*
* File:: b_io.c
*
* Description:: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "fsDir.h"
#include <stdint.h>
#include <unistd.h>
#include "fsPath.h"
#include "fsBitmap.h"
#include "mfs.h"
#include "fsLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	int flag;
	char * path;
    int loc;
	u_int64_t offset;
	char * buf;		//holds the open file buffer
	int64_t block_index;
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	DirEntry * dirEntry;
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
        
    printf("b_open\n");
	b_io_fd returnFd;
    ppinfo* ppi = malloc(sizeof(ppinfo));
	if(ppi == NULL){
		perror("Memory allocation failed");
		return -1;
	}

        char* pathCpy = malloc(strlen(filename)+1);
        if(pathCpy == NULL){
            perror("Memory allocation failed");
            free(ppi);
            return -1;
        }
	
	    strcpy(pathCpy, filename);
        if (parsePath(pathCpy, ppi) < 0) {
        printf("Error parsing path: %s\n", filename);
        free(ppi);
        free(pathCpy);
        return -1;
    }

    int DEloc = findNameInDir(ppi->parent, ppi->lastElement);
    DirEntry* dirEntry = loadDir(ppi->parent, 0);
    int at = ppi->posInParent;
    // need to create
    if (DEloc == -1 && (flags & O_CREAT)) {
           
        at = findUnusedDE(dirEntry);
          if ((at == -1)) {
            free(ppi);
            free(pathCpy);
            printf("No free directory entry available.\n");
            return -1;
        }

        int newLoc = nextFree(100); // Find free block
        strncpy(dirEntry[at].name, ppi->lastElement, 254);

        dirEntry[at].creationTime = time(NULL);
        dirEntry[at].modificationTime = dirEntry->creationTime;
        dirEntry[at].accessTime = dirEntry->creationTime;
        dirEntry[at].permissions |= FILE | flags;
        dirEntry[at].size = 200;
        dirEntry[at].gg = 0;
        dirEntry[at].location = newLoc;


        for(int i = 0; i < 100; i++){
        setBit(newLoc+i);       // Mark block as used
        }
        writeBits(); // Update the free space bitmap
        LBAwrite(dirEntry, dirEntry->size, dirEntry->location);
    } else if (DEloc == -1) {
        free(ppi);
        free(pathCpy);
        return -1;
    }

    if (startup == 0) b_init();

    b_io_fd fd = b_getFCB();
    if (fd < 0) {
        free(ppi);
        free(pathCpy);
        return -1;
        }; // No available FCB
    
// if(entryIsDir(dirEntry)){
//     free(ppi);
//         free(pathCpy);
//         return -1;
// }
    fcbArray[fd].path = strdup(filename);
    fcbArray[fd].offset = (flags & O_APPEND) ? dirEntry->size : 0;
    fcbArray[fd].loc = at;
    fcbArray[fd].dirEntry = DirToMem(dirEntry->location);
    fcbArray[fd].block_index = dirEntry[at].location;
    fcbArray[fd].buf = (void*)-1;
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;
    fcbArray[fd].flag = flags;

    free(ppi);
	free(pathCpy);


	return (fd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
		
    switch (whence) {
        case SEEK_SET:
            fcbArray[fd].offset = offset;
            break;
        case SEEK_CUR:
            fcbArray[fd].offset += offset;
            break;
        case SEEK_END:
            fcbArray[fd].offset = fcbArray[fd].dirEntry->size + offset;
            break;
        default:
            return -1; // Invalid whence
    }

    if (fcbArray[fd].offset < 0) {
        fcbArray[fd].offset = 0;
        return -1; // Offset cannot be negative
    }

    return fcbArray[fd].offset;
}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}


    // Locate the FCB
    b_fcb *fcb = &fcbArray[fd];
    
    // Check if the file is read-only
    if (fcb->flag & O_RDONLY) {
        printf("b_write error: File is read-only.\n");
        return -1;
    }

    if(fcb->dirEntry[fcb->loc].gg + count > fcb->dirEntry[fcb->loc].size * B_CHUNK_SIZE ){
        //TODO 
        //COPY FILE TO NEW LOCATION WITH MORE SiZE
        printf("FILE \"%s\" @ %d | TOO BIG| size is: %d > %d\n",__FILE__, __LINE__,fcb->dirEntry[fcb->loc].gg, fcb->dirEntry[fcb->loc].size * B_CHUNK_SIZE);
    }

    if (fcb->buf == (char *)-1) {
        fcb->buf = malloc(B_CHUNK_SIZE);
        fcb->block_index = fcb->dirEntry[fcb->loc].location;
        LBAread(fcb->buf, 1, fcb->block_index);
        fcb->buflen = 0;
        fcb->dirEntry[fcb->loc].gg = 0;
    }

    // Update access time
    fcb->dirEntry->accessTime = time(NULL);

    // Update directory entry location and size
    int remaining = count;
    int p1Remainder = B_CHUNK_SIZE - fcb->buflen;
    int added = 0;
    // int offset = fcb->offset;
    int part1 = 0;
    int part2 = 0;
    int part3 = 0;

    
    part1 = p1Remainder < remaining? p1Remainder : remaining;
    memcpy(fcb->buf+fcb->buflen, buffer, part1);
    added+=part1;
    remaining -= part1;
    fcb->buflen += part1;

    if(remaining / B_CHUNK_SIZE < 0){
        LBAwrite(fcb->buf, 1, fcb->block_index);
        fcb->block_index += 1;
        int canAdd = remaining / B_CHUNK_SIZE;
        LBAwrite(buffer+added, canAdd, fcb->block_index);
        fcb->block_index += canAdd;
        part2 = canAdd * B_CHUNK_SIZE;
        remaining -= canAdd * B_CHUNK_SIZE;
        fcb->buflen = 0;
        added += part2;
    }

     if(remaining > 0){
        part3 = remaining;
        LBAwrite(fcb->buf, 1, fcb->block_index);
        fcb->block_index += 1;
        LBAread(fcb->buf, 1, fcb->block_index);
        memcpy(fcb->buf, buffer+added, part3);
        fcb->buflen = part3;
        added+=part3;
        remaining = 0;
    }
    LBAwrite(fcb->buf, 1, fcb->block_index);
    

    fcb->dirEntry[fcb->loc].gg += added;
    // printf("File Size is: %d @ %d\n", fcb->dirEntry->size, fcb->dirEntry->location);

    LBAwrite(fcb->dirEntry, fcb->dirEntry->size, fcb->dirEntry->location);
    return added; // return the number of bytes written
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{
    
	if (startup == 0) b_init();  //Initialize our system
    

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

    b_fcb *fcb = &fcbArray[fd];
    if (fcb->buf == (char *)-1) {
        fcb->buf = malloc(B_CHUNK_SIZE);
        fcb->block_index = fcb->dirEntry[fcb->loc].location;
        LBAread(fcb->buf, 1, fcb->block_index);
        fcb->block_index++;
        fcb->buflen = 0;
        fcb->offset = 0;
    }

    // DEF: offset = current pos in file
    int fileSize = fcb->dirEntry[fcb->loc].gg;

    // printf("FILE SIZE: %d\n",fileSize);

    // Prevent reading past the end of the file
    if (fcb->offset + count > fileSize) {
       count = fileSize - fcb->offset;
    }

    // Check if at end of file
    if (fcb->offset >= fileSize) {
        return 0; // EOF
    }

    int needToBeRead = count;
    int bufferWriteIndex = 0;
    int part1, part2, part3;

    part1 = B_CHUNK_SIZE - fcb->buflen;
    part1 = (needToBeRead < part1) ? needToBeRead : part1;
    needToBeRead -= part1;
    part2 = needToBeRead - (needToBeRead % B_CHUNK_SIZE);
    needToBeRead -= part2;      
    part3 = needToBeRead;
        
        // Part 1: Read from existing buffer
        if (part1 > 0) {
            memcpy(buffer+bufferWriteIndex, fcb->buf+fcb->buflen, part1);
            fcb->buflen += part1;
            fcb->offset += part1;
            bufferWriteIndex += part1;
            count -= part1;
        }

        // Part 2: Read full blocks directly to buffer if needed
        if (part2 > 0) {
            int blocks = part2 / B_CHUNK_SIZE;
            LBAread(buffer+bufferWriteIndex, blocks, fcb->block_index+1);
            fcb->block_index += blocks;
            bufferWriteIndex += part2;
            fcb->offset += part2;
            count -= part2;
        }

        // Part 3: Read remaining bytes into buffer and copy to user buffer

        if (part3 > 0) {
            LBAread(fcb->buf, 1, fcb->block_index);
            memcpy(buffer+bufferWriteIndex, fcb->buf, part3);
            fcb->block_index++;
            fcb->buflen = part3;
            fcb->offset += part3;
            bufferWriteIndex += part3;
            count -= part3;
        }
    

    // Update access time
    fcb->dirEntry->accessTime = time(NULL);

    return bufferWriteIndex;
	}


// Interface to Close the file	
int b_close (b_io_fd fd)
	{

		// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
	}
    
	fcbArray[fd].dirEntry->modificationTime = time(NULL);
    // Update the directory entry in its parent directory
    if(fcbArray[fd].buf != (char*)-1){
    free(fcbArray[fd].buf);
    }
    free(fcbArray[fd].path);
    free(fcbArray[fd].dirEntry);
    // Clear the FCB
    fcbArray[fd].path = NULL;
    fcbArray[fd].buf = NULL;
    fcbArray[fd].dirEntry = NULL;
    fcbArray[fd].flag = 0;
    fcbArray[fd].offset = 0;
    fcbArray[fd].block_index = -1;
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;
	return 0;
	}
