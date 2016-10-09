/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   audio_globals.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "RKOS.h"

#ifndef _AUDIO_GLOBALS_H_
#define _AUDIO_GLOBALS_H_

#include "audio_main.h"
#include "effect.h"

//****************************************************************************
// The following values are the IOCTLs which are sent to the individual codec
// drivers.
//****************************************************************************

enum
{
    SUBFN_CODEC_GETNAME,
    SUBFN_CODEC_GETARTIST,
    SUBFN_CODEC_GETTITLE,
    SUBFN_CODEC_GETBITRATE,
    SUBFN_CODEC_GETSAMPLERATE,
    SUBFN_CODEC_GETCHANNELS,
    SUBFN_CODEC_GETLENGTH,
    SUBFN_CODEC_GETTIME,
    SUBFN_CODEC_OPEN_DEC,
    SUBFN_CODEC_OPEN_ENC,
    SUBFN_CODEC_DEC_GETBUFFER,
    SUBFN_CODEC_ENC_GETBUFFER,
    SUBFN_CODEC_SETBUFFER,
    SUBFN_CODEC_DECODE,
    SUBFN_CODEC_ENCODE,
    SUBFN_CODEC_SEEK,
    SUBFN_CODEC_CLOSE,
    SUBFN_CODEC_ZOOM,
    SUBFN_CODEC_GET_BIT_PER_SAMPLE,
    SUBFN_CODEC_GET_FRAME_LEN,
};

//****************************************************************************
// Function prototypes and global variables.
//****************************************************************************

// pSBC.C
extern unsigned long SbcDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);


// pMP3.C
extern unsigned long MP3Function(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

// pXXX.C
extern unsigned long XXXFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

//pFLAC.C
extern unsigned long FLACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

// pAPE.C
extern unsigned long APEDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

// pMSADPCM.C
extern unsigned long MSADPCMFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                    unsigned long ulParam2,unsigned long ulParam3);

// pWAV.C
extern unsigned long PCMFunction(unsigned long ulIoctl, unsigned long ulParam1,
                                unsigned long ulParam2, unsigned long ulParam3);
// pAMR.C
extern unsigned long AmrFunction(unsigned long ulIoctl, unsigned long ulParam1,
                                unsigned long ulParam2,unsigned long ulParam3);

//pAAC.c
extern unsigned long
AACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long
OGGDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long  HIFI_APEDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long  HIFI_ALACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long  HIFI_FLACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3);

//pWavEnc.c
extern unsigned long PCMEncFunction(unsigned long ulIoctl, unsigned long ulParam1,
             unsigned long ulParam2, unsigned long ulParam3
            );

//pAmrEnc.c
extern unsigned long AMREncFunction(unsigned long ulIoctl, unsigned long ulParam1,
             unsigned long ulParam2, unsigned long ulParam3
            );

//pXXXEnc.c
extern unsigned long XXX_EncFunction(unsigned long ulIoctl,
                                           unsigned long ulParam1,
                                           unsigned long ulParam2,
                                           unsigned long ulParam3);


// pSBC.C
extern unsigned long SbcDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);


// pMP3.C
extern unsigned long MP3Function2(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

// pXXX.C
extern unsigned long XXXFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

//pFLAC.C
extern unsigned long FLACDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

// pAPE.C
extern unsigned long APEDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                 unsigned long ulParam2,unsigned long ulParam3);

// pMSADPCM.C
extern unsigned long MSADPCMFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                    unsigned long ulParam2,unsigned long ulParam3);

// pWAV.C
extern unsigned long PCMFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                                unsigned long ulParam2, unsigned long ulParam3);
// pAmr.C
extern unsigned long AmrFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                                unsigned long ulParam2,unsigned long ulParam3);

//pAAC.c
extern unsigned long
AACDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long
OGGDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long  HIFI_APEDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long  HIFI_ALACDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3);
extern unsigned long  HIFI_FLACDecFunction2(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3);


extern unsigned long PCMEncFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                                           unsigned long ulParam2, unsigned long ulParam3);

extern unsigned long AMREncFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                                           unsigned long ulParam2, unsigned long ulParam3);

extern unsigned long XXX_EncFunction2(unsigned long ulIoctl, unsigned long ulParam1,
                                            unsigned long ulParam2, unsigned long ulParam3);



// From codec.c
///////////////////////////////////////////////////////////////////////////////
extern unsigned long CodeOpenDec(void);
extern unsigned long CodeOpenEnc(unsigned long arg, short *ppsBuffer, long *plLength);
extern unsigned long CodecGetSampleRate(unsigned long *pulSampleRate);
extern unsigned long CodecGetChannels(unsigned long *pulChannels);
extern unsigned long CodecGetLength(unsigned long *pulLength);
extern unsigned long CodecGetTime(unsigned long *pulTime);
extern unsigned long CodecGetCaptureBuffer(short *ppsBuffer, long *plLength);
extern unsigned long CodecSetBuffer(short *psBuffer);
extern unsigned long CodecDecode(void);
extern unsigned long CodecEncode(void);
extern unsigned long CodecSeek(unsigned long ulTime, unsigned long ulSeekType);
extern unsigned long CodecClose(void);
extern unsigned long CodecGetDecBitrate(unsigned long *pulBitrate);
extern unsigned long CodecGetFrameLen(long *plLength);

//------------------------------------------------------------
enum
{
    CODEC_MP3_DEC = 0,
    CODEC_XXX_DEC,
    CODEC_AAC_DEC,
    CODEC_WAV_DEC,
    CODEC_AMR_DEC,
    CODEC_APE_DEC,
    CODEC_FLAC_DEC,
    CODEC_OGG_DEC,
    CODEC_HIFI_APE_DEC,
    CODEC_HIFI_FLAC_DEC,
    CODEC_HIFI_ALAC_DEC,
    CODEC_SBC_DEC,
    CODEC_WAV_ENC,
    CODEC_AMR_ENC,
    CODEC_XXX_ENC,
    NUMCODECS
};

//-------------------------------------------------------------

typedef struct mediaBlock
{
    unsigned long CurrentPlayTime;
    unsigned long TotalPlayTime;
    unsigned int BitRate;
    //unsigned int Bps;
    unsigned int BitPerSample;
    unsigned int SampleRate;
    unsigned int Channel;
    unsigned int Total_length;
    unsigned int Outptr;
    unsigned int OutLength;
    unsigned int DecodeOver;
    unsigned int DecodeErr;
    unsigned int needDecode;

}MediaBlock;



typedef struct _FILE_HANDLE_
{
    unsigned char handle1;
    unsigned char handle2;
    unsigned char handle3;
    unsigned long filesize;
    unsigned long curfileoffset[3];
    int           codecType;

}FILE_HANDLE_t;

typedef struct _RecFileHdl_t
{
    unsigned int  fileHandle;
    unsigned int   fileOffset;
    unsigned long filesize;
    int           codecType;

}RecFileHdl_t;


typedef struct _FILE_SEEK_OP
{
    int offset;
    unsigned int whence;
    unsigned long handle;
}FILE_SEEK_OP_t;

typedef struct _FILE_READ_OP
{
    unsigned char *pData;
    unsigned int  NumBytes;
    unsigned long handle;
}FILE_READ_OP_t;

typedef struct _FILE_WRITE_OP
{
    unsigned char *buf;
    unsigned int size;
    unsigned long handle;
}FILE_WRITE_OP_t;

typedef struct _FLAC_SEEKFAST_OP
{
    int offset;
    int clus;
    int fp;
}FLAC_SEEKFAST_OP_t;

typedef struct _AUDIO_EQ_ARG
{
    int32  *audioLen;
    uint32 *playVolume;

}AUDIO_EQ_ARG;

typedef struct
{
    short  *pPCMBuf;
    short  *EncOutBuf;
    long   flag;
    RKEffect EffectCtl;

}AudioInOut_Type;

extern  uint32 gDecCmd;
extern  uint32 gDecData;
extern pSemaphore osAudioDecodeOk;
extern HDC hAudioDma;
extern  uint32 gACKDone;

extern  FILE_SEEK_OP_t    *gpFileSeekParm;
extern  FILE_READ_OP_t    *gpFileReadParm;
extern  FILE_WRITE_OP_t   *gpFileWriteParm;
extern  FLAC_SEEKFAST_OP_t *gpFlacSeekFastParm;

extern  MediaBlock    gpMediaBlock;


extern unsigned char  * DecDataBuf[2];

extern unsigned char DecBufID;


//global variables
extern int CurrentDecCodec;
extern int CurrentEncCodec;
extern int CurrentDecCodec2;
extern int CurrentEncCodec2;

#endif
