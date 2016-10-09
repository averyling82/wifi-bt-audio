/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\FM\FmUiTask.h
* Owner: cjh
* Date: 2016.3.15
* Time: 14:08:13
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.15     14:08:13   1.0
********************************************************************************************
*/


#ifndef __APP_FM_FMUITASK_H__
#define __APP_FM_FMUITASK_H__

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

typedef  struct _FM_AUDIO_INFO
{
    uint32 FMInputType;
    uint32 Samplerate;
    uint32 channels;
    uint32 bitpersample;
    uint32 PlayVolume;
    uint32 playerr;
    uint32 EqMode;

    uint32 CurFreq;
    uint32 CurFmPresetNo;
    uint32 FmPresetNum;
    uint32 FmStereo;
    uint32 FmSearchMode;

}FM_AUDIO_INFO;

typedef enum
{
    FM_MEUN_ENTER = 0x11,
    FM_MEUN_DLETE_MSGBOX,

} eFM_MEUN_TYPE;


typedef struct _RK_TASK_FMUI_ARG
{
    uint32 xxx;
}RK_TASK_FMUI_ARG;

#define _APP_FM_FMUITASK_COMMON_  __attribute__((section("app_fm_fmuitask_common")))
#define _APP_FM_FMUITASK_INIT_  __attribute__((section("app_fm_fmuitask_common")))
#define _APP_FM_FMUITASK_SHELL_  __attribute__((section("app_fm_fmuitask_shell")))
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
extern rk_err_t FMPlayer_GetTotalTime(uint32 * time);
extern rk_err_t FMPlayer_GetCurTime(uint32 * time);
extern rk_err_t FMUITask_SetSpectrumEn(uint32 status);
extern rk_err_t FmUiTask_Resume(uint32 ObjectID);
extern rk_err_t FmUiTask_Suspend(uint32 ObjectID);
extern void FmUiTask_Enter(void * arg);
extern rk_err_t FmUiTask_DeInit(void *pvParameters);
extern rk_err_t FmUiTask_Init(void *pvParameters, void *arg);



#endif
