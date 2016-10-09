#ifndef _BTBUFFER_H_
#define _BTBUFFER_H_
#include "bt_config.h"


//----- DEFINES -----

//VS FiFo
#ifdef _AEC_DECODE
#define VS_BUFSIZE                     4//(1024 * 12) // 30 kBytes
#else
#define VS_BUFSIZE                     4//(1024 * 12) // 30 kBytes
#endif

//----- PROTOTYPES -----
extern void         bt_buf_puts(const unsigned char *s, unsigned int len);
extern unsigned int bt_buf_size(void);
extern unsigned int bt_buf_free_size(void);
extern unsigned int bt_buf_len(void);
extern void         bt_buf_reset(void);
extern void bt_mediadata_input(const unsigned char *s, unsigned int len);

/*
 *-----wifi audio buffer function interfaces.
*/
extern unsigned long  RKBT_FLength(char *in);
extern unsigned short RKBT_FRead(unsigned char *b, unsigned short s, void *f);
extern unsigned char  RKBT_FSeek(unsigned long offset, unsigned char Whence, char Handle);
extern unsigned long  RKBT_FTell(void *in);
extern unsigned char  RKBT_FClose(char Handle);
extern int RKBT_FEOF(void *f);

#endif //_BUFFER_H_
