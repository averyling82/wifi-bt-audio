/*
    FreeRTOS V7.3.0 - Copyright (C) 2012 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest versions, license
    and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

/*
 * A sample implementation of pvPortMalloc() and vPortFree() that combines
 * (coalescences) adjacent memory blocks as they are freed, and in so doing
 * limits memory fragmentation.
 *
 * See heap_1.c, heap_2.c and heap_3.c for alternative implementations, and the
 * memory management pages of http://www.FreeRTOS.org for more information.
 */
#include <stdlib.h>

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
all the API functions to use the MPU wrappers.  That should only be done when
task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

#include "FreeRTOS.h"
#include "task.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE    ( ( size_t ) ( heapSTRUCT_SIZE * 2 ) )


extern void IntDefaultHandler_displaytasklist (void);



/* Allocate the memory for the heap.  The struct is used to force byte
alignment without using any non-portable code. */
__attribute__((section("HeapStack"),zero_init)) __align(8)
static union xRTOS_HEAP
{
    #if portBYTE_ALIGNMENT == 8
        //volatile portDOUBLE dDummy;
      char Dummy[8];
    #else
        volatile unsigned long ulDummy;
    #endif
    unsigned char ucHeap[ configTOTAL_HEAP_SIZE ];
} xHeap;


__attribute__((section("AppHeapStack"),zero_init)) __align(8)
static union xRTOS_APP_HEAP
{
    #if portBYTE_ALIGNMENT == 8
        //volatile portDOUBLE dDummy;
      char Dummy[8];
    #else
        volatile unsigned long ulDummy;
    #endif
    unsigned char ucHeap[ configTOTAL_APP_HEAP_SIZE];
} xAppHeap;


/*-----------------------------------------------------------*/

/*
 * Inserts a block of memory that is being freed into the correct position in
 * the list of free memory blocks.  The block being freed will be merged with
 * the block in front it and/or the block behind it if the memory blocks are
 * adjacent to each other.
 */
static void prvInsertBlockIntoFreeList( xBlockLink *pxBlockToInsert, size_t mempool);

/*
 * Called automatically to setup the required heap structures the first time
 * pvPortMalloc() is called.
 */
static void prvHeapInit( void );

/*-----------------------------------------------------------*/

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const unsigned short heapSTRUCT_SIZE    = sizeof( xBlockLink );//( sizeof( xBlockLink ) + portBYTE_ALIGNMENT - ( sizeof( xBlockLink ) % portBYTE_ALIGNMENT ) );

/* Ensure the pxEnd pointer will end up on the correct byte alignment. */
const size_t xTotalHeapSize[2] = {  ( ( size_t ) configTOTAL_HEAP_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK ),
                                    ( ( size_t ) configTOTAL_APP_HEAP_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK )};

const size_t xHeapFirstAddr[2] = {(size_t)&xHeap, (size_t)&xAppHeap};
/* Create a couple of list links to mark the start and end of the list. */
static xBlockLink xStart[2], *pxEnd[2] = {NULL, NULL};

/* Keeps track of the number of free bytes remaining, but says nothing about
fragmentation. */
static size_t xFreeBytesRemaining[2] = {  ( ( size_t ) configTOTAL_HEAP_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK ),
                                    ( ( size_t ) configTOTAL_APP_HEAP_SIZE ) & ( ( size_t ) ~portBYTE_ALIGNMENT_MASK )};

/* STATIC FUNCTIONS ARE DEFINED AS MACROS TO MINIMIZE THE FUNCTION CALL DEPTH. */

/*-----------------------------------------------------------*/
void *pvPortMalloc( size_t xWantedSize, size_t mempool)
{
xBlockLink *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
void *pvReturn = NULL;

    vTaskSuspendAll();
    {
        /* If this is the first call to malloc then the heap will require
        initialisation to setup the list of free blocks. */
        #if config_HEAP_BLOCK == 1
        mempool = 1;
        #endif
        if( pxEnd[mempool] == NULL )
        {
            prvHeapInit();
        }

        /* The wanted size is increased so it can contain a xBlockLink
        structure in addition to the requested amount of bytes. */
        if( xWantedSize > 0 )
        {
            xWantedSize += heapSTRUCT_SIZE;

            /* Ensure that blocks are always aligned to the required number of
            bytes. */
            if( xWantedSize & portBYTE_ALIGNMENT_MASK )
            {
                /* Byte alignment required. */
                xWantedSize += ( portBYTE_ALIGNMENT - ( xWantedSize & portBYTE_ALIGNMENT_MASK ) );
            }
        }

        if( ( xWantedSize > 0 ) && ( xWantedSize < xTotalHeapSize[mempool] ) )
        {
            /* Traverse the list from the start    (lowest address) block until one
            of adequate size is found. */
            pxPreviousBlock = &xStart[mempool];
            pxBlock = xStart[mempool].pxNextFreeBlock;
            while( ( pxBlock->xBlockSize < xWantedSize ) && ( pxBlock->pxNextFreeBlock != NULL ) )
            {
                pxPreviousBlock = pxBlock;
                pxBlock = pxBlock->pxNextFreeBlock;
            }

            /* If the end marker was reached then a block of adequate size was
            not found. */
            if( pxBlock != pxEnd[mempool] )
            {
                /* Return the memory space - jumping over the xBlockLink structure
                at its start. */
                pvReturn = ( void * ) ( ( ( unsigned char * ) pxPreviousBlock->pxNextFreeBlock ) + heapSTRUCT_SIZE );

                /* This block is being returned for use so must be taken out of
                the    list of free blocks. */
                pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                /* If the block is larger than required it can be split into two. */
                if( ( pxBlock->xBlockSize - xWantedSize ) > heapMINIMUM_BLOCK_SIZE )
                {
                    /* This block is to be split into two.  Create a new block
                    following the number of bytes requested. The void cast is
                    used to prevent byte alignment warnings from the compiler. */
                    pxNewBlockLink = ( void * ) ( ( ( unsigned char * ) pxBlock ) + xWantedSize );

                    /* Calculate the sizes of two blocks split from the single
                    block. */
                    pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                    pxBlock->xBlockSize = xWantedSize;

                    /* Insert the new block into the list of free blocks. */
                    prvInsertBlockIntoFreeList( ( pxNewBlockLink ), mempool);
                }

                xFreeBytesRemaining[mempool] -= pxBlock->xBlockSize;
            }
        }
    }

#ifdef 0
    //printf("pvPortMalloc = 0x%04x , xWantedSize = %d\n", pvReturn, xWantedSize);
    if(memory_check_flag)
    {
        int i=0;
        unsigned int *sp = 0;
        for(i=0;i<MEMORY_CHECK_MAX;i++)
        {
            if(memory_check_buf[i] == 0)
            {
                memory_check_buf[i] = (size_t)pvReturn;
                memcpy(memory_check_task_buf[i],pxCurrentTCB->pcTaskName,16);
                break;
            }

            if(i == MEMORY_CHECK_MAX)
            {
                printf("memory_check buf overflow\n");
            }
        }
    }
#endif

    xTaskResumeAll();

    #if( configUSE_MALLOC_FAILED_HOOK == 1 )
    {
        if( (xWantedSize) && (pvReturn == NULL ))
        {
            //extern void vApplicationMallocFailedHook( void );
            //vApplicationMallocFailedHook();
            printf("\nheap4 xWantedSize = %d, xFreeBytesRemaining = %d, mempool = %d\n", xWantedSize, xFreeBytesRemaining[mempool], mempool);
            return NULL;
            //IntDefaultHandler_displaytasklist();
            //while(1);
        }
    }
    #endif

    return pvReturn;
}
/*-----------------------------------------------------------*/

void vPortFree( void *pv, size_t mempool)
{
unsigned char *puc = ( unsigned char * ) pv;
xBlockLink *pxLink;

    if( pv != NULL )
    {

        /* The memory being freed will have an xBlockLink structure immediately
        before it. */
        puc -= heapSTRUCT_SIZE;

        /* This casting is to keep the compiler from issuing warnings. */
        pxLink = ( void * ) puc;

        vTaskSuspendAll();
        #if config_HEAP_BLOCK == 1
        mempool = 1;
        #endif
#ifdef 0
        //printf("vPortFree = 0x%04x\n", pv);

        if(memory_check_flag)
        {
            int i=0;
            for(i=0;i<MEMORY_CHECK_MAX;i++)
            {
                if(memory_check_buf[i] == (size_t)pv)
                {
                    memory_check_buf[i] = 0;
                    break;
                }
            }
        }
#endif

        {
            /* Add this block to the list of free blocks. */
            xFreeBytesRemaining[mempool] += pxLink->xBlockSize;
            prvInsertBlockIntoFreeList( ( ( xBlockLink * ) pxLink ), mempool);
        }
        xTaskResumeAll();
    }
}
/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( size_t mempool )
{
    return xFreeBytesRemaining[mempool];
}
/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void )
{
    /* This just exists to keep the linker quiet. */
}
/*-----------------------------------------------------------*/

static void prvHeapInit( void )
{
xBlockLink *pxFirstFreeBlock;
unsigned char *pucHeapEnd;

    /* Ensure the start of the heap is aligned. */
    configASSERT( ( ( ( unsigned long ) xAppHeap.ucHeap ) & ( ( unsigned long ) portBYTE_ALIGNMENT_MASK ) ) == 0UL );
    configASSERT( ( ( ( unsigned long ) xHeap.ucHeap ) & ( ( unsigned long ) portBYTE_ALIGNMENT_MASK ) ) == 0UL );

    /* xStart is used to hold a pointer to the first item in the list of free
    blocks.  The void cast is used to prevent compiler warnings. */
    xStart[1].pxNextFreeBlock = ( void * ) xAppHeap.ucHeap;
    xStart[1].xBlockSize = ( size_t ) 0;

    xStart[0].pxNextFreeBlock = ( void * ) xHeap.ucHeap;
    xStart[0].xBlockSize = ( size_t ) 0;

    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    pucHeapEnd = xAppHeap.ucHeap + xTotalHeapSize[1];
    pucHeapEnd -= heapSTRUCT_SIZE;
    pxEnd[1] = ( void * ) pucHeapEnd;
    configASSERT( ( ( ( unsigned long ) pxEnd[1] ) & ( ( unsigned long ) portBYTE_ALIGNMENT_MASK ) ) == 0UL );
    pxEnd[1]->xBlockSize = 0;
    pxEnd[1]->pxNextFreeBlock = NULL;


    /* pxEnd is used to mark the end of the list of free blocks and is inserted
    at the end of the heap space. */
    pucHeapEnd = xHeap.ucHeap + xTotalHeapSize[0];
    pucHeapEnd -= heapSTRUCT_SIZE;
    pxEnd[0] = ( void * ) pucHeapEnd;
    configASSERT( ( ( ( unsigned long ) pxEnd[0] ) & ( ( unsigned long ) portBYTE_ALIGNMENT_MASK ) ) == 0UL );
    pxEnd[0]->xBlockSize = 0;
    pxEnd[0]->pxNextFreeBlock = NULL;

    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = ( void * ) xAppHeap.ucHeap;
    pxFirstFreeBlock->xBlockSize = xTotalHeapSize[1] - heapSTRUCT_SIZE;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd[1];

    /* The heap now contains pxEnd. */
    xFreeBytesRemaining[1] -= heapSTRUCT_SIZE;


    /* To start with there is a single free block that is sized to take up the
    entire heap space, minus the space taken by pxEnd. */
    pxFirstFreeBlock = ( void * ) xHeap.ucHeap;
    pxFirstFreeBlock->xBlockSize = xTotalHeapSize[0] - heapSTRUCT_SIZE;
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd[0];

    /* The heap now contains pxEnd. */
    xFreeBytesRemaining[0] -= heapSTRUCT_SIZE;
}
/*-----------------------------------------------------------*/

static void prvInsertBlockIntoFreeList( xBlockLink *pxBlockToInsert, size_t mempool)
{
xBlockLink *pxIterator;
unsigned char *puc;

    /* Iterate through the list until a block is found that has a higher address
    than the block being inserted. */
    for( pxIterator = &xStart[mempool]; pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
    {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
    make a contiguous block of memory? */
    puc = ( unsigned char * ) pxIterator;
    if( ( puc + pxIterator->xBlockSize ) == ( unsigned char * ) pxBlockToInsert )
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }

    /* Do the block being inserted, and the block it is being inserted before
    make a contiguous block of memory? */
    puc = ( unsigned char * ) pxBlockToInsert;
    if( ( puc + pxBlockToInsert->xBlockSize ) == ( unsigned char * ) pxIterator->pxNextFreeBlock )
    {
        if( pxIterator->pxNextFreeBlock != pxEnd[mempool] )
        {
            /* Form one big block from the two blocks. */
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd[mempool];
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
    before and the block after, then it's pxNextFreeBlock pointer will have
    already been set, and should not be set here as that would make it point
    to itself. */
    if( pxIterator != pxBlockToInsert )
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
}

