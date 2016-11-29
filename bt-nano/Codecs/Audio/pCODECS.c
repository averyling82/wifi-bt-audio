/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\Common\pCODECS.c
Desc    :

Author  : Vincent Hsiung
Date    : 2009-01-08
Notes   :

$Log    :
*
*
*/
/****************************************************************/
#include "RKOS.h"
#include "audio_globals.h"


//global variables
_APP_AUDIO_AUDIOCONTROLTASK_DATA_ int CurrentDecCodec;

#ifdef _RECORD_
_APP_RECORD_RECORDCONTROLTASK_DATA_ int CurrentEncCodec;
#endif


//be sure to keep consistent with array order of IOCTL enum that is defined in audio_globals.h.
_APP_AUDIO_AUDIOCONTROLTASK_DATA_
static unsigned long (*CodecDecPFn[NUMCODECS])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{
    #ifdef MP3_DEC_INCLUDE
    MP3Function,
    #else
    0,
    #endif

    #ifdef XXX_DEC_INCLUDE
    XXXFunction,
    #else
    0,
    #endif

    #ifdef AAC_DEC_INCLUDE
    AACDecFunction,
    #else
    0,
    #endif

    #ifdef WAV_DEC_INCLUDE
    PCMFunction,
    #else
    0,
    #endif

    #ifdef AMR_DEC_INCLUDE
    AmrFunction,
    #else
    0,
    #endif

    #ifdef APE_DEC_INCLUDE
    APEDecFunction,
    #else
    0,
    #endif

    #ifdef FLAC_DEC_INCLUDE
    FLACDecFunction,
    #else
    0,
    #endif

    #ifdef OGG_DEC_INCLUDE
    OGGDecFunction,
    #else
    0,
    #endif

    #ifdef HIFI_APE_DECODE
    HIFI_APEDecFunction,
    #else
    0,
    #endif

    #ifdef HIFI_FlAC_DECODE
    HIFI_FLACDecFunction,
    #else
    0,
    #endif

    #ifdef HIFI_AlAC_DECODE
    HIFI_ALACDecFunction,
    #else
    0,
    #endif

    #ifdef SBC_INCLUDE
    SbcDecFunction,
    #else
    0,
    #endif

    #ifdef WAV_ENC_INCLUDE
    PCMEncFunction,
    #else
    0,
    #endif

    #ifdef AMR_ENC_INCLUDE
    AMREncFunction,
    #else
    0,
    #endif

    #ifdef XXX_ENC_INCLUDE
    XXX_EncFunction,
    #else
    0,
    #endif
};

#ifdef _RECORD_
_APP_RECORD_RECORDCONTROLTASK_DATA_
static unsigned long (*CodecEncPFn[NUMCODECS])(unsigned long ulSubFn,
                                                   unsigned long ulParam1,
                                                   unsigned long ulParam2,
                                                   unsigned long ulParam3) =
{
    #ifdef MP3_DEC_INCLUDE
    MP3Function,
    #else
    0,
    #endif

    #ifdef XXX_DEC_INCLUDE
    XXXFunction,
    #else
    0,
    #endif

    #ifdef AAC_DEC_INCLUDE
    AACDecFunction,
    #else
    0,
    #endif

    #ifdef WAV_DEC_INCLUDE
    PCMFunction,
    #else
    0,
    #endif

    #ifdef AMR_DEC_INCLUDE
    AmrFunction,
    #else
    0,
    #endif

    #ifdef APE_DEC_INCLUDE
    APEDecFunction,
    #else
    0,
    #endif

    #ifdef FLAC_DEC_INCLUDE
    FLACDecFunction,
    #else
    0,
    #endif

    #ifdef OGG_DEC_INCLUDE
    OGGDecFunction,
    #else
    0,
    #endif

    #ifdef HIFI_APE_DECODE
    HIFI_APEDecFunction,
    #else
    0,
    #endif

    #ifdef HIFI_FlAC_DECODE
    HIFI_FLACDecFunction,
    #else
    0,
    #endif

    #ifdef HIFI_AlAC_DECODE
    HIFI_ALACDecFunction,
    #else
    0,
    #endif

    #ifdef SBC_INCLUDE
    SbcDecFunction,
    #else
    0,
    #endif

    #ifdef WAV_ENC_INCLUDE
    PCMEncFunction,
    #else
    0,
    #endif

    #ifdef AMR_ENC_INCLUDE
    AMREncFunction,
    #else
    0,
    #endif

    #ifdef XXX_ENC_INCLUDE
    XXX_EncFunction,
    #else
    0,
    #endif
};


_APP_RECORD_RECORDCONTROLTASK_COMMON_
unsigned long CodeOpenEnc(unsigned long arg, short *ppsBuffer, long *plLength)
{
    unsigned long ulRet;

    rk_printf("CurrentEncCodec = %d", CurrentEncCodec);

    if(CurrentEncCodec == 0xff)
        return -1;


    // Pass the open request to the entry point for the codec.
    ulRet = (CodecEncPFn[CurrentEncCodec])(SUBFN_CODEC_OPEN_ENC, arg, (unsigned long)ppsBuffer, (unsigned long)plLength);

    // Return the result to the caller.
    return(ulRet);
}
#endif

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodeOpenDec(unsigned long directplay, unsigned long savememory)
{
    unsigned long ulRet;

    if(CurrentDecCodec == 0xff)
        return -1;

	rk_printf("directplay=%d savememory=%d\n",directplay,savememory);
    // Pass the open request to the entry point for the codec.
    ulRet = (CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_OPEN_DEC, directplay, savememory, 0);
	rk_printf("20161129 111aaaa1\n");
    // Return the result to the caller.
    return(ulRet);
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecDecode(void)
{
    if(CurrentDecCodec == 0xff)
        return -1;

    return((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_DECODE, 0, 0, 0));
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecSeek(unsigned long ulTime, unsigned long ulSeekType)
{
    // Pass the seek request to the entry point for the specified codec.
    unsigned long ret;

    if(CurrentDecCodec == 0xff)
        return -1;

    ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_SEEK, ulTime, ulSeekType,0));

    return ret;
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetTime(unsigned long *pulTime)
{
    // Pass the time request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec == 0xff)
        return -1;

    ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GETTIME, (unsigned long)pulTime, 0, 0));

    return ret;
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetDecBitrate(unsigned long *pulBitrate)
{
   // Pass the bitrate request to the entry point for the specified codec.
   unsigned long ret;
   if(CurrentDecCodec == 0xff)
        return -1;

   ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));

   return ret;
}

#ifdef _RECORD_
_APP_RECORD_RECORDCONTROLTASK_COMMON_
unsigned long CodecGetEncBitrate(unsigned long *pulBitrate)
{
   // Pass the bitrate request to the entry point for the specified codec.
   unsigned long ret;
   if(CurrentEncCodec == 0xff)
        return -1;

   ret=((CodecEncPFn[CurrentEncCodec])(SUBFN_CODEC_GETBITRATE, (unsigned long)pulBitrate, 0, 0));

   return ret;
}
#endif

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetSampleRate(unsigned long *pulSampleRate)
{
    // Pass the sample rate request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec == 0xff)
        return -1;
    ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GETSAMPLERATE, (unsigned long)pulSampleRate, 0, 0));

    return ret;
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetChannels(unsigned long *pulChannels)
{
    // Pass the channels request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec == 0xff)
        return -1;

    ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GETCHANNELS, (unsigned long)pulChannels, 0, 0));

      return ret;
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetLength(unsigned long *pulLength)
{
    // Pass the length request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec == 0xff)
        return -1;

    ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GETLENGTH, (unsigned long)pulLength, 0, 0));

    return ret;
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecSetBuffer(short* psBuffer)
{
    // Pass the set buffer request to the entry point for the specified codec.
    unsigned long ret;
    if(CurrentDecCodec == 0xff)
        return -1;


    ret=((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_SETBUFFER, (unsigned long)psBuffer, 0, 0));
    return ret;
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecClose(void)
{
    unsigned long ulRet;

    if(CurrentDecCodec == 0xff)
        return -1;

    // Pass the close request to the entry point for the specified codec.
    ulRet = (CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_CLOSE, 0, 0, 0);

    return(ulRet);
}

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetDecBuffer(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    if(CurrentDecCodec == 0xff)
        return -1;

    return((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_DEC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}

#ifdef _RECORD_
_APP_RECORD_RECORDCONTROLTASK_COMMON_
unsigned long CodecGetEncBuffer(short *ppsBuffer, long *plLength)
{
    // Pass the get capture buffer request to the entry point for the specified
    // codec.
    if(CurrentEncCodec == 0xff)
        return -1;

    return((CodecEncPFn[CurrentEncCodec])(SUBFN_CODEC_ENC_GETBUFFER,(unsigned long)ppsBuffer,
            (unsigned long)plLength, 0));
}



_APP_RECORD_RECORDCONTROLTASK_COMMON_
unsigned long CodecEncode(void)
{
    // Pass the encode request to the entry point for the specified codec.
    if(CurrentEncCodec == 0xff)
        return -1;

    return((CodecEncPFn[CurrentEncCodec])(SUBFN_CODEC_ENCODE, 0, 0, 0));
}
#endif

_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetBitPerSample(long *audioBps)
{
    unsigned long ulRet;

    if(CurrentDecCodec == 0xff)
        return -1;
    // Pass the encode request to the entry point for the specified codec.
    ulRet = ((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GET_BIT_PER_SAMPLE, (unsigned long)audioBps, 0, 0));

    return ulRet;
}


_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
unsigned long CodecGetFrameLen(long *plLength)
{
    unsigned long ulRet;

    if(CurrentDecCodec == 0xff)
        return -1;
    // Pass the encode request to the entry point for the specified codec.
    ulRet = ((CodecDecPFn[CurrentDecCodec])(SUBFN_CODEC_GET_FRAME_LEN, (unsigned long)plLength, 0, 0));

    return ulRet;
}
