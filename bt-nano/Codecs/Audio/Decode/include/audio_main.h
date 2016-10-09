/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   audio_main.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung     2009-1-8          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _AUDIO_MAIN_H_
#define _AUDIO_MAIN_H_

#include "BspConfig.h"

/************************************************************
1. Select any Audio CODEC plug-in
*************************************************************/
#ifdef _AMR_DECODE_
#define AMR_DEC_INCLUDE
#endif

#ifdef _MP3_DECODE_
#define MP3_DEC_INCLUDE
#endif

#ifdef _XXX_DECODE_
#define XXX_DEC_INCLUDE
#define WMA_DEC_INCLUDE
#endif

#ifdef _WAV_DECODE_
#define WAV_DEC_INCLUDE
#endif

#ifdef _APE_DECODE_
#define APE_DEC_INCLUDE
#endif

#ifdef _FLAC_DECODE_
#define FLAC_DEC_INCLUDE
#endif

#ifdef _AAC_DECODE_
#define AAC_DEC_INCLUDE
#endif

#ifdef _HIFI_APE_DEC
#define HIFI_APE_DECODE
#endif

#ifdef _HIFI_FLAC_DEC
#define HIFI_FlAC_DECODE
#endif

#ifdef _HIFI_ALAC_DEC
#define HIFI_AlAC_DECODE
#endif
#ifdef _OGG_DECODE_
#define OGG_DEC_INCLUDE
#endif
#ifdef _SBC_DECODE_
#define SBC_INCLUDE
#endif

#ifdef VIDEO_MP2_DECODE
#define MP2_INCLUDE
#endif

/************************************************************
2. Include XXX Encoding  or ADPCM Encoding
                   Use Only One Encoding
*************************************************************/
/*------------------------------------------------------------
- Select One of Encoding
------------------------------------------------------------*/
#ifdef _WAV_ENCODE_
#define WAV_ENC_INCLUDE
#endif

#ifdef _AMR_ENCODE_
#define AMR_ENC_INCLUDE
#endif

#ifdef _XXX_ENCODE_
#define XXX_ENC_INCLUDE
#define MP3_ENC_INCLUDE
#endif


#ifdef _RK_EQ_
#define RK_MP3_EQ_WAIT_SYNTH
#endif


#ifdef RK_MP3_EQ_WAIT_SYNTH
#define MP3_EQ_WAIT_SYNTH  1
#else
#define MP3_EQ_WAIT_SYNTH  0
#endif


typedef enum _MEDIA_MSGBOX_DECODE_CMD
{
    MEDIA_MSGBOX_CMD_DECODE_NULL,

    MEDIA_MSGBOX_CMD_FILE_OPEN,
    MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL,

    MEDIA_MSGBOX_CMD_FILE_CLOSE,
    MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL,

    MEDIA_MSGBOX_CMD_DEC_OPEN,
    MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL,    /*解析音频文件头信息成功*/

    MEDIA_MSGBOX_CMD_DECODE,
    MEDIA_MSGBOX_CMD_DECODE_CMPL,      /*完成一次解码操作*/

    MEDIA_MSGBOX_CMD_DECODE_GETBUFFER,
    MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_GETTIME,
    MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_SEEK,
    MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL,

    MEDIA_MSGBOX_CMD_DECODE_CLOSE,
    MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL,

    MEDIA_MSGBOX_CMD_FLAC_SEEKFAST,
    MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_CMPL,

    MEDIA_MSGBOX_CMD_FLAC_GETSEEK_INFO,
    MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_INFO_CMPL,
    MEDIA_MSGBOX_CMD_DECODE_NUM


}MEDIA_MSGBOX_DECODE_CMD;


/* sections define */


//------------------------------------------------------------------------------
//Music Section define
#define     _ATTR_AUDIO_TEXT_          __attribute__((section("AudioCode")))
#define     _ATTR_AUDIO_DATA_          __attribute__((section("AudioData")))
#define     _ATTR_AUDIO_BSS_           __attribute__((section("AudioBss"),zero_init))

//-------------------------------------------XXX----------------------------------------------------------
#define     _ATTR_XXXDEC_TEXT_          __attribute__((section("XXXCommonCode")))
#define     _ATTR_XXXDEC_DATA_          __attribute__((section("XXXCommonData")))
#define     _ATTR_XXXDEC_BSS_           __attribute__((section("XXXCommonBss"),zero_init))

//-------------------------------------------SBC----------------------------------------------------------
#define _ATTR_SBCDEC_TEXT_     __attribute__((section("SbcDecCode")))
#define _ATTR_SBCDEC_DATA_     __attribute__((section("SbcDecData")))
#define _ATTR_SBCDEC_BSS_      __attribute__((section("SbcDecBss"),zero_init))

//-------------------------------------Aec algorithm----------------------------------------------------------
#define     _ATTR_AECALG_TEXT_          __attribute__((section("AecAlgCode")))
#define     _ATTR_AECALG_DATA_          __attribute__((section("AecAlgData")))
#define     _ATTR_AECALG_BSS_           __attribute__((section("AecAlgBss"),zero_init))

//-------------------------------------------AMR----------------------------------------------------------
#define _ATTR_AMRDEC_TEXT_     __attribute__((section("AmrDecCode")))
#define _ATTR_AMRDEC_DATA_     __attribute__((section("AmrDecData")))
#define _ATTR_AMRDEC_BSS_      __attribute__((section("AmrDecBss"),zero_init))

#define _ATTR_AMRENC_TEXT_     __attribute__((section("EncodeAmrCode")))
#define _ATTR_AMRENC_DATA_     __attribute__((section("EncodeAmrData")))
#define _ATTR_AMRENC_BSS_      __attribute__((section("EncodeAmrBss"),zero_init))



//-------------------------------------------MP3----------------------------------------------------------
#define _ATTR_MP3DEC_TEXT_     __attribute__((section("Mp3DecCode")))
#define _ATTR_MP3DEC_DATA_     __attribute__((section("Mp3DecData")))
#define _ATTR_MP3DEC_BSS_      __attribute__((section("Mp3DecBss"),zero_init))

#define _ATTR_XXXENC_TEXT_     __attribute__((section("EncodeXXXCode")))
#define _ATTR_XXXENC_DATA_     __attribute__((section("EncodeXXXData")))
#define _ATTR_XXXENC_BSS_      __attribute__((section("EncodeXXXBss"),zero_init))


//-------------------------------------------WAV----------------------------------------------------------
#define _ATTR_WAVDEC_TEXT_     __attribute__((section("WavDecCode")))
#define _ATTR_WAVDEC_DATA_     __attribute__((section("WavDecData")))
#define _ATTR_WAVDEC_BSS_      __attribute__((section("WavDecBss"),zero_init))

#define _ATTR_WAVENC_TEXT_     __attribute__((section("WavEncCode")))
#define _ATTR_WAVENC_DATA_     __attribute__((section("WavEncData")))
#define _ATTR_WAVENC_BSS_      __attribute__((section("WavEncBss"),zero_init))


//-----------------------------------------FLAC----------------------------------------------------------
#define     _ATTR_FLACDEC_TEXT_          __attribute__((section("FlacDecCode")))
#define     _ATTR_FLACDEC_DATA_          __attribute__((section("FlacDecData")))
#define     _ATTR_FLACDEC_BSS_           __attribute__((section("FlacDecBss"),zero_init))
//-----------------------------------------AAC----------------------------------------------------------
#define     _ATTR_AACDEC_TEXT_          __attribute__((section("AacDecCode")))
#define     _ATTR_AACDEC_DATA_          __attribute__((section("AacDecData")))
#define     _ATTR_AACDEC_BSS_           __attribute__((section("AacDecBss"),zero_init))
#define     _ATTR_AACDEC_DATA_RO        __attribute__((section("AacROData")))

//-----------------------------------------APE----------------------------------------------------------
#define     _ATTR_APEDEC_TEXT_          __attribute__((section("ApeDecCode")))
#define     _ATTR_APEDEC_DATA_          __attribute__((section("ApeDecData")))
#define     _ATTR_APEDEC_BSS_           __attribute__((section("ApeDecBss"),zero_init))

//-----------------------------------------OGG----------------------------------------------------------
#define     _ATTR_OGGDEC_TEXT_          __attribute__((section("OggDecCode")))
#define     _ATTR_OGGDEC_DATA_          __attribute__((section("OggDecData")))
#define     _ATTR_OGGDEC_BSS_           __attribute__((section("OggDecBss"),zero_init))


//-----------------------------------------HIFI APE----------------------------------------------------------
#define     _ATTR_HIFI_APEDEC_TEXT_          __attribute__((section("ApeHDecCode")))
#define     _ATTR_HIFI_APEDEC_DATA_          __attribute__((section("ApeHDecData")))
#define     _ATTR_HIFI_APEDEC_BSS_           __attribute__((section("ApeHDecBss"),zero_init))


//-----------------------------------------HIFI FLAC----------------------------------------------------------
#define     _ATTR_HIFI_FLACDEC_TEXT_          __attribute__((section("FlacHDecCode")))
#define     _ATTR_HIFI_FLACDEC_DATA_          __attribute__((section("FlacHDecData")))
#define     _ATTR_HIFI_FLACDEC_BSS_           __attribute__((section("FlacHDecBss"),zero_init))


//-----------------------------------------HIFI ALAC----------------------------------------------------------
#define     _ATTR_HIFI_ALACDEC_TEXT_          __attribute__((section("AlacHDecCode")))
#define     _ATTR_HIFI_ALACDEC_DATA_          __attribute__((section("AlacHDecData")))
#define     _ATTR_HIFI_ALACDEC_BSS_           __attribute__((section("AlacHDecBss"),zero_init))



//-------------------------------------------XXX BIN----------------------------------------------------------
#define     _ATTR_XXXDEC_BIN_TEXT_          __attribute__((used, section("XXXCommonBinCode")))
#define     _ATTR_XXXDEC_BIN_DATA_          __attribute__((used, section("XXXCommonBinData")))
#define     _ATTR_XXXDEC_BIN_BSS_           __attribute__((used, section("XXXCommonBinBss"),zero_init))

//-------------------------------------------SBC BIN----------------------------------------------------------
#define _ATTR_SBCDEC_BIN_TEXT_     __attribute__((used, section("SbcDecBinCode")))
#define _ATTR_SBCDEC_BIN_DATA_     __attribute__((used, section("SbcDecBinData")))
#define _ATTR_SBCDEC_BIN_BSS_      __attribute__((used, section("SbcDecBinBss"),zero_init))


//-------------------------------------------MP3 BIN----------------------------------------------------------
#define _ATTR_MP3DEC_BIN_TEXT_     __attribute__((used, section("Mp3DecBinCode")))
#define _ATTR_MP3DEC_BIN_DATA_     __attribute__((used, section("Mp3DecBinData")))
#define _ATTR_MP3DEC_BIN_BSS_      __attribute__((used, section("Mp3DecBinBss"),zero_init))

#define _ATTR_XXXENC_BIN_TEXT_     __attribute__((used, section("XXXEncBinCode")))
#define _ATTR_XXXENC_BIN_DATA_     __attribute__((used, section("XXXEncBinData")))
#define _ATTR_XXXENC_BIN_BSS_      __attribute__((used, section("XXXEncBinBss"),zero_init))

//-------------------------------------------WAV BIN----------------------------------------------------------
#define _ATTR_WAVDEC_BIN_TEXT_     __attribute__((used, section("WavDecBinCode")))
#define _ATTR_WAVDEC_BIN_DATA_     __attribute__((used, section("WavDecBinData")))
#define _ATTR_WAVDEC_BIN_BSS_      __attribute__((used, section("WavDecBinBss"),zero_init))

#define _ATTR_WAVENC_BIN_TEXT_     __attribute__((used, section("WavEncBinCode")))
#define _ATTR_WAVENC_BIN_DATA_     __attribute__((used, section("WavEncBinData")))
#define _ATTR_WAVENC_BIN_BSS_      __attribute__((used, section("WavEncBinBss"),zero_init))

//-------------------------------------------AMR BIN----------------------------------------------------------
#define _ATTR_AMRDEC_BIN_TEXT_     __attribute__((used, section("AmrDecBinCode")))
#define _ATTR_AMRDEC_BIN_DATA_     __attribute__((used, section("AmrDecBinData")))
#define _ATTR_AMRDEC_BIN_BSS_      __attribute__((used, section("AmrDecBinBss"),zero_init))

#define _ATTR_AMRENC_BIN_TEXT_     __attribute__((used, section("AmrEncBinCode")))
#define _ATTR_AMRENC_BIN_DATA_     __attribute__((used, section("AmrEncBinData")))
#define _ATTR_AMRENC_BIN_BSS_      __attribute__((used, section("AmrEncBinBss"),zero_init))

//-----------------------------------------FLAC BIN----------------------------------------------------------
#define     _ATTR_FLACDEC_BIN_TEXT_          __attribute__((used, section("FlacDecBinCode")))
#define     _ATTR_FLACDEC_BIN_DATA_          __attribute__((used, section("FlacDecBinData")))
#define     _ATTR_FLACDEC_BIN_BSS_           __attribute__((used, section("FlacDecBinBss"),zero_init))


//-----------------------------------------AAC BIN----------------------------------------------------------
#define     _ATTR_AACDEC_BIN_TEXT_          __attribute__((used, section("AacDecBinCode")))
#define     _ATTR_AACDEC_BIN_DATA_          __attribute__((used, section("AacDecBinData")))
#define     _ATTR_AACDEC_BIN_BSS_           __attribute__((used, section("AacDecBinBss"),zero_init))


//-----------------------------------------APE BIN----------------------------------------------------------
#define     _ATTR_APEDEC_BIN_TEXT_          __attribute__((used, section("ApeDecBinCode")))
#define     _ATTR_APEDEC_BIN_DATA_          __attribute__((used, section("ApeDecBinData")))
#define     _ATTR_APEDEC_BIN_BSS_           __attribute__((used, section("ApeDecBinBss"),zero_init))

//-----------------------------------------OGG BIN----------------------------------------------------------
#define     _ATTR_OGGDEC_BIN_TEXT_          __attribute__((used,section("OggDecBinCode")))
#define     _ATTR_OGGDEC_BIN_DATA_          __attribute__((used,section("OggDecBinData")))
#define     _ATTR_OGGDEC_BIN_BSS_           __attribute__((used,section("OggDecBinBss"),zero_init))


//-----------------------------------------HIFI APE BIN----------------------------------------------------------
#define     _ATTR_HIFI_APEDEC_BIN_TEXT_          __attribute__((used,section("ApeHDecBinCode")))
#define     _ATTR_HIFI_APEDEC_BIN_DATA_          __attribute__((used,section("ApeHDecBinData")))
#define     _ATTR_HIFI_APEDEC_BIN_BSS_           __attribute__((used,section("ApeHDecBinBss"),zero_init))


//-----------------------------------------HIFI FLAC BIN----------------------------------------------------------
#define     _ATTR_HIFI_FLACDEC_BIN_TEXT_          __attribute__((used,section("FlacHDecBinCode")))
#define     _ATTR_HIFI_FLACDEC_BIN_DATA_          __attribute__((used,section("FlacHDecBinData")))
#define     _ATTR_HIFI_FLACDEC_BIN_BSS_           __attribute__((used,section("FlacHDecBinBss"),zero_init))


//-----------------------------------------HIFI ALAC BIN----------------------------------------------------------
#define     _ATTR_HIFI_ALACDEC_BIN_TEXT_          __attribute__((used,section("AlacHDecBinCode")))
#define     _ATTR_HIFI_ALACDEC_BIN_DATA_          __attribute__((used,section("AlacHDecBinData")))
#define     _ATTR_HIFI_ALACDEC_BIN_BSS_           __attribute__((used,section("AlacHDecBinBss"),zero_init))


//-------------------------------------------ID3----------------------------------------------------------
#define _ATTR_ID3_TEXT_     __attribute__((section("Id3Code")))
#define _ATTR_ID3_DATA_     __attribute__((section("Id3Data")))
#define _ATTR_ID3_BSS_      __attribute__((section("Id3Bss"),zero_init))

//-------------------------------------------ID3 JPG------------------------------------------------------
//#define _ATTR_ID3JPG_TEXT_     __attribute__((section("Id3JpgCode")))
//#define _ATTR_ID3JPG_DATA_     __attribute__((section("Id3JpgData")))
//#define _ATTR_ID3JPG_BSS_      __attribute__((section("Id3JpgBss"),zero_init))


//-------------------------------------------MP3 Encode----------------------------------------------------------
#define _ATTR_MSEQ_TEXT_     __attribute__((section("MsEqCode")))
#define _ATTR_MSEQ_DATA_     __attribute__((section("MsEqData")))
#define _ATTR_MSEQ_BSS_      __attribute__((section("MsEqBss"),zero_init))

//-------------------------------------------MP2 Encode----------------------------------------------------------
#define _ATTR_MP2DEC_TEXT_     __attribute__((section("Mp2Code"/*"MP3DEC_CODE_SEG"*/)))
#define _ATTR_MP2DEC_DATA_     __attribute__((section("Mp2Data"/*"MP3DEC_DATA_SEG"*/)))
#define _ATTR_MP2DEC_BSS_      __attribute__((section("Mp2Bss"/*"MP3DEC_BSS_SEG"*/),zero_init))
//-------------------------------------------OGG Decode----------------------------------------------------------




//aac
#ifdef B_CORE_DECODE
#define aac_DEBUG  DEBUG2
#define aac_printf rk_printf2
#define aac_delayms DelayMs2

#define aac_MemSet MemSet2
#define aac_Memcpy MemCpy2
#else
#define aac_DEBUG  DEBUG
#define aac_printf rk_printf
#define aac_delayms DelayMs

#define aac_MemSet memset
#define aac_Memcpy memcpy
#endif

//hifi ape
#ifdef B_CORE_DECODE
#define Hifi_Ape_MemSet MemSet2
#define Hifi_Ape_DelayMs DelayMs2
#define Hifi_Ape_Memcpy  MemCpy2

#define Hifi_Ape_Printf rk_printf2
#else
#define Hifi_Ape_MemSet MemSet
#define Hifi_Ape_DelayMs DelayMs
#define Hifi_Ape_Memcpy  MemCpy

#define Hifi_Ape_Printf rk_printf
#endif

//hifi alac
#ifdef B_CORE_DECODE
#define Hifi_Alac_MemSet MemSet2
#define Hifi_Alac_DelayMs DelayMs2
#define Hifi_Alac_Memcpy  MemCpy2
#define Hifi_Alac_Printf rk_printf2
#else
#define Hifi_Alac_MemSet MemSet
#define Hifi_Alac_DelayMs DelayMs
#define Hifi_Alac_Memcpy  MemCpy
#define Hifi_Alac_Printf rk_printf
#endif

//hifi flac
#ifdef B_CORE_DECODE
#define Hifi_Flac_MemSet MemSet2
#define Hifi_Flac_DelayMs DelayMs2
#define Hifi_Flac_Memcpy  MemCpy2
#define Hifi_Flac_Printf rk_printf2
#else
#define Hifi_Flac_MemSet MemSet
#define Hifi_Flac_DelayMs DelayMs
#define Hifi_Flac_Memcpy  MemCpy
#define Hifi_Flac_Printf rk_printf
#endif

//mp3
#ifdef B_CORE_DECODE
#ifdef _MP3_LOG_
#define mp3_printf rk_printf2
#else
#define mp3_printf
#endif
#else
#ifdef _MP3_LOG_
#define mp3_printf rk_printf
#else
#define mp3_printf
#endif
#endif
//wav
#ifdef B_CORE_DECODE
#define wav_DEBUG DEBUG2
#define wav_printf rk_printf2
#else
#define wav_DEBUG DEBUG2
#define wav_printf rk_printf
#endif
//XXX
#ifdef B_CORE_DECODE
#define XXX_DEBUG DEBUG2
#else
#define XXX_DEBUG DEBUG
#endif

//flac
#ifdef B_CORE_DECODE
#define flac_MemSet MemSet2
#define flac_DEBUG DEBUG2
#else
#define flac_MemSet MemSet
#define flac_DEBUG DEBUG
#endif

//ape
#ifdef B_CORE_DECODE
#define ape_MemSet MemSet2
#define ape_DEBUG DEBUG2
#define ape_Memcpy MemCpy2
#else
#define ape_MemSet MemSet
#define ape_DEBUG DEBUG
#define ape_Memcpy MemCpy
#endif

//ogg
#ifdef B_CORE_DECODE
#define ogg_MemSet MemSet2
#define ogg_DEBUG DEBUG2
#define ogg_Memcpy MemCpy2
#else
#define ogg_MemSet MemSet
#define ogg_DEBUG DEBUG
#define ogg_Memcpy MemCpy
#endif

#endif        // _AUDIO_MAIN_H_
