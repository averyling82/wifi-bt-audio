/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Line_In\LineInControlTask.h
* Owner: cjh
* Date: 2016.5.11
* Time: 9:02:01
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.11     9:02:01   1.0
********************************************************************************************
*/


#ifndef __APP_LINE_IN_LINEINCONTROLTASK_H__
#define __APP_LINE_IN_LINEINCONTROLTASK_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum _LINEIN_CMD
{
    LINEIN_CMD_INIT,
    LINEIN_CMD_START,
    LINEIN_CMD_DEINIT,
    LINEIN_CMD_VOLUMESET, //LINEIN volum
    LINEIN_CMD_PLAY, // play
    LINEIN_CMD_PAUSE_RESUME, // Pause// Resume
    LINEIN_CMD_RECORD, //record
    LINEIN_CMD_RECORD_STOP, //record
    LINEIN_CMD_Idle,

}LINEIN_CMD;

typedef enum _LINEIN_PLAYER_STATE
{
    LINEIN_PLAYER_STATE_PLAY,
    LINEIN_PLAYER_STATE_PAUSE,
    LINEIN_PLAYER_STATE_STOP,
    LINEIN_PLAYER_STATE_RECORDING,
    LINEIN_PLAYER_STATE_PAUSE_RECORD,
#ifdef _RK_SPECTRUM_
    LINEIN_PLAYER_STATE_UPDATA_SPECTRUM,
#endif
    LINEIN_PLAYER_STATE_IDLE,
}LINEIN_PLAYER_STATE;

typedef enum _LINEIN_STATE
{
    LINEIN_STATE_PREPARE,
    LINEIN_STATE_BEING,
    LINEIN_STATE_PAUSE,
    LINEIN_STATE_STOP,
    LINEIN_STATE_TIME_CHANGE,
    LINEIN_STATE_ERROR,
    LINEIN_STATE_MAX

}LINEIN_STATE;

typedef rk_err_t (* P_REC_RECIVE_MSG)(uint8 * buf, uint32 SamplesPerBlock);

typedef void (* P_LINEIN_APP_CALLBACK)(uint32 state);      //Line in callback funciton

typedef  struct _LINEIN_APP_CALLBACK
{
    struct _LINEIN_APP_CALLBACK * pNext;
    P_LINEIN_APP_CALLBACK pLineInStateForAPP;

}LINEIN_APP_CALLBACK;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _APP_LINE_IN_LINEINCONTROLTASK_COMMON_  __attribute__((section("app_line_in_lineincontroltask_common")))
#define _APP_LINE_IN_LINEINCONTROLTASK_INIT_  __attribute__((section("app_line_in_lineincontroltask_common")))
#define _APP_LINE_IN_LINEINCONTROLTASK_SHELL_  __attribute__((section("app_line_in_lineincontroltask_shell")))
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
extern rk_err_t LineInControlTask_SetStateChangeFunc(P_LINEIN_APP_CALLBACK old, P_LINEIN_APP_CALLBACK new);
extern rk_err_t LineInControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode);
extern rk_err_t LineInControlTask_SetSpectrumEn(uint32 status);
extern rk_err_t LineInControlTask_GetCurSpectrum(uint8** pSpectrum);
extern uint32 LineInControlTask_GetAudioInfo(LINEIN_AUDIO_INFO * AudioInfo);
extern rk_err_t LineInControlTask_Resume(uint32 ObjectID);
extern rk_err_t LineInControlTask_Suspend(uint32 ObjectID);
extern void LineInControlTask_Enter(void * arg);
extern rk_err_t LineInControlTask_DeInit(void *pvParameters);
extern rk_err_t LineInControlTask_Init(void *pvParameters, void *arg);

#endif
