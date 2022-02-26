///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020-2021 Deb Deppeler based on work by Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission Fall 2020, CS354-deppeler
//
///////////////////////////////////////////////////////////////////////////////
// Main File:        myHeap.c
// This File:        myHeap.c
// Other Files:      (name of all other files if any)
// Semester:         CS 354 Spring 2021
//
// Author:           Yating Tian
// Email:            ytian83@wisc.edu
// CS Login:         yating
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   Fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   Avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   of any information you find.
////////////////////////////////////////////////////////////////////////////////

// DEB'S PARTIAL SOLUTION FOR SPRING 2021 DO NOT SHARE
 
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include "myHeap.h"
 
/*
 * This structure serves as the header for each allocated and free block.
 * It also serves as the footer for each free block but only containing size.
 */
typedef struct blockHeader {           

    int size_status;
    /*
     * Size of the block is always a multiple of 8.
     * Size is stored in all block headers and in free block footers.
     *
     * Status is stored only in headers using the two least significant bits.
     *   Bit0 => least significant bit, last bit
     *   Bit0 == 0 => free block
     *   Bit0 == 1 => allocated block
     *
     *   Bit1 => second last bit 
     *   Bit1 == 0 => previous block is free
     *   Bit1 == 1 => previous block is allocated
     * 
     * End Mark: 
     *  The end of the available memory is indicated using a size_status of 1.
     * 
     * Examples:
     * 
     * 1. Allocated block of size 24 bytes:
     *    Allocated Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 25
     *      If the previous block is allocated p-bit=1 size_status would be 27
     * 
     * 2. Free block of size 24 bytes:
     *    Free Block Header:
     *      If the previous block is free      p-bit=0 size_status would be 24
     *      If the previous block is allocated p-bit=1 size_status would be 26
     *    Free Block Footer:
     *      size_status should be 24
     */
} blockHeader;         

/* Global variable - DO NOT CHANGE. It should always point to the first block,
 * i.e., the block at the lowest address.
 */
blockHeader *heapStart = NULL;     

/* Size of heap allocation padded to round to nearest page size.
 */
int allocsize;

/*
 * Additional global variables may be added as needed below
 */

 
/* 
 * Function for allocating 'size' bytes of heap memory.
 * Argument size: requested size for the payload
 * Returns address of allocated block (payload) on success.
 * Returns NULL on failure.
 *
 * This function must:
 * - Check size - Return NULL if not positive or if larger than heap space.
 * - Determine block size rounding up to a multiple of 8 
 *   and possibly adding padding as a result.
 *
 * - Use BEST-FIT PLACEMENT POLICY to chose a free block
 *
 * - If the BEST-FIT block that is found is exact size match
 *   - 1. Update all heap blocks as needed for any affected blocks
 *   - 2. Return the address of the allocated block payload
 *
 * - If the BEST-FIT block that is found is large enough to split 
 *   - 1. SPLIT the free block into two valid heap blocks:
 *         1. an allocated block
 *         2. a free block
 *         NOTE: both blocks must meet heap block requirements 
 *       - Update all heap block header(s) and footer(s) 
 *              as needed for any affected blocks.
 *   - 2. Return the address of the allocated block payload
 *
 * - If a BEST-FIT block found is NOT found, return NULL
 *   Return NULL unable to find and allocate block for desired size
 *
 * Note: payload address that is returned is NOT the address of the
 *       block header.  It is the address of the start of the 
 *       available memory for the requesterr.
 *
 * Tips: Be careful with pointer arithmetic and scale factors.
 */
void* myAlloc(int size) {     
	blockHeader *ptr;
    int block_size;
    ptr =NULL;

    //check size and compute block size 
    if (size <= 0 || size > allocsize){
        return NULL;
    }
    block_size = sizeof(int) + size ;
    if (block_size % 8 != 0 ){
        block_size += 8-block_size % 8;
    }
    
    //find the perfect block, if not perfect, return 
    blockHeader *curr = heapStart; //runner 
    blockHeader *best = NULL; //track the second best 
    int best_size = allocsize;
    int curr_size_nopa= 0; 
    while ((curr->size_status) != 1){
        curr_size_nopa = curr->size_status >> 2 << 2;
        if  ((curr->size_status & 1) == 1 || curr_size_nopa < block_size ){ //allocated or not large enough
            curr = (blockHeader*)((void*)curr+curr_size_nopa);
        }
        else{//free and large enough 
            if ( curr_size_nopa == block_size){ //find perfect fit case 
                ptr = curr;
                break;  
            }
            else{ //find best fit case 
                if (curr_size_nopa >= block_size + 8 && best_size >= curr_size_nopa &&(curr->size_status & 1) == 0 ){ //if big enough
                    best=curr;
                    best->size_status= curr->size_status;
                    best_size=curr_size_nopa;
                }
                curr=(blockHeader*)((void*)curr + curr_size_nopa);
            } 
        }
        
    }
    if (ptr == NULL && best==NULL) {
        return NULL;
    }
    //after find perfect match or second best match, update all of the related blocks
    if (curr_size_nopa == block_size){ //perfict match case 
        blockHeader *next=(blockHeader*)((void*)ptr+ curr_size_nopa);
        ptr->size_status=curr->size_status;
        ptr->size_status+=1;
        if (next->size_status!=1){
            next->size_status+=2;
        }
        return (blockHeader*)((void*)ptr + sizeof(blockHeader));
    }
    else if (best_size >= block_size+8 && best_size <= allocsize){ //split case 
        ptr=best;
        ptr->size_status= block_size + ((best->size_status) % 8);
        ptr->size_status+=1;
        blockHeader *second=(blockHeader*)((void*)ptr+ block_size);
        second->size_status=best_size-block_size;
        second->size_status+=2;
        blockHeader *footer=(blockHeader*)((void*)second+ ((best_size-block_size)-4));
        footer->size_status = best_size-block_size;
        return (blockHeader*)((void*)ptr + sizeof(blockHeader));
    }
    return NULL;
} 
 
/* 
 * Function for freeing up a previously allocated block.
 * Argument ptr: address of the block to be freed up.
 * Returns 0 on success.
 * Returns -1 on failure.
 * This function should:
 * - Return -1 if ptr is NULL.
 * - Return -1 if ptr is not a multiple of 8.
 * - Return -1 if ptr is outside of the heap space.
 * - Return -1 if ptr block is already freed.
 * - Update header(s) and footer as needed.
 */                    
int myFree(void *ptr) {    
    blockHeader* pointer =(blockHeader*)((void*)ptr-4);
    blockHeader *endmark=(blockHeader*)((void*)heapStart + allocsize);
    int p_bit= (((blockHeader*)((void*)ptr-4))->size_status)%8;
    if (ptr==NULL||(int) ptr % 8 != 0 ||pointer<heapStart|| pointer>endmark || p_bit==0 || p_bit ==2){
        return -1;
    }
    else{
        int block_size= pointer->size_status>>2<<2;
        pointer->size_status-=1;

        blockHeader *footer=(blockHeader*)((void*)pointer+block_size-4);
        footer->size_status=block_size; 

        blockHeader *next=(blockHeader*)((void*)pointer+block_size);
        if (next->size_status!= 1 && next<endmark){
            next->size_status-=2;
        }
        return 0;
    }
    return -1;
} 

/*
 * Function for traversing heap block list and coalescing all adjacent 
 * free blocks.
 *
 * This function is used for delayed coalescing.
 * Updated header size_status and footer size_status as needed.
 */
int coalesce() {
    blockHeader *curr = heapStart; //runner 
    int block_size=0;
    int next_size=0;
    blockHeader *next=NULL;
    int re=0;
    while ((curr->size_status) != 1){
        block_size=curr->size_status>>2 <<2;
         if ((curr->size_status & 1) == 1){ //alloced case 
            curr=(blockHeader*)((void*)curr+block_size);     
            continue;
         }
         else {    //free case 
            next=(blockHeader*)((void*)curr+block_size);
            if (next->size_status==1){
                break;
            }
            int running_size=0;
            while ((next->size_status & 1) == 0 && next->size_status != 1){ //get the follow free blocks
                next_size+=next->size_status>>2<<2;
                running_size=next->size_status>>2<<2;
                next=(blockHeader*)((void*)next+running_size);
                re+=1;
            }
        }
        //update the related headers and footers 
        curr->size_status=block_size+next_size;
        curr->size_status+=2;
        blockHeader *footer=(blockHeader*)((void*)next-4);
        footer->size_status=block_size+next_size;
        curr=(blockHeader*)((void*)next);
    
    }
    return re;
    
}

 
/* 
 * Function used to initialize the memory allocator.
 * Intended to be called ONLY once by a program.
 * Argument sizeOfRegion: the size of the heap space to be allocated.
 * Returns 0 on success.
 * Returns -1 on failure.
 */                    
int myInit(int sizeOfRegion) {    
 
    static int allocated_once = 0; //prevent multiple myInit calls
 
    int pagesize;   // page size
    int padsize;    // size of padding when heap size not a multiple of page size
    void* mmap_ptr; // pointer to memory mapped area
    int fd;

    blockHeader* endMark;
  
    if (0 != allocated_once) {
        fprintf(stderr, 
        "Error:mem.c: InitHeap has allocated space during a previous call\n");
        return -1;
    }

    if (sizeOfRegion <= 0) {
        fprintf(stderr, "Error:mem.c: Requested block size is not positive\n");
        return -1;
    }

    // Get the pagesize
    pagesize = getpagesize();

    // Calculate padsize as the padding required to round up sizeOfRegion 
    // to a multiple of pagesize
    padsize = sizeOfRegion % pagesize;
    padsize = (pagesize - padsize) % pagesize;

    allocsize = sizeOfRegion + padsize;

    // Using mmap to allocate memory
    fd = open("/dev/zero", O_RDWR);
    if (-1 == fd) {
        fprintf(stderr, "Error:mem.c: Cannot open /dev/zero\n");
        return -1;
    }
    mmap_ptr = mmap(NULL, allocsize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (MAP_FAILED == mmap_ptr) {
        fprintf(stderr, "Error:mem.c: mmap cannot allocate space\n");
        allocated_once = 0;
        return -1;
    }
  
    allocated_once = 1;

    // for double word alignment and end mark
    allocsize -= 8;

    // Initially there is only one big free block in the heap.
    // Skip first 4 bytes for double word alignment requirement.
    heapStart = (blockHeader*) mmap_ptr + 1;

    // Set the end mark
    endMark = (blockHeader*)((void*)heapStart + allocsize);
    endMark->size_status = 1;

    // Set size in header
    heapStart->size_status = allocsize;

    // Set p-bit as allocated in header
    // note a-bit left at 0 for free
    heapStart->size_status += 2;

    // Set the footer
    blockHeader *footer = (blockHeader*) ((void*)heapStart + allocsize - 4);
    footer->size_status = allocsize;
  
    return 0;
} 
                  
/* 
 * Function to be used for DEBUGGING to help you visualize your heap structure.
 * Prints out a list of all the blocks including this information:
 * No.      : serial number of the block 
 * Status   : free/used (allocated)
 * Prev     : status of previous block free/used (allocated)
 * t_Begin  : address of the first byte in the block (where the header starts) 
 * t_End    : address of the last byte in the block 
 * t_Size   : size of the block as stored in the block header
 */                     
void dispMem() {     
 
    int counter;
    char status[6];
    char p_status[6];
    char *t_begin = NULL;
    char *t_end   = NULL;
    int t_size;

    blockHeader *current = heapStart;
    counter = 1;

    int used_size = 0;
    int free_size = 0;
    int is_used   = -1;

    fprintf(stdout, 
	"*********************************** Block List **********************************\n");
    fprintf(stdout, "No.\tStatus\tPrev\tt_Begin\t\tt_End\t\tt_Size\n");
    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
  
    while (current->size_status != 1) {
        t_begin = (char*)current;
        t_size = current->size_status;
    
        if (t_size & 1) {
            // LSB = 1 => used block
            strcpy(status, "alloc");
            is_used = 1;
            t_size = t_size - 1;
        } else {
            strcpy(status, "FREE ");
            is_used = 0;
        }

        if (t_size & 2) {
            strcpy(p_status, "alloc");
            t_size = t_size - 2;
        } else {
            strcpy(p_status, "FREE ");
        }

        if (is_used) 
            used_size += t_size;
        else 
            free_size += t_size;

        t_end = t_begin + t_size - 1;
    
        fprintf(stdout, "%d\t%s\t%s\t0x%08lx\t0x%08lx\t%4i\n", counter, status, 
        p_status, (unsigned long int)t_begin, (unsigned long int)t_end, t_size);
    
        current = (blockHeader*)((char*)current + t_size);
        counter = counter + 1;
    }

    fprintf(stdout, 
	"---------------------------------------------------------------------------------\n");
    fprintf(stdout, 
	"*********************************************************************************\n");
    fprintf(stdout, "Total used size = %4d\n", used_size);
    fprintf(stdout, "Total free size = %4d\n", free_size);
    fprintf(stdout, "Total size      = %4d\n", used_size + free_size);
    fprintf(stdout, 
	"*********************************************************************************\n");
    fflush(stdout);

    return;  
} 


// end of myHeap.c (sp 2021)                                         

