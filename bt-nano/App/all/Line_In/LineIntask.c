/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Line_In\LineIntask.c
* Owner: cjh
* Date: 2016.5.11
* Time: 9:02:05
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.11     9:02:05   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_LINE_IN_LINEINTASK_C__

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
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    LINEIN_TYPE_KEY = 0,
    LINEIN_TYPE_LINEIN,
    LINEIN_TYPE_RECORD,
    LINEIN_TYPE_MSGBOX,
} eRECORD_TYPE_CMD;

typedef enum
{
    LINE_IN_PLAY_MOD = 0,
    LINE_IN_RECORD_MOD,
} eLINE_IN_CMD;


typedef  struct _LINEIN_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}LINEIN_RESP_QUEUE;

typedef  struct _LINEIN_ASK_QUEUE
{
    uint32 cmd;
    uint32 type;

}LINEIN_ASK_QUEUE;

typedef  struct _LINEIN_TASK_DATA_BLOCK
{
    pQueue  LineInAskQueue;
    HGC hBackdrop;
    HGC hPlayerState;
    HGC hProgressBar;
    HGC hVolumeBar;
    HGC hVolumeValue;
    HGC hLineInTotalTimer;
    HGC hLineInProgressTimer;
    HGC hEQMode;
    HGC hTitle;
    HGC hMsgBox;
    HGC hSpecrm;
    HGC hFileName;
    LINEIN_AUDIO_INFO LineInInf;
    RECORD_INFO RecordInf;
    int16  RecordState;
    uint32 PlayerState;
    uint32 CurMode;
    uint32 LineInTotalTimer;
    uint32 LineInProgressTimer;
    uint32 GetCurTime;
    LINEIN_APP_CALLBACK * pLineInStateForAPP;
    uint8  QueueFull;
}LINEIN_TASK_DATA_BLOCK;

#define MaximumVolume                   30

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static LINEIN_TASK_DATA_BLOCK * gpstLineInData;



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
rk_err_t LineInTask_AudioEventProcess(uint32 AudioState);
rk_err_t LineInTask_DeleteGuiHandle(void);
rk_err_t LineInTask_RecordEvent(uint32 State);
void LineIn_PlayerDisplayAll(void);
rk_err_t LineIn_StartLineInControl(void);
rk_err_t LineIntask_StartRecord(void);
rk_err_t LineInTask_AudioEventCallBack(uint32 AudioState);
rk_err_t LineIn_RecordFileNameDisplay(void);
rk_err_t LineIn_PlayerSetPopupWindow(void * text, uint16 text_cmd);
void LineIn_PlayerMusicEqDisplay(void);
void LineIn_PlayerVolumeDisplay(void);
void LineIn_PlayerProcessDisplay(void);
void LineIn_StateDisplay(void);
void LineIn_PlayerSpectrumDisplay(void);
void LineIn_PlayerTitle(void *text, uint32 len, eTEXT_DISPLAY_CMD cmd);
rk_err_t LineInTask_GuiInit(void);
rk_err_t LineInTask_KeyEvent(uint32 KeyVal);
rk_err_t LineInTask_RecordCallBack(uint32 RecordState);
rk_err_t LineInTask_ButtonCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc);
rk_err_t LineIn_PlayerDisplayMsg(uint32 Sid);
rk_err_t LineIn_PlayerBackGroundDisplay(void);
rk_err_t LineInTask_PlayerOpen(void);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: LineInTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:29:42
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON API rk_err_t LineInTask_Resume(uint32 ObjectID)
{

}

/*******************************************************************************
** Name: LineInTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:29:42
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON API rk_err_t LineInTask_Suspend(uint32 ObjectID)
{

}

/*******************************************************************************
** Name: LineInTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 9:29:42
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON API void LineInTask_Enter(void * arg)
{
    LINEIN_ASK_QUEUE LineInAskQueue;
#ifdef _USE_GUI_
    LineInTask_PlayerOpen();
    LineIn_StartLineInControl();
    LineInTask_GuiInit();
    LineInControlTask_SetSpectrumEn(1);
#endif

    MainTask_SetStatus(MAINTASK_APP_LINE_IN,1);

    while(1)
    {
        rkos_queue_receive(gpstLineInData->LineInAskQueue, &LineInAskQueue, MAX_DELAY);

        switch(LineInAskQueue.type)
        {
            case LINEIN_TYPE_KEY:
                LineInTask_KeyEvent(LineInAskQueue.cmd);
                gpstLineInData->QueueFull = 0;
                break;

            case LINEIN_TYPE_LINEIN:
                LineInTask_AudioEventProcess(LineInAskQueue.cmd);
                break;

            case LINEIN_TYPE_RECORD:
                LineInTask_RecordEvent(LineInAskQueue.cmd);
                break;

            default :
                break;
        }
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
** Name: LineInTask_AudioEventProcess
** Input:uint32 AudioState
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.16
** Time: 11:29:20
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_AudioEventProcess(uint32 AudioState)
{
    switch (AudioState)
    {
#ifdef _RK_SPECTRUM_
        case LINEIN_PLAYER_STATE_UPDATA_SPECTRUM:
            //printf("SPECTRUM\n");
            gpstLineInData->GetCurTime += 100;
            if((gpstLineInData->GetCurTime % 1000) == 0)
            {
                LineIn_PlayerProcessDisplay();
            }
            LineIn_PlayerSpectrumDisplay();
            //printf("SPECTRUM over\n");
            break;
#endif
        case AUDIO_STATE_ERROR:
            if(gpstLineInData->LineInInf.playerr)
            {
                LineIn_PlayerSetPopupWindow((void *)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
            }
            break;

        default:
            break;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineInTask_DeleteGuiHandle
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 16:19:58
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_DeleteGuiHandle(void)
{
#ifdef _USE_GUI_
    if (gpstLineInData->hFileName!= NULL)
    {
        GuiTask_OperWidget(gpstLineInData->hFileName, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstLineInData->hFileName);
        gpstLineInData->hFileName = NULL;
    }

    if (gpstLineInData->hBackdrop = NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hBackdrop);
        gpstLineInData->hBackdrop = NULL;
    }

    if (gpstLineInData->hEQMode != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hEQMode);
        gpstLineInData->hEQMode = NULL;
    }

    if (gpstLineInData->hFileName != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hFileName);
        gpstLineInData->hFileName = NULL;
    }

    if (gpstLineInData->hLineInProgressTimer != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hLineInProgressTimer);
        gpstLineInData->hLineInProgressTimer = NULL;
    }

    if (gpstLineInData->hLineInTotalTimer != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hLineInTotalTimer);
        gpstLineInData->hLineInTotalTimer = NULL;
    }

    if (gpstLineInData->hMsgBox != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hMsgBox);
        gpstLineInData->hMsgBox = NULL;
    }

    if (gpstLineInData->hPlayerState != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hPlayerState);
        gpstLineInData->hPlayerState = NULL;
    }

    if (gpstLineInData->hProgressBar != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hProgressBar);
        gpstLineInData->hProgressBar = NULL;
    }

    if (gpstLineInData->hSpecrm != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hSpecrm);
        gpstLineInData->hSpecrm = NULL;
    }

    if (gpstLineInData->hTitle != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hTitle);
        gpstLineInData->hTitle = NULL;
    }

    if (gpstLineInData->hVolumeBar != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hVolumeBar);
        gpstLineInData->hVolumeBar = NULL;
    }

    if (gpstLineInData->hVolumeValue != NULL)
    {
        GuiTask_DeleteWidget(gpstLineInData->hVolumeValue);
        gpstLineInData->hVolumeValue = NULL;
    }
#endif
}
/*******************************************************************************
** Name: LineInTask_RecordEvent
** Input:uint32 State
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 15:32:37
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_RecordEvent(uint32 State)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineIn_PlayerDisplayAll
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 15:22:55
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_PlayerDisplayAll(void)
{
#ifdef _USE_GUI_
    GuiTask_ScreenLock();
    //rk_printf("LineIn_PlayerDisplayAll...\n");
    //LINEINPlay_PlayOrderDisplay();

    LineIn_PlayerTitle((void *)SID_LINE_IN, 0, TEXT_CMD_ID);
    LineIn_PlayerProcessDisplay();
    gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_PLAY;
    LineIn_StateDisplay();

    if(gpstLineInData->LineInInf.playerr)
    {
        LineIn_PlayerSetPopupWindow((void *)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
    }

    GuiTask_ScreenUnLock();
#endif
}
/*******************************************************************************
** Name: LineIn_StartLineInControl
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 15:11:47
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineIn_StartLineInControl(void)
{
#ifdef _USE_GUI_
    rk_err_t ret;
    RK_TASK_LINEINCONTROL_ARG lineInArg;
    LINEIN_AUDIO_INFO AudioInfo;

    if(RKTaskFind(TASK_ID_LINEINCONTROL, 0) != NULL)
    {
        LineIn_PlayerBackGroundDisplay();
        LineIn_PlayerDisplayMsg(SID_PLAYER_FAILED);
        return RK_ERROR;
    }
    else
    {
#ifdef __APP_LINE_IN_LINEINCONTROLTASK_C__
        rk_printf("start_LINEINCONTROL \n");
        lineInArg.LRChannel = 2;
        lineInArg.LineInInputType = gpstLineInData->LineInInf.InputType;//RECORD_TYPE_LINEIN2
        lineInArg.SamplesPerBlock = 4096;
        lineInArg.bits = 16;
        lineInArg.Samples = 44100;

        lineInArg.pLineInStateCallBcakForAPP = LineInTask_AudioEventCallBack;
        ret = RKTaskCreate(TASK_ID_LINEINCONTROL, 0, &lineInArg, SYNC_MODE);
        if(ret != RK_SUCCESS)
        {
            LineIn_PlayerBackGroundDisplay();
            LineIn_PlayerDisplayMsg(SID_PLAYER_FAILED);
            rk_printf("TASK_ID_LINEINCONTROL ERROR ret = %d", ret);
            return RK_ERROR;
        }
#endif
        return RK_SUCCESS;
    }

#else //无屏幕操作

#endif
}

/*******************************************************************************
** Name: LineIntask_StartRecord
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 14:33:02
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineIntask_StartRecord(void)
{
#ifdef __APP_RECORD_RECORDCONTROL_C__
    RK_TASK_RECORDCONTROL_ARG Arg;
    Arg.RecordEncodeType     = RECORD_ENCODE_TYPE_WAV;
    Arg.RecordType          = RECORD_TYPE_MIC_STERO;
    Arg.RecordEncodeSubType   = 0;
    Arg.RecordSampleRate    = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
    Arg.RecordChannel       = RECORD_CHANNEL_STERO;
    Arg.RecordDataWidth     = RECORD_DATAWIDTH_16BIT;
    Arg.RecordSource        = 0;
    Arg.pfRecordState = LineInTask_RecordCallBack;

    if(RKTaskCreate(TASK_ID_RECORDCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("Record control task create failure");
    }

    while(Recorder_GetRecordInf(&gpstLineInData->RecordInf) != RECORD_STATE_PREPARE)
    {
        rkos_sleep(10);
    }
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineInTask_AudioEventCallBack
** Input:uint32 AudioState
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 14:10:33
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_AudioEventCallBack(uint32 AudioState)
{
    rk_err_t ret = 0;
    LINEIN_ASK_QUEUE LineInAskQueue;
    LINEIN_ASK_QUEUE LineInAskQueue_tmp;

    LineInAskQueue.type = LINEIN_TYPE_LINEIN;
    LineInAskQueue.cmd = AudioState;
    ret = rkos_queue_send(gpstLineInData->LineInAskQueue, &LineInAskQueue, 0);//MAX_DELAY
    if(ret == RK_ERROR)
    {
       //rk_printf("AudioState send fail...\n");
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineIn_RecordFileNameDisplay
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 11:21:24
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineIn_RecordFileNameDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    RKGUI_TEXT_ARG pstTextArg;
    Ucs2 FileName[30];

    for(i= 0; i< 12; i++)
    {
        FileName[i] = (Ucs2)gpstLineInData->RecordInf.FileName[i];
    }
    FileName[i] = 0;
    pstTextArg.text= FileName;

    if(gpstLineInData->hFileName == NULL)
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
        pstTextArg.x= 25;
        pstTextArg.y= 125;
        pstTextArg.xSize= -1;//45
        pstTextArg.ySize= -1;
        pstTextArg.BackdropX= 25;
        pstTextArg.BackdropY= 125;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstLineInData->hFileName= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        pstTextArg.opercmd= TEXT_SET_CONTENT;
        GuiTask_OperWidget(gpstLineInData->hFileName, OPERATE_SET_CONTENT, &pstTextArg, SYNC_MODE);
    }
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineIn_PlayerSetPopupWindow
** Input:void * text, uint16 text_cmd
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 11:17:44
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineIn_PlayerSetPopupWindow(void * text, uint16 text_cmd)
{
#ifdef _USE_GUI_

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

    if(gpstLineInData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstLineInData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstLineInData->hMsgBox);
        gpstLineInData->hMsgBox = NULL;
    }

    if(gpstLineInData->hMsgBox == NULL)
    {
        gpstLineInData->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
#endif
    return RK_SUCCESS;
}

#ifdef _RK_EQ_
/*******************************************************************************
** Name: LineIn_PlayerMusicEqDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 11:16:04
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_PlayerMusicEqDisplay(void)
{
#ifdef _USE_GUI_
    uint32 i;
    uint32 SidEq[9] = {SID_EQ_HEAVY,SID_EQ_POP,SID_EQ_JAZZ, SID_EQ_UNIQUE, SID_EQ_SELECT, SID_EQ_BASS, 0, SID_EQ_CUSTOM, SID_EQ_NONE};

    RKGUI_TEXT_ARG LineInplayerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    if(gpstLineInData->hEQMode == NULL)
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
        pstTextArg.resource = SidEq[gpstLineInData->LineInInf.EqMode - EQ_HEAVY];
        pstTextArg.align= TEXT_ALIGN_X_RIGHT;
        gpstLineInData->hEQMode = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        LineInplayerTextContentArg.opercmd = TEXT_SET_CONTENT;
        LineInplayerTextContentArg.resource = SidEq[gpstLineInData->LineInInf.EqMode - EQ_HEAVY];
        GuiTask_OperWidget(gpstLineInData->hEQMode, OPERATE_SET_CONTENT, &LineInplayerTextContentArg, SYNC_MODE);
    }
#endif
}
#endif

/*******************************************************************************
** Name: LineIn_PlayerVolumeDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 11:13:51
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_PlayerVolumeDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    char buf[3];
    Ucs2 VolumeValue[3];

    RKGUI_TEXT_ARG LineInplayerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    RKGUI_TEXT_ARG pstTextArg;
    RKGUI_PROGRESSBAR_ARG pstProgressArg;

    sprintf(buf, "%02d", gpstLineInData->LineInInf.PlayVolume);
    for (i= 0; i< sizeof(buf); i++)
    {
        VolumeValue[i]= (Ucs2)buf[i];
    }
    //printf ("Volume buf=%s Volume=%d i =%d\n",buf, gpstLineInData->LineInInf.PlayVolume, i);
    VolumeValue[2] = 0;

    if(gpstLineInData->hVolumeValue == NULL)
    {
        //printf ("hVolumeValue == NULL\n");
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
        gpstLineInData->hVolumeValue = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        //printf ("hVolumeValue != NULL\n");
        LineInplayerTextContentArg.opercmd = TEXT_SET_CONTENT;
        LineInplayerTextContentArg.text = VolumeValue;
        GuiTask_OperWidget(gpstLineInData->hVolumeValue, OPERATE_SET_CONTENT, &LineInplayerTextContentArg, SYNC_MODE);
    }

    if(gpstLineInData->hVolumeBar == NULL)
    {
        pstProgressArg.x= 70;
        pstProgressArg.y= 147;
        pstProgressArg.display = 1;
        pstProgressArg.level= 0;

        pstProgressArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstProgressArg.BackdropX= 70;
        pstProgressArg.BackdropY= 147;

        pstProgressArg.Bar= IMG_ID_VOLUME_BAR;
        pstProgressArg.percentage= gpstLineInData->LineInInf.PlayVolume* 100/ MaximumVolume;
        gpstLineInData->hVolumeBar = GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        pstProgressBarContent.cmd= PROGRESSBAR_SET_CONTENT;
        pstProgressBarContent.percentage = gpstLineInData->LineInInf.PlayVolume* 100/ MaximumVolume;
        //PLAYMENU_DEBUG("percentage=%d\n",pstProgressBarContent.percentage);
        GuiTask_OperWidget(gpstLineInData->hVolumeBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: LineIn_PlayerProcessDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 11:10:46
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_PlayerProcessDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    char buf[9];
    Ucs2 Time[9];
    RKGUI_PROGRESSBAR_ARG pstProgressArg;
    RKGUI_TEXT_ARG pstTextArg;

    uint8 Sec, Min, Hour;

    #if 1
    RKGUI_TEXT_ARG LineInplayerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    #endif

    gpstLineInData->LineInProgressTimer = gpstLineInData->GetCurTime;
    gpstLineInData->LineInTotalTimer = 59*3600000 + 59*60000 + 59*1000;

    Sec = (gpstLineInData->LineInTotalTimer / 1000) % 60;
    Min = (gpstLineInData->LineInTotalTimer / 60000) % 60;
    Hour = gpstLineInData->LineInTotalTimer / 3600000;

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

    if(gpstLineInData->hLineInTotalTimer == NULL)
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
        gpstLineInData->hLineInTotalTimer= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        LineInplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
        LineInplayerTextContentArg.text = Time;

        GuiTask_OperWidget(gpstLineInData->hLineInTotalTimer, OPERATE_SET_CONTENT, &LineInplayerTextContentArg, SYNC_MODE);
    }

    Sec = (gpstLineInData->LineInProgressTimer / 1000) % 60;
    Min = (gpstLineInData->LineInProgressTimer / 60000) % 60;
    Hour = gpstLineInData->LineInProgressTimer / 3600000;

    sprintf(buf, "%02d:%02d:%02d", Hour, Min, Sec);
    //printf ("CurrentTime=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        Time[i]= (Ucs2)buf[i];
    }

    Time[i] = 0;


    if(gpstLineInData->hLineInProgressTimer == NULL)
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
        gpstLineInData->hLineInProgressTimer= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        LineInplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
        LineInplayerTextContentArg.text = Time;
        GuiTask_OperWidget(gpstLineInData->hLineInProgressTimer, OPERATE_SET_CONTENT, &LineInplayerTextContentArg, SYNC_MODE);
    }

    if (gpstLineInData->LineInTotalTimer== 0)return;

    pstProgressBarContent.cmd = PROGRESSBAR_SET_CONTENT;
    pstProgressBarContent.percentage = gpstLineInData->LineInProgressTimer * 100 / gpstLineInData->LineInTotalTimer;

    if(gpstLineInData->hProgressBar == NULL)
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
        gpstLineInData->hProgressBar= GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        GuiTask_OperWidget(gpstLineInData->hProgressBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: LineIn_StateDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 11:06:19
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_StateDisplay(void)
{
#ifdef _USE_GUI_
    uint32 PlayerState;
    RKGUI_ICON_ARG pstIconArg;

    if (gpstLineInData->PlayerState == LINEIN_PLAYER_STATE_PAUSE)
    {
        PlayerState = IMG_ID_MUSIC_PAUSE_STATE;
    }
    else if(gpstLineInData->PlayerState == LINEIN_PLAYER_STATE_PLAY)
    {
        PlayerState = IMG_ID_MUSIC_PLAY_STATE;
    }
    else if(gpstLineInData->PlayerState == LINEIN_PLAYER_STATE_RECORDING)
    {
        PlayerState = IMG_ID_RECORDING;
    }
    else if(gpstLineInData->PlayerState == LINEIN_PLAYER_STATE_PAUSE_RECORD)
    {
        PlayerState = IMG_ID_RECORD_PAUSE;
    }

    if(gpstLineInData->hPlayerState == NULL)
    {
        pstIconArg.resource = PlayerState;//IMG_ID_MUSIC_PLAY_STATE
        pstIconArg.x= 45;
        pstIconArg.y= 141;
        pstIconArg.level = 0;
        pstIconArg.display= 1;
        gpstLineInData->hPlayerState= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
    }
    else
    {
        GuiTask_OperWidget(gpstLineInData->hPlayerState, OPERATE_SET_CONTENT, (void *)PlayerState, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: LineIn_PlayerSpectrumDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 11:04:32
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_PlayerSpectrumDisplay(void)
{
#ifdef _USE_GUI_
    RKGUI_SPECTRUM_ARG pstSpectrumArg;
    RKGUI_SPECTRUM_ARG pstSpectrumContent;

    uint8 buf[12]= {0,0,0,0,0,0,0,0,0,0,0,0};

    if(gpstLineInData->hSpecrm == NULL)
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
        gpstLineInData->hSpecrm = GUITask_CreateWidget(GUI_CLASS_SPECTRUM, &pstSpectrumArg);
    }
    else
    {
        pstSpectrumContent.cmd= SPECTRUM_SET_CONTENT;
        LineInControlTask_GetCurSpectrum(&pstSpectrumContent.StripDat);
        GuiTask_OperWidget(gpstLineInData->hSpecrm, OPERATE_SET_CONTENT, &pstSpectrumContent, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: LineIn_PlayerTitle
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 10:53:37
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN void LineIn_PlayerTitle(void *text, uint32 len, eTEXT_DISPLAY_CMD cmd)
{
#ifdef _USE_GUI_
    LINEIN_AUDIO_INFO LineInAudioInfo;
    Ucs2 LineInTitle[9];
    char buf[9];
    uint8 i=0;

    RKGUI_TEXT_ARG LineInplayerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    LineInControlTask_GetAudioInfo(&LineInAudioInfo);

    if(cmd == TEXT_CMD_ID)
    {
        pstTextArg.resource = (int16)text;
        //printf("resource=%d",pstTextArg.resource);
    }
    else
    {
        sprintf(buf, "%s",text);

        for (i= 0; i< sizeof(buf); i++)
        {
            LineInTitle[i]= (Ucs2)buf[i];
        }
        LineInTitle[len] = 0;
        LineInplayerTextContentArg.text = LineInTitle;
    }
    if (gpstLineInData->hTitle != NULL)
    {
        //printf("hTitle == NULL");
        GuiTask_DeleteWidget(gpstLineInData->hTitle);
        gpstLineInData->hTitle = NULL;
    }
    if (gpstLineInData->hTitle == NULL)
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
        pstTextArg.x= 30;
        pstTextArg.y= 109;//102
        pstTextArg.xSize= 80;
        pstTextArg.ySize= -1;
        pstTextArg.BackdropX= 30;
        pstTextArg.BackdropY= 109;//102
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstLineInData->hTitle = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
        //GuiTask_OperWidget(gpstLineInData->hTitle, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);

    }
    //else
    //{
    //    LineInplayerTextContentArg.opercmd= TEXT_SET_CONTENT;
    //    GuiTask_OperWidget(gpstLineInData->hTitle, OPERATE_SET_CONTENT, &LineInplayerTextContentArg, SYNC_MODE);
    //}
#endif
}

/*******************************************************************************
** Name: LineInTask_GuiInit
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 10:49:35
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_GuiInit(void)
{
#ifdef _USE_GUI_
    gpstLineInData->PlayerState = LineInControlTask_GetAudioInfo(&gpstLineInData->LineInInf);
    GuiTask_ScreenLock();
    LineIn_PlayerBackGroundDisplay();
    LineIn_PlayerVolumeDisplay();
    #ifdef _RK_EQ_
    LineIn_PlayerMusicEqDisplay();
    #endif
    LineIn_PlayerDisplayAll();
    if(gpstLineInData->LineInInf.playerr)
    {
        LineIn_PlayerSetPopupWindow((void *)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
    }

    GuiTask_ScreenUnLock();
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineInTask_KeyEvent
** Input:uint32 KeyVal
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 10:40:17
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_KeyEvent(uint32 KeyVal)
{
    if(gpstLineInData->hMsgBox != NULL)
    {
        return RK_SUCCESS;
    }
    else
    {
        if(gpstLineInData->CurMode == LINE_IN_RECORD_MOD)
        {
            switch (KeyVal)
            {
                case KEY_VAL_PLAY_SHORT_UP:
                    //int16 resource;
                    if(gpstLineInData->RecordState == RECORD_STATE_BEING)
                    {
                        #ifdef _RECORD_
                        RecordControlTask_SendCmd(RECORD_CMD_PAUSE, NULL, SYNC_MODE);
                        #endif
                        gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_PAUSE_RECORD;
                    }
                    else if(gpstLineInData->RecordState == RECORD_STATE_PAUSE)
                    {
                        #ifdef _RECORD_
                        RecordControlTask_SendCmd(RECORD_CMD_RESUME, NULL, SYNC_MODE);
                        #endif
                        gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_RECORDING;
                    }
                    //printf("LineIn_PlayerTitle= %d RecordState=%d\n",gpstLineInData->PlayerState ,gpstLineInData->RecordState);
                    #ifdef _USE_GUI_
                    //LineIn_PlayerTitle((void *)SID_LINE_IN_REC, 0, TEXT_CMD_ID);
                    LineIn_StateDisplay();
                    //GuiTask_OperWidget(gpstRecodData->hStatus, OPERATE_SET_CONTENT, &TextContentArg, SYNC_MODE);
                    #endif
                    break;

                case KEY_VAL_ESC_SHORT_UP:
                    #ifdef __APP_RECORD_RECORDCONTROL_C__
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                    {
                        LineInControlTask_SendCmd(LINEIN_CMD_RECORD_STOP, (void *)0, SYNC_MODE);
                        #ifdef _RECORD_
                        RecordControlTask_SendCmd(RECORD_CMD_STOP, (void *)1, SYNC_MODE);
                        #endif
                        gpstLineInData->CurMode = LINE_IN_PLAY_MOD;
                        #ifdef _USE_GUI_
                        if (gpstLineInData->hFileName!= NULL)
                        {
                            GuiTask_OperWidget(gpstLineInData->hFileName, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstLineInData->hFileName);
                            gpstLineInData->hFileName = NULL;
                        }
                        #endif
                        gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_PLAY;
                        LineIn_StateDisplay();
                    }
                    #endif
                    break;

                case KEY_VAL_PLAY_PRESS_START://关机
                    #ifdef __APP_RECORD_RECORDCONTROL_C__
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                    {
                        LineInControlTask_SendCmd(LINEIN_CMD_RECORD_STOP, (void *)0, SYNC_MODE);
                        if(RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE)!= RK_SUCCESS)
                        {
                            rk_printf("delete TASK_ID_RECORDCONTROL fault\n");
                        }
                    }
                    #endif
                    #ifdef __APP_LINE_IN_LINEINTASK_C__
                    LineInControlTask_SendCmd(LINEIN_CMD_DEINIT, (void *)0, SYNC_MODE);
                    #endif

                    MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                    break;

                case KEY_VAL_MENU_SHORT_UP: //Start and Stop
                    //FMControlTask_SendCmd(LINEIN_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
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
                    if(gpstLineInData->PlayerState != LINEIN_PLAYER_STATE_PLAY)
                    {
                        LineInControlTask_SendCmd(LINEIN_CMD_PAUSE_RESUME, (void *)0, SYNC_MODE);
                        gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_PLAY;
                    }
                    else
                    {
                        LineInControlTask_SendCmd(LINEIN_CMD_PAUSE_RESUME, (void *)1, SYNC_MODE);
                        gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_PAUSE;
                    }
                    LineIn_StateDisplay();
                    break;

                case KEY_VAL_ESC_SHORT_UP:
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                    {
                        RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE);
                    }

#ifdef _USE_GUI_
#ifdef __APP_LINE_IN_LINEINTASK_C__
                    LineInControlTask_SendCmd(LINEIN_CMD_DEINIT, (void *)0, SYNC_MODE);
                    MainTask_TaskSwtich(TASK_ID_LINEIN, 0,  TASK_ID_MAIN, 0,  NULL);
#endif
                    while(1)
                    {
                        rkos_sleep(2000);
                        rk_printf("TASK_ID_LINEIN\n");
                    }

#endif


                    break;

                case KEY_VAL_PLAY_PRESS_START://关机
                    #ifdef _USE_GUI_
                    #ifdef __APP_RECORD_RECORDCONTROL_C__
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) != NULL)
                    {
                        if(RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE)!= RK_SUCCESS)
                        {
                            rk_printf("delete TASK_ID_RECORDCONTROL fault\n");
                        }
                    }
                    #endif

                    #ifdef __APP_LINE_IN_LINEINTASK_C__
                    LineInControlTask_SendCmd(LINEIN_CMD_DEINIT, (void *)0, SYNC_MODE);
                    #endif
                    #endif
                    MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                    break;

                case KEY_VAL_UP_PRESS:
                case KEY_VAL_UP_SHORT_UP: //semi-automatic search +
                    if(gpstLineInData->LineInInf.PlayVolume >= MaximumVolume)
                    {
                        break;
                    }
                    gpstLineInData->LineInInf.PlayVolume++;

                    if(LineInControlTask_SendCmd(LINEIN_CMD_VOLUMESET, (void *)gpstLineInData->LineInInf.PlayVolume, SYNC_MODE) == RK_SUCCESS)
                    {
                        LineIn_PlayerVolumeDisplay();
                    }
                    break;

                case KEY_VAL_DOWN_PRESS:
                case KEY_VAL_DOWN_SHORT_UP: //semi-automatic search -
                    if(gpstLineInData->LineInInf.PlayVolume == 0)
                    {
                        break;
                    }
                    gpstLineInData->LineInInf.PlayVolume--;

                    if(LineInControlTask_SendCmd(LINEIN_CMD_VOLUMESET, (void *)gpstLineInData->LineInInf.PlayVolume, SYNC_MODE) == RK_SUCCESS)
                    {
                        LineIn_PlayerVolumeDisplay();
                    }
                    break;

                case KEY_VAL_MENU_SHORT_UP: //Start and Stop
                    if(RKTaskFind(TASK_ID_RECORDCONTROL, 0) == NULL)
                    {
                        if(gpstLineInData->PlayerState == LINEIN_PLAYER_STATE_PLAY)
                        {
                            #ifdef _RECORD_
                            RK_TASK_RECORDCONTROL_ARG Arg;

                            Arg.RecordEncodeType     = RECORD_ENCODE_TYPE_WAV;
                            Arg.RecordType          = RECORD_TYPE_LINEIN2;
                            Arg.RecordEncodeSubType   = 0;
                            Arg.RecordSampleRate    = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
                            Arg.RecordChannel       = RECORD_CHANNEL_STERO;
                            Arg.RecordDataWidth     = RECORD_DATAWIDTH_16BIT;
                            Arg.RecordSource        = 2;
                            Arg.pfRecordState = LineInTask_RecordCallBack;

                            if(RKTaskCreate(TASK_ID_RECORDCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
                            {
                                 rk_printf("Record control task create failure");
                            }

                            while(Recorder_GetRecordInf(&gpstLineInData->RecordInf) != RECORD_STATE_PREPARE)
                            {
                                rkos_sleep(10);
                            }

                            #ifdef _RECORD_
                            RecordControlTask_SendCmd(RECORD_CMD_START, (void *)1, SYNC_MODE);
                            #endif

                            LineInControlTask_SendCmd(LINEIN_CMD_RECORD, (void *)0, SYNC_MODE);
                            LineIn_RecordFileNameDisplay();
                            gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_RECORDING;
                            gpstLineInData->RecordState == RECORD_STATE_BEING;
                            LineIn_StateDisplay();
                            gpstLineInData->CurMode = LINE_IN_RECORD_MOD;
                            #endif

                        }
                    }
                    else
                    {
                        if(gpstLineInData->PlayerState == LINEIN_PLAYER_STATE_PLAY)
                        {
                            #ifdef _RECORD_
                            while(Recorder_GetRecordInf(&gpstLineInData->RecordInf) != RECORD_STATE_PREPARE)
                            {
                                rkos_sleep(10);
                            }
                            RecordControlTask_SendCmd(RECORD_CMD_START, (void *)1, SYNC_MODE);

                            LineInControlTask_SendCmd(LINEIN_CMD_RECORD, (void *)0, SYNC_MODE);
                            LineIn_RecordFileNameDisplay();
                            gpstLineInData->PlayerState = LINEIN_PLAYER_STATE_RECORDING;
                            gpstLineInData->RecordState == RECORD_STATE_BEING;
                            LineIn_StateDisplay();
                            gpstLineInData->CurMode = LINE_IN_RECORD_MOD;
                            #endif
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineInTask_RecordCallBack
** Input:uint32 RecordState
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 10:33:25
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_RecordCallBack(uint32 RecordState)
{
    LINEIN_ASK_QUEUE LineInAskQueue;

    switch( RecordState )
    {
        case RECORD_STATE_PREPARE:
            gpstLineInData->RecordState = RECORD_STATE_PREPARE;
            break;

        case RECORD_STATE_BEING:
            gpstLineInData->RecordState = RECORD_STATE_BEING;
            break;

        case RECORD_STATE_PAUSE:
            gpstLineInData->RecordState = RECORD_STATE_PAUSE;
            break;

        case RECORD_STATE_STOP:
            break;

        case RECORD_STATE_TIME_CHANGE:
            break;

        default:
            break;
    }
    LineInAskQueue.type = LINEIN_TYPE_RECORD;
    LineInAskQueue.cmd  = RecordState;
    rkos_queue_send(gpstLineInData->LineInAskQueue, &LineInAskQueue, 0);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineInTask_ButtonCallBack
** Input:APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 10:14:04
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    LINEIN_ASK_QUEUE LineInAskQueue;
    LINEIN_ASK_QUEUE LineInAskQueue_tmp;
    if(((event & KEY_VAL_FFW) == KEY_VAL_FFW) || ((event & KEY_VAL_FFD) == KEY_VAL_FFD))
    {
        return RK_SUCCESS;
    }

    if ((gpstLineInData->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP))
    {
        rk_printf("LineIn lose key QueueFull=%d\n",gpstLineInData->QueueFull);
        return RK_SUCCESS;
    }
    switch(event_type)
    {
        case APP_RECIVE_MSG_EVENT_KEY:
        {
            gpstLineInData->QueueFull = 1;
            LineInAskQueue.cmd = event;
            LineInAskQueue.type = LINEIN_TYPE_KEY;
            ret = rkos_queue_send(gpstLineInData->LineInAskQueue, &LineInAskQueue, 0);
            if(ret == RK_ERROR)
            {
                gpstLineInData->QueueFull = 0;
                rk_printf("send key fail");
            }
        }
        break;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineIn_PlayerDisplayMsg
** Input:uint32 Sid
** Return: rk_er_t
** Owner:cjh
** Date: 2016.5.11
** Time: 10:07:38
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineIn_PlayerDisplayMsg(uint32 Sid)
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
    gpstLineInData->hTitle = GUITask_CreateWidget(GUI_CLASS_TEXT, &TextArg);
#endif
}

/*******************************************************************************
** Name: LineIn_PlayerBackGroundDisplay
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 10:06:49
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineIn_PlayerBackGroundDisplay(void)
{
#ifdef _USE_GUI_
    RKGUI_ICON_ARG pstIconArg;

    pstIconArg.resource= IMG_ID_MUSIC_BACKGROUND;
    pstIconArg.x= 0;
    pstIconArg.y= 0;
    pstIconArg.level = 0;
    pstIconArg.display= 1;
    gpstLineInData->hBackdrop = GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
#endif
}

/*******************************************************************************
** Name: LineInTask_PlayerOpen
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 10:03:56
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON FUN rk_err_t LineInTask_PlayerOpen(void)
{
#ifdef _USE_GUI_
    LINEIN_ASK_QUEUE LineInAskQueue;
    if(MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)
        || MainTask_GetStatus(MAINTASK_APP_BT_PLAYER)
        || MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)
        || MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER))
    {
        LineIn_PlayerBackGroundDisplay();
        LineIn_PlayerDisplayMsg(SID_PLAYER_FAILED);
        while (1)
        {
            rkos_queue_receive(gpstLineInData->LineInAskQueue, &LineInAskQueue, MAX_DELAY);
            switch (LineInAskQueue.cmd)
            {
                case KEY_VAL_ESC_SHORT_UP:
                    MainTask_TaskSwtich(TASK_ID_LINEIN, 0, TASK_ID_MAIN, 0, NULL);
                    while(1)
                    {
                        rkos_sleep(2000);
                    }
                    break;
                default:
                    gpstLineInData->QueueFull = 0;
                    break;
            }
        }
        return RK_ERROR;
    }

#endif
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LineInTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:29:42
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_INIT_
INIT API rk_err_t LineInTask_DeInit(void *pvParameters)
{
    rk_err_t ret;
    rk_printf("LineInTask_DeInit....\n");
    MainTask_SetStatus(MAINTASK_APP_LINE_IN,0);
#ifdef _USE_GUI_
    LineInTask_DeleteGuiHandle();
    GuiTask_AppUnReciveMsg(LineInTask_ButtonCallBack);
#endif
#ifdef __APP_LINE_IN_LINEINCONTROLTASK_C__
    if(RKTaskFind(TASK_ID_LINEINCONTROL, 0) != NULL)
    {
        LineInControlTask_SetStateChangeFunc(LineInTask_AudioEventCallBack, NULL);
        RKTaskDelete(TASK_ID_LINEINCONTROL, 0, ASYNC_MODE);
    }
#endif

    rkos_queue_delete(gpstLineInData->LineInAskQueue);
    rkos_memory_free(gpstLineInData);
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_MUSIC_PLAY_MENU_TASK);
#endif
    gpstLineInData = NULL;
    rk_printf("LineInTask_DeInit over....\n");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LineInTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:29:42
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_INIT_
INIT API rk_err_t LineInTask_Init(void *pvParameters, void *arg)
{
#ifdef _USE_GUI_
    RK_TASK_LINEIN_ARG * pArg = (RK_TASK_LINEIN_ARG *)arg;
    LINEIN_TASK_DATA_BLOCK*  pLineInTaskData;
    if(pArg == NULL)
    {
        rk_printf("pArg NULL\n");
        return RK_PARA_ERR;
    }

    rk_printf("LineInTask_Init source = %D\n",pArg->source);

    pLineInTaskData = rkos_memory_malloc(sizeof(LINEIN_TASK_DATA_BLOCK));
    if(pLineInTaskData == NULL)
    {
        rk_printf("LineInTask_Init malloc fail....\n");
        return RK_ERROR;
    }

    memset(pLineInTaskData, 0, sizeof(pLineInTaskData));

    pLineInTaskData->LineInInf.InputType = pArg->source;

    pLineInTaskData->LineInAskQueue= rkos_queue_create(2, sizeof(LINEIN_ASK_QUEUE));
    if( pLineInTaskData->LineInAskQueue== NULL )
    {
        rkos_memory_free(pLineInTaskData);
        return RK_ERROR;
    }
    gpstLineInData = pLineInTaskData;
    GuiTask_AppReciveMsg(LineInTask_ButtonCallBack);
#endif
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
