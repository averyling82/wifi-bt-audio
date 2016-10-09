#include "BspConfig.h"
#include "FreeRTOS.h"
#include "RKOS.h"
#include <stdlib.h>
#include <string.h>

#include "rk_tls_heap.h"

#define MEM_ASSERT(x)                                \
{   unsigned int a = x;                                      \
    if(a == 0)                                       \
    {                                                \
        printf("value ==0\n");                       \
    }                                                \
    else if(a >= 0x03000000 && a <= 0x03050000 )     \
    {                                                \
                                                     \
    }                                                \
    else if(a >= 0x03050000 && a <= 0x03090000)      \
    {                                                \
                                                     \
    }                                                \
    else                                             \
    {                                                \
        printf("value INVALID =0x%04x\n",a);         \
    }                                                \
                                                     \
}

//static const unsigned short heapSTRUCT_SIZE    = ( sizeof( xBlockLink ) + portBYTE_ALIGNMENT - ( sizeof( xBlockLink ) % portBYTE_ALIGNMENT ) );
static const unsigned short heapSTRUCT_SIZE    =  sizeof( xBlockLink );
extern uint32 rkos_GetFreeHeapSize( void );
void *tls_malloc(size_t xWantedSize)
{
    void *p;

    p = rkos_memory_malloc(xWantedSize);

    if (p == NULL)
    {
        printf("malloc fail xwantedSize = %d\n", xWantedSize);
    }
    return p;
}

void *tls_calloc(size_t n, size_t size)
{
    void *p;
    size_t xWantedSize = n * size;

    p = rkos_memory_malloc(xWantedSize);
    if (p != NULL)
    {
        memset(p, 0, xWantedSize);
        return p;
    }
    else if (p == NULL)
    {
        printf("calloc fail xwantedSize = %d\n", xWantedSize);
    }
    return p;
}


void tls_free( void *pv )
{
    rkos_memory_free(pv);
}
