#ifndef  _RK_TLS_HEAP_H_
#define _RK_TLS_HEAP_H_

typedef unsigned int size_t;
void *tls_malloc(size_t xWantedSize) ;
void *tls_calloc(size_t n, size_t xWantedSize) ;
void tls_free( void *pv ) ;
#endif
