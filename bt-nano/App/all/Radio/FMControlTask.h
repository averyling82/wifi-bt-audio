/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\FM\FMControlTask.h
* Owner: cjh
* Date: 2016.3.15
* Time: 11:43:34
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.15     11:43:34   1.0
********************************************************************************************
*/


#ifndef __APP_FM_FMCONTROLTASK_H__
#define __APP_FM_FMCONTROLTASK_H__

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
typedef enum _FM_CMD
{
    FM_CMD_INIT,
    FM_CMD_START,
    FM_CMD_MANUAL, //hand
    FM_CMD_HAND_AUTOSEARCH,
    FM_CMD_AUTOSEARCH,
    FM_CMD_AUTOSEARCH_RESUME,
    FM_CMD_STEROSWITCH,
    FM_CMD_STEPFREQ,
    FM_CMD_STEPSTATION,
    FM_CMD_GETSTEROSTATUS,
    FM_CMD_DEINIT,
    FM_CMD_VOLUMESET, //FM volum
    FM_CMD_SAVE_CH, //save
    FM_CMD_DELETE_CH, //delete
    FM_CMD_DELETE_ALL_CH, //delete
    FM_CMD_PLAY, // play
    FM_CMD_PAUSE_RESUME, // Pause// Resume
    FM_CMD_RECORD, //record
    FM_CMD_RECORD_STOP, //record
    FM_CMD_STOP_SEARCH,

    FM_CMD_Idle,

}FM_CMD;


typedef enum
{
    FM_DIRECT_DEC = -1,       //direct of decreasing
    FM_DIRECT_CUR = 0,
    FM_DIRECT_INC = 1,           //direct of increment
    FM_DIRECT_Idle,
}FM_DIRECT;

typedef enum _FM_STATE
{
    FM_STATE_HANDSTEPFREQ = 0,
    FM_STATE_AUTOSEARCH,
    FM_STATE_HANDSEARCH,
    FM_STATE_STEPSTATION,
    FM_STATE_SEARCHING,
    FM_STATE_SEARCH_STOP,
    FM_STATE_ERROR,
    FM_STATE_IDLE,
}FM_STATE;

typedef enum _FM_PLAYER_STATE
{
    FM_PLAYER_STATE_PLAY,
    FM_PLAYER_STATE_PAUSE,
    FM_PLAYER_STATE_STOP,
    FM_PLAYER_STATE_RECORDING,
    FM_PLAYER_STATE_PAUSE_RECORD,
#ifdef _RK_SPECTRUM_
    FM_PLAYER_STATE_UPDATA_SPECTRUM,
#endif
    FM_STATE_UPDATA_FM_PRESET,
    FM_STATE_UPDATA_FM_CH,
    FM_STATE_SAVE_CUR_FM_CH,
    FM_STATE_SAVE_CUR_FM_CH_FULL,
    FM_STATE_DELETE_PRESET_FM_CH,
    FM_STATE_DELETE_PRESET_ALL_CH,
    FM_STATE_NO_EXIST_FM_STATION,
    FM_PLAYER_STATE_AUTOSEACH_END,

    FM_PLAYER_STATE_IDLE,
}FM_PLAYER_STATE;

typedef void (* P_FM_APP_CALLBACK)(uint32 fm_state);      //fm callback funciton

typedef  struct _FM_APP_CALLBACK
{
    struct _FM_APP_CALLBACK * pNext;
    P_FM_APP_CALLBACK pfmStateForAPP;

}FM_APP_CALLBACK;

#define _APP_FM_FMCONTROLTASK_COMMON_  __attribute__((section("app_fm_fmcontroltask_common")))
#define _APP_FM_FMCONTROLTASK_INIT_  __attribute__((section("app_fm_fmcontroltask_common")))
#define _APP_FM_FMCONTROLTASK_SHELL_  __attribute__((section("app_fm_fmcontroltask_shell")))

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
extern rk_err_t FMControlTask_CurFreqIsInPreset(void);
extern rk_err_t FMPlayer_GetCurSpectrum(uint8** pSpectrum);
extern uint32 FMUI_GetAudioInfo(FM_AUDIO_INFO * AudioInfo);
extern rk_err_t FMUITask_SetSpectrumEn(uint32 status);
extern rk_err_t FMControlTask_SetStateChangeFunc(P_FM_APP_CALLBACK old, P_FM_APP_CALLBACK new);
extern rk_err_t FMControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode);
extern rk_err_t FMControlTask_Resume(uint32 ObjectID);
extern rk_err_t FMControlTask_Suspend(uint32 ObjectID);
extern void FMControlTask_Enter(void * arg);
extern rk_err_t FMControlTask_DeInit(void *pvParameters);
extern rk_err_t FMControlTask_Init(void *pvParameters, void *arg);



#endif
