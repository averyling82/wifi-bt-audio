/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   audio_file_access.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _AUDIO_FILE_ACCESS_H_
#define _AUDIO_FILE_ACCESS_H_

#include <stdio.h>

#define RKNANO_FS

#define SEEK_SET 0 /* start of stream (see fseek) */
#define SEEK_CUR 1 /* current position in stream (see fseek) */
#define SEEK_END 2 /* end of stream (see fseek) */
#define NULL     0

#ifdef A_CORE_DECODE

#ifdef CODEC_24BIT
#define AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 36)        // File data buffer 8K
                                                    // XXX output buffer 24K
                                                    // DcFilter need (64+128+320) *8 = 4k byte


#else
#define AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 22)     // File data buffer 8K
                                                    // XXX output buffer 12k
                                                    // DcFilter need (64+128+320) *4 = 2k byte

#endif

#else

#ifdef CODEC_24BIT
#define AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 48)     //HIFI FLAC output buffer 48K
#else
#define AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 12)     //HIFI FLAC output buffer 48K
#endif

#endif

#ifdef CODEC_24BIT
#define HIFI_AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 64) // File data buffer 8K
                                                     // HIFI FLAC output buffer 48K
                                                     // DcFilter need (64+128+320) *8 = 4k byte
#else
#define HIFI_AUDIO_FILE_PIPO_BUF_SIZE    (1024 * 22)     // File data buffer 8K
                                                    // XXX output buffer 12k
                                                    // DcFilter need (64+128+320) *4 = 2k byte
#endif



#define AUDIO_BUF_SIZE              (2 * AUDIO_FILE_PIPO_BUF_SIZE)


#define HIFI_AUDIO_BUF_SIZE         (2 * HIFI_AUDIO_FILE_PIPO_BUF_SIZE)


typedef unsigned int size_t;

extern size_t   (*RKFIO_FOpen)();
extern size_t   (*RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
extern int      (*RKFIO_FSeek)(long int /*offset*/, int /*whence*/ ,FILE * /*stream*/);
extern long int (*RKFIO_FTell)(FILE * /*stream*/);
extern size_t   (*RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
extern unsigned long (*RKFIO_FLength)(FILE *in /*stream*/);
extern int      (*RKFIO_FClose)(FILE * /*stream*/);
extern int (*RKFIO_FEof)(FILE *);
extern void (*RKFIO_BWrite)(char *DataBuf, size_t DataLen);

extern int FLAC_FileSeekFast(int offset, int clus, FILE *in);
extern int FLAC_FileGetSeekInfo(int *pOffset, int *pClus, FILE *in);


extern FILE *pRawFileCache,*pFlacFileHandleBake,*pAacFileHandleSize,*pAacFileHandleOffset, *pRecordFile;
#endif
