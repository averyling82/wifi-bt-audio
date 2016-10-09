/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Driver\RockCodec\RockCodecDevice.c
* Owner: HJ
* Date: 2014.3.10
* Time: 13:51:36
* Desc: Uart Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
*      hj     2014.3.10     13:51:36   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_ROCKCODEC_ROCKCODECDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "BSP.h"
#include "device.h"
#include "codec.h"
#include "rockcodec.h"
#include "HW_I2s.h"
#include "i2s.h"
#include "DriverInclude.h"
#include "I2sDevice.h"
#include "RockCodecDevice.h"
#include "effect.h"
#include "audio_globals.h"
#include "FwAnalysis.h"
#include "ShellTask.h"
#include "global.h"
#include "powermanager.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
typedef struct _SRUCT_CODEC_CONFIG
{
    UINT8 DacAnaVol;
    UINT8 DacDigVol;
}SRUCT_CODEC_CONFIG;
*/
typedef struct _SRUCT_CODEC_CONFIG
{
    UINT8 HP_AMPVol;
    UINT8 Dac_DigVol;


    UINT8 HP_ANTIPOPVol;
    UINT8 BeepFadeDeep;
    UINT8 EQ_POP_HEAVY_HP_AMPVol;
    UINT8 EQ_POP_HEAVY_DacDigVol;
    UINT8 EQ_JAZZ_UNIQUE_HP_AMPVol;
    UINT8 EQ_JAZZ_UNIQUE_DacDigVol;
    UINT8 EQ_USER_HP_AMPVol;
    UINT8 EQ_USER_DacDigVol;
    UINT8 EQ_BASS_HP_AMPVol;
    UINT8 EQ_BASS_DacDigVol;

}SRUCT_CODEC_CONFIG;

typedef struct _SRUCT_ACODEC_CONFIG
{
    UINT8 HP_AMPVol;
    UINT8 Dac_DigVol;

}SRUCT_ACODEC_CONFIG;

#if 1
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_DATA_
SRUCT_CODEC_CONFIG ACodec_LineOutVol_General[MAX_VOLUME + 1] =
{
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
 0, 140, 15, 1,  0, 165,   0, 157,      0, 124,   0, 124, //  1
 0, 135, 15, 1,  0, 153,   0, 145,      0, 132,   0, 132, //  2
 0, 123, 15, 1,  0, 148,   0, 140,      0, 127,   0, 127, //  3
 0, 123, 11, 1,  0, 145,   0, 137,      0, 124,   0, 124, //  4
 0, 123, 7,  1,  0, 143,   0, 135,      0, 122,   0, 122, //  5
 0, 123, 4,  1,  0, 144,   0, 136,      0, 123,   0, 123, //  6
 0, 123, 2,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  7
 0, 123, 0,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  8
 0, 117, 0,  1,  0, 134,   0, 126,      0, 113,   0, 113, //  9
 0, 111, 0,  1,  0, 127,   0, 119,      0, 106,   0, 106, //  10
 0, 107, 0,  1,  0, 122,   0, 114,      0, 101,   0, 101, //  11
 0, 101, 0,  1,  0, 116,   0, 108,      0, 95,    0, 95,  //  12
 0, 96,  0,  1,  0, 110,   0, 102,      0, 89,    0, 89,  //  13
 0, 91,  0,  1,  0, 103,   0, 95,       0, 82,    0, 82,  //  14
 0, 87,  0,  1,  0, 98,    0, 90,       0, 77,    0, 77,  //  15
 0, 81,  0,  1,  0, 93,    0, 85,       0, 72,    0, 72,  //  16
 0, 77,  0,  1,  0, 84,    0, 76,       0, 63,    0, 63,  //  17
 0, 72,  0,  1,  0, 80,    0, 72,       0, 59,    0, 59,  //  18
 0, 67,  0,  1,  0, 77,    0, 69,       0, 56,    0, 56,  //  19
 0, 62,  0,  1,  0, 69,    0, 61,       0, 48,    0, 48,  //  20
 0, 57,  0,  1,  0, 61,    0, 53,       0, 40,    0, 40,  //  21
 0, 52,  0,  1,  0, 56,    0, 48,       0, 35,    0, 35,  //  22
 0, 47,  0,  1,  0, 50,    0, 42,       0, 29,    0, 29,  //  23
 0, 43,  0,  1,  0, 45,    0, 37,       0, 24,    0, 24,  //  24
 0, 38,  0,  1,  0, 40,    0, 32,       0, 19,    0, 19,  //  25
 0, 34,  0,  1,  0, 33,    0, 25,       0, 12,    0, 12,  //  26
 0, 28,  0,  1,  0, 28,    0, 20,       0, 7,     0, 7,   //  27
 0, 24,  0,  1,  0, 22,    0, 14,       0, 1,     0, 1,    //  28
 0, 19,  0,  1,  0, 17,    0, 9,        1, 4,     1, 4,    //  29
 0, 14,  0,  1,  0, 14,    0, 6,        1, 1,     1, 1,    //  30
 0, 9,   0,  1,  0, 17,    0, 9,        1, 4,     1, 4,    //  31
 0, 4,   0,  1,  0, 14,    0, 6,        1, 1,     1, 1,    //  32
};

// HP_AMPVol;EQ_POP_HEAVY_HP_AMPVol;EQ_JAZZ_UNIQUE_HP_AMPVol;EQ_USER_HP_AMPVol;EQ_BASS_HP_AMPVol
// Description : increase the HP amplitude from 3dB to 9dB
//               0: 0 dB;1: 3 dB;2: 6 dB;3: 9 dB

// Dac_DigVol;EQ_POP_HEAVY_Dac_DigVol;EQ_JAZZ_UNIQUE_Dac_DigVol;EQ_USER_Dac_DigVol;EQ_BASS_Dac_DigVol
// Description : digital volume of DAC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// HP_ANTIPOPVol
// Description : decrease the HP amplitude from 0dB to -15dB
//               1 db/step: 0 dB - (-15) dB
//               0x00: 0dB; 0xf : -15dB

_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_DATA_
SRUCT_CODEC_CONFIG ACodec_HPoutVol_General[MAX_VOLUME + 1] =
{
#if 1
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
 0, 144, 15, 1,  0, 165,   0, 157,      0, 124,   0, 124, //  1
 0, 140, 15, 1,  0, 153,   0, 145,      0, 132,   0, 132, //  2
 0, 137, 15, 1,  0, 148,   0, 140,      0, 127,   0, 127, //  3
 0, 135, 12, 1,  0, 145,   0, 137,      0, 124,   0, 124, //  4
 0, 135, 9,  1,  0, 143,   0, 135,      0, 122,   0, 122, //  5
 0, 135, 6,  1,  0, 144,   0, 136,      0, 123,   0, 123, //  6
 0, 135, 3,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  7
 0, 135, 0,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  8
 0, 132, 0,  1,  0, 134,   0, 126,      0, 113,   0, 113, //  9
 0, 129, 0,  1,  0, 127,   0, 119,      0, 106,   0, 106, //  10
 0, 122, 0,  1,  0, 122,   0, 114,      0, 101,   0, 101, //  11
 0, 115, 0,  1,  0, 116,   0, 108,      0, 95,    0, 95,  //  12
 0, 110, 0,  1,  0, 110,   0, 102,      0, 89,    0, 89,  //  13
 0, 104, 0,  1,  0, 103,   0, 95,       0, 82,    0, 82,  //  14
 0, 98,  0,  1,  0, 98,    0, 90,       0, 77,    0, 77,  //  15
 0, 92,  0,  1,  0, 93,    0, 85,       0, 72,    0, 72,  //  16
 0, 87,  0,  1,  0, 84,    0, 76,       0, 63,    0, 63,  //  17
 0, 81,  0,  1,  0, 80,    0, 72,       0, 59,    0, 59,  //  18
 0, 75,  0,  1,  0, 77,    0, 69,       0, 56,    0, 56,  //  19
 0, 70,  0,  1,  0, 69,    0, 61,       0, 48,    0, 48,  //  20
 0, 64,  0,  1,  0, 61,    0, 53,       0, 40,    0, 40,  //  21
 0, 58,  0,  1,  0, 56,    0, 48,       0, 35,    0, 35,  //  22
 0, 52,  0,  1,  0, 50,    0, 42,       0, 29,    0, 29,  //  23
 0, 47,  0,  1,  0, 45,    0, 37,       0, 24,    0, 24,  //  24
 0, 41,  0,  1,  0, 40,    0, 32,       0, 19,    0, 19,  //  25
 0, 35,  0,  1,  0, 33,    0, 25,       0, 12,    0, 12,  //  26
 0, 30,  0,  1,  0, 28,    0, 20,       0, 7,     0, 7,   //  27
 0, 24,  0,  1,  0, 22,    0, 14,       0, 7,     0, 7,    //  28
 0, 19,  0,  1,  0, 17,    0, 9,        0, 7,     0, 7,    //  29
 0, 16,  0,  1,  0, 14,    0, 6,        0, 7,     0, 7,    //  30
 0, 14,  0,  1,  0, 17,    0, 9,        0, 7,     0, 7,    //  31
 0, 13,  0,  1,  0, 14,    0, 6,        0, 7,     0, 7,    //  32
#else
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254,//  0
 0, 176, 15, 1,  0, 166,   0, 157,      0, 124,   0, 124,//  1
 0, 164, 15, 1,  0, 154,   0, 145,      0, 132,   0, 132,//  2
 0, 159, 15, 1,  0, 149,   0, 140,      0, 127,   0, 127,//  3
 0, 156, 11, 1,  0, 146,   0, 137,      0, 124,   0, 124,//  4
 0, 154, 9,  1,  0, 144,   0, 135,      0, 122,   0, 122,//  5
 0, 155, 5,  1,  0, 143,   0, 136,      0, 123,   0, 123,//  6
 0, 153, 3,  1,  0, 143,   0, 134,      0, 121,   0, 121,//  7
 0, 153, 0,  1,  0, 143,   0, 134,      0, 121,   0, 121,//  8
 0, 145, 0,  1,  0, 135,   0, 126,      0, 113,   0, 113,//  9
 0, 138, 0,  1,  0, 128,   0, 119,      0, 106,   0, 106,//  10
 0, 133, 0,  1,  0, 123,   0, 114,      0, 101,   0, 101,//  11
 0, 127, 0,  1,  0, 117,   0, 108,      0, 95,    0, 95,  //  12
 0, 121, 0,  1,  0, 111,   0, 102,      0, 89,    0, 89,  //  13
 0, 114, 0,  1,  0, 104,   0, 95,       0, 82,    0, 82,  //  14
 0, 109, 0,  1,  0, 99,    0, 90,       0, 77,    0, 77,  //  15
 0, 104, 0,  1,  0, 94,    0, 85,       0, 72,    0, 72,  //  16
 0, 95,  0,  1,  0, 85,    0, 76,       0, 63,    0, 63,  //  17
 0, 91,  0,  1,  0, 81,    0, 72,       0, 59,    0, 59,  //  18
 0, 88,  0,  1,  0, 78,    0, 69,       0, 56,    0, 56,  //  19
 0, 80,  0,  1,  0, 70,    0, 61,       0, 48,    0, 48,  //  20
 0, 72,  0,  1,  0, 62,    0, 53,       0, 40,    0, 40,  //  21
 0, 67,  0,  1,  0, 57,    0, 48,       0, 35,    0, 35,  //  22
 0, 61,  0,  1,  0, 51,    0, 42,       0, 29,    0, 29,  //  23
 0, 56,  0,  1,  0, 46,    0, 37,       0, 24,    0, 24,  //  24
 0, 51,  0,  1,  0, 41,    0, 32,       0, 19,    0, 19,  //  25
 0, 44,  0,  1,  0, 34,    0, 25,       0, 12,    0, 12,  //  26
 0, 39,  0,  1,  0, 29,    0, 20,       0, 7,     0, 7,   //  27
 0, 33,  0,  1,  0, 23,    0, 14,       0, 7,     0, 7,    //  28
 0, 28,  0,  1,  0, 18,    0, 9,        0, 7,     0, 7,    //  29
 0, 25,  0,  1,  0, 15,    0, 7,        0, 7,     0, 7,    //  30
 0, 28,  0,  1,  0, 18,    0, 9,        0, 7,     0, 7,    //  31
 0, 25,  0,  1,  0, 15,    0, 7,        0, 7,     0, 7,    //  32
#endif

};

// HP_AMPVol;EQ_POP_HEAVY_HP_AMPVol;EQ_JAZZ_UNIQUE_HP_AMPVol;EQ_USER_HP_AMPVol;EQ_BASS_HP_AMPVol
// Description : increase the HP amplitude from 3dB to 9dB
//               0: 0 dB;1: 3 dB;2: 6 dB;3: 9 dB

// Dac_DigVol;EQ_POP_HEAVY_Dac_DigVol;EQ_JAZZ_UNIQUE_Dac_DigVol;EQ_USER_Dac_DigVol;EQ_BASS_Dac_DigVol
// Description : digital volume of DAC channel
//               0.375db/step: 0 dB - (-95) dB
//               0x00: 0dB; 0xff : -95dB

// HP_ANTIPOPVol
// Description : decrease the HP amplitude from 0dB to -15dB
//               1 db/step: 0 dB - (-15) dB
//               0x00: 0dB; 0xf : -15dB

_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_DATA_
SRUCT_CODEC_CONFIG CodecConfig_Europe[MAX_VOLUME + 1] =
{
//+0dB            //+4dB     //+7db       //+12dB   //BASS
 0, 254, 15, 1,  0, 254,   0, 254,      0, 254,   0, 254, //  0
 0, 144, 15, 1,  0, 165,   0, 157,      0, 124,   0, 124, //  1
 0, 140, 15, 1,  0, 153,   0, 145,      0, 132,   0, 132, //  2
 0, 137, 15, 1,  0, 148,   0, 140,      0, 127,   0, 127, //  3
 0, 135, 12, 1,  0, 145,   0, 137,      0, 124,   0, 124, //  4
 0, 135, 9,  1,  0, 143,   0, 135,      0, 122,   0, 122, //  5
 0, 135, 6,  1,  0, 144,   0, 136,      0, 123,   0, 123, //  6
 0, 135, 3,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  7
 0, 135, 0,  1,  0, 142,   0, 134,      0, 121,   0, 121, //  8
 0, 132, 0,  1,  0, 134,   0, 126,      0, 113,   0, 113, //  9
 0, 129, 0,  1,  0, 127,   0, 119,      0, 106,   0, 106, //  10
 0, 122, 0,  1,  0, 122,   0, 114,      0, 101,   0, 101, //  11
 0, 115, 0,  1,  0, 116,   0, 108,      0, 95,    0, 95,  //  12
 0, 110, 0,  1,  0, 110,   0, 102,      0, 89,    0, 89,  //  13
 0, 104, 0,  1,  0, 103,   0, 95,       0, 82,    0, 82,  //  14
 0, 98,  0,  1,  0, 98,    0, 90,       0, 77,    0, 77,  //  15
 0, 92,  0,  1,  0, 93,    0, 85,       0, 72,    0, 72,  //  16
 0, 87,  0,  1,  0, 84,    0, 76,       0, 63,    0, 63,  //  17
 0, 81,  0,  1,  0, 80,    0, 72,       0, 59,    0, 59,  //  18
 0, 75,  0,  1,  0, 77,    0, 69,       0, 56,    0, 56,  //  19
 0, 70,  0,  1,  0, 69,    0, 61,       0, 48,    0, 48,  //  20
 0, 64,  0,  1,  0, 61,    0, 53,       0, 40,    0, 40,  //  21
 0, 58,  0,  1,  0, 56,    0, 48,       0, 35,    0, 35,  //  22
 0, 52,  0,  1,  0, 50,    0, 42,       0, 29,    0, 29,  //  23
 0, 47,  0,  1,  0, 45,    0, 37,       0, 24,    0, 24,  //  24
 0, 41,  0,  1,  0, 40,    0, 32,       0, 19,    0, 19,  //  25
 0, 35,  0,  1,  0, 33,    0, 25,       0, 12,    0, 12,  //  26
 0, 30,  0,  1,  0, 28,    0, 20,       0, 7,     0, 7,   //  27
 0, 24,  0,  1,  0, 22,    0, 14,       0, 7,     0, 7,    //  28
 0, 19,  0,  1,  0, 17,    0, 9,        0, 7,     0, 7,    //  29
 0, 16,  0,  1,  0, 14,    0, 6,        0, 7,     0, 7,    //  30
 0, 14,  0,  1,  0, 17,    0, 9,        0, 7,     0, 7,    //  31
 0, 13,  0,  1,  0, 14,    0, 6,        0, 7,     0, 7,    //  32

};
#endif

typedef  struct _RockCodec_DEVICE_CLASS
{
    DEVICE_CLASS    stRockCodecDevice;            //device father class


    pSemaphore osRockCodecWriteSem;
    pSemaphore osRockCodecControlReqSem;
    SRUCT_CODEC_CONFIG *CodecVolumeTable;
    HDC        hI2S;

    //CodecMode_en_t DacOutMode;
    CodecMode_en_t AdcinMode;
    CodecMode_en_t Codecmode;
    CodecFS_en_t CodecFS;
    CodecFS_en_t CurCodecFS;
    CodecFS_en_t RxCodecFS;
    uint8*  stRxBuffer[2];
    uint32  RxItemID;
    eACodecI2sDATA_WIDTH_t DataWidth;
    eACodecI2sDATA_WIDTH_t AdcDataWidth;

}RockCodec_DEVICE_CLASS;

/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
RockCodec_DEVICE_CLASS * gpstRockcodecDevInf;

//__attribute__((aligned (8))) const

//#define _RockTest_

#ifdef _RockTest_
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
uint8 Codecoutptr[32][176] =
{
    //0
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //8
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //16
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //24
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //32
};

_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
uint32 length = 176 * 32 / 4;
#endif


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
rk_err_t RockcodecDevCheckHandler(HDC dev);
rk_err_t RockCodecShellRead(HDC dev, uint8 * pstr);
rk_err_t RockcodecDevConfigI2S(HDC dev, eI2sDATA_WIDTH_t datawdt, I2sFS_en_t i2S_FS);
rk_err_t RockCodecShellCreate(HDC dev, uint8 * pstr);
rk_err_t RockCodecShellDel(HDC dev, uint8 * pstr);
rk_err_t RockCodecShellHelp(HDC dev, uint8 * pstr);
#if 1
rk_err_t RockcodecDevResume(HDC dev);
rk_err_t RockcodecDevSuspend(HDC dev, uint32 Level);
void RockcodecIntIsr(void);
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RockCodecDev_ExitMode
** Input:uint32 Type
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.6
** Time: 16:04:53
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
COMMON API rk_err_t RockCodecDev_ExitMode(HDC dev, uint32 Type)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);
    if(Type <= Codec_DACoutLINE)
    {
        while(I2sDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS)
        {
            rkos_sleep(10);
        }
    }
    else
    {
        while(I2sRxDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS)
        {
            rkos_sleep(10);
        }
        pRockCodecDev->stRxBuffer[0] = NULL;
        pRockCodecDev->stRxBuffer[1] = NULL;
    }
    Codec_ExitMode(Type);
}
/*******************************************************************************
** Name: RockcodecDev_SetAdcDataWidth
** Input:HDC dev, eI2sDATA_WIDTH_t adcDatawdt
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.14
** Time: 14:20:56
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
COMMON API rk_err_t RockcodecDev_SetAdcDataWidth(HDC dev, eI2sDATA_WIDTH_t adcDatawdt)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    while(I2sRxDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS)
    {
        rkos_sleep(10);
    }
    //rkos_sleep(10);
    if(adcDatawdt == ACodec_I2S_DATA_WIDTH16)
    {
        ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
        I2sDev_Control(pRockCodecDev->hI2S, I2S_DEVICE_SET_RX_DW, (void *)I2S_DATA_WIDTH16);
    }
    else
    {
        ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,I2S_DATA_WIDTH24,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
        I2sDev_Control(pRockCodecDev->hI2S, I2S_DEVICE_SET_RX_DW, (void *)I2S_DATA_WIDTH24);
    }
    pRockCodecDev->AdcDataWidth = adcDatawdt;
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDev_SetAdcInputMode
** Input:HDC dev,  CodecMode_en_t Codecmode
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.13
** Time: 18:53:33
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
COMMON API rk_err_t RockcodecDev_SetAdcInputMode(HDC dev,  CodecMode_en_t AdcinMode)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    while(I2sRxDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS)
    {
        rkos_sleep(10);
    }
    //rkos_sleep(10);
    Codec_SetMode(AdcinMode,pRockCodecDev->CodecFS);
    pRockCodecDev->AdcinMode = AdcinMode;
    pRockCodecDev->stRxBuffer[0] = NULL;
    pRockCodecDev->stRxBuffer[1] = NULL;
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: RockcodecDev_SetDataWidth
** Input:HDC dev, eI2sDATA_WIDTH_t datawdt
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.4
** Time: 15:59:39
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
COMMON API rk_err_t RockcodecDev_SetDataWidth(HDC dev, eI2sDATA_WIDTH_t datawdt)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    while(I2sDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS)
    {
        rkos_sleep(10);
    }
    //rkos_sleep(10);
    if(datawdt == ACodec_I2S_DATA_WIDTH16)
    {
        ACodec_Set_I2S_RX_Mode(TFS_RX_I2S_MODE,I2S_DATA_WIDTH16,IBM_RX_BUS_MODE_NORMAL,I2S_MST_MASTER);
        I2sDev_Control(pRockCodecDev->hI2S, I2S_DEVICE_SET_DW, (void *)I2S_DATA_WIDTH16);
    }
    else
    {
        ACodec_Set_I2S_RX_Mode(TFS_RX_I2S_MODE,I2S_DATA_WIDTH24,IBM_RX_BUS_MODE_NORMAL,I2S_MST_MASTER);
        I2sDev_Control(pRockCodecDev->hI2S, I2S_DEVICE_SET_DW, (void *)I2S_DATA_WIDTH24);
    }
    pRockCodecDev->DataWidth = datawdt;
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDev_SetMode
** Input:HDC dev,  CodecMode_en_t Codecmode,  CodecFS_en_t CodecFS
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_SetMode(HDC dev, CodecMode_en_t Codecmode)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    while(I2sDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS)
    {
        rkos_sleep(10);
    }
    //rkos_sleep(10);
    Codec_SetMode(Codecmode,pRockCodecDev->CodecFS);
    pRockCodecDev->Codecmode = Codecmode;
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: RockcodecDev_SetRate
** Input:HDC dev,  CodecFS_en_t CodecFS
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_RxSetRate(HDC dev,  CodecFS_en_t CodecFS)
{
    rk_err_t ret;
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    if((pRockCodecDev->CurCodecFS != CodecFS) || (pRockCodecDev->RxCodecFS != CodecFS))
    {
        while((I2sDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS) || (I2sRxDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS))
        {
            rkos_sleep(10);
        }
        //rkos_sleep(10);
        ret = I2sDev_Control(pRockCodecDev->hI2S, I2S_DEVICE_SET_FS, (void *)CodecFS);
        if (RK_ERROR == ret)
        {
            rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
            return ret;
        }
        pRockCodecDev->RxCodecFS = CodecFS;
        pRockCodecDev->CurCodecFS = CodecFS;
        Codec_SetSampleRate(CodecFS);
        //Codec_SetMode(pRockCodecDev->Codecmode, CodecFS);
    }
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDev_SetRate
** Input:HDC dev,  CodecFS_en_t CodecFS
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_SetRate(HDC dev,  CodecFS_en_t CodecFS)
{
    rk_err_t ret;
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    if((pRockCodecDev->CurCodecFS != CodecFS)  || (pRockCodecDev->CodecFS != CodecFS))
    {
        while((I2sDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS) || (I2sRxDev_Idle(pRockCodecDev->hI2S) !=  RK_SUCCESS))
        {
            rkos_sleep(10);
        }
        //rkos_sleep(10);

        ret = I2sDev_Control(pRockCodecDev->hI2S, I2S_DEVICE_SET_FS, (void *)CodecFS);
        if (RK_ERROR == ret)
        {
            rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
            return ret;
        }
        pRockCodecDev->CodecFS = CodecFS;
        pRockCodecDev->CurCodecFS = CodecFS;
        Codec_SetSampleRate(CodecFS);
        //Codec_SetMode(pRockCodecDev->Codecmode, CodecFS);
    }
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDev_SetVol
** Input:HDC dev, uint32 userEQMod, uint32 vol
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_SetVol(HDC dev, uint32 userEQMod, uint32 vol)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);
    uint32 VolumeMode = EQ_NOR;
    //rk_printf("Volume = %d",vol);
    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    //rkos_enter_critical();

#ifdef _RK_EQ_
    if(userEQMod < EQ_NOR)
    {
        //printf("*** eqMode=%d***\n", userEQMod);
        VolumeMode = userEQMod;
    }
#endif
    //printf("*** vol = %d  VolumeMode=%d EQ_NOR=%d***\n", vol, VolumeMode ,EQ_NOR);

    if (vol == 0)
    {
        Codec_DACMute();
        while(!ACodec_Get_DAC_MTST())
        {
            rkos_sleep(1);
        }
    }
    else
    {
        if (ACodec_Get_DAC_MTST())
        {
            //printf("ACodec_Get_DAC_MTST \n");
            Codec_DACUnMute();
        }

#ifdef _RK_EQ_

        switch (VolumeMode)
        {
            case EQ_NOR:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].Dac_DigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(CodecConfig_Europe[vol].HP_AMPVol);
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[vol].Dac_DigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[vol].HP_ANTIPOPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].Dac_DigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        //printf("HPAMPVol = %d\n",ACodec_HPoutVol_General[vol].HP_AMPVol);
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[vol].HP_AMPVol);

                        //printf("DacDigVol = %d\n",ACodec_HPoutVol_General[vol].Dac_DigVol);
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[vol].Dac_DigVol);

                        //printf("HP_Gain = %d\n",ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                    }
                }

                break;
            case EQ_POP:
            case EQ_HEAVY:
                //printf("\n!!!cjh!!! EQ_HEAVY\n");
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_POP_HEAVY_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(CodecConfig_Europe[vol].EQ_POP_HEAVY_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[vol].EQ_POP_HEAVY_DacDigVol);
                        //printf("\n!!!cjh!!! vol=%d table=%d\n",vol ,CodecConfig_Europe[vol].HP_ANTIPOPVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[vol].HP_ANTIPOPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_POP_HEAVY_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_POP_HEAVY_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[vol].EQ_POP_HEAVY_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[vol].EQ_POP_HEAVY_DacDigVol);
                        //printf("\n!!!cjh!!!2  vol=%d table=%d\n",vol ,ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                    }
                }
                break;
            case EQ_JAZZ:
            case EQ_UNIQUE:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                   if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                   {
                       ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_JAZZ_UNIQUE_HP_AMPVol);
                       ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_JAZZ_UNIQUE_DacDigVol);
                       ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                   }
                   else
                   {
                       ACodec_Set_HP_AMP(CodecConfig_Europe[vol].EQ_JAZZ_UNIQUE_HP_AMPVol);
                       ACodec_Set_DAC_DigVol(CodecConfig_Europe[vol].EQ_JAZZ_UNIQUE_DacDigVol);
                       ACodec_Set_HP_Gain(CodecConfig_Europe[vol].HP_ANTIPOPVol);
                   }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_JAZZ_UNIQUE_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_JAZZ_UNIQUE_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[vol].EQ_JAZZ_UNIQUE_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[vol].EQ_JAZZ_UNIQUE_DacDigVol);
                        //printf("\n!!!cjh!!!3  vol=%d table=%d\n",vol ,ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                    }
                }
                break;

            case EQ_USER:

                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_USER_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_USER_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(CodecConfig_Europe[vol].EQ_USER_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[vol].EQ_USER_DacDigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[vol].HP_ANTIPOPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_USER_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_USER_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[vol].EQ_USER_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[vol].EQ_USER_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                    }
                }
                break;

            case EQ_BASS:
                if(VOLTAB_CONFIG == VOL_Europe)
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_BASS_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_BASS_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(CodecConfig_Europe[vol].EQ_BASS_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(CodecConfig_Europe[vol].EQ_BASS_DacDigVol);
                        ACodec_Set_HP_Gain(CodecConfig_Europe[vol].HP_ANTIPOPVol);
                    }
                }
                else
                {
                    if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
                    {
                        ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].EQ_BASS_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].EQ_BASS_DacDigVol);
                        ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
                    }
                    else
                    {
                        ACodec_Set_HP_AMP(ACodec_HPoutVol_General[vol].EQ_BASS_HP_AMPVol);
                        ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[vol].EQ_BASS_DacDigVol);
                        ACodec_Set_HP_Gain(ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                    }
                }

                break;

            default:
                break;
        }
#else
        if(VOLTAB_CONFIG == VOL_Europe)
        {
            if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
            {
                ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].HP_AMPVol);
                ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].Dac_DigVol);
                ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
            }
            else
            {
                ACodec_Set_HP_AMP(CodecConfig_Europe[vol].HP_AMPVol);
                ACodec_Set_DAC_DigVol(CodecConfig_Europe[vol].Dac_DigVol);
                ACodec_Set_HP_Gain(CodecConfig_Europe[vol].HP_ANTIPOPVol);
            }
        }
        else
        {
            if(ACODEC_OUT_CONFIG == ACODEC_OUT_LINE)
            {
                ACodec_Set_HP_AMP(ACodec_LineOutVol_General[vol].HP_AMPVol);
                ACodec_Set_DAC_DigVol(ACodec_LineOutVol_General[vol].Dac_DigVol);
                ACodec_Set_LO_Gain(ACodec_LineOutVol_General[vol].HP_ANTIPOPVol);
            }
            else
            {
                //printf("HPAMPVol = %d\n",ACodec_HPoutVol_General[vol].HP_AMPVol);
                ACodec_Set_HP_AMP(ACodec_HPoutVol_General[vol].HP_AMPVol);

                //printf("DacDigVol = %d\n",ACodec_HPoutVol_General[vol].Dac_DigVol);
                ACodec_Set_DAC_DigVol(ACodec_HPoutVol_General[vol].Dac_DigVol);

                //printf("HP_Gain = %d\n",ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
                ACodec_Set_HP_Gain(ACodec_HPoutVol_General[vol].HP_ANTIPOPVol);
            }
        }
#endif
    }

    DelayUs(10);
    //Codec_SetVolumet(vol);
    //rkos_exit_critical();
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: RockcodecDev_SetVolTable
** Input:HDC dev,  uint32 voltable
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_SetVolTable(HDC dev,  uint32 voltable)
{
    RockCodec_DEVICE_CLASS * pRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);

    rkos_semaphore_take(pRockCodecDev->osRockCodecControlReqSem, MAX_DELAY);
    //rkos_enter_critical();
    //VolumeTable set
    //if null,set default table
#if 1
    if (voltable == VOLTABLE_HPOUTVOL_GENERAL)
    {
        pRockCodecDev->CodecVolumeTable = ACodec_HPoutVol_General;
    }
    else if (voltable == VOLTABLE_LINOUTVOL_GENERAL)
    {
        pRockCodecDev->CodecVolumeTable = ACodec_LineOutVol_General;
    }
    else
    {
        pRockCodecDev->CodecVolumeTable = CodecConfig_Europe;
    }
#endif
#if 0
    if (voltable == VOLTABLE_GENERAL)
    {
        pRockCodecDev->CodecVolumeTable = CodecConfig_General;
    }
    else
    {
        pRockCodecDev->CodecVolumeTable = CodecConfig_Europe;
    }
#endif
    //rkos_exit_critical();
    rkos_semaphore_give(pRockCodecDev->osRockCodecControlReqSem);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: RockcodecDev_Read
** Input:DEVICE_CLASS* dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_size_t
** Owner:chad.ma
** Date: 2016.01.16
** Time: 13:46:51
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_Read(HDC dev, uint8* buffer, uint32 size,uint8 mode)
{
    RockCodec_DEVICE_CLASS * pstRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);
    HDC hI2S = pstRockCodecDev->hI2S;
    int32  ret = RK_SUCCESS;
    uint32 index;

    if(pstRockCodecDev->CurCodecFS != pstRockCodecDev->RxCodecFS)
    {
        RockcodecDev_RxSetRate(pstRockCodecDev,  pstRockCodecDev->RxCodecFS);
    }
    pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID] = buffer;
    if (mode == SYNC_MODE)
    {
        //DEBUG("RockcodecDev_Read: mode == SYNC_MODE");
        ret = I2sDev_Read(hI2S, (uint32 *)buffer, size, SYNC_MODE);
        if (ret != RK_SUCCESS)
        {
            return ret;
        }
    }
    else if (mode == ASYNC_MODE)
    {
        //DEBUG("RockcodecDev_Read: mode == ASYNC_MODE");
        ret = I2sDev_Read(hI2S, (uint32 *)buffer, size, ASYNC_MODE);
        if (ret != RK_SUCCESS)
        {
            return ret;
        }
    }
    #if 1
    pstRockCodecDev->RxItemID ^= 1;
    //printf("size%d\n",size);
    if((pstRockCodecDev->AdcinMode == Codec_Mic1Mono) && (pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID] != NULL))
    {
        if(VDW_TX_WIDTH_16BIT == pstRockCodecDev->AdcDataWidth)
        {
            uint16 * pInputBuffere16 =  (uint16 *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint16 *)RecordInputBuffer;
            uint16 * pOutputBuffere16 = (uint16 *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint16 *)RecordOutputBuffer;
            for(index = 0; index < size/4; index++)
            {
                pOutputBuffere16[index] =   pInputBuffere16[2 * index];
            }
        }
        else if (VDW_TX_WIDTH_24BIT == pstRockCodecDev->AdcDataWidth)
        {
            uint8 * pInputBuffere8=  (uint8  *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint8 *)RecordInputBuffer;
            uint8 * pOutputBuffere8 = (uint8  *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint8 *)RecordOutputBuffer;
            for (index = 0; index < size/6; index++)
            {
                pOutputBuffere8[3 * index + 0] =  pInputBuffere8[6 * index + 0];
                pOutputBuffere8[3 * index + 1] =  pInputBuffere8[6 * index + 1];
                pOutputBuffere8[3 * index + 2] =  pInputBuffere8[6 * index + 2];
            }
        }
    }
    else if((pstRockCodecDev->AdcinMode == Codec_Mic2Mono) && (pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID] != NULL))
    {
        if(VDW_TX_WIDTH_16BIT == pstRockCodecDev->AdcDataWidth)
        {
            uint16 * pInputBuffere16 =  (uint16  *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint16 *)RecordInputBuffer;
            uint16 * pOutputBuffere16 = (uint16  *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint16 *)RecordOutputBuffer;

            for (index = 0; index < size/4; index++)
            {
                pOutputBuffere16[index] =   pInputBuffere16[2 * index + 1];
            }
        }
        else if (VDW_TX_WIDTH_24BIT == pstRockCodecDev->AdcDataWidth)
        {
            uint8 * pInputBuffere8=  (uint8  *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint8 *)RecordInputBuffer;
            uint8 * pOutputBuffere8 = (uint8  *)pstRockCodecDev->stRxBuffer[pstRockCodecDev->RxItemID];//(uint8 *)RecordOutputBuffer;
            for (index = 0; index < size/6; index++)
            {
                pOutputBuffere8[3 * index + 0] =  pInputBuffere8[6 * index + 3];
                pOutputBuffere8[3 * index + 1] =  pInputBuffere8[6 * index + 4];
                pOutputBuffere8[3 * index + 2] =  pInputBuffere8[6 * index + 5];
            }
        }
    }
    #endif
    return size;
}

/*******************************************************************************
** Name: RockcodecDevWrite
** Input:DEVICE_CLASS* dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:46:51
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
rk_err_t RockcodecDev_Write(HDC dev, uint8* buffer, uint32 size,uint8 mode)
{
    RockCodec_DEVICE_CLASS * pstRockCodecDev =  (RockCodec_DEVICE_CLASS *)(dev);
    HDC hI2S = pstRockCodecDev->hI2S;
    int32  ret = RK_SUCCESS;

    if(pstRockCodecDev->CurCodecFS != pstRockCodecDev->CodecFS)
    {
        RockcodecDev_SetRate(pstRockCodecDev,  pstRockCodecDev->CodecFS);
    }
    if (mode == SYNC_MODE)
    {
        ret = I2sDev_Write(hI2S, (uint32 *)buffer, size, SYNC_MODE);
        if (ret != RK_SUCCESS)
        {
            return ret;
        }
        return size;
    }
    else if (mode == ASYNC_MODE)
    {
        ret = I2sDev_Write(hI2S, (uint32 *)buffer, size, ASYNC_MODE);
        if (ret != RK_SUCCESS)
        {
            return ret;
        }
        return size;
    }

}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RockcodecDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 17:41:09
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
COMMON FUN rk_err_t RockcodecDevCheckHandler(HDC dev)
{
    if(gpstRockcodecDevInf != dev)
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDevConfigI2S
** Input:HDC dev
** Return: void
** Owner:cjh
** Date: 2015.6.12
** Time: 10:07:32
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
COMMON FUN rk_err_t RockcodecDevConfigI2S(HDC dev, eI2sDATA_WIDTH_t datawdt, I2sFS_en_t i2S_FS)
{
    int32  ret = RK_SUCCESS;
    RockCodec_DEVICE_CLASS * pRockCodecDev = (RockCodec_DEVICE_CLASS *)(dev);
    HDC hI2S = pRockCodecDev->hI2S;
    I2S_DEVICE_CONFIG_REQ_ARG stI2sDevArg;

    stI2sDevArg.i2smode = I2S_SLAVE_MODE;//;I2S_SLAVE_MODE
    stI2sDevArg.i2sCS = I2S_IN; //I2S_EXT
    stI2sDevArg.I2S_FS = i2S_FS;//I2S_FS_44100Hz;
    stI2sDevArg.Data_width = datawdt;
    stI2sDevArg.BUS_FORMAT = I2S_FORMAT;
    stI2sDevArg.I2S_Bus_mode = I2S_NORMAL_MODE;

    ret = I2sDev_Control(hI2S, I2S_DEVICE_INIT_CMD, &stI2sDevArg);

    if (RK_ERROR == ret)
    {
        return ret;
    }
    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: RockcodecDevCreate
** Input:void
** Return: DEVICE_CLASS *
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:40:31
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
INIT API HDC RockCodecDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS* pstDev;
    RockCodec_DEVICE_CLASS * psRockCodecDev;
    ROCKCODEC_DEV_ARG * pstRockCodecArg;

    if (arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    pstRockCodecArg = (ROCKCODEC_DEV_ARG *)arg;

    psRockCodecDev =  rkos_memory_malloc(sizeof(RockCodec_DEVICE_CLASS));
    if (psRockCodecDev == NULL)
    {
        return (HDC) RK_ERROR;
    }

    psRockCodecDev->osRockCodecWriteSem = rkos_semaphore_create(1,1);
    psRockCodecDev->osRockCodecControlReqSem = rkos_semaphore_create(1,1);

    if ((      psRockCodecDev->osRockCodecWriteSem
               && psRockCodecDev->osRockCodecControlReqSem) == 0)
    {
        rkos_semaphore_delete(psRockCodecDev->osRockCodecWriteSem);
        rkos_semaphore_delete(psRockCodecDev->osRockCodecControlReqSem);

        rkos_memory_free(psRockCodecDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)psRockCodecDev;

    pstDev->Idle1EventTime = 0;
    pstDev->SuspendMode = ENABLE_MODE;
    pstDev->suspend = RockcodecDevSuspend;
    pstDev->resume  = RockcodecDevResume;

    psRockCodecDev->hI2S = pstRockCodecArg->hI2s;
    psRockCodecDev->CodecFS = pstRockCodecArg->arg.SampleRate; //I2S_FS_44100Hz;
    psRockCodecDev->Codecmode = pstRockCodecArg->arg.DacOutMode;
    psRockCodecDev->AdcinMode = pstRockCodecArg->arg.AdcinMode;
    psRockCodecDev->DataWidth = pstRockCodecArg->arg.DataWidth;
//    psRockCodecDev->AudioInOutType.EffectCtl.Mode = pstRockCodecArg->eqMode;
//    psRockCodecDev->CodecVolumeTable = ACodec_HPoutVol_General;//// CodecConfig_General;(SRUCT_CODEC_CONFIG *)(pstRockCodecArg->pCodecVolumeTable);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_ROCKCODEC_DEV, SEGMENT_OVERLAY_ALL);
#endif
    //device init...
    RockCodecDevHwInit(DevID, 0);
    if (RockcodecDevInit(psRockCodecDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(psRockCodecDev->osRockCodecWriteSem);
        rkos_semaphore_delete(psRockCodecDev->osRockCodecControlReqSem);

        rkos_memory_free(psRockCodecDev);
        printf("create RockCodecDev fail");
        return (HDC) RK_ERROR;
    }
    gpstRockcodecDevInf = psRockCodecDev;
    //printf("create RockCodecDev success");
    return (HDC)pstDev;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RockcodecDevResume
** Input:DEVICE_CLASS *
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:57:19
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
rk_err_t RockcodecDevResume(HDC dev)
{
     RockCodec_DEVICE_CLASS * pstRockCodecDev = (RockCodec_DEVICE_CLASS *)dev;
     if(RockcodecDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     RockCodecDevHwInit(pstRockCodecDev->stRockCodecDevice.DevClassID, 0);
     RockcodecDevInit(dev);

     pstRockCodecDev->stRockCodecDevice.State = DEV_STATE_WORKING;
     RKDev_Resume(pstRockCodecDev->hI2S);
     return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDevSuspend
** Input:HDC dev, uint32 Level
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:56:05
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
rk_err_t RockcodecDevSuspend(HDC dev, uint32 Level)
{
    RockCodec_DEVICE_CLASS * pstRockCodecDev = (RockCodec_DEVICE_CLASS *)dev;
    if(RockcodecDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstRockCodecDev->stRockCodecDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstRockCodecDev->stRockCodecDevice.State = DEV_SATE_IDLE2;
    }

    RockCodecDevHwDeInit(pstRockCodecDev->stRockCodecDevice.DevID, 0);

    RKDev_Suspend(pstRockCodecDev->hI2S);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDevDelete
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:41:18
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
rk_err_t RockCodecDev_Delete(uint32 DevID, void * arg)
{
    ROCKCODEC_DEV_ARG *stRockCodecDevArg = (ROCKCODEC_DEV_ARG *)arg;

    stRockCodecDevArg->hI2s = gpstRockcodecDevInf->hI2S;
    RockcodecDevDeInit(gpstRockcodecDevInf);

    rkos_semaphore_delete( gpstRockcodecDevInf->osRockCodecWriteSem );
    rkos_semaphore_delete( gpstRockcodecDevInf->osRockCodecControlReqSem );

    rkos_memory_free(gpstRockcodecDevInf);
    gpstRockcodecDevInf = NULL;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_ROCKCODEC_DEV);
#endif

    RockCodecDevHwDeInit(DevID, 0);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockcodecDevDeInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:38:30
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
rk_err_t RockcodecDevDeInit(HDC dev)
{
    RockCodec_DEVICE_CLASS * pstRockCodecDev = (RockCodec_DEVICE_CLASS *) dev;

#ifdef PA_CS8508L
    //rk_printf("select I2S_DEV1_PA_EN\n");
    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin0, IOMUX_GPIO1A0_IO);
    Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin0, GPIO_IN);
    Grf_GPIO_SetPinPull(GPIO_CH1, GPIOPortA_Pin0, DISABLE);
#endif

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: RockcodecDevInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:44:46
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
rk_err_t RockcodecDevInit(HDC dev)
{
    RockCodec_DEVICE_CLASS * psRockCodecDev = (RockCodec_DEVICE_CLASS *)dev;

    if(psRockCodecDev == NULL)
    {
        printf("RockcodecDevInit ERROR\n");
        return RK_ERROR;
    }
    Codec_PowerOnInitial();
    ACodec_Set_I2S_RX_Mode(TFS_RX_I2S_MODE,psRockCodecDev->DataWidth,IBM_RX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,psRockCodecDev->DataWidth,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);

#ifndef _BROAD_LINE_OUT_
    Codec_SetSampleRate(psRockCodecDev->CodecFS);
    Codec_SetMode(Codec_DACoutHP,psRockCodecDev->CodecFS);
    psRockCodecDev->Codecmode = Codec_DACoutHP;
#else
    Codec_SetMode(Codec_DACoutLINE,psRockCodecDev->CodecFS);
    psRockCodecDev->Codecmode = Codec_DACoutLINE;
#endif

#ifdef PA_CS8508L
    //rk_printf("select I2S_DEV1_PA_EN\n");
    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin0, IOMUX_GPIO1A0_IO);
    Gpio_SetPinDirection(GPIO_CH1, GPIOPortA_Pin0, GPIO_OUT);
    Gpio_SetPinLevel(GPIO_CH1, GPIOPortA_Pin0, GPIO_HIGH);
    Grf_GPIO_SetPinPull(GPIO_CH1, GPIOPortA_Pin0, ENABLE);
#endif

    psRockCodecDev->stRxBuffer[0] = NULL;
    psRockCodecDev->stRxBuffer[1] = NULL;

    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _USE_SHELL_
#ifdef _RK_ACODE_SHELL_
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_DATA_
static SHELL_CMD ShellRockCodecName[] =
{
    "pcb",NULL,"NULL","NULL",
    "create",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",
    "suspend",NULL,"NULL","NULL",
    "resume",NULL,"NULL","NULL",
    "read",NULL,"NULL","NULL",
    "write",NULL,"NULL","NULL",
    "control",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "\b", NULL,"NULL","NULL",                         // the end
};

/*******************************************************************************
** Name: RockCodecDev_Shell
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
rk_err_t RockCodecDev_Shell(HDC dev,  uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;
    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellRockCodecName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;


    pItem += StrCnt;
    pItem++;                        //remove '.',the point is the useful item
    switch (i)
    {
        case 0x00:  //pcb
            ret = RockCodecShellPcb(dev,pItem);
            break;

        case 0x01:  //create
            ret = RockCodecShellCreate(dev,pItem);
            break;

        case 0x02:  //test
            ret = RockCodecShellTest(dev,pItem);
            break;

        case 0x03:  //Del
            ret = RockCodecShellDel(dev,pItem);
            break;

        case 0x04:  //suspend
            //ret = RockCodecShellSuspend(dev,pItem);
            break;

        case 0x05:  //resume
            //ret = RockCodecShellResume(dev,pItem);
            break;

        case 0x06:  //read  chad.ma add for test
            ret = RockCodecShellRead(dev,pItem);
            break;

        case 0x07:  //write
            //ret = RockCodecShellWrite(dev,pItem);
            break;

        case 0x08:  //control
            //ret = RockCodecShellControl(dev,pItem);
            break;

        case 0x09:  //help
            ret = RockCodecShellHelp(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RockCodecShellRead
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.22
** Time: 11:36:43
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
SHELL FUN rk_err_t RockCodecShellRead(HDC dev, uint8 * pstr)
{
    HDC hRockCodec;
    rk_err_t ret;

    uint8  pBuffer[1024]= {0};
    uint32 size;


#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("RockCodec.read : read RockCodec data\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    SetI2SFreq(I2S_DEV0, I2S_XIN12M, NULL);//测试默认12M
    hRockCodec = RKDev_Open(DEV_CLASS_ROCKCODEC,0,NOT_CARE);

#ifndef _BROAD_LINE_OUT_
    RockcodecDev_SetMode(hRockCodec,Codec_MicStero);
#else
    RockcodecDev_SetMode(hRockCodec,Codec_MicStero);
#endif


    size = RockcodecDev_Read(hRockCodec,pBuffer,1024,ASYNC_MODE);

    printf("size = %d \n",size);

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: RockCodecShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.15
** Time: 8:24:52
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
SHELL FUN rk_err_t RockCodecShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;

    //Get RockCodecDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    if (RKDev_Delete(DEV_CLASS_ROCKCODEC, DevID, &stRockCodecDevArg) != RK_SUCCESS)
    {
        printf("RockCodecDev delete failure DevID %d\n",DevID);
    }
    if (stRockCodecDevArg.hI2s != NULL)
    {
        if (RKDev_Close(stRockCodecDevArg.hI2s) != RK_SUCCESS)
        {
            printf("rockCode i2s failure\n");
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: RockcodecDevShellHelp
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.11.3
** Time: 11:19:43
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
SHELL FUN rk_err_t RockCodecShellHelp(HDC dev, uint8 * pstr)
{
    pstr--;

    if ( StrLenA( pstr) != 0)
        return RK_ERROR;

    rk_print_string("Rockcodec命令集提供了一系列的命令对Rockcodec进行操作\r\n");
    rk_print_string("Rockcodec包含的子命令如下:          \r\n");
    rk_print_string("pcb       显示pcb信息               \r\n");
    rk_print_string("open      打开Rockcodec             \r\n");
    rk_print_string("test      测试Rockcodec             \r\n");
    rk_print_string("close     关闭Rockcodec             \r\n");
    rk_print_string("suspend   suspend Rockcodec         \r\n");
    rk_print_string("resume    resume Rockcodec          \r\n");
    rk_print_string("read      read Rockcodec            \r\n");
    rk_print_string("write     write Rockcodec           \r\n");
    rk_print_string("control   control Rockcodec         \r\n");
    rk_print_string("help      显示Rockcodec命令帮助信息 \r\n");

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: RockCodecShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
rk_err_t RockCodecShellPcb(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("RockCodec.pcb : pcd info \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: RockCodecShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
rk_err_t RockCodecShellTest(HDC dev, uint8 * pstr)
{
    HDC hRockCodec;
    //RockCodec_DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 vol,time = 0;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    I2S_DEVICE_CONFIG_REQ_ARG stI2sDevArg;
    I2S_DEV_ARG stI2Sarg;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("RockCodec.test : test RockCodec \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    SetI2SFreq(I2S_DEV0, I2S_XIN12M, NULL);//测试默认12M
    //uint32 SetI2SFreq(UINT32 I2sId,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
//    #ifdef _JTAG_printf_
//    ret=RKDev_Create(DEV_CLASS_DMA, DMA_CHN1, NULL);
//    if (ret != RK_SUCCESS)
//    {
//        printf("DMA RKDev_Create failure\n");
//        return RK_ERROR;
//    }
//    printf("DMA RKDev_Create success\n");
//    #endif

//    stI2Sarg.hDma = RKDev_Open(DEV_CLASS_DMA, DMA_CHN0, NOT_CARE);
//    if (stI2Sarg.hDma == NULL)
//    {
//        printf("DMA RKDev_Open failure\n");
//        return RK_ERROR;
//    }
//    printf("DMA RKDev_Open success\n");
//    stI2Sarg.Channel = I2S_SEL_ACODEC;
//    ret=RKDev_Create(DEV_CLASS_I2S, I2S_DEV0, &stI2Sarg);
//    if (ret != RK_SUCCESS)
//    {
//        printf("I2S RKDev_Create failure\n");
//        return RK_ERROR;
//    }
//    printf("\nI2S RKDev_Create success\n");
//    stRockCodecDevArg.hI2s = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);
//    if (stRockCodecDevArg.hI2s == NULL)
//    {
//        printf("I2SDev Open failure\n");
//        return RK_ERROR;
//    }
//    printf("\n I2SDev Open OK\n");
//    stRockCodecDevArg.arg.SampleRate = I2S_FS_44100Hz;
//#ifndef _BROAD_LINE_OUT_
//    stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutHP;
//#else
//    stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutLINE;
//#endif
//    stRockCodecDevArg.arg.AdcinMode = Codec_Standby;
//#ifdef CODEC_24BIT
//        stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_24BIT;
//#else
//        stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_16BIT;
//#endif//    ret = RKDev_Create(DEV_CLASS_ROCKCODEC, 0,&stRockCodecDevArg);
//    if (ret != RK_SUCCESS)
//    {
//        printf("ROCKCODEC Create failure\n");
//        return RK_ERROR;
//    }
//    printf("ROCKCODEC Create success\n");
    hRockCodec = RKDev_Open(DEV_CLASS_ROCKCODEC,0,NOT_CARE);

    //RockcodecDevInit(hRockCodec);//Open I2S clk CODEC Initialization is the first configuration step

    RockcodecDev_SetRate(hRockCodec,CodecFS_44100Hz);//Smaple rate PLL  CodecFS_44100Hz
    //RockcodecDev_SetVol(hRockCodec,27);
    RockcodecDev_SetVol(hRockCodec, EQ_NOR, 30);

    //ACodec InitI2S_TX/RX En I2SCLK
    //ACodec_Set_I2S_Mode(I2S_MASTER_MODE,I2S_DATA_WIDTH16,I2S_NORMAL_MODE,I2S_MASTER_MODE);

    //enable ADC power on
#ifndef _BROAD_LINE_OUT_
    RockcodecDev_SetMode(hRockCodec,Codec_DACoutHP);
#else
    RockcodecDev_SetMode(hRockCodec,Codec_DACoutLINE);
#endif
    while (1)
    {
#ifdef RockTest
        RockcodecDev_Write(hRockCodec,(uint8 *)Codecoutptr, length, ASYNC_MODE);
#endif
        time++;
        if (time == 800)
        {
            UartDev_Write(dev, "test over",9,SYNC_MODE,NULL);
            break;
        }
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: RockCodecShellOpen
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
rk_err_t RockCodecShellOpen(HDC dev, uint8 * pstr)
{
    HDC hRockCodec;
    RockCodec_DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 vol;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    I2S_DEVICE_CONFIG_REQ_ARG stI2sDevArg;
    I2S_DEV_ARG  stI2Sarg;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("RockCodec.open : openRockCodec \r\n");
            return RK_SUCCESS;
        }
    }
#endif
#ifdef __DRIVER_I2S_I2SDEVICE_C__
    stI2Sarg.hDma = RKDev_Open(DEV_CLASS_DMA, DMA_CHN0, NOT_CARE);
    stI2Sarg.Channel = I2S_SEL_DEV1_PA;

    stI2Sarg.req_arg.i2smode = I2S_SLAVE_MODE;
    stI2Sarg.req_arg.i2sCS = I2S_IN;
    stI2Sarg.req_arg.I2S_FS = I2S_FS_44100Hz;
    stI2Sarg.req_arg.BUS_FORMAT = I2S_FORMAT;
    stI2Sarg.req_arg.Data_width = I2S_DATA_WIDTH24;
    stI2Sarg.req_arg.I2S_Bus_mode = I2S_NORMAL_MODE;

    ret=RKDev_Create(DEV_CLASS_I2S, I2S_DEV1, &stI2Sarg);
    if (ret != RK_SUCCESS)
    {
        UartDev_Write(dev, "I2S RKDev_Create failure\n",25,SYNC_MODE,NULL);
    }

    stRockCodecDevArg.hI2s = RKDev_Open(DEV_CLASS_I2S, I2S_DEV1, NOT_CARE);
    stRockCodecDevArg.arg.SampleRate = I2S_FS_44100Hz;
#ifndef _BROAD_LINE_OUT_
    stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutHP;
#else
    stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutLINE;
#endif
    stRockCodecDevArg.arg.AdcinMode = Codec_Standby;
#ifdef CODEC_24BIT
    stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_24BIT;
#else
    stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_16BIT;
#endif
    ret = RKDev_Create(DEV_CLASS_ROCKCODEC, 0,&stRockCodecDevArg);

    if (ret != RK_SUCCESS)
    {
        UartDev_Write(dev, "open failure",17,SYNC_MODE,NULL);
    }
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RockCodecShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
rk_err_t RockCodecShellCreate(HDC dev, uint8 * pstr)
{
    HDC hRockCodec;
    RockCodec_DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 vol;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    I2S_DEVICE_CONFIG_REQ_ARG stI2sDevArg;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("RockCodec.open : openRockCodec \r\n");
            return RK_SUCCESS;
        }
    }
#endif

#ifdef __DRIVER_I2S_I2SDEVICE_C__
    stRockCodecDevArg.hI2s = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);
    stRockCodecDevArg.arg.SampleRate = I2S_FS_44100Hz;
#ifndef _BROAD_LINE_OUT_
    stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutHP;
#else
    stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutLINE;
#endif
    stRockCodecDevArg.arg.AdcinMode = Codec_Standby;
#ifdef CODEC_24BIT
    stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_24BIT;
#else
    stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_16BIT;
#endif
    ret = RKDev_Create(DEV_CLASS_ROCKCODEC, 0,&stRockCodecDevArg);

    if (ret != RK_SUCCESS)
    {
        printf("open failure");
    }
#endif

    return RK_SUCCESS;
}

#endif
#endif
#endif




