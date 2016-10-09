/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\FM\FmUiTask.c
* Owner: cjh
* Date: 2016.3.15
* Time: 14:08:27
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.15     14:08:27   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_FM_FMUITASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"
#include "record_globals.h"
#include "AudioControlTask.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _FMUI_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}FMUI_RESP_QUEUE;
typedef  struct _FMUI_ASK_QUEUE
{
    uint32 cmd;
    uint32 type;
    uint32 offset;
    void * fun;

}FMUI_ASK_QUEUE;

typedef enum _FM_DIALOG
{
    FM_DIALOG_STOP_AUTOSEARCH,
    FM_DIALOG_AUTOSEARCH,
    FM_DIALOG_SAVE_CH, //save
    FM_DIALOG_ABANDON_SAVE_CH, //abandon
    FM_DIALOG_DELETE_CH, //save
    FM_DIALOG_ABANDON_DELETE_CH, //abandon
    FM_DIALOG_DELETE_ALL_CH, //save
    FM_DIALOG_ABANDON_DELETE_ALL_CH, //abandon

    FM_DIALOG_MAX,

}FM_DIALOG;

typedef enum _FM_TASK_EVENT
{
    FMTASK_KEY_EVENT,
    FMTASK_AUDIO_EVENT,
    FMTASK_MSGBOX_EVENT,
    FMTASK_DIALOG_EVENT,
    FMTASK_NOSCREEN_EVENT,
    FMMENU_KEY_EVENT,
    FMTASK_EVENT_MAX,
    FMTASK_EVENT_RECORD,

}FM_TASK_EVENT;

typedef enum
{
    FM_STATE_PLAY = 1,
    FM_STATE_PLAY_REC,

}eFM_CUR_STATE;

typedef  struct _FMUI_TASK_DATA_BLOCK
{
    pQueue FmUiAskQueue;
    pQueue FmUiRespQueue;
    HGC hBackdrop;
    //HGC hLoop;
    //HGC hOrder;
    HGC hPlayerState;
    //HGC hPage;
    HGC hProgressBar;
    HGC hVolumeBar;
    HGC hVolumeValue;
    HGC hFMTotalTimer;
    HGC hFMProgressTimer;
    HGC hEQMode;
    HGC hFMFreq;
    HGC hMsgBox;
    HGC hSpecrm;
    HGC hMenu;
    HGC hDialog;
    HGC hFMState;
    HGC hFileName;

    FM_AUDIO_INFO stAudioInfo;
    uint32 PlayerState;
    uint32 CurFreq;
    uint32 FMTotalTimer;
    uint32 FMProgressTimer;
    uint32 GetCurTime;
    int16  Recordstate;
    UINT16 FMPlayerMod;//rec or play
    RECORD_INFO RecordInf;
    uint8  QueueFull;
}FMUI_TASK_DATA_BLOCK;

#define MaximumVolume                   30

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static FMUI_TASK_DATA_BLOCK * gpstFmUiData;

_APP_FM_FMUITASK_COMMON_
uint8 FmUi_Mode[8] =
{
    SID_MANUAL_MOD - SID_MANUAL_MOD,//0
    SID_PRESET_MOD - SID_MANUAL_MOD,//1
    SID_AUTO_MOD - SID_MANUAL_MOD,//4
    SID_SAVE_CHANNEL - SID_MANUAL_MOD,//6
    SID_DELETE_CHANNEL - SID_MANUAL_MOD,//7
    SID_RECORD - SID_MANUAL_MOD,//3
    SID_PLAY_REC_DATA - SID_MANUAL_MOD,//5
    //8,//SID_DELETE_ALL - SID_MANUAL_MOD,//8
};

typedef enum
{
    MOD_MANUAL = 0x11,
    MOD_MANUAL_SEMI_AUTO,
    MOD_AUTO_PRESET,
    MOD_REC,
    MOD_REC_DATA,
    MOD_PRESET_STATION,

} eFM_MEUN_MOD;
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
rk_err_t FMPlay_DialogCallBackForDeleteCurChannel(uint32 EVENT_type, uint32 event, void * arg, HGC pGc);
rk_err_t FMPlay_DialogCallBackForDeleteAllChannel(uint32 EVENT_type, uint32 event, void * arg, HGC pGc);
rk_err_t FMPlay_DialogForSaveChannel(uint32 EVENT_type, uint32 event, void * arg, HGC pGc);
void FMPlay_AudioCallBackNull(uint32 audio_state);
rk_err_t FMPlayer_RecordFileNameDisplay(void);
rk_err_t FMUITask_RecordCallBack(uint32 record_state);
rk_err_t FMPlay_FMModeDisplay(void *text, uint32 len, uint32 contentSelect, eTEXT_DISPLAY_CMD cmd);
rk_err_t FMPlay_DialogCallBack(uint32 EVENT_type, uint32 event, void * arg, HGC pGc);
rk_err_t FMPlay_SetDialog(void * text, uint16 text_cmd, P_APP_RECIVE_MSG fun);
rk_err_t FMUITask_SendCmdToFMControl(uint32 Cmd, void * msg, uint32 Mode);
rk_err_t FMMenu_KeyEvent(uint32 Cmd, uint32 Offset);
rk_err_t FMUITask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc);
rk_err_t FMUITask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, RKGUI_SELECT_ITEM * arg, int16 offset);
rk_err_t FMUITask_Menu(uint32 CursorID);
void FMPlayer_SpectrumDisplay(void);
rk_err_t FMUITask_DeleteGuiHandle(void);
rk_err_t FMPlay_StartPlayer();
rk_err_t FMPlay_SetPopupWindow(void * text, uint16 text_cmd);
void FMPlay_StateDisplay(void);
void FMPlay_PlayProcessDisplay(void);
void FMPlay_FMFreqDisplay(void);
void FMPlay_DisplayMusicInfo(void);
//void FMPlay_PlayOrderDisplay(void);
void FMPlay_DisplayAll(void);
void FMPlay_MusicEqDisplay(void);
void FMPlay_VolumeDisplay(void);
void FMPlay_DisplayMsg(uint32 Sid);
void FMPlay_BackGroundDisplay(void);
void FMPlay_AudioCallBack(uint32 audio_state);
rk_err_t FMUITask_DisplayInit(void);
rk_err_t FMUITask_GuiCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc);
rk_err_t FMUITask_NoScreenEvent(uint32 cmd, int16 type);
rk_err_t FMUITask_DiaLogEvent(uint32 cmd, P_APP_RECIVE_MSG fun);
rk_err_t FMUITask_AudioEvent(uint32 AudioState);
rk_err_t FMUITask_NoScreenKeyRev(uint32 keyvalue);
rk_err_t FMUITask_KeyEvent(uint32 KeyVal);
rk_err_t FMUITask_PlayerOpen();
rk_err_t FMUITaskPlay_DisplayInit(void);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FMPlayer_GetTotalTime
** Input:uint32 * time
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 15:32:40
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON API rk_err_t FMPlayer_GetTotalTime(uint32 * time)
{
    *time = 59*3600000 + 59*60000 + 59*1000;
}
/*******************************************************************************
** Name: FMPlayer_GetCurTime
** Input:(uint32 * time)
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 15:31:55
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON API rk_err_t FMPlayer_GetCurTime(uint32 * time)
{
     *time = gpstFmUiData->GetCurTime;
}

/*******************************************************************************
** Name: FmUiTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 14:08:44
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON API rk_err_t FmUiTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: FmUiTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 14:08:44
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON API rk_err_t FmUiTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: FmUiTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2016.3.15
** Time: 14:08:44
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON API void FmUiTask_Enter(void * arg)
{
    FMUI_ASK_QUEUE FmUIAskQueue;

    gpstFmUiData->QueueFull = 0;

#ifdef _USE_GUI_
    FMUITask_PlayerOpen();
    FMPlay_StartPlayer();
    FMUITask_DisplayInit();
    FMControlTask_SetSpectrumEn(1);
#endif
    MainTask_SetStatus(MAINTASK_APP_FMUI,1);
    while(1)
    {
        rkos_queue_receive(gpstFmUiData->FmUiAskQueue, &FmUIAskQueue, MAX_DELAY);
        switch(FmUIAskQueue.type)
        {
            case FMTASK_KEY_EVENT:
                #ifdef _USE_GUI_
                FMUITask_KeyEvent(FmUIAskQueue.cmd);
                gpstFmUiData->QueueFull = 0;
                #else

                #endif
                //rk_printf("fm KEY_EVENT QueueFull= %d over\n",gpstFmUiData->QueueFull);
                break;

            case FMTASK_AUDIO_EVENT:
                FMUITask_AudioEvent(FmUIAskQueue.cmd);
                break;

            case FMTASK_DIALOG_EVENT:
                FMUITask_DiaLogEvent(FmUIAskQueue.cmd, FmUIAskQueue.fun);
                gpstFmUiData->QueueFull = 0;
                break;

            case FMTASK_MSGBOX_EVENT:
                #ifdef _USE_GUI_
                if(gpstFmUiData->hMsgBox != NULL)
                {
                    GuiTask_OperWidget(gpstFmUiData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
                    GuiTask_DeleteWidget(gpstFmUiData->hMsgBox);
                    gpstFmUiData->hMsgBox = NULL;
                    FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
                }
                //if(gpstFmUiData->PlayerState != FM_PLAYER_STATE_PLAY)
                {
                    FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_PLAY;
                    //gpstFmUiData->PlayerState = FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
                    FMPlay_StateDisplay();
                }
                gpstFmUiData->QueueFull = 0;
                #endif
                break;

            case FMTASK_NOSCREEN_EVENT:

                break;

            case FMMENU_KEY_EVENT:
                //rk_printf("FMMENU_KEY_EVENT\n");
                FMMenu_KeyEvent(FmUIAskQueue.cmd, FmUIAskQueue.offset);
                gpstFmUiData->QueueFull = 0;
                //rk_printf("FMMENU_KEY_EVENT ui end..!!! ");
                break;

            case FMTASK_EVENT_RECORD:

                break;

            default:
                rk_printf("default\n");
                break;
        }
        //rk_printf(" ui end..!!! ");
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
** Name: FMPlay_DialogCallBackForDeleteCurChannel
** Input:uint32 EVENT_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.17
** Time: 8:47:14
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_DialogCallBackForDeleteCurChannel(uint32 EVENT_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FmUiAskQueue;
    FMUI_ASK_QUEUE FMUIAskQueue_tmp;

    if(EVENT_type == APP_RECIVE_MSG_EVENT_DIALOG)
    {
        FmUiAskQueue.type = FMTASK_DIALOG_EVENT;
        if((int16)arg == 0)
        {
            FmUiAskQueue.cmd = FM_DIALOG_DELETE_CH;
        }
        else
        {
            FmUiAskQueue.cmd = FM_DIALOG_ABANDON_DELETE_CH;
        }
        FmUiAskQueue.fun = FMPlay_DialogCallBackForDeleteCurChannel;
        rk_printf("del cur channel..on_off:%d\n",(int16)arg);
        ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FmUiAskQueue, MAX_DELAY);//MAX_DELAY
        if(ret == RK_ERROR)
        {
           rk_printf("DialogCallBack send fail...\n");
        }

    }
    else if(EVENT_type == APP_RECIVE_MSG_EVENT_KEY)
    {
        printf("APP_RECIVE_MSG_EVENT_KEY:%x\n",event);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMPlay_DialogCallBackForDeleteAllChannel
** Input:uint32 EVENT_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.17
** Time: 8:43:15
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_DialogCallBackForDeleteAllChannel(uint32 EVENT_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FmUiAskQueue;
    FMUI_ASK_QUEUE FMUIAskQueue_tmp;

    if(EVENT_type == APP_RECIVE_MSG_EVENT_DIALOG)
    {
        FmUiAskQueue.type = FMTASK_DIALOG_EVENT;
        if((int16)arg == 0)
        {
            FmUiAskQueue.cmd = FM_DIALOG_DELETE_ALL_CH;
        }
        else
        {
            FmUiAskQueue.cmd = FM_DIALOG_ABANDON_DELETE_ALL_CH;
        }
        FmUiAskQueue.fun = FMPlay_DialogCallBackForDeleteAllChannel;
        rk_printf("del all channel..on_off:%d\n",(int16)arg);
        ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FmUiAskQueue, MAX_DELAY);//MAX_DELAY
        if(ret == RK_ERROR)
        {
           rk_printf("DialogCallBack send fail...\n");
        }

    }
    else if(EVENT_type == APP_RECIVE_MSG_EVENT_KEY)
    {
        printf("APP_RECIVE_MSG_EVENT_KEY:%x\n",event);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMPlay_DialogForSaveChannel
** Input:uint32 EVENT_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.16
** Time: 18:12:42
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_DialogForSaveChannel(uint32 EVENT_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FmUiAskQueue;
    FMUI_ASK_QUEUE FMUIAskQueue_tmp;

    if(EVENT_type == APP_RECIVE_MSG_EVENT_DIALOG)
    {
        FmUiAskQueue.type = FMTASK_DIALOG_EVENT;
        if((int16)arg == 0)
        {
            FmUiAskQueue.cmd = FM_DIALOG_SAVE_CH;
        }
        else
        {
            FmUiAskQueue.cmd = FM_DIALOG_ABANDON_SAVE_CH;
        }
        FmUiAskQueue.fun = FMPlay_DialogForSaveChannel;
        rk_printf("Save CH ..on_off:%d\n",(int16)arg);
        ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FmUiAskQueue, MAX_DELAY);//MAX_DELAY
        if(ret == RK_ERROR)
        {
           rk_printf("DialogCallBack send fail...\n");
        }

    }
    else if(EVENT_type == APP_RECIVE_MSG_EVENT_KEY)
    {
        printf("APP_RECIVE_MSG_EVENT_KEY:%x\n",event);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMPlay_AudioCallBackNull
** Input:uint32 audio_state
** Return: void
** Owner:cjh
** Date: 2016.5.16
** Time: 10:54:02
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_AudioCallBackNull(uint32 audio_state)
{

}
/*******************************************************************************
** Name: FMPlayer_RecordFileNameDisplay
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.7
** Time: 14:16:51
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlayer_RecordFileNameDisplay(void)
{
#ifdef _USE_GUI_
int16 i;
    RKGUI_TEXT_ARG pstTextArg;
    Ucs2 FileName[30];

    for(i= 0; i< 12 ; i++)
    {
        FileName[i]= (Ucs2)gpstFmUiData->RecordInf.FileName[i];
    }
    FileName[i] = 0;
    pstTextArg.text= FileName;

    if(gpstFmUiData->hFileName == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 30;
        pstTextArg.y= 109;
        pstTextArg.xSize= -1;//45
        pstTextArg.ySize= -1;
        pstTextArg.BackdropX= 30;
        pstTextArg.BackdropY= 109;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstFmUiData->hFileName= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        pstTextArg.opercmd= TEXT_SET_CONTENT;
        GuiTask_OperWidget(gpstFmUiData->hFileName, OPERATE_SET_CONTENT, &pstTextArg, SYNC_MODE);
    }
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_RecordCallBack
** Input:uint32 record_state
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.5
** Time: 17:04:11
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_RecordCallBack(uint32 record_state)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FmUiAskQueue;
    switch(record_state)
    {
        case RECORD_STATE_PREPARE:
            gpstFmUiData->Recordstate= 0;
            break;

        case RECORD_STATE_BEING:
            gpstFmUiData->Recordstate= 1;
            break;

        case RECORD_STATE_PAUSE:
            gpstFmUiData->Recordstate= 2;
            break;

        case RECORD_STATE_STOP:
            gpstFmUiData->Recordstate= 3;
            break;

        case RECORD_STATE_TIME_CHANGE:
            break;

        default:
            break;
    }
    FmUiAskQueue.type = FMTASK_EVENT_RECORD;
    FmUiAskQueue.cmd  = record_state;
    rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FmUiAskQueue, 0);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FMPlay_FMModeDisplay
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.5
** Time: 13:59:54
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_FMModeDisplay(void *text, uint32 len, uint32 contentSelect, eTEXT_DISPLAY_CMD cmd)
{
#ifdef _USE_GUI_
    FM_AUDIO_INFO FMAudioInfo;
    Ucs2 FMFreq[13];
    char buf[13];
    uint8 i=0;
    //uint8 tempInt, tempDecimal;

    RKGUI_TEXT_ARG FMplayerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    if (gpstFmUiData->hFMState != NULL)
    {
        //printf("delete gpstFmUiData->hFMState");
        //GuiTask_OperWidget(gpstFmUiData->hFMState, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hFMState);
        gpstFmUiData->hFMState = NULL;
    }

    if (gpstFmUiData->hFMState == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= cmd;
        pstTextArg.x= 65;
        pstTextArg.y= 125;//102
        pstTextArg.xSize= 63;
        pstTextArg.ySize= -1;
        pstTextArg.BackdropX= 65;
        pstTextArg.BackdropY= 125;//102
        if(cmd == TEXT_CMD_ID)
        {
            pstTextArg.resource = (int16)text;
            //printf("resource=%d",pstTextArg.resource);
        }
        else
        {
            if(len > 12)
            {
                rk_printf("EROR: len > 11");
                return RK_ERROR;
            }

            if(contentSelect == 1)
            {
                FMUI_GetAudioInfo(&FMAudioInfo);
                sprintf(buf, "%s%02d",text, FMAudioInfo.CurFmPresetNo);
                len += 2;
            }
            else
            {
                sprintf(buf, "%s",text);
            }
            for (i= 0; i< sizeof(buf); i++)
            {
                FMFreq[i]= (Ucs2)buf[i];
            }
            //sprintf(buf, "%s%02d",text, FMAudioInfo.CurFmPresetNo);
            //printf("%s",buf);
            FMFreq[len] = 0;
            pstTextArg.text= FMFreq;
        }
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstFmUiData->hFMState= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
        GuiTask_OperWidget(gpstFmUiData->hFMState, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);

    }
    //else
    //{
        //FMplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
        //FMplayerTextContentArg.text = FMFreq;
        //GuiTask_OperWidget(gpstFmUiData->hFMState, OPERATE_SET_CONTENT, &FMplayerTextContentArg, SYNC_MODE);
    //}
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMPlay_DialogCallBack
** Input:uint32 EVENT_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.5
** Time: 10:16:16
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_DialogCallBack(uint32 EVENT_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FmUiAskQueue;
    FMUI_ASK_QUEUE FMUIAskQueue_tmp;

    if(EVENT_type == APP_RECIVE_MSG_EVENT_DIALOG)
    {
        FmUiAskQueue.type = FMTASK_DIALOG_EVENT;
        if((int16)arg == 0)
        {
            FmUiAskQueue.cmd = FM_DIALOG_STOP_AUTOSEARCH;
        }
        else
        {
            FmUiAskQueue.cmd = FM_DIALOG_AUTOSEARCH;
        }
        printf("..on_off:%d\n",(int16)arg);
        FmUiAskQueue.fun = FMPlay_DialogCallBack;
        ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FmUiAskQueue, MAX_DELAY);//MAX_DELAY
        if(ret == RK_ERROR)
        {
           rk_printf("DialogCallBack send fail...\n");
        }

    }
    else if(EVENT_type == APP_RECIVE_MSG_EVENT_KEY)
    {
        printf("APP_RECIVE_MSG_EVENT_KEY:%x\n",event);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMPlay_SetDialog
** Input:void * text, uint16 text_cmd
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.5
** Time: 10:12:35
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_SetDialog(void * text, uint16 text_cmd, P_APP_RECIVE_MSG fun)
{
    if(fun == NULL)
    {
        return RK_ERROR;
    }
#ifdef _USE_GUI_
    RKGUI_MSGBOX_ARG pstMsgBoxArg;

    FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBack, FMPlay_AudioCallBackNull);

    printf("---FMPlay_SetDialog...\n");
    GuiTask_AppReciveMsg(fun);
    pstMsgBoxArg.cmd = MSGBOX_CMD_DIALOG;
    pstMsgBoxArg.x = 20;
    pstMsgBoxArg.y = 30;
    pstMsgBoxArg.display = 1;
    pstMsgBoxArg.level = 0;
    pstMsgBoxArg.title = SID_WARNING;
    pstMsgBoxArg.text_cmd = text_cmd;//TEXT_CMD_BUF
    pstMsgBoxArg.text = text;
    pstMsgBoxArg.Button_On = SID_YES;
    pstMsgBoxArg.Button_Off = SID_NO;
    pstMsgBoxArg.align= TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;

    if(gpstFmUiData->hDialog!= NULL)
    {
        GuiTask_OperWidget(gpstFmUiData->hDialog, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hDialog);
        gpstFmUiData->hDialog = NULL;
    }

    if(gpstFmUiData->hDialog == NULL)
    {
        gpstFmUiData->hDialog = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_SendCmdToFMControl
** Input:uint32 Cmd, void * msg, uint32 Mode
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.5
** Time: 9:36:05
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_SendCmdToFMControl(uint32 Cmd, void * msg, uint32 Mode)
{
#ifdef _USE_GUI_
    if(gpstFmUiData->hMenu != NULL)
    {
        GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hMenu);
        gpstFmUiData->hMenu = NULL;
        GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
        FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
    }
#endif
    if(Cmd != NULL)
        FMControlTask_SendCmd(Cmd, msg, Mode);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMMenu_KeyEvent
** Input:uint32 KeyVal
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.4
** Time: 17:35:55
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMMenu_KeyEvent(uint32 Cmd, uint32 Offset)
{
    if(gpstFmUiData->hMsgBox != NULL)
    {
        return RK_SUCCESS;
    }
    switch (Cmd)
    {
        case KEY_VAL_ESC_SHORT_UP:
            #ifdef _USE_GUI_
            //printf("ESC....\n");
            GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
            GuiTask_DeleteWidget(gpstFmUiData->hMenu);
            gpstFmUiData->hMenu = NULL;
            FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
            GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
            FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
            gpstFmUiData->PlayerState = FM_PLAYER_STATE_PLAY;
            //gpstFmUiData->PlayerState = FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
            FMPlay_StateDisplay();
            #endif
            break;

        case KEY_VAL_PLAY_PRESS_START://关机
            #ifdef _USE_GUI_
            GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
            #endif
            FMControlTask_SendCmd(FM_CMD_DEINIT, (void *)0, SYNC_MODE);
            MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
            break;

        case FM_MEUN_ENTER:
            printf("Offset = %d\n",Offset);
            switch (Offset)
            {
                case 0://SID_MANUAL_MOD - SID_MANUAL_MOD:
                    if(gpstFmUiData->FMPlayerMod != MOD_MANUAL)
                    {
                        gpstFmUiData->FMPlayerMod = MOD_MANUAL;
                        printf("MANUAL....\n");
                        FMUITask_SendCmdToFMControl(FM_CMD_MANUAL, (void *)FM_DIRECT_CUR, SYNC_MODE);
                        //if(gpstFmUiData->PlayerState != FM_PLAYER_STATE_PLAY)
                        {
                            //FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
                            gpstFmUiData->PlayerState = FM_PLAYER_STATE_PLAY;
                            //gpstFmUiData->PlayerState = FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
                            FMPlay_StateDisplay();
                            FMPlay_FMModeDisplay((void *)SID_MANUAL_MOD, 7, 0, TEXT_CMD_ID);
                        }
                    }
                    else
                    {
                    #ifdef _USE_GUI_
                        if(gpstFmUiData->hMenu != NULL)
                        {
                            GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstFmUiData->hMenu);
                            gpstFmUiData->hMenu = NULL;
                            GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
                            FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
                        }
                    #endif
                    }
                    break;

                case 1://SID_PRESET_MOD - SID_MANUAL_MOD:
                    if(gpstFmUiData->FMPlayerMod != MOD_PRESET_STATION)
                    {
                        printf("PRESET....\n");
                        FMUITask_SendCmdToFMControl(FM_CMD_STEPSTATION, (void *)FM_DIRECT_CUR, SYNC_MODE);
                        gpstFmUiData->FMPlayerMod = MOD_PRESET_STATION;
                        FMPlay_FMModeDisplay((void *)SID_PRESET_MOD, 7, 0, TEXT_CMD_ID);
                    }
                    else
                    {
                    #ifdef _USE_GUI_
                        if(gpstFmUiData->hMenu != NULL)
                        {
                            GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstFmUiData->hMenu);
                            gpstFmUiData->hMenu = NULL;
                            GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
                            FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
                        }
                    #endif
                    }
                    break;

                case 2://SID_AUTO_MOD - SID_MANUAL_MOD:
                    if(gpstFmUiData->FMPlayerMod != MOD_AUTO_PRESET)
                    {
                        gpstFmUiData->FMPlayerMod = MOD_AUTO_PRESET;
                        FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)1, SYNC_MODE);
                        printf("AUTO....\n");
                        FMUITask_SendCmdToFMControl(FM_CMD_AUTOSEARCH, (void *)FM_DIRECT_INC, SYNC_MODE);
                        FMPlay_FMModeDisplay((void *)SID_AUTO_MOD, 7, 0, TEXT_CMD_ID);
                    }
                    break;

                case 3://SID_SAVE_CHANNEL - SID_MANUAL_MOD:
                    #ifdef _USE_GUI_
                    if(gpstFmUiData->hMenu != NULL)
                    {
                        GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstFmUiData->hMenu);
                        gpstFmUiData->hMenu = NULL;
                        GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
                        FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
                    }
                    #endif
                    //gpstFmUiData->FMPlayerMod = MOD_PRESET_STATION;
                    FMPlay_SetDialog((void *)SID_SAVE_CHANNEL, TEXT_CMD_ID, FMPlay_DialogForSaveChannel);
                    break;

                case 4://SID_DELETE_CHANNEL - SID_MANUAL_MOD:
                    #ifdef _USE_GUI_
                    if(gpstFmUiData->hMenu != NULL)
                    {
                        GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstFmUiData->hMenu);
                        gpstFmUiData->hMenu = NULL;
                        GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
                        FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
                    }
                    #endif
                    rk_printf("SID_DELETE_CHANNEL....\n");
                    FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
                    if(gpstFmUiData->stAudioInfo.FmPresetNum > 0)
                    {
                        if(FMControlTask_CurFreqIsInPreset() == RK_SUCCESS)
                        {
                            FMPlay_SetDialog((void *)SID_DELETE_CHANNEL, TEXT_CMD_ID, FMPlay_DialogCallBackForDeleteCurChannel);
                        }
                        else
                        {
                            #ifdef _USE_GUI_
                            FMPlay_SetPopupWindow((void *)SID_WITHOUT_THE_CHANNEL, TEXT_CMD_ID);
                            #endif
                        }
                    }
                    else
                    {
                        #ifdef _USE_GUI_
                        FMPlay_SetPopupWindow((void *)SID_WITHOUT_CHANNEL, TEXT_CMD_ID);
                        #endif
                    }
                    break;

                case 5://SID_RECORD - SID_MANUAL_MOD:
                    if(gpstFmUiData->PlayerState == FM_PLAYER_STATE_PLAY)
                    {
                        RK_TASK_RECORDCONTROL_ARG Arg;

                        Arg.RecordEncodeType    = RECORD_ENCODE_TYPE_WAV;
                        Arg.RecordType          = RECORD_TYPE_MIC_STERO;
                        Arg.RecordEncodeSubType   = 0;
                        Arg.RecordSampleRate    = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
                        Arg.RecordChannel       = RECORD_CHANNEL_STERO;
                        Arg.RecordDataWidth     = RECORD_DATAWIDTH_16BIT;
                        Arg.RecordSource        = 1;
                        Arg.pfRecordState       = FMUITask_RecordCallBack;

                        if(RKTaskCreate(TASK_ID_RECORDCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
                        {
                             rk_printf("Record control task create failure");
                        }
                        printf("\n\nstartrec...\n\n");

                        while(Recorder_GetRecordInf(&gpstFmUiData->RecordInf) != RECORD_STATE_PREPARE)
                        {
                            rkos_sleep(10);
                        }

                        gpstFmUiData->FMPlayerMod = MOD_REC;
                        RecordControlTask_SendCmd(RECORD_CMD_START, NULL, SYNC_MODE);
                        gpstFmUiData->PlayerState = FM_PLAYER_STATE_RECORDING;
                        //gpstFmUiData->Recordstate = RECORD_STATE_BEING;
                        FMControlTask_SendCmd(FM_CMD_RECORD, (void *)0, SYNC_MODE);
                        FMUITask_SendCmdToFMControl(NULL, (void *)0, SYNC_MODE);
                        //FMPlay_FMModeDisplay((void *)SID_RECORDING, 7, 0, TEXT_CMD_ID);
                        FMPlayer_RecordFileNameDisplay();
                        FMPlay_StateDisplay();
                    }

                    break;

                case 6://SID_PLAY_REC_DATA - SID_MANUAL_MOD:
                #ifdef __APP_RECORD_RECORDCONTROL_C__
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                    {
                        FMControlTask_SendCmd(FM_CMD_RECORD_STOP,(void *)0, SYNC_MODE);
                        RecordControlTask_SendCmd(RECORD_CMD_STOP,(void *)0, SYNC_MODE);
                        gpstFmUiData->Recordstate= RECORD_STATE_STOP;
                        if(RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE)!= RK_SUCCESS)
                        {
                            rk_printf("delete TASK_ID_RECORDCONTROL fault\n");
                        }
                    }
                #endif
                #ifdef __APP_FM_FMUITASK_C__
                    FMControlTask_SendCmd(FM_CMD_DEINIT, (void *)0, SYNC_MODE);
                    printf(".... FMUI  MainTask_TaskSwtich");

                    #ifdef __APP_BROWSER_BROWSERUITASK_C__
                    RK_TASK_BROWSER_ARG BrowerArg;
                    //UINT8 RecordPathStr[]      = {'C', ':', '\\', 'R', 'E', 'C', 'O', 'R', 'D', '\\', '\0'};
                    memcpy(BrowerArg.filepath, L"C:\\RECORD\\", StrLenW(L"C:\\RECORD\\")*2);
                    BrowerArg.filepath[StrLenW(L"C:\\RECORD\\")] = 0x0000;
                    BrowerArg.SelSouceType = SOURCE_FROM_RECORD;
                    BrowerArg.Flag = 0;
                    MainTask_TaskSwtich(TASK_ID_FMUI, 0, TASK_ID_BROWSERUI, 0, &BrowerArg);
                    #endif

                #endif
                    break;

                /*case 2://SID_DEL - SID_MANUAL_MOD:
                    #ifdef _USE_GUI_
                    if(gpstFmUiData->hMenu != NULL)
                    {
                        GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstFmUiData->hMenu);
                        gpstFmUiData->hMenu = NULL;
                        GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
                        FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
                    }
                    #endif

                    if(gpstFmUiData->FMPlayerMod == MOD_PRESET_STATION)
                    {
                        rk_printf("PRESET....\n");
                        FMPlay_SetDialog((void *)SID_DELETE_ALL, TEXT_CMD_ID, FMPlay_DialogCallBackForDeleteAllChannel);
                    }
                    else
                    {
                        #ifdef _USE_GUI_
                        FMPlay_SetPopupWindow((void *)SID_NO_PRESET, TEXT_CMD_ID);
                        #endif
                    }
                    break;
                */

                default:
                    break;
            }
            break;

        case KEY_VAL_PLAY_SHORT_UP: //menu

            break;

        default:
            break;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_ButtonCallBack
** Input:APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.4
** Time: 16:36:21
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FMUIMenuAskQueue;
    FMUI_ASK_QUEUE FMUIMenuAskQueue_tmp;
    if ((gpstFmUiData->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP))
    {
        rk_printf("FmUi lose key QueueFull=%d\n",gpstFmUiData->QueueFull);
        return RK_SUCCESS;
    }
    switch(event_type)
    {
       case APP_RECIVE_MSG_EVENT_KEY:
        {
            gpstFmUiData->QueueFull = 1;
            FMUIMenuAskQueue.cmd = event;
            FMUIMenuAskQueue.type = FMMENU_KEY_EVENT;
            FMUIMenuAskQueue.offset = NULL;
            ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIMenuAskQueue, 0);
            if(ret == RK_ERROR)
            {
                gpstFmUiData->QueueFull = 0;
                rk_printf("send key fail");
            }
        }
        break;

        case APP_RECIVE_MSG_EVENT_WARING:
        {
            FMUIMenuAskQueue.cmd= event;
            FMUIMenuAskQueue.type = event_type;
            FMUIMenuAskQueue.offset = NULL;
            gpstFmUiData->QueueFull = 1;
            ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIMenuAskQueue, 0);
            if(ret == RK_ERROR)
            {
                rkos_queue_receive(gpstFmUiData->FmUiAskQueue, &FMUIMenuAskQueue_tmp, 0);//MAX_DELAY
                rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIMenuAskQueue, 0);
            }

        }
        break;

        case APP_RECIVE_MSG_EVENT_DIALOG:
        {
            FMUIMenuAskQueue.cmd = event;
            FMUIMenuAskQueue.type = event_type;
            FMUIMenuAskQueue.offset = NULL;
            ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIMenuAskQueue, 0);
        }
        break;

        default:
        break;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_SelectCallBack
** Input:HGC pGc, eSELECT_EVENT_TYPE event_type, RKGUI_SELECT_ITEM * arg, int16 offset
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.4
** Time: 11:41:05
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, RKGUI_SELECT_ITEM * arg, int16 offset)
{
    RKGUI_SELECT_ITEM * item;
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FMUIAskQueue;
    FMUI_ASK_QUEUE FMUIAskQueue_tmp;
    int16 Cursor;

    //printf("system offset = %d event_type=0x%x arg=0x%x\n",offset,event_type, (uint32)arg);
    switch(event_type)
    {
        case SELECT_ENVEN_ENTER:
        {
            FMUIAskQueue.cmd= FM_MEUN_ENTER;
            FMUIAskQueue.type = FMMENU_KEY_EVENT;
            FMUIAskQueue.offset = offset;
            //printf("send offset %d 0x%x\n",FMUIAskQueue.offset,FMUIAskQueue.event_type);
            ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue, 0);
            if(ret == RK_ERROR)
            {
                printf("send enter enven\n");
                rkos_queue_receive(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue_tmp, 0);
                rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue, 0);
            }
        }
        break;

        case SELECT_ENVEN_UPDATA:
        {
            item = (RKGUI_SELECT_ITEM *)arg;

            item->cmd = TEXT_CMD_ID;
            item->text_id = SID_MANUAL_MOD + FmUi_Mode[offset];

            item->sel_icon = IMG_ID_ICON_SEL;
            item->unsel_icon = IMG_ID_PONIT_NOSEL;
        }
        break;

        case SELECT_ENVEN_MOVE:
            break;

        default:
            break;
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_Menu
** Input:uint32 SelectID, uint32 CursorID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.4
** Time: 11:36:21
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_Menu(uint32 CursorID)
{
#ifdef _USE_GUI_
    MENU_TEXT_INFO_STRUCT SetMenuTxtInfo;
    RKGUI_SELECT_ARG stSelectArg;
    rk_err_t ret;

    stSelectArg.x = 0;
    stSelectArg.y = 20;
    stSelectArg.xSize = 128;
    stSelectArg.ySize = 137;
    stSelectArg.display = 1;
    stSelectArg.level = 0;

    stSelectArg.Background = IMG_ID_BROWSER_BACKGROUND;
    stSelectArg.SeekBar = IMG_ID_BROWSER_SCOLL2;

    stSelectArg.Cursor = CursorID;
    stSelectArg.ItemStartOffset= 0;

    stSelectArg.itemNum = 7;
    stSelectArg.MaxDisplayItem= MAX_DISP_ITEM_NUM;

    stSelectArg.pReviceMsg = FMUITask_SelectCallBack;
    stSelectArg.CursorStyle= IMG_ID_SEL_ICON;
    stSelectArg.IconBoxSize= 16;
    FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBack, FMPlay_AudioCallBackNull);
    if(gpstFmUiData->hMenu)
    {
        if(GuiTask_DeleteWidget(gpstFmUiData->hMenu) != RK_SUCCESS)
        {
           rk_printf("\n -- hMenu dele NG... \n");
        }
    }
    gpstFmUiData->hMenu = GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
    if(gpstFmUiData->hMenu == NULL)
    {
        rk_printf("error:pGc == NULL\n");
    }
    //GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_DISPLAY, NULL, SYNC_MODE);
    //GuiTask_DeleteWidget(gpstFmUiData->hMenu);
    GuiTask_AppReciveMsg(FMUITask_ButtonCallBack);
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMPlayer_SpectrumDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.3
** Time: 14:38:50
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlayer_SpectrumDisplay(void)
{
#ifdef _USE_GUI_
    RKGUI_SPECTRUM_ARG pstSpectrumArg;
    RKGUI_SPECTRUM_ARG pstSpectrumContent;

    uint8 buf[12]= {0,0,0,0,0,0,0,0,0,0,0,0};

    if(gpstFmUiData->hSpecrm == NULL)
    {
        pstSpectrumArg.x= 23;
        pstSpectrumArg.y= 29;
        pstSpectrumArg.level= 0;
        pstSpectrumArg.display= 1;
        pstSpectrumArg.StripNum= 12;
        pstSpectrumArg.StripStyle= IMG_ID_MUSIC_SPECTRUM20;
        pstSpectrumArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstSpectrumArg.BackdropX= 23;
        pstSpectrumArg.BackdropY= 29;
        pstSpectrumArg.StripDat= buf;
        gpstFmUiData->hSpecrm = GUITask_CreateWidget(GUI_CLASS_SPECTRUM, &pstSpectrumArg);
    }
    else
    {
        pstSpectrumContent.cmd= SPECTRUM_SET_CONTENT;
        FMPlayer_GetCurSpectrum(&pstSpectrumContent.StripDat);
        GuiTask_OperWidget(gpstFmUiData->hSpecrm, OPERATE_SET_CONTENT, &pstSpectrumContent, SYNC_MODE);
    }
#endif
}
/*******************************************************************************
** Name: FMUITask_DeleteGuiHandle
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.22
** Time: 11:31:39
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_DeleteGuiHandle(void)
{
#ifdef _USE_GUI_
    if (gpstFmUiData->hFileName!= NULL)
    {
        //GuiTask_OperWidget(gpstFmUiData->hFileName, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hFileName);
        gpstFmUiData->hFileName = NULL;
    }

    if (gpstFmUiData->hDialog!= NULL)
    {
        GuiTask_AppUnReciveMsg(FMPlay_DialogCallBack);
        //GuiTask_OperWidget(gpstFmUiData->hDialog, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hDialog);
        gpstFmUiData->hDialog = NULL;
    }
    if (gpstFmUiData->hMenu!= NULL)
    {
        GuiTask_AppUnReciveMsg(FMUITask_ButtonCallBack);
        //GuiTask_OperWidget(gpstFmUiData->hMenu, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hMenu);
        gpstFmUiData->hMenu = NULL;
    }

    if (gpstFmUiData->hFMState!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hFMState);
        gpstFmUiData->hFMState = NULL;
    }

    if (gpstFmUiData->hBackdrop != NULL)
    {
        //GuiTask_OperWidget(gpstFmUiData->hDlna, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hBackdrop);
        gpstFmUiData->hBackdrop = NULL;
    }

    if (gpstFmUiData->hProgressBar != NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hProgressBar);
        gpstFmUiData->hProgressBar = NULL;
    }

    if (gpstFmUiData->hVolumeBar!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hVolumeBar);
        gpstFmUiData->hVolumeBar = NULL;
    }

    if (gpstFmUiData->hVolumeValue!= NULL)
    {
        printf("delete hVolumeValue....\n");
        GuiTask_DeleteWidget(gpstFmUiData->hVolumeValue);
        gpstFmUiData->hVolumeValue = NULL;
    }

    if (gpstFmUiData->hFMTotalTimer!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hFMTotalTimer);
        gpstFmUiData->hFMTotalTimer = NULL;
    }

    if (gpstFmUiData->hFMProgressTimer!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hFMProgressTimer);
        gpstFmUiData->hFMProgressTimer = NULL;
    }

    if (gpstFmUiData->hEQMode!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hEQMode);
        gpstFmUiData->hEQMode = NULL;
    }

    if (gpstFmUiData->hFMFreq!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hFMFreq);
        gpstFmUiData->hFMFreq = NULL;
    }

    if (gpstFmUiData->hMsgBox!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hMsgBox);
        gpstFmUiData->hMsgBox = NULL;
    }

    if (gpstFmUiData->hPlayerState!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hPlayerState);
        gpstFmUiData->hPlayerState = NULL;
    }

    if (gpstFmUiData->hSpecrm!= NULL)
    {
        GuiTask_DeleteWidget(gpstFmUiData->hSpecrm);
        gpstFmUiData->hSpecrm = NULL;
    }
#endif
}

/*******************************************************************************
** Name: FMPlay_StartPlayer
** Input:uint32 SourecePlayer
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 11:16:26
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_StartPlayer()
{
#ifdef _USE_GUI_
    rk_err_t ret;
    RK_TASK_FMCONTROL_ARG fmArg;
    FM_AUDIO_INFO AudioInfo;

    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
    {
        FMPlay_BackGroundDisplay();
        FMPlay_DisplayMsg(SID_PLAYER_FAILED);
        return RK_ERROR;
    }
    else if(RKTaskFind(TASK_ID_FMCONTROL, 0) == NULL)
    {
#ifdef __APP_FM_FMCONTROLTASK_C__
        rk_printf("start_fm \n");
        fmArg.FmArea = RADIO_CHINA;
        fmArg.FmStereo = 1;
        fmArg.LRChannel = 2;
        fmArg.FMInputType = gpstFmUiData->stAudioInfo.FMInputType;//RECORD_TYPE_LINEIN1
        fmArg.SamplesPerBlock = 4096;
        fmArg.pfmStateCallBcakForAPP = FMPlay_AudioCallBack;
        gSysConfig.RecordConfig.RecordQuality = RECORD_QUALITY_NORMAL;
        ret = RKTaskCreate(TASK_ID_FMCONTROL, 0, &fmArg, SYNC_MODE);
        if(ret != RK_SUCCESS)
        {
            FMPlay_BackGroundDisplay();
            FMPlay_DisplayMsg(SID_PLAYER_FAILED);
            rk_printf("TASK_ID_FMCONTROL ERROR ret = %d", ret);
            return RK_ERROR;
        }
#endif
        return RK_SUCCESS;
    }

#else //无屏幕操作

#endif
}
/*******************************************************************************
** Name: FMPlay_SetPopupWindow
** Input:int16 type
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 11:09:34
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMPlay_SetPopupWindow(void * text, uint16 text_cmd)
{
#ifdef _USE_GUI_
    FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBack, FMPlay_AudioCallBackNull);

    RKGUI_MSGBOX_ARG pstMsgBoxArg;

    pstMsgBoxArg.cmd = MSGBOX_CMD_WARNING;
    pstMsgBoxArg.x = 4;
    pstMsgBoxArg.y = 36;
    pstMsgBoxArg.display = 1;
    pstMsgBoxArg.level = 0;
    pstMsgBoxArg.title = SID_WARNING;
    pstMsgBoxArg.text_cmd = text_cmd;//TEXT_CMD_BUF
    pstMsgBoxArg.text = text;
    pstMsgBoxArg.align= TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;

    if(gpstFmUiData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstFmUiData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hMsgBox);
        gpstFmUiData->hMsgBox = NULL;
    }

    if(gpstFmUiData->hMsgBox == NULL)
    {
        gpstFmUiData->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FMPlay_StateDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 11:07:42
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_StateDisplay(void)
{
#ifdef _USE_GUI_
    uint32 PlayerState;
    RKGUI_ICON_ARG pstIconArg;

    if (gpstFmUiData->PlayerState == FM_PLAYER_STATE_PAUSE)
    {
        PlayerState = IMG_ID_MUSIC_PAUSE_STATE;
    }
    else if (gpstFmUiData->PlayerState == FM_PLAYER_STATE_PLAY)
    {
        PlayerState = IMG_ID_MUSIC_PLAY_STATE;
    }
    else if (gpstFmUiData->PlayerState == FM_PLAYER_STATE_RECORDING)
    {
        PlayerState = IMG_ID_RECORDING;
    }
    else if (gpstFmUiData->PlayerState == FM_PLAYER_STATE_PAUSE_RECORD)
    {
        PlayerState = IMG_ID_RECORD_PAUSE;
    }

    if(gpstFmUiData->hPlayerState == NULL)
    {
        pstIconArg.resource = PlayerState;//IMG_ID_MUSIC_PLAY_STATE
        pstIconArg.x= 45;
        pstIconArg.y= 141;
        pstIconArg.level = 0;
        pstIconArg.display= 1;
        gpstFmUiData->hPlayerState= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
    }
    else
    {
        GuiTask_OperWidget(gpstFmUiData->hPlayerState, OPERATE_SET_CONTENT, (void *)PlayerState, SYNC_MODE);
    }
#endif
}
/*******************************************************************************
** Name: FMPlay_PlayProcessDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 11:06:16
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_PlayProcessDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    char buf[9];
    Ucs2 Time[9];
    RKGUI_PROGRESSBAR_ARG pstProgressArg;
    RKGUI_TEXT_ARG pstTextArg;

    uint8 Sec, Min, Hour;

    #if 1
    RKGUI_TEXT_ARG FMplayerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    #endif

    FMPlayer_GetCurTime(&gpstFmUiData->FMProgressTimer);
    FMPlayer_GetTotalTime(&gpstFmUiData->FMTotalTimer);

    Sec = (gpstFmUiData->FMTotalTimer / 1000) % 60;
    Min = (gpstFmUiData->FMTotalTimer / 60000) % 60;
    Hour = gpstFmUiData->FMTotalTimer / 3600000;

    sprintf(buf, "%02d:%02d:%02d", Hour, Min, Sec);
    //printf ("TotalTime=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        Time[i]= (Ucs2)buf[i];
    }

    Time[i] = 0;

    pstTextArg.display= 1;
    pstTextArg.level = 0;
    pstTextArg.lucency= OPACITY;
    pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
    pstTextArg.ForegroundR= 0;
    pstTextArg.ForegroundG= 0;
    pstTextArg.ForegroundB= 0;
    pstTextArg.BackgroundR= 255;
    pstTextArg.BackgroundG= 255;
    pstTextArg.BackgroundB= 255;

    if(gpstFmUiData->hFMTotalTimer == NULL)
    {
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 84;
        pstTextArg.y= 85;
        pstTextArg.xSize= -1;//42
        pstTextArg.ySize= -1;
        pstTextArg.text= Time;
        pstTextArg.BackdropX= 84;
        pstTextArg.BackdropY= 85;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstFmUiData->hFMTotalTimer= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        FMplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
        FMplayerTextContentArg.text = Time;

        GuiTask_OperWidget(gpstFmUiData->hFMTotalTimer, OPERATE_SET_CONTENT, &FMplayerTextContentArg, SYNC_MODE);
    }

    Sec = (gpstFmUiData->FMProgressTimer / 1000) % 60;
    Min = (gpstFmUiData->FMProgressTimer / 60000) % 60;
    Hour = gpstFmUiData->FMProgressTimer / 3600000;

    sprintf(buf, "%02d:%02d:%02d", Hour, Min, Sec);
    //printf ("CurrentTime=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        Time[i]= (Ucs2)buf[i];
    }

    Time[i] = 0;


    if(gpstFmUiData->hFMProgressTimer == NULL)
    {
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 4;
        pstTextArg.y= 85;
        pstTextArg.xSize= -1;//43
        pstTextArg.ySize= -1;
        pstTextArg.text= Time;
        pstTextArg.BackdropX= 4;
        pstTextArg.BackdropY= 85;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstFmUiData->hFMProgressTimer= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        FMplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
        FMplayerTextContentArg.text = Time;
        GuiTask_OperWidget(gpstFmUiData->hFMProgressTimer, OPERATE_SET_CONTENT, &FMplayerTextContentArg, SYNC_MODE);
    }

    if (gpstFmUiData->FMTotalTimer== 0)return;

    pstProgressBarContent.cmd = PROGRESSBAR_SET_CONTENT;
    pstProgressBarContent.percentage = gpstFmUiData->FMProgressTimer* 100 / gpstFmUiData->FMTotalTimer;

    if(gpstFmUiData->hProgressBar == NULL)
    {
        pstProgressArg.x= 7;
        pstProgressArg.y= 78;
        pstProgressArg.display = 1;
        pstProgressArg.level = 0;

        pstProgressArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstProgressArg.BackdropX= 7;
        pstProgressArg.BackdropY= 78;

        pstProgressArg.Bar= IMG_ID_PROGRESS_BAR;
        pstProgressArg.percentage = pstProgressBarContent.percentage;
        gpstFmUiData->hProgressBar= GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        GuiTask_OperWidget(gpstFmUiData->hProgressBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: FMPlay_FMFreqDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 11:04:37
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_FMFreqDisplay(void)
{
#ifdef _USE_GUI_
    FM_AUDIO_INFO FMAudioInfo;
    Ucs2 FMFreq[9];
    char buf[9];
    uint8 i=0;
    uint8 tempInt, tempDecimal;

    RKGUI_TEXT_ARG FMplayerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    FMUI_GetAudioInfo(&FMAudioInfo);

    tempInt = FMAudioInfo.CurFreq / 100;
    tempDecimal = (FMAudioInfo.CurFreq % 100)/10;
    sprintf(buf, "%d.%d MHZ", tempInt, tempDecimal);

    for (i= 0; i< sizeof(buf); i++)
    {
        FMFreq[i]= (Ucs2)buf[i];
    }


    if (gpstFmUiData->hFMFreq == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 30;
        pstTextArg.y= 96;//102
        pstTextArg.xSize= 55;
        pstTextArg.ySize= -1;
        pstTextArg.BackdropX= 30;
        pstTextArg.BackdropY= 96;//102
        pstTextArg.text= FMFreq;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstFmUiData->hFMFreq= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
        GuiTask_OperWidget(gpstFmUiData->hFMFreq, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);

    }
    else
    {
        FMplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
        FMplayerTextContentArg.text = FMFreq;
        GuiTask_OperWidget(gpstFmUiData->hFMFreq, OPERATE_SET_CONTENT, &FMplayerTextContentArg, SYNC_MODE);
    }
#endif
}
/*******************************************************************************
** Name: FMPlay_DisplayMusicInfo
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 11:03:29
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_DisplayMusicInfo(void)
{
    FMPlay_FMFreqDisplay();
    FMPlay_PlayProcessDisplay();
    FMPlay_StateDisplay();
}

/*******************************************************************************
** Name: FMPlay_DisplayAll
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 10:58:01
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_DisplayAll(void)
{
#ifdef _USE_GUI_
    GuiTask_ScreenLock();
    rk_printf("FMPlay_DisplayAll...\n");
    //FMPlay_PlayOrderDisplay();
    FMPlay_DisplayMusicInfo();

    if(gpstFmUiData->stAudioInfo.playerr)
    {
        FMPlay_SetPopupWindow((void *)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
    }

    GuiTask_ScreenUnLock();
#endif
}
/*******************************************************************************
** Name: FMPlay_MusicEqDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 10:57:00
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_MusicEqDisplay(void)
{
#ifdef _USE_GUI_
    uint32 i;
    uint32 SidEq[9] = {SID_EQ_HEAVY,SID_EQ_POP,SID_EQ_JAZZ, SID_EQ_UNIQUE, SID_EQ_SELECT, SID_EQ_BASS, 0, SID_EQ_CUSTOM, SID_EQ_NONE};

    RKGUI_TEXT_ARG FMplayerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    if(gpstFmUiData->hEQMode == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_ID;
        pstTextArg.x= 90;
        pstTextArg.y= 96;//113
        pstTextArg.xSize= -1;//46
        pstTextArg.ySize= -1;
        pstTextArg.text= NULL;
        pstTextArg.BackdropX= 90;
        pstTextArg.BackdropY= 96;//113
        pstTextArg.resource = SidEq[gpstFmUiData->stAudioInfo.EqMode - EQ_HEAVY];
        pstTextArg.align= TEXT_ALIGN_X_RIGHT;
        gpstFmUiData->hEQMode = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        FMplayerTextContentArg.opercmd = TEXT_SET_CONTENT;
        FMplayerTextContentArg.resource = SidEq[gpstFmUiData->stAudioInfo.EqMode - EQ_HEAVY];
        GuiTask_OperWidget(gpstFmUiData->hEQMode, OPERATE_SET_CONTENT, &FMplayerTextContentArg, SYNC_MODE);
    }
#endif
}
/*******************************************************************************
** Name: FMPlay_VolumeDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 10:56:00
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_VolumeDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    char buf[3];
    Ucs2 VolumeValue[3];

    RKGUI_TEXT_ARG FMplayerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    RKGUI_TEXT_ARG pstTextArg;
    RKGUI_PROGRESSBAR_ARG pstProgressArg;

    sprintf(buf, "%02d", gpstFmUiData->stAudioInfo.PlayVolume);
    for (i= 0; i< sizeof(buf); i++)
    {
        VolumeValue[i]= (Ucs2)buf[i];
    }
    printf ("Volume buf=%s Volume=%d i =%d\n",buf, gpstFmUiData->stAudioInfo.PlayVolume, i);
    VolumeValue[2] = 0;

    if(gpstFmUiData->hVolumeValue == NULL)
    {
        printf ("hVolumeValue == NULL\n");
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 114;
        pstTextArg.y= 143;
        pstTextArg.xSize= -1;
        pstTextArg.ySize= -1;
        pstTextArg.text= VolumeValue;
        pstTextArg.BackdropX= 114;
        pstTextArg.BackdropY= 143;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstFmUiData->hVolumeValue = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        printf ("hVolumeValue != NULL\n");
        FMplayerTextContentArg.opercmd = TEXT_SET_CONTENT;
        FMplayerTextContentArg.text = VolumeValue;
        GuiTask_OperWidget(gpstFmUiData->hVolumeValue, OPERATE_SET_CONTENT, &FMplayerTextContentArg, SYNC_MODE);
    }

    if(gpstFmUiData->hVolumeBar == NULL)
    {
        pstProgressArg.x= 70;
        pstProgressArg.y= 147;
        pstProgressArg.display = 1;
        pstProgressArg.level= 0;

        pstProgressArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstProgressArg.BackdropX= 70;
        pstProgressArg.BackdropY= 147;

        pstProgressArg.Bar= IMG_ID_VOLUME_BAR;
        pstProgressArg.percentage= gpstFmUiData->stAudioInfo.PlayVolume* 100/ MaximumVolume;
        gpstFmUiData->hVolumeBar = GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        pstProgressBarContent.cmd= PROGRESSBAR_SET_CONTENT;
        pstProgressBarContent.percentage = gpstFmUiData->stAudioInfo.PlayVolume* 100/ MaximumVolume;
        //PLAYMENU_DEBUG("percentage=%d\n",pstProgressBarContent.percentage);
        GuiTask_OperWidget(gpstFmUiData->hVolumeBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
    return ;
}
/*******************************************************************************
** Name: FMPlay_DisplayMsg
** Input:uint32 Sid
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 10:41:05
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_DisplayMsg(uint32 Sid)
{
#ifdef _USE_GUI_
    RKGUI_TEXT_ARG TextArg;

    TextArg.cmd= TEXT_CMD_ID;
    TextArg.lucency= OPACITY;
    TextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
    TextArg.ForegroundR= 0;
    TextArg.ForegroundG= 0;
    TextArg.ForegroundB= 0;
    TextArg.BackgroundR= 255;
    TextArg.BackgroundG= 255;
    TextArg.BackgroundB= 255;
    TextArg.align= TEXT_ALIGN_X_LEFT;
    TextArg.display = 1;
    TextArg.level = 0;
    TextArg.BackdropX= 25;
    TextArg.BackdropY= 103;
    TextArg.x= 25;
    TextArg.y= 103;
    TextArg.xSize= 103;//103
    TextArg.ySize= -1;

    TextArg.resource = Sid;
    gpstFmUiData->hFMFreq = GUITask_CreateWidget(GUI_CLASS_TEXT, &TextArg);
#endif
}
/*******************************************************************************
** Name: FMPlay_BackGroundDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 10:39:20
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_BackGroundDisplay(void)
{
#ifdef _USE_GUI_
    RKGUI_ICON_ARG pstIconArg;

    pstIconArg.resource= IMG_ID_MUSIC_BACKGROUND;
    pstIconArg.x= 0;
    pstIconArg.y= 0;
    pstIconArg.level = 0;
    pstIconArg.display= 1;
    gpstFmUiData->hBackdrop = GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
#endif
}
/*******************************************************************************
** Name: FMPlay_AudioCallBack
** Input:uint32 audio_state
** Return: void
** Owner:cjh
** Date: 2016.4.21
** Time: 10:17:35
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN void FMPlay_AudioCallBack(uint32 audio_state)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FmUiAskQueue;

    if(audio_state == FM_PLAYER_STATE_AUTOSEACH_END)
    {
        gpstFmUiData->FMPlayerMod = MOD_PRESET_STATION;
    }

    FmUiAskQueue.type = FMTASK_AUDIO_EVENT;

    FmUiAskQueue.cmd = audio_state;

    //printf("\nfm call back... ch =%d", audio_state);
    ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FmUiAskQueue, 0);//MAX_DELAY
    if(ret == RK_ERROR)
    {
       //rk_printf("audio_state send fail...\n");
    }
}

/*******************************************************************************
** Name: FMUITask_DisplayInit
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 9:22:23
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_DisplayInit(void)
{
#ifdef _USE_GUI_
    gpstFmUiData->PlayerState = FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
    rk_printf("gpstFmUiData->stAudioInfo Samplerate = %d PlayVolume =%d CurFreq = %d\n"
        , gpstFmUiData->stAudioInfo.Samplerate
        , gpstFmUiData->stAudioInfo.PlayVolume
        , gpstFmUiData->stAudioInfo.CurFreq
    );

    GuiTask_ScreenLock();

    FMPlay_BackGroundDisplay();
    FMPlay_VolumeDisplay();
    FMPlay_MusicEqDisplay();
    FMPlay_DisplayAll();
    if(gpstFmUiData->stAudioInfo.playerr)
    {
        FMPlay_SetPopupWindow((void *)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
    }

    GuiTask_ScreenUnLock();
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_GuiCallBack
** Input:APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 8:40:33
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_GuiCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    FMUI_ASK_QUEUE FMUIAskQueue;
    FMUI_ASK_QUEUE FMUIAskQueue_tmp;

    if ((gpstFmUiData->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP))
    {
        rk_printf(".lose key QueueFull=%d\n",gpstFmUiData->QueueFull);
        return RK_SUCCESS;
    }
    if(evnet_type==APP_RECIVE_MSG_EVENT_KEY)
    {
        gpstFmUiData->QueueFull = 1;
        FMUIAskQueue.cmd = event;
        FMUIAskQueue.type = FMTASK_KEY_EVENT;
        ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue, 0);
        if(ret == RK_ERROR)
        {
            gpstFmUiData->QueueFull = 0;
            rk_printf("send key fail");
        }
    }
    else if(evnet_type==APP_RECIVE_MSG_EVENT_WARING)
    {
        gpstFmUiData->QueueFull = 1;

        FMUIAskQueue.type = FMTASK_MSGBOX_EVENT;
        FMUIAskQueue.cmd = event;
        printf("\nfm WARING EVENT...\n ");

        ret = rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue, 0);
        if (ret == RK_ERROR)
        {
            rkos_queue_receive(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue_tmp, 0);//MAX_DELAY
            rkos_queue_send(gpstFmUiData->FmUiAskQueue, &FMUIAskQueue, 0);
        }
    }
    //rk_printf("\nfm GuiCallBack over...\n ");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_NoScreenEvent
** Input:uint32 cmd, int16 type
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 18:04:27
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_NoScreenEvent(uint32 cmd, int16 type)
{
    switch (cmd)
    {
        case 0:
            FMPlay_StartPlayer();
            break;
        default:
            break;
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_DiaLogEvent
** Input:uint32 cmd, int16 type
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 18:03:23
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_DiaLogEvent(uint32 cmd, P_APP_RECIVE_MSG fun)
{
#ifdef _USE_GUI_
    if(gpstFmUiData->hDialog != NULL)
    {
        GuiTask_AppUnReciveMsg(fun);
        GuiTask_OperWidget(gpstFmUiData->hDialog, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstFmUiData->hDialog);
        rk_printf("Delete hDialog \n");
        gpstFmUiData->hDialog = NULL;
        FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBackNull, FMPlay_AudioCallBack);
    }
    switch(cmd)
    {
        case FM_DIALOG_STOP_AUTOSEARCH:
            rk_printf("FM_CMD_STEPSTATION....\n");
            FMUITask_SendCmdToFMControl(FM_CMD_STEPSTATION, (void *)FM_DIRECT_CUR, SYNC_MODE);
            break;

        case FM_DIALOG_AUTOSEARCH:
            gpstFmUiData->FMPlayerMod = MOD_AUTO_PRESET;
            FMUITask_SendCmdToFMControl(FM_CMD_AUTOSEARCH_RESUME, NULL, SYNC_MODE);
            rk_printf("RESUME AUTO....\n");
            break;

        case FM_DIALOG_SAVE_CH:
            FMUITask_SendCmdToFMControl(FM_CMD_SAVE_CH, NULL, SYNC_MODE);
            break;

        case FM_DIALOG_ABANDON_SAVE_CH:

            break;

        case FM_DIALOG_DELETE_CH:
            FMUITask_SendCmdToFMControl(FM_CMD_DELETE_CH, NULL, SYNC_MODE);
            break;

        case FM_DIALOG_ABANDON_DELETE_CH:

            break;

        case FM_DIALOG_DELETE_ALL_CH:
            FMUITask_SendCmdToFMControl(FM_CMD_DELETE_ALL_CH, NULL, SYNC_MODE);
            break;

        case FM_DIALOG_ABANDON_DELETE_ALL_CH:

            break;

        default:
            break;
    }
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_AudioEvent
** Input:uint32 AudioState
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 18:02:31
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_AudioEvent(uint32 AudioState)
{
    FM_AUDIO_INFO FMAudioInfo;
    switch (AudioState)
    {
        case FM_PLAYER_STATE_PLAY: //play or pause
            gpstFmUiData->PlayerState = FM_PLAYER_STATE_PLAY;
            FMPlay_StateDisplay();
            break;

        case FM_PLAYER_STATE_PAUSE: //play or pause
            gpstFmUiData->PlayerState = FM_PLAYER_STATE_PAUSE;
            FMPlay_StateDisplay();
            break;

        case FM_PLAYER_STATE_RECORDING:
            gpstFmUiData->PlayerState = FM_PLAYER_STATE_RECORDING;
            FMPlay_StateDisplay();
            break;

        case FM_PLAYER_STATE_PAUSE_RECORD://
            gpstFmUiData->PlayerState = FM_PLAYER_STATE_PAUSE_RECORD;
            FMPlay_StateDisplay();
            //MusicPlay_PlayProcessDisplay();
            break;

        case FM_STATE_UPDATA_FM_CH: //update
            FMPlay_FMFreqDisplay();
            break;

        case FM_PLAYER_STATE_AUTOSEACH_END: //update
            FMPlay_FMFreqDisplay();
            FMPlay_FMModeDisplay("Preset:", 7, 1, TEXT_CMD_BUF);
            break;

        case FM_STATE_UPDATA_FM_PRESET: //update
            FMPlay_FMFreqDisplay();
            FMPlay_FMModeDisplay("Preset:", 7, 1, TEXT_CMD_BUF);
            break;

        case FM_STATE_NO_EXIST_FM_STATION:
            FMPlay_SetPopupWindow((void *)SID_WITHOUT_CHANNEL, TEXT_CMD_ID);//preset station null
            break;

#ifdef _RK_SPECTRUM_
        case FM_PLAYER_STATE_UPDATA_SPECTRUM:
            gpstFmUiData->GetCurTime += 100;
            if((gpstFmUiData->GetCurTime % 1000) == 0)
            {
                FMPlay_PlayProcessDisplay();
            }
            FMPlayer_SpectrumDisplay();
            break;
#endif
        case AUDIO_STATE_ERROR:
            if(gpstFmUiData->stAudioInfo.playerr)
            {
                FMPlay_SetPopupWindow((void *)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
            }
            break;

        case FM_STATE_SAVE_CUR_FM_CH:
            FMPlay_FMModeDisplay("Preset:", 7, 1, TEXT_CMD_BUF);
            //FMPlay_SetPopupWindow((void *)SID_SAVE_CHANNEL, TEXT_CMD_ID);
            break;

        case FM_STATE_SAVE_CUR_FM_CH_FULL:
            //FMPlay_FMModeDisplay("Preset:", 7, 1, TEXT_CMD_BUF);
            FMPlay_SetPopupWindow((void *)SID_SAVE_CHANNEL, TEXT_CMD_ID);
            break;

        case FM_STATE_DELETE_PRESET_FM_CH:
            //FMUI_GetAudioInfo(&FMAudioInfo);
            //if(FMAudioInfo.CurFmPresetNo > 0)
            //{
            //    FMPlay_FMModeDisplay("Preset:", 7, 1, TEXT_CMD_BUF);
            //}
            //else
            //{
            if(gpstFmUiData->FMPlayerMod == MOD_PRESET_STATION)
            {
                FMPlay_FMModeDisplay((void *)SID_PRESET_MOD, 7, 0, TEXT_CMD_ID);
            }
            else
            {
                FMPlay_FMModeDisplay((void *)SID_MANUAL_MOD, 7, 0, TEXT_CMD_ID);
            }
            //}
            //FMPlay_SetPopupWindow((void *)SID_DEL, TEXT_CMD_ID);
            break;

        case FM_STATE_DELETE_PRESET_ALL_CH:
            FMPlay_FMModeDisplay((void *)SID_PRESET_MOD, 7, 0, TEXT_CMD_ID);
            //FMPlay_SetPopupWindow((void *)SID_DELETE_ALL, TEXT_CMD_ID);
            break;

        default:
            break;


    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_NoScreenKeyRev
** Input:uint32 keyvalue
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 18:01:32
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_NoScreenKeyRev(uint32 keyvalue)
{

}
/*******************************************************************************
** Name: FMUITask_KeyEvent
** Input:uint32 KeyVal
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 18:00:28
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_KeyEvent(uint32 KeyVal)
{
    if(gpstFmUiData->hMsgBox != NULL)
    {
        return RK_SUCCESS;
    }

    if(gpstFmUiData->FMPlayerMod == MOD_AUTO_PRESET)
    {
        gpstFmUiData->FMPlayerMod = MOD_PRESET_STATION;
        FMControlTask_SendCmd(FM_CMD_Idle, NULL, SYNC_MODE);
        FMPlay_SetDialog((void *)SID_PRESET_MOD, TEXT_CMD_ID, FMPlay_DialogCallBack);
    }
    else if(gpstFmUiData->FMPlayerMod == MOD_MANUAL_SEMI_AUTO)
    {
        gpstFmUiData->FMPlayerMod = MOD_MANUAL;
        FMControlTask_SendCmd(FM_CMD_Idle, NULL, SYNC_MODE);
        FMPlay_FMModeDisplay((void *)SID_MANUAL_MOD, 7, 0, TEXT_CMD_ID);
    }
    else if(gpstFmUiData->FMPlayerMod == MOD_REC)
    {
        switch(KeyVal)
        {
            case KEY_VAL_PLAY_SHORT_UP:
                int16 resource;
                if(gpstFmUiData->Recordstate == RECORD_STATE_BEING)
                {
                    RecordControlTask_SendCmd(RECORD_CMD_PAUSE, NULL, SYNC_MODE);
                    resource = SID_RECORD_PAUSE;
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_PAUSE_RECORD;
                }
                else if(gpstFmUiData->Recordstate == RECORD_STATE_PAUSE)
                {
                    RecordControlTask_SendCmd(RECORD_CMD_RESUME, NULL, SYNC_MODE);
                    resource = SID_RECORDING;
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_RECORDING;
                }
                else
                {
                    RecordControlTask_SendCmd(RECORD_CMD_START, NULL, SYNC_MODE);
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_RECORDING;
                    resource = SID_RECORDING;
                }
                #ifdef _USE_GUI_
                //FMPlay_FMModeDisplay((void *)resource, 7, 0, TEXT_CMD_ID);
                FMPlay_StateDisplay();
                //GuiTask_OperWidget(gpstRecodData->hStatus, OPERATE_SET_CONTENT, &TextContentArg, SYNC_MODE);
                #endif
                break;
            case KEY_VAL_ESC_SHORT_UP:
                #ifdef __APP_RECORD_RECORDCONTROL_C__
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                    {
                        FMControlTask_SendCmd(FM_CMD_RECORD_STOP,(void *)0, SYNC_MODE);
                        RecordControlTask_SendCmd(RECORD_CMD_STOP,(void *)0, SYNC_MODE);
                        gpstFmUiData->Recordstate= RECORD_STATE_STOP;
                        if(RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE)!= RK_SUCCESS)
                        {
                            rk_printf("delete TASK_ID_RECORDCONTROL fault\n");
                        }
                    }
                #endif
                #ifdef _USE_GUI_
                    if (gpstFmUiData->hFileName!= NULL)
                    {
                        GuiTask_OperWidget(gpstFmUiData->hFileName, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstFmUiData->hFileName);
                        gpstFmUiData->hFileName = NULL;
                    }

                    FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
                    if(gpstFmUiData->stAudioInfo.FmSearchMode == FM_CMD_STEPSTATION)
                    {
                        gpstFmUiData->FMPlayerMod = MOD_PRESET_STATION;
                        FMPlay_FMModeDisplay("Preset:", 7, 1, TEXT_CMD_BUF);
                    }
                    else
                    {
                        gpstFmUiData->FMPlayerMod = MOD_MANUAL;
                        FMPlay_FMModeDisplay("Manual", 6, 0, TEXT_CMD_BUF);
                    }
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_PLAY;
                    FMPlay_StateDisplay();
                #endif

                break;

            case KEY_VAL_ESC_PRESS:

                break;

            case KEY_VAL_PLAY_PRESS_START://关机
                #ifdef __APP_RECORD_RECORDCONTROL_C__
                if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                {
                    FMControlTask_SendCmd(FM_CMD_RECORD_STOP,(void *)0, SYNC_MODE);
                    RecordControlTask_SendCmd(RECORD_CMD_STOP,(void *)0, SYNC_MODE);
                    gpstFmUiData->Recordstate= RECORD_STATE_STOP;
                    if(RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE)!= RK_SUCCESS)
                    {
                        rk_printf("delete TASK_ID_RECORDCONTROL fault\n");
                    }
                }
                #endif
                FMControlTask_SendCmd(FM_CMD_DEINIT, (void *)0, SYNC_MODE);
                MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                break;

            case KEY_VAL_MENU_SHORT_UP: //Start and Stop
                //FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
                break;
            default:
                break;
        }
    }
    else
    {
        switch (KeyVal)
        {
            case KEY_VAL_PLAY_SHORT_UP:
                //FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)1, SYNC_MODE);
                //gpstFmUiData->PlayerState = FM_PLAYER_STATE_PAUSE;
                #ifdef _USE_GUI_
                FMUITask_Menu(0);
                printf("select over ...");
                #else

                #endif
                break;
            case KEY_VAL_ESC_SHORT_UP:
                #ifdef _USE_GUI_
                    //FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBack, MainTask_AudioCallBack);
                    printf("FMtask.. ESC\n");
                #ifdef __APP_FM_FMUITASK_C__
                    FMControlTask_SendCmd(FM_CMD_DEINIT, (void *)0, SYNC_MODE);
                    //printf(".... FMUI  MainTask_TaskSwtich");
                    MainTask_TaskSwtich(TASK_ID_FMUI, 0,  TASK_ID_MAIN, 0,  NULL);
                #endif
                    while(1)
                    {
                        rkos_sleep(2000);
                        rk_printf("TASK_ID_FMUI\n");
                    }
                #else

                #endif

                break;

            case KEY_VAL_ESC_PRESS:

                break;


            case KEY_VAL_PLAY_PRESS_START://关机
                FMControlTask_SendCmd(FM_CMD_DEINIT, (void *)0, SYNC_MODE);
                MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                break;

            case KEY_VAL_FFW_PRESS:
                //printf(" ..FFW.HAND_AUTOSEARCH... ");
                if(gpstFmUiData->FMPlayerMod != MOD_PRESET_STATION)
                {
                    FMControlTask_SendCmd(FM_CMD_HAND_AUTOSEARCH, (void *)1, SYNC_MODE);
                }
                else
                {
                    FMControlTask_SendCmd(FM_CMD_STEPSTATION, (void *)1, SYNC_MODE);
                }
                break;

            case KEY_VAL_FFW_SHORT_UP: //Next station
                printf(" ..FFW SHORT_UP.HAND_AUTOSEARCH... ");
                if(gpstFmUiData->FMPlayerMod != MOD_PRESET_STATION)
                {
                    FMControlTask_SendCmd(FM_CMD_MANUAL, (void *)1, SYNC_MODE);
                }
                else
                {
                    FMControlTask_SendCmd(FM_CMD_STEPSTATION, (void *)1, SYNC_MODE);
                }
                break;

            case KEY_VAL_FFD_PRESS:
                printf(" .FFD..HAND_AUTOSEARCH... ");
                if(gpstFmUiData->FMPlayerMod != MOD_PRESET_STATION)
                {
                    FMControlTask_SendCmd(FM_CMD_HAND_AUTOSEARCH, (void *)-1, SYNC_MODE);
                }
                else
                {
                    FMControlTask_SendCmd(FM_CMD_STEPSTATION, (void *)-1, SYNC_MODE);
                }
                break;
            case KEY_VAL_FFD_SHORT_UP: // Last station
                printf("FFD SHORT_UP..HAND_AUTOSEARCH ..\n");
                if(gpstFmUiData->FMPlayerMod != MOD_PRESET_STATION)
                {
                    FMControlTask_SendCmd(FM_CMD_MANUAL, (void *)-1, SYNC_MODE);
                }
                else
                {
                    FMControlTask_SendCmd(FM_CMD_STEPSTATION, (void *)-1, SYNC_MODE);
                }
                break;

            case KEY_VAL_UP_PRESS:
            case KEY_VAL_UP_SHORT_UP: //semi-automatic search +
                if(gpstFmUiData->stAudioInfo.PlayVolume >= MaximumVolume)
                {
                    break;
                }
                gpstFmUiData->stAudioInfo.PlayVolume++;

                if(FMControlTask_SendCmd(FM_CMD_VOLUMESET, (void *)gpstFmUiData->stAudioInfo.PlayVolume, SYNC_MODE) == RK_SUCCESS)
                {
                    FMPlay_VolumeDisplay();
                }
                break;

            case KEY_VAL_DOWN_PRESS:
            case KEY_VAL_DOWN_SHORT_UP: //semi-automatic search -
                if(gpstFmUiData->stAudioInfo.PlayVolume == 0)
                {
                    break;
                }
                gpstFmUiData->stAudioInfo.PlayVolume--;

                if(FMControlTask_SendCmd(FM_CMD_VOLUMESET, (void *)gpstFmUiData->stAudioInfo.PlayVolume, SYNC_MODE) == RK_SUCCESS)
                {
                    FMPlay_VolumeDisplay();
                }
                break;

            case KEY_VAL_MENU_SHORT_UP: //Start and Stop
                //printf("MENU_SHORT_UP..\n");
                if(gpstFmUiData->PlayerState != FM_PLAYER_STATE_PLAY)
                {
                    FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_PLAY;
                }
                else
                {
                    FMControlTask_SendCmd(FM_CMD_PAUSE_RESUME, (void *)1, SYNC_MODE);
                    gpstFmUiData->PlayerState = FM_PLAYER_STATE_PAUSE;
                }
                //gpstFmUiData->PlayerState = FMUI_GetAudioInfo(&gpstFmUiData->stAudioInfo);
                FMPlay_StateDisplay();
                break;
            default:
                break;
        }
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUITask_PlayerOpen
** Input:int16 type
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 17:56:05
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITask_PlayerOpen()
{
    FMUI_ASK_QUEUE FmUIAskQueue;
#ifdef _USE_GUI_
    if(MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)
        || MainTask_GetStatus(MAINTASK_APP_BT_PLAYER)
        || MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)
        || MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER))
    {
        FMPlay_BackGroundDisplay();
        FMPlay_DisplayMsg(SID_PLAYER_FAILED);
        while (1)
        {
            rkos_queue_receive(gpstFmUiData->FmUiAskQueue, &FmUIAskQueue, MAX_DELAY);
            switch (FmUIAskQueue.cmd)
            {
                case KEY_VAL_ESC_SHORT_UP:
                    MainTask_TaskSwtich(TASK_ID_FMUI, 0, TASK_ID_MAIN, 0, NULL);
                    while(1)
                    {
                        rkos_sleep(2000);
                    }
                    break;
                default:
                    gpstFmUiData->QueueFull = 0;
                    break;
            }
        }
        return RK_ERROR;
    }
#else //无屏幕操作打开播放器

#endif
}

/*******************************************************************************
** Name: FMUITaskPlay_DisplayInit
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 17:51:19
*******************************************************************************/
_APP_FM_FMUITASK_COMMON_
COMMON FUN rk_err_t FMUITaskPlay_DisplayInit(void)
{

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FmUiTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 14:08:44
*******************************************************************************/
_APP_FM_FMUITASK_INIT_
INIT API rk_err_t FmUiTask_DeInit(void *pvParameters)
{
    rk_err_t ret;
    //MainTask_SetStatus(MAINTASK_APP_FM_PLAYER, 0);
    rk_printf("FmUiTask_DeInit....\n");
    MainTask_SetStatus(MAINTASK_APP_FMUI,0);

#ifdef __APP_FM_FMCONTROLTASK_C__
    if(RKTaskFind(TASK_ID_FMCONTROL, 0) != NULL)
    {
        FMControlTask_SetStateChangeFunc(FMPlay_AudioCallBack, NULL);
        RKTaskDelete(TASK_ID_FMCONTROL, 0, ASYNC_MODE);
    }
#endif

#ifdef _USE_GUI_
    FMUITask_DeleteGuiHandle();
    GuiTask_AppUnReciveMsg(FMUITask_GuiCallBack);
#endif

    rkos_queue_delete(gpstFmUiData->FmUiAskQueue);
    rkos_queue_delete(gpstFmUiData->FmUiRespQueue);
    rkos_memory_free(gpstFmUiData);
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_MUSIC_PLAY_MENU_TASK);
#endif
    gpstFmUiData = NULL;
    rk_printf("FmUiTask_DeInit over....\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmUiTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 14:08:43
*******************************************************************************/
_APP_FM_FMUITASK_INIT_
INIT API rk_err_t FmUiTask_Init(void *pvParameters, void *arg)
{

    RK_TASK_CLASS*   pFmUiTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_FM_ARG * pArg = (RK_TASK_FM_ARG *)arg;
    FMUI_TASK_DATA_BLOCK*  pFmUiTaskData;
    if (pFmUiTask == NULL)
    {
        rk_printf("pFmUiTask NULL\n");
        return RK_PARA_ERR;
    }

    if(pArg == NULL)
    {
        rk_printf("pArg NULL\n");
        return RK_PARA_ERR;
    }

    rk_printf("FmUiTask_Init\n");

    pFmUiTaskData = rkos_memory_malloc(sizeof(FMUI_TASK_DATA_BLOCK));
    if(pFmUiTaskData == NULL)
    {
        rk_printf("FmUiTask_Init malloc fail....\n");
    }
    memset(pFmUiTaskData, NULL, sizeof(FMUI_TASK_DATA_BLOCK));

    pFmUiTaskData->stAudioInfo.FMInputType = pArg->source;
    pFmUiTaskData->FmUiAskQueue = rkos_queue_create(2, sizeof(FMUI_ASK_QUEUE));
    pFmUiTaskData->FmUiRespQueue = rkos_queue_create(1, sizeof(FMUI_RESP_QUEUE));

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_FMUI_TASK, SEGMENT_OVERLAY_ALL);
#endif

#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(FMUITask_GuiCallBack);
#else

#endif
    gpstFmUiData = pFmUiTaskData;
    gpstFmUiData->GetCurTime = 0;
    gpstFmUiData->QueueFull = 0;
    gpstFmUiData->FMPlayerMod = MOD_MANUAL;
    rk_printf("FmUiTask_Init OVER\n");
    return RK_SUCCESS;

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
