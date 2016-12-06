#ifndef  _AIRPLAY_HEAP_H_
#define _AIRPLAY_HEAP_H_

typedef unsigned int size_t;
void *airplay_malloc(size_t xWantedSize) ;
void *airplay_calloc(size_t n, size_t size);
void airplay_free( void *pv ) ;
void *airplay_realloc(void *pv , size_t xWantedSize);
void *airplay_memmove( void* dest, const void* src, size_t count );
#endif
