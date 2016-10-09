/*
********************************************************************************************
*
*   Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                        All rights reserved.
*
* FileName: BBSystem\pCODECS.c
* Owner: WJR
* Date: 2014.12.23
* Time: 16:24:28
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.12.23     16:24:28   1.0
********************************************************************************************
*/

#include "RKOS.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "audio_globals.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_PCODECS2_READ_  __attribute__((section("bbsystem_pcodecs2_read")))
#define _BBSYSTEM_PCODECS2_WRITE_ __attribute__((section("bbsystem_pcodecs2_write")))
#define _BBSYSTEM_PCODECS2_INIT_  __attribute__((section("bbsystem_pcodecs2_init")))
#define _BBSYSTEM_PCODECS2_SHELL_  __attribute__((section("bbsystem_pcodecs2_shell")))

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
int CurrentDecCodec2;
int CurrentEncCodec2;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_BBSYSTEM_PCODECS2_READ_
 static unsigned long (*DecCodecPFn2[2])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{

    #ifdef MP3_DEC_INCLUDE2
    MP3Function2,
    #endif

    #ifdef XXX_DEC_INCLUDE2
    XXXFunction2,
    #endif

    #ifdef AAC_DEC_INCLUDE2
    AACDecFunction2,
    #endif

    #ifdef WAV_DEC_INCLUDE2
    PCMFunction2,
    #endif

    #ifdef AMR_DEC_INCLUDE2
    AmrFunction2,
    #endif

    #ifdef APE_DEC_INCLUDE2
    APEDecFunction2,
    #endif

    #ifdef FLAC_DEC_INCLUDE2
    FLACDecFunction2,
    #endif

    #ifdef OGG_DEC_INCLUDE2
    OGGDecFunction2,
    #endif

    #ifdef HIFI_APE_DECODE2
    HIFI_APEDecFunction2,
    #endif

    #ifdef HIFI_FlAC_DECODE2
    HIFI_FLACDecFunction2,
    #endif

    #ifdef HIFI_AlAC_DECODE2
    HIFI_ALACDecFunction2,
    #endif

    #ifdef SBC_DEC_INCLUDE2
    SbcDecFunction2,
    #endif
    0
};


_BBSYSTEM_PCODECS2_READ_
 static unsigned long (*EnCodecPFn2[2])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{
    #ifdef WAV_ENC_INCLUDE2
    PCMEncFunction2,
    #endif

    #ifdef AMR_ENC_INCLUDE2
    AMREncFunction2,
    #endif

    #ifdef XXX_ENC_INCLUDE2
    XXX_EncFunction2,
    #endif
    0
};

_BBSYSTEM_PCODECS2_READ_
unsigned long CodecOpenEnc2(unsigned long arg, short *ppsBuffer, long *plLength)
{
    unsigned long ulRet;

    if(CurrentEncCodec2 == 0xff)
        return -1;
    // Pass the open request to the entry point for the codec.
    ulRet = (EnCodecPFn2[CurrentEncCodec2])(SUBFN_CODEC_OPEN_ENC, arg, (unsigned long)ppsBuffer, (unsigned long)plLength);

    // Return the result to the caller.
    return(ulRet);
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecOpenDec2()
{
    unsigned long ulRet;

    if(CurrentDecCodec2 == 0xff)
        return -1;
    // Pass the open request to the entry point for the codec.
    ulRet = (DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_OPEN_DEC, 0, 0, 0);

    // Return the result to the caller.
    return(ulRet);
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecDecode2(void)
{
    if(CurrentDecCodec2 == 0xff)
        return -1;
    return((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_DECODE, 0, 0, 0));
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecSeek2(unsigned long ulTime, unsigned long ulSeekType)
{
    // Pass the seek request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec2 == 0xff)
        return -1;

    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_SEEK, ulTime, ulSeekType,0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetTime2(unsigned long *pulTime)
{
    unsigned long ret;

    if(CurrentDecCodec2 == 0xff)
        return -1;
    // Pass the time request to the entry point for the specified codec.

    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_GETTIME, (unsigned long)pulTime, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetDecBitreate2(unsigned long *pulBitrate)
{
    // Pass the bitrate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec2 == 0xff)
        return -1;
    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetEncBitreate2(unsigned long *pulBitrate)
{
    // Pass the bitrate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentEncCodec2 == 0xff)
        return -1;
    ret=((EnCodecPFn2[CurrentEncCodec2])(SUBFN_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetSampleRate2(unsigned long *pulSampleRate)
{
    // Pass the sample rate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec2 == 0xff)
        return -1;

    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_GETSAMPLERATE, (unsigned long)pulSampleRate, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetChannels2(unsigned long *pulChannels)
{
    // Pass the channels request to the entry point for the specified codec.
    unsigned long ret;

    if(CurrentDecCodec2 == 0xff)
        return -1;

    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_GETCHANNELS, (unsigned long)pulChannels, 0, 0));
      return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetLength2(unsigned long *pulLength)
{
    // Pass the length request to the entry point for the specified codec.
    unsigned long ret;

    if(CurrentDecCodec2 == 0xff)
        return -1;
    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_GETLENGTH, (unsigned long)pulLength, 0, 0));
    return ret;
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecSetBuffer2(short* psBuffer)
{
    // Pass the set buffer request to the entry point for the specified codec.
    unsigned long ret;
    ret=((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_SETBUFFER, (unsigned long)psBuffer, 0, 0));
    return ret;
}

_BBSYSTEM_PCODECS2_READ_
unsigned long CodecClose2(void)
{
    unsigned long ulRet;

    if(CurrentDecCodec2 == 0xff)
        return -1;
    // Pass the close request to the entry point for the specified codec.
    ulRet = (DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_CLOSE, 0, 0, 0);

    return(ulRet);
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetDecBuffer2(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    if(CurrentDecCodec2 == 0xff)
        return -1;

    return((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_DEC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetEncBuffer2(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    if(CurrentEncCodec2 == 0xff)
        return -1;

    return((EnCodecPFn2[CurrentEncCodec2])(SUBFN_CODEC_ENC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}

_BBSYSTEM_PCODECS2_READ_
unsigned long CodecEncode2(void)
{
    // Pass the encode request to the entry point for the specified codec.
    if(CurrentEncCodec2 == 0xff)
        return -1;

    return((EnCodecPFn2[CurrentEncCodec2])(SUBFN_CODEC_ENCODE, 0, 0, 0));
}


_BBSYSTEM_PCODECS2_READ_
unsigned long CodecGetBitPerSample2(long *audioBps)
{
    if(CurrentDecCodec2 == 0xff)
        return -1;
    // Pass the encode request to the entry point for the specified codec.
    return((DecCodecPFn2[CurrentDecCodec2])(SUBFN_CODEC_GET_BIT_PER_SAMPLE, (unsigned long)audioBps, 0, 0));
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

