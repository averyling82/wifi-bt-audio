#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include <stdlib.h>
#include <string.h>
#include "airplay_heap.h"


void *airplay_malloc(size_t xWantedSize)
{
    void * ptr;
    ptr = rkos_memory_malloc(xWantedSize);
    if(ptr == NULL)
    {
        printf("airplay_malloc fail xWantedSize = %d\n", xWantedSize);
    }

    return ptr;
}

void *airplay_calloc(size_t n, size_t size)
{
    size_t xWantedSize;
    void *ptr;

    if(n == 0 || size == 0)
        return NULL;

    xWantedSize = n * size;
    ptr = rkos_memory_malloc(xWantedSize);
    if(ptr == NULL)
    {
        printf("airplay_calloc fail xWantedSize = %d\n", xWantedSize);
    }
    else
    {
        memset(ptr, 0, xWantedSize);
    }

    return ptr;
}

void airplay_free( void *pv )
{
    rkos_memory_free(pv);
}

void *airplay_realloc(void *pv , size_t xWantedSize)
{
    #if 0
    void *new_point = NULL;
    xBlockLink *block_point = NULL;
    size_t copy_size = 0;

    if (!pv)
        return airplay_malloc(xWantedSize);

    new_point = airplay_malloc(xWantedSize);
    if (new_point == NULL) {
        printf("airplay_realloc fail xWantedSize = %d\n", xWantedSize);
        airplay_free(pv);
        return NULL;
    }

    block_point = (void *)(( unsigned char * )pv - heapSTRUCT_SIZE);

    copy_size = block_point->xBlockSize < xWantedSize ? block_point->xBlockSize  : xWantedSize;
    memcpy(new_point, pv, copy_size);
    airplay_free(pv);

    return new_point;
    #else
    void * ptr;
    ptr = rkos_memory_realloc(pv, xWantedSize);
    if(ptr == NULL)
        printf("airplay_realloc fail xWantedSize = %d\n", xWantedSize);
    return ptr;
    #endif
}

void *airplay_memmove( void* dest, const void* src, size_t count )
{
    void * new_buffer = NULL;
    new_buffer = airplay_malloc(count);
    memcpy(new_buffer, src, count);
    memcpy(dest, new_buffer, count);
    airplay_free(new_buffer);

    return dest;
}
#endif

