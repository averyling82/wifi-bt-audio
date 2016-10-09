/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Codecs\Audio\Decode\Amr\pAmr.h
* Owner: cjh
* Date: 2016.5.26
* Time: 14:33:35
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.26     14:33:35   1.0
********************************************************************************************
*/


#ifndef __CODECS_AUDIO_DECODE_AMR_PAMR_H__
#define __CODECS_AUDIO_DECODE_AMR_PAMR_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//#define _CODECS_AUDIO_DECODE_AMR_PAMR_COMMON_  __attribute__((section("codecs_audio_decode_amr_pamr_common")))
//#define _CODECS_AUDIO_DECODE_AMR_PAMR_INIT_  __attribute__((section("codecs_audio_decode_amr_pamr_init")))
//#define _CODECS_AUDIO_DECODE_AMR_PAMR_SHELL_  __attribute__((section("codecs_audio_decode_amr_pamr_shell")))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct
{
    uint32 usSampleRate;
    uint8  ucChannels;
    uint32 usOffset;
    // The number of valid bytes in the encoded data buffer.
    uint32 usValid;
    uint32 ulLength;
    // The number of samples in each encoded block of audio.
    uint32 usSamplesPerBlock; // block
    uint32 PerBlockSeekLen;   // seek len
    // The length of the file in milliseconds.
    uint32 ulTimeLength;
    uint32 ulBitRate;
    // The number of samples that have been encoded/decoded.
    uint32 ulTimePos;
    uint32 total_samples;
    uint32 bitPerSamp;
    uint32 OutLength;
    int    CurDecodeIdx;
    // The buffer containing MS ADAMR data.
    uint8  amrCodedData[32];
    uint16 speech[L_FRAME];
    uint32 FileHeadLen;   // File Head len
    uint8  SpeechHead;

} tAMR;

extern tAMR gAMRStruct;           //decoding output struct for AMR format.
extern void *pAmrRawFileCache;    //amr file pointer.

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
//extern void *malloc(size_t xWantedSize);


#endif
