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
	b_io_fd returnFd;

    if (!(flags & (O_RDONLY | O_WRONLY | O_RDWR | O_CREAT | O_APPEND | O_TRUNC))) {
        printf("Invalid flags: %d\n", flags);
        return -1;
    }

    char *pathCpy = strdup(filename);
    ppinfo ppi;
    if (parsePath(pathCpy, &ppi) < 0) {
        printf("Error parsing path: %s\n", filename);
        free(pathCpy);
        return -1;
    }
    free(pathCpy);

    DirEntry *dirEntry = findNameInDir(ppi.parent, ppi.lastElement) != -1 ? &ppi.parent[ppi.posInParent] : NULL;

    if (!dirEntry && (flags & O_CREAT)) {
        if ((ppi.posInParent = findUnusedDE(ppi.parent)) == -1) {
            printf("No free directory entry available.\n");
            return -1;
        }

        dirEntry = &ppi.parent[ppi.posInParent];
        strcpy(dirEntry->name, ppi.lastElement);
        dirEntry->size = 0;
        dirEntry->location = nextFree(1); // Find free block
        setBit(dirEntry->location);       // Mark block as used
        dirEntry->creationTime = time(NULL);
        dirEntry->modificationTime = dirEntry->creationTime;
        dirEntry->accessTime = dirEntry->creationTime;
        dirEntry->permissions = flags;

        writeBits(); // Update the free space bitmap
        fs_setcwd(ppi.parent->name); // Update parent directory
    } else if (!dirEntry) {
        printf("File not found: %s\n", filename);
        return -1;
    }

    b_io_fd fd = b_getFCB();
    if (fd < 0) return -1; // No available FCB

    fcbArray[fd].path = strdup(filename);
    fcbArray[fd].offset = (flags & O_APPEND) ? dirEntry->size : 0;
    fcbArray[fd].dirEntry = dirEntry;
    fcbArray[fd].block_index = -1;
    fcbArray[fd].buf = malloc(B_CHUNK_SIZE);
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;
    fcbArray[fd].flag = flags;

    if (flags & O_TRUNC) {
        fcbArray[fd].dirEntry->size = 0;
    }

	
	return (returnFd);						// all set
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

    // Update access time
    fcb->dirEntry->accessTime = time(NULL);
    uint64_t newSize = fcb->offset + count;

    // Calculate the number of blocks required
    uint64_t requiredBlocks = (newSize + B_CHUNK_SIZE - 1) / B_CHUNK_SIZE;

    int64_t *positions = (int64_t *)malloc(requiredBlocks * sizeof(int64_t));
    if (positions == NULL) {
        printf("Memory allocation failed for file blocks.\n");
        return -1;
    }

    // Update directory entry location and size
    fcb->dirEntry->location = positions[0];
    fcb->dirEntry->size = newSize;

    uint64_t remaining = count;
    char *data = buffer;

    while (remaining > 0) {
        uint64_t toCopy = remaining;
        uint64_t spaceLeft = B_CHUNK_SIZE - fcb->index;

        if (toCopy > spaceLeft) {
            toCopy = spaceLeft;
        }

        int64_t blockPos = positions[fcb->offset / B_CHUNK_SIZE];

        if (fcb->block_index != blockPos) {
            LBAread(fcb->buf, 1, blockPos);
            fcb->block_index = blockPos;
        }

        memcpy(&fcb->buf[fcb->index], &data[count - remaining], toCopy);
        LBAwrite(fcb->buf, 1, blockPos);

        fcb->index += toCopy;
        remaining -= toCopy;

        if (fcb->index >= B_CHUNK_SIZE) {
            fcb->index = 0;
            fcb->block_index = -1;
        }

        fcb->offset += toCopy;
        if (fcb->offset > fcb->dirEntry->size) {
            fcb->dirEntry->size = fcb->offset;
        }
    }

    return count; // Return the number of bytes written
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

    // Check if file is write-only
    if (fcb->flag & O_WRONLY) {
        printf("b_read error: File is write-only.\n");
        return -1;
    }

    // Prevent reading past the end of the file
    if (fcb->offset + count > fcb->dirEntry->size) {
        count = fcb->dirEntry->size - fcb->offset;
    }

    // Check if at end of file
    if (fcb->offset >= fcb->dirEntry->size) {
        return 0; // EOF
    }

    int needToBeRead = count;
    int bufferWriteIndex = 0;
    int part1, part2, part3;

    while (needToBeRead > 0) {
        // Part 1: Read from existing buffer
        part1 = B_CHUNK_SIZE - fcb->index;
        part1 = (needToBeRead < part1) ? needToBeRead : part1;

        if (part1 > 0) {
            memcpy(&buffer[bufferWriteIndex], &fcb->buf[fcb->index], part1);
            fcb->index += part1;
            fcb->offset += part1;
            bufferWriteIndex += part1;
            needToBeRead -= part1;
        }

        // Part 2: Read full blocks directly to buffer if needed
        part2 = needToBeRead - (needToBeRead % B_CHUNK_SIZE);

        if (part2 > 0) {
            int blockToRead = fcb->dirEntry->location + (fcb->offset / B_CHUNK_SIZE);
            int blocks = part2 / B_CHUNK_SIZE;
            LBAread(&buffer[bufferWriteIndex], blocks, blockToRead);
            fcb->offset += part2;
            bufferWriteIndex += part2;
            needToBeRead -= part2;
        }

        // Part 3: Read remaining bytes into buffer and copy to user buffer
        part3 = needToBeRead;

        if (part3 > 0) {
            int blockToRead = fcb->dirEntry->location + (fcb->offset / B_CHUNK_SIZE);

            if (fcb->block_index != blockToRead || fcb->index >= B_CHUNK_SIZE) {
                LBAread(fcb->buf, 1, blockToRead);
                fcb->block_index = blockToRead;
                fcb->index = 0;
            }

            memcpy(&buffer[bufferWriteIndex], &fcb->buf[fcb->index], part3);
            fcb->index += part3;
            fcb->offset += part3;
            bufferWriteIndex += part3;
            needToBeRead -= part3;
        }
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
	if (fcbArray[fd].dirEntry == NULL){
		return 0;
	}

	fcbArray[fd].dirEntry->modificationTime = time(NULL);
    // Update the directory entry in its parent directory
    char *pathCpy = strdup(fcbArray[fd].path);
    ppinfo ppi;
    if (parsePath(pathCpy, &ppi) >= 0)
    {
        // Update the parent directory entry
        ppi.parent[ppi.posInParent] = *fcbArray[fd].dirEntry;
        freeIfNotNeeded(ppi.parent);
    }
    free(fcbArray[fd].path);
    free(fcbArray[fd].buf);

    // Clear the FCB
    fcbArray[fd].path = NULL;
    fcbArray[fd].buf = NULL;
    fcbArray[fd].dirEntry = NULL;
    fcbArray[fd].flag = 0;
    fcbArray[fd].offset = 0;
    fcbArray[fd].block_index = -1;
    fcbArray[fd].index = 0;
    fcbArray[fd].buflen = 0;

	}
