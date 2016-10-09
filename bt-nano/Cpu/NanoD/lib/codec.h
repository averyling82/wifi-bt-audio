/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   Hw_codec.h
*
* Description:
*
* History:      <author>          <time>        <version>
*                               2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"

#ifndef _HW_CODEC_H
#define _HW_CODEC_H

#define CODEC_ROCKC     0
#define CODEC_WM8987    1
#define CODEC_ALC5633   2

#define ACODEC_OUT_HP     1
#define ACODEC_OUT_LINE   2

#ifndef _BROAD_LINE_OUT_
#define ACODEC_OUT_CONFIG     ACODEC_OUT_HP
#else
#define ACODEC_OUT_CONFIG     ACODEC_OUT_LINE
#endif

#define VOL_General     0  //for suit FrenchRegionVal, set VOL_General = 0,  RK original = 1
#define VOL_Europe      1  //for suit FrenchRegionVal, set VOL_General = 1,  RK original = 0

//#define VOLTAB_CONFIG    gSysConfig.VolTableIndex
#define VOLTAB_CONFIG    VOL_General
#define CODEC_CONFIG    CODEC_ROCKC

#define MAX_VOLUME      32

#define     RECORD_QUALITY_HIGH                 0
#define     RECORD_QUALITY_NORMAL               1
typedef enum CodecMode
{
    Codec_DACoutHP,     // for MP3, MP4 playback
    Codec_DACoutLINE,   // for BT/WIFI BOX
    Codec_FMin,         // for FM playback
    Codec_FMADC,        // for FM recording, output remain FM singal
    Codec_Line1ADC,        // for Line1 in recording
    Codec_Line1in,        // for Line1 in playback
    Codec_Line2ADC,      // for Line2 in recording
    Codec_Line2in,       // for Line2 in playback
    Codec_MicStero,       // for MIC Stero recording
    Codec_Mic1Mono,       // for MIC 1 mono recording
    Codec_Mic2Mono,       // for MIC 2 mono recording
    Codec_Standby       // for player no application work, like main menu, or exit from FM
}CodecMode_en_t;

typedef enum CodecPower_Status
{
    Codec_Power_on,
    Codec_Power_down,
    Codec_Power_null
}CodecPower_Status_t;
typedef enum CodecFS
{
    CodecFS_8000Hz   = 8000,
    CodecFS_11025Hz  = 11025,
    CodecFS_12KHz    = 12000,
    CodecFS_16KHz    = 16000,
    CodecFS_22050Hz  = 22050,
    CodecFS_24KHz    = 24000,
    CodecFS_32KHz    = 32000,
    CodecFS_44100Hz  = 44100,
    CodecFS_48KHz    = 48000,
    CodecFS_64KHz    = 64000,
    CodecFS_88200Hz   = 88200,
    CodecFS_96KHz    = 96000,
    CodecFS_128KHz   = 128000,
    CodecFS_1764KHz  = 176400,
    CodecFS_192KHz   = 192000,
    CodecFSSTOP      = 192000
} CodecFS_en_t;
typedef enum
{
    CodecMIX_ENABLE,
    CodecMIX_DISABLE,
}CodecMIX_Mode_t;

void Codec_PowerOnInitial(void);
void Codec_SetMode(CodecMode_en_t Codecmode,CodecFS_en_t CodecFS);
void Codec_SetSampleRate(CodecFS_en_t CodecFS);
void Codec_SetVolumet(unsigned int Volume);
void Codec_DACMute(void);
void Codec_DACUnMute(void);
void Codec_DeInitial(void);

void Codec_ExitMode(CodecMode_en_t Codecmode);
void Codec_ADC_UnMute(void);
void Codec_ADC_MUTE(void);
void ACodec_WriteReg(uint32 regaddr,uint32 data);
void ACodec_ReadReg_Debug(uint32 regaddr);
void ACodec_ADC2DAC_MIX(CodecMIX_Mode_t MIX_en);
uint8 ACodec_get_over_current_value(void);

/*
********************************************************************************
*
*                         End of Codec.h
*
********************************************************************************
*/

#endif

