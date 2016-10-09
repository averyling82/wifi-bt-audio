/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Driver\RockCodec\RockCodecDevice.h
* Owner: HJ
* Date: 2014.3.10
* Time:
* Desc: Uart Device Class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
*      HJ     2014.3.10                    1.0
********************************************************************************************
*/

#ifndef __DRIVER_ROCK_CODECDEVICE_H__
#define __DRIVER_ROCK_CODECDEVICE_H__

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
// Interrupt status bit

#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_  __attribute__((section("driver_rockcodec_rockcodecdevice_common")))
#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_  __attribute__((section("driver_rockcodec_rockcodecdevice_init")))
#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_  __attribute__((section("driver_rockcodec_rockcodecdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_DATA_      _DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_
#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_DATA_      _DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_SHELL_DATA_      __attribute__((section("driver_rockcodec_rockcodecdevice_shell_data")))
#define _DRIVER_ROCKCODEC_ROCKCODECDEVICE_COMMON_DATA_      __attribute__((section("driver_rockcodec_rockcodecdevice_common_data")))
#else
#error Unknown compiling tools.
#endif

#if 1
#define VOLTABLE_HPOUTVOL_GENERAL     1
#define VOLTABLE_EUROPE               2
#define VOLTABLE_LINOUTVOL_GENERAL    3
#endif

#if 0
#define VOLTABLE_GENERAL  1
#define VOLTABLE_EUROPE   2
#endif

typedef enum _CODEC_DEVICE_CONTROL_CMD
{
    CODEC_DEVICE_CONFIG_I2S_CMD,
    CODEC_DEVICE_SET_MODE_CMD,
    CODEC_DEVICE_SET_SAMPLERATE_CMD,
    CODEC_DEVICE_SET_VOLUME_CMD,
    CODEC_DEVICE_SET_VOLTABLE_CMD,
    CODEC_DEVICE_NUM

}CODEC_DEVICE_CONTROL_CMD;

typedef struct _ROCKCODEC_DEV_CONFIG_ARG
{
    //CodecMode_en_t Codecmode;
    CodecMode_en_t DacOutMode;
    CodecMode_en_t AdcinMode;
    CodecFS_en_t SampleRate;
    eACodecI2sDATA_WIDTH_t DataWidth;

}ROCKCODEC_DEV_CONFIG_ARG;

typedef struct _ROCKCODEC_DEV_ARG
{
    HDC hI2s;
    ROCKCODEC_DEV_CONFIG_ARG arg;
    //void *pCodecVolumeTable;
}ROCKCODEC_DEV_ARG;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t RockCodecDev_ExitMode(HDC dev, uint32 Type);
extern rk_err_t RockcodecDev_SetAdcDataWidth(HDC dev, eI2sDATA_WIDTH_t adcDatawdt);
extern rk_err_t RockcodecDev_SetAdcInputMode(HDC dev,  CodecMode_en_t AdcinMode);
extern rk_err_t RockcodecDev_SetDataWidth(HDC dev, eI2sDATA_WIDTH_t datawdt);
extern rk_err_t RockcodecDev_SetVol(HDC dev, uint32 userEQMod, uint32 vol);
extern rk_err_t RockcodecDev_Write(HDC dev, uint8* buffer, uint32 size,uint8 mode);
extern rk_err_t RockcodecDev_SetVolTable(HDC dev, uint32 voltable);
extern rk_err_t RockcodecDev_RxSetRate(HDC dev, CodecFS_en_t CodecFS);
extern rk_err_t RockcodecDev_SetRate(HDC dev, CodecFS_en_t CodecFS);
extern rk_err_t RockcodecDev_SetMode(HDC dev, CodecMode_en_t Codecmode);
extern HDC RockCodecDev_Create(uint32 DevID, void *arg);
extern rk_err_t RockCodecDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t RockCodecDev_Delete(uint32 DevID, void * arg);

#endif  //#ifndef _RKCODEC_H
/*
********************************************************************************
*
*                         End of rockcedec.h
*
********************************************************************************
*/

