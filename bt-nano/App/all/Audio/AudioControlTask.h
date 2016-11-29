/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Audio\AudioControlTask.h
* Owner: aaron.sun
* Date: 2015.6.18
* Time: 10:19:58
* Version: 1.0
* Desc: Audio Control Task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.18     10:19:58   1.0
********************************************************************************************
*/


#ifndef __APP_AUDIO_AUDIOCONTROLTASK_H__
#define __APP_AUDIO_AUDIOCONTROLTASK_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "global.h"
#include "SysInfoSave.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//播放顺序定义
typedef enum
{
    AUDIO_INTURN,
    AUDIO_RAND

}AUDIOPLAYMODE;

typedef struct
{
    uint32 ABRequire;
    uint32 AudioABStart;
    uint32 AudioABEnd;
    uint32 PlayDirect;
    uint8  playVolume;
    uint8  VolumeCnt;

}AUDIOCONTROL;

typedef enum _AUDIO_CMD
{
    AUDIO_CMD_DECSTART,
    AUDIO_CMD_PAUSE,
    AUDIO_CMD_RESUME,
    AUDIO_CMD_STOP,
    AUDIO_CMD_FFD,
    AUDIO_CMD_FFW,
    AUDIO_CMD_FF_STOP,
    AUDIO_CMD_VOLUMESET,
    AUDIO_CMD_EQSET,
    AUDIO_CMD_REPEATSET,
    AUDIO_CMD_SHUFFLESET,
    AUDIO_CMD_NEXTFILE,
    AUDIO_CMD_PREVFILE,
    AUDIO_CMD_FF_PAUSE,
    AUDIO_CMD_FF_RESUME,
    AUDIO_CMD_SEEKTO
}AUDIO_CMD;

typedef enum _AUDIO_STATE
{
    AUDIO_STATE_PLAY,
    AUDIO_STATE_FFD,
    AUDIO_STATE_FFW,
    AUDIO_STATE_PAUSE,
    AUDIO_STATE_STOP,
    AUDIO_STATE_PLAY_FFD,
    AUDIO_STATE_PLAY_FFW,
    AUDIO_STATE_PAUSE_FFD,
    AUDIO_STATE_PAUSE_FFW,
    AUDIO_STATE_FFD_PLAY,
    AUDIO_STATE_FFW_PLAY,
    AUDIO_STATE_TIME_CHANGE,
    AUDIO_STATE_VOLUME_CHANGE,
    AUDIO_STATE_MUSIC_CHANGE,
    AUDIO_STATE_EQ_CHANGE,
    AUDIO_STATE_SHUFFLE_CHANGE,
    AUDIO_STATE_REPEATE_CHANGE,
    AUDIO_STATE_ERROR,
    AUDIO_STATE_UPDATA_SPECTRUM,
    AUDIO_STATE_DELETE,
    AUDIO_STATE_CREATE,
    AUDIO_STATE_CUSTOM,

}AUDIO_STATE;

//reapeat mode
typedef enum _AUDIO_REPEAT_MODE
{
    AUDIO_FOLDER_ONCE,    //once directory.
    AUIDO_FOLDER_REPEAT,  //directory cycle
    AUDIO_REPEAT,         //repeat one song
    AUDIO_ALLONCE,        //repeat once all song.
    AUDIO_ALLREPEAT,      //cycle play all song.
    AUDIO_REPEAT1
}AUDIO_REPEAT_MODE;

//audio effect define of user.
typedef enum
{
    EQ_N12DB,
    EQ_N8DB,
    EQ_N4DB,
    EQ_0DB,
    EQ_4DB,
    EQ_8DB,
    EQ_12DB

}USER_EQ;

typedef enum
{
    SOURCE_FROM_NET = 0x55,
    SOURCE_FROM_HTTP,
    SOURCE_FROM_BT,
    SOURCE_FROM_XXX,
    SOURCE_FROM_CUR_AUDIO,
    SOURCE_FROM_FILE_BROWSER,
    SOURCE_FROM_M3U_BROWSER,
    SOURCE_FROM_CUE_BROWSER,
    SOURCE_FROM_MEDIA_LIBRARY,
    SOURCE_FROM_DB_MYFAVORITE,
    SOURCE_FROM_DB_FLODER,
    SOURCE_FROM_RECORD,
    SOURCE_FROM_BEEP,
    SOURCE_MAX
}AUDIO_SOURCE;

typedef  struct _AUDIO_INFO
{
    uint32 TotalFiles;
    uint32 CurrentFileNum; // 当前文件编号
    uint32 AudioSource;
    uint32 Samplerate;
    uint32 Bitrate;
    uint32 channels;
    uint32 bitpersample;
    uint32 RepeatMode;
    uint16 path[MAX_DIRPATH_LEN];
    uint32 PlayOrder; // random or order
    uint32 PlayVolume;
    uint32 playerr;
    uint32 EqMode;
    uint32 BaseID;
    MEDIA_FLODER_INFO_STRUCT MediaFloderInfo;
}AUDIO_INFO;

typedef void (* P_AUDIO_CALLBACK)(uint32 audio_state);      //audio callback funciton

typedef  struct _AUDIO_CALLBACK
{
    struct _AUDIO_CALLBACK * pNext;
    P_AUDIO_CALLBACK pfAudioState;

}AUDIO_CALLBACK;


//Play Range define
#define     FIND_FILE_RANGE_DIR            2// once direction
#define     FIND_FILE_RANGE_ALL            3// cycle in direction

#define _APP_AUDIO_AUDIOCONTROLTASK_COMMON_  __attribute__((section("app_audio_audiocontroltask_common")))
#define _APP_AUDIO_AUDIOCONTROLTASK_INIT_  __attribute__((section("app_audio_audiocontroltask_common")))
#define _APP_AUDIO_AUDIOCONTROLTASK_SHELL_  __attribute__((section("app_audio_audiocontroltask_shell")))
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define _APP_AUDIO_AUDIOCONTROLTASK_DATA_ _APP_AUDIO_AUDIOCONTROLTASK_COMMON_
#elif defined(__arm__) && defined(__GNUC__)
#define _APP_AUDIO_AUDIOCONTROLTASK_DATA_  __attribute__((section("app_audio_audiocontroltask_data")))
#else
#error Unknown compiling tools.
#endif

#define AUDIO_DMACHANNEL_IIS    (DMA_CHN_MAX - 1)
#define AUDIO_READ_DMACHANNEL_IIS    (DMA_CHN_MAX - 2)

#define Audio_Stop_Normal       0
#define Audio_Stop_NextFile     1
#define Audio_Stop_PrevFile    -1
#define Audio_Stop_Force        2

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern char        AudioFileExtString[];
extern char        RecordFileExtString[];

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t AudioPlayer_SetSpectrumEn(uint32 status);
extern rk_err_t AudioPlayer_GetCurSpectrum(uint8** pSpectrum);
extern void AudioPlayer_SetFloderInfo(void);
extern uint32 AudioPlayer_GetAudioInfo(AUDIO_INFO * AudioInfo);
extern rk_err_t AudioPlayer_GetTotalTime(uint32 * time);
extern rk_err_t AudioPlayer_GetCurTime(uint32 * time);
extern rk_err_t AudioPlayer_GetState(uint32 * state);
extern rk_err_t AudioControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode);
extern void AudioControlTask_Enter(void);
extern rk_err_t AudioControlTask_DeInit(void *pvParameters);
extern rk_err_t AudioControlTask_Init(void *pvParameters, void *arg);
extern rk_err_t AudioControlTask_SetStateChangeFunc(P_AUDIO_CALLBACK old, P_AUDIO_CALLBACK new);

#endif
