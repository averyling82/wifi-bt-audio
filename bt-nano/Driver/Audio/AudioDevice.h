/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Driver\Audio\AudioDevice.h
* Owner: Aaron.sun
* Date: 2014.5.7
* Time: 17:48:32
* Desc: Audio Device Class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.5.7     17:48:32   1.0
********************************************************************************************
*/

#ifndef __DRIVER_AUDIO_AUDIODEVICE_H__
#define __DRIVER_AUDIO_AUDIODEVICE_H__

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
#define _DRIVER_AUDIO_AUDIODEVICE_COMMON_  __attribute__((section("driver_audio_audiodevice_common")))
#define _DRIVER_AUDIO_AUDIODEVICE_INIT_  __attribute__((section("driver_audio_audiodevice_init")))
#define _DRIVER_AUDIO_AUDIODEVICE_SHELL_  __attribute__((section("driver_audio_audiodevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_AUDIO_AUDIODEVICE_DATA_      _DRIVER_AUDIO_AUDIODEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_AUDIO_AUDIODEVICE_DATA_      __attribute__((section("driver_audio_audiodevice_data")))
#else
#error Unknown compiling tools.
#endif

typedef enum
{
    AUDIO_CMD_SEND_DATA_TO_FMCONTROL = 0,
    AUDIO_CMD_SEND_DATA_TO_CODEC,
    AUDIO_CMD_NULL,
}AUDIO_MSG_EVENT;

typedef enum _AUDIO_PROCESS
{
    AUDIO_PROCESS_CLOSE,
    AUDIO_PROCESS_EQ,
    AUDIO_PROCESS_FILTER,
    AUDIO_PROCESS_FADE_IN,
    AUDIO_PROCESS_FADE_OUT,
    AUDIO_PROCESS_CHANGE_SPEED,
    AUDIO_PROCESS_CHANGE_TONE,
    AUDIO_PROCESS_ECHO_CANCEL,
    AUDIO_PROCESS_MIX,
    AUDIO_PROCESS_MIX_0,
    AUDIO_PROCESS_MIX_1,
    AUDIO_PROCESS_MIX_2,
    AUDIO_PROCESS_MIX_3,
    AUDIO_PROCESS_MIX_4,
    AUDIO_PROCESS_MIX_5,
    AUDIO_PROCESS_MIX_6,
    AUDIO_PROCESS_MIX_7,
    AUDIO_PROCESS_MIX_8,
    AUDIO_PROCESS_MIX_9,
    AUDIO_PROCESS_MIX_10,
    AUDIO_PROCESS_MIX_11,
    AUDIO_PROCESS_MIX_12,
    AUDIO_PROCESS_MIX_13

}AUDIO_PROCESS;

typedef enum _TRACK_NO
{
    TRACK_NO_MAIN,
    TRACK_NO_SUB1,
    TRACK_NO_SUB2,
    TRACK_NO_SUB3,
    TRACK_NO_SUB4,
    TRACK_NO_NUM

}TRACK_NO;


typedef void (* P_PCM_CALLBACK)(uint8 * buf, uint32 Samples);      //audio callback funciton

typedef struct _AUDIO_DEV_ARG
{
    HDC hCodec;
    uint32 SampleRate;
    uint32 Bit;
    uint32 EQMode;
    uint32 Vol;
    uint32 RecordType;
    uint32 SamplesPerBlock;
    P_PCM_CALLBACK pfPcmCallBack;
}AUDIO_DEV_ARG;

#ifndef _FADE_PROCESS_
#define FADE_IN     0
#define FADE_OUT    1
#define FADE_NULL   -1
#endif


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
extern void AudioDev_SetByPass(HDC dev, uint32 bypass);
extern rk_err_t AudioDev_ReadEnable(HDC dev, uint32 Flag);
extern rk_err_t AudioDev_FadeOk(HDC dev);
extern rk_err_t AudioDev_FadeInit(HDC dev,  uint32 samples, uint32 type);
extern void Bit_Convertor_shift(short *ppsBuffer, uint32 Length, int16 bitpersample);
extern void Bit_Convertor_DEC(short *ppsBuffer, uint32 Length, int16 bitpersample);
extern void AudioDev_SetEQ(HDC dev, uint32 userEQMod);
extern rk_err_t AudioDev_SetVol(HDC dev, uint32 vol);
extern rk_err_t AudioDev_SetTrackProcess(HDC dev, uint32 TrackNo, uint8 subTrackProcess);
extern rk_err_t AudioDev_RealseMainTrack(HDC dev);
extern rk_err_t AudioDev_SetTrackLen(HDC dev, uint32 len);
extern rk_err_t AudioDev_GetSubTrack(HDC dev);
extern rk_err_t AudioDev_GetMainTrack(HDC dev);
extern rk_err_t AudioDev_Write(HDC dev , uint32 TrackNo, uint8 * buf);
extern rk_err_t AudioDev_SetMixScale(HDC dev ,uint32 TrackNo, uint32 MixMain, uint32 MixSub);
extern rk_err_t AudioDev_SetBit(HDC dev, uint32 TrackNo, uint32 Bit);
extern rk_err_t AudioDev_SetChannel(HDC dev, uint32 TrackNo, uint32 Channel);
extern rk_err_t AudioDev_SetRxSampleRate(HDC dev, uint32 TrackNo, uint32 SamleRate);
extern rk_err_t AudioDev_SetTxSampleRate(HDC dev, uint32 TrackNo, uint32 SamleRate);
extern rk_err_t AudioDev_SetModel(HDC dev,  uint32 TrackNo, uint32 ProcessNo, uint32 ProcessPos);
extern rk_err_t AudioDev_Shell(HDC dev, uint8 * pstr);
extern HDC AudioDev_Create(uint32 DevID, void *arg);
extern rk_err_t AudioDev_Delete(uint32 DevID, void * arg);

//cjh add
/*----------------------------------------------------------------------------*/
extern rk_err_t AudioDevPlayServiceTask_Resume(uint32 ObjectID);
extern rk_err_t AudioDevPlayServiceTask_Suspend(uint32 ObjectID);
extern void AudioDevPlayServiceTask_Enter(void * arg);
extern rk_err_t AudioDevPlayServiceTask_DeInit(void *pvParameters);
extern rk_err_t AudioDevPlayServiceTask_Init(void *pvParameters, void *arg);
/*----------------------------------------------------------------------------*/

//chad.ma add
/*----------------------------------------------------------------------------*/
extern void AudioDevService_Task_Enter(void);
extern rk_err_t AudioDevService_Task_DeInit(void *pvParameters);
extern rk_err_t AudioDevService_Task_Init(void *pvParameters,void* arg);
/*----------------------------------------------------------------------------*/


#endif

