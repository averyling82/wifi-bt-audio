#ifndef  _RK_HEAP_H_
#define _RK_HEAP_H_

typedef unsigned int size_t;
void *malloc(size_t xWantedSize) ;
void free( void *pv ) ;
void *realloc(void *pv , size_t xWantedSize);
void *memmove( void* dest, const void* src, size_t count );
void *rk_realloc(void *pv , size_t xmySize, size_t xWantedSize);
#endif
