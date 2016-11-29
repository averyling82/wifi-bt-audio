/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Record\recod_task.c
* Owner: aaron.sun
* Date: 2016.3.11
* Time: 17:11:22
* Version: 1.0
* Desc: record ui
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.3.11     17:11:22   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_RECORD_RECOD_TASK_C__

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
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"
#include "recod_task.h"
#include "record.h"
#include "record_globals.h"
#include "RecordControlTask.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    RECOD_TYPE_KEY = 0,
    RECORD_TYPE_RECORD,
} eRECOD_TYPE_CMD;

typedef  struct _RECOD_ASK_QUEUE
{
    eRECOD_TYPE_CMD type;
    uint32 cmd;
}RECOD_ASK_QUEUE;

typedef  struct _RECOD_TASK_DATA_BLOCK
{
    pQueue RecodAskQueue;

    HGC hBackdrop;
    HGC hTitle;
    HGC hStatus;
    HGC hFileName;
    HGC hCurrTimer;
    HGC hTotalTimer;
    HGC hBitrate;
    HGC hBit;
    HGC hTimerBar;
    HGC hMode;
    RECORD_INFO RecordInf;
    uint32 playerExist;
    int16 state;
}RECOD_TASK_DATA_BLOCK;

static RECOD_TASK_DATA_BLOCK * gpstRecodData;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t RecodTask_ButtonCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc);
void RecodTask_RecordCallBack(uint32 record_state);

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
void Recorder_NameDisplay(void);
rk_err_t Record_DisplayMsg(uint32 cmd, void* arg);
rk_err_t RecordTask_RecordEventHD(uint32 event);
void Recorder_BitrateDisplay(void);
void Recorder_ProcessDisplay(void);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RecodTask_Enter
** Input:void
** Return: void
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON API void RecodTask_Enter(void)
{
    RECOD_ASK_QUEUE RecodAskQueue;
    printf("gpstRecodData->playerExist = %d\n",gpstRecodData->playerExist );
    {
        if(RKTaskFind(TASK_ID_DLNA, 0) != NULL)
        {
            gpstRecodData->playerExist = 1;
        }
        if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
        {
            gpstRecodData->playerExist = 1;
        }
    }
    if(gpstRecodData->playerExist == 0)
    {
        RK_TASK_RECORDCONTROL_ARG Arg;
        #if 1
            Arg.RecordEncodeType    = RECORD_ENCODE_TYPE_WAV; //RECORD_ENCODE_TYPE_XXX RECORD_ENCODE_TYPE_WAV
            Arg.RecordType          = RECORD_TYPE_MIC1_MONO;
            Arg.RecordEncodeSubType   = RECORD_ADPCM;
            Arg.RecordSampleRate    = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
            Arg.RecordChannel       = RECORD_CHANNEL_MONO;
            Arg.RecordDataWidth     = RECORD_DATAWIDTH_16BIT;
            Arg.RecordSource        = 0;
            Arg.pfRecordState = RecodTask_RecordCallBack;
        #else //AMR
            Arg.RecordEncodeType    = RECORD_ENCODE_TYPE_AMR; //RECORD_ENCODE_TYPE_XXX RECORD_ENCODE_TYPE_WAV
            Arg.RecordType          = RECORD_TYPE_MIC2_MONO;
            Arg.RecordEncodeSubType   = 0;
            Arg.RecordSampleRate    = RECORD_SAMPLE_FREQUENCY_8KHZ;
            Arg.RecordChannel       = RECORD_CHANNEL_MONO;
            Arg.RecordDataWidth     = RECORD_DATAWIDTH_16BIT;
            Arg.RecordSource        = 0;
            Arg.pfRecordState = RecodTask_RecordCallBack;
        #endif
        if(RKTaskCreate(TASK_ID_RECORDCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
        {
             rk_printf("Record control task create failure");
        }

        while(Recorder_GetRecordInf(&gpstRecodData->RecordInf) != RECORD_STATE_PREPARE)
        {
            rkos_sleep(10);
        }
    }

#ifdef _USE_GUI_
    GuiTask_ScreenLock();
    RecodTask_GuiInit();
    if(gpstRecodData->playerExist)
    {
        Record_DisplayMsg(TEXT_CMD_ID, (void *)SID_PLAYER_FAILED);
    }
    GuiTask_ScreenUnLock();
#endif

    while(1)
    {
        rkos_queue_receive(gpstRecodData->RecodAskQueue, &RecodAskQueue, MAX_DELAY);
        if(gpstRecodData->playerExist == 0)
        {
            switch(RecodAskQueue.type)
            {
                case RECOD_TYPE_KEY:
                    RecodTask_KeyEventHD(RecodAskQueue.cmd);
                    break;

                case RECORD_TYPE_RECORD:
                    RecordTask_RecordEventHD(RecodAskQueue.cmd);
                    break;

                default :
                    break;
            }
        }
        else
        {
            if(KEY_VAL_ESC_SHORT_UP == RecodAskQueue.cmd)
            {
                printf("record KEY_VAL_ESC_SHORT_UP...\n");
                RecodTask_GuiDeInit();
                MainTask_TaskSwtich(TASK_ID_RECOD, 0, TASK_ID_MAIN, 0, NULL);
                while(1)
                {
                    rkos_sleep(2000);
                }
                break;
            }
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
** Name: Recorder_NameDisplay
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.6.8
** Time: 14:13:29
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON FUN void Recorder_NameDisplay(void)
{
    int16 i;
    Ucs2 FileName[30];
    RKGUI_TEXT_ARG pstTextArg;

    for(i= 0; i< 12 ; i++)
    {
        FileName[i]= (Ucs2)gpstRecodData->RecordInf.FileName[i];
    }
    FileName[i] = 0;

    #ifdef _USE_GUI_
    if(gpstRecodData->hFileName == NULL)
    {
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;

        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;
        pstTextArg.BackdropX= 0;
        pstTextArg.BackdropY= 20;

        pstTextArg.align= TEXT_ALIGN_X_CENTER;
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 0;
        pstTextArg.y= 90;
        pstTextArg.BackdropX= 0;
        pstTextArg.BackdropY= 90;
        pstTextArg.text= FileName;

        pstTextArg.display = 1;
        pstTextArg.level= 0;

        pstTextArg.xSize= 128;
        pstTextArg.ySize= -1;

        gpstRecodData->hFileName = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        pstTextArg.opercmd = TEXT_SET_CONTENT;
        pstTextArg.text = FileName;
        GuiTask_OperWidget(gpstRecodData->hFileName, OPERATE_SET_CONTENT, &pstTextArg, SYNC_MODE);
    }
    #endif
}
/*******************************************************************************
** Name: Record_DisplayMsg
** Input:uint32 Sid
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.27
** Time: 16:01:09
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON FUN rk_err_t Record_DisplayMsg(uint32 cmd, void* arg)
{
#ifdef _USE_GUI_
    RKGUI_TEXT_ARG RecordTextArg;

    RecordTextArg.cmd= cmd;
    RecordTextArg.lucency= OPACITY;
    RecordTextArg.Backdrop= IMG_ID_MIC_BACKGROUND;
    RecordTextArg.ForegroundR= 0;
    RecordTextArg.ForegroundG= 0;
    RecordTextArg.ForegroundB= 0;
    RecordTextArg.BackgroundR= 255;
    RecordTextArg.BackgroundG= 255;
    RecordTextArg.BackgroundB= 255;
    RecordTextArg.align= TEXT_ALIGN_X_LEFT;
    RecordTextArg.display = 1;
    RecordTextArg.level = 0;
    RecordTextArg.BackdropX= 25;
    RecordTextArg.BackdropY= 103;
    RecordTextArg.x= 25;
    RecordTextArg.y= 103;
    RecordTextArg.xSize= 103;
    RecordTextArg.ySize= -1;
    if(cmd == TEXT_CMD_BUF)
    {
        RecordTextArg.text = arg;
    }
    else if(cmd == TEXT_CMD_ID)
    {
        RecordTextArg.resource = (int16)arg;
    }
    gpstRecodData->hFileName = GUITask_CreateWidget(GUI_CLASS_TEXT, &RecordTextArg);
#endif
}
/*******************************************************************************
** Name: RecordTask_RecordEventHD
** Input:uint32 event
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.11
** Time: 19:00:39
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON FUN rk_err_t RecordTask_RecordEventHD(uint32 event)
{
    switch(event)
    {
        case RECORD_STATE_PREPARE:
            #ifdef _USE_GUI_
            Recorder_GetRecordInf(&gpstRecodData->RecordInf);
            GuiTask_ScreenLock();
            Recorder_NameDisplay();
            Recorder_ProcessDisplay();
            Recorder_BitrateDisplay();
            GuiTask_ScreenUnLock();
            #endif
            break;

        case RECORD_STATE_BEING:
            #ifdef _USE_GUI_
            Recorder_GetRecordInf(&gpstRecodData->RecordInf);
            GuiTask_ScreenLock();
            Recorder_ProcessDisplay();
            Recorder_BitrateDisplay();
            GuiTask_ScreenUnLock();
            #endif
            break;

        case RECORD_STATE_TIME_CHANGE:
            Recorder_GetRecordInf(&gpstRecodData->RecordInf);
            Recorder_ProcessDisplay();
            break;

        default :
            break;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: Recorder_BitrateDisplay
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.3.11
** Time: 18:54:31
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON FUN void Recorder_BitrateDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    char buf[9];
    Ucs2 Bitrate[9];

    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    sprintf(buf, "%dkbps", gpstRecodData->RecordInf.Bitrate / 1000);

    for (i= 0; i< sizeof(buf); i++)
    {
        Bitrate[i]= (Ucs2)buf[i];
    }

    Bitrate[i] = 0;

    if(gpstRecodData->hBitrate == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 0;
        pstTextArg.y= 147;
        pstTextArg.xSize= 45;
        pstTextArg.ySize= -1;
        pstTextArg.text = Bitrate;
        pstTextArg.BackdropX= 0;
        pstTextArg.BackdropY= 147;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstRecodData->hBitrate = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        playerTextContentArg.opercmd = TEXT_SET_CONTENT;
        playerTextContentArg.text = Bitrate;
        GuiTask_OperWidget(gpstRecodData->hBitrate, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: Recorder_ProcessDisplay
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.3.11
** Time: 17:30:35
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON FUN void Recorder_ProcessDisplay(void)
{
#ifdef _USE_GUI_
    int16 i;
    char buf[9];
    Ucs2 Time[9];
    RKGUI_PROGRESSBAR_ARG pstProgressArg;
    RKGUI_TEXT_ARG pstTextArg;

    uint8 Sec, Min, Hour;

    #if 1
    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    #endif

    uint32 CurrTimer,TotalTimer;

    Recorder_GetCurTime(&CurrTimer);
    Recorder_GetTotalTime(&TotalTimer);

    Sec = (TotalTimer / 1000) % 60;
    Min = (TotalTimer / 60000) % 60;
    Hour = TotalTimer / 3600000;

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
    pstTextArg.Backdrop= IMG_ID_MIC_BACKGROUND;
    pstTextArg.ForegroundR= 0;
    pstTextArg.ForegroundG= 0;
    pstTextArg.ForegroundB= 0;
    pstTextArg.BackgroundR= 255;
    pstTextArg.BackgroundG= 255;
    pstTextArg.BackgroundB= 255;


    if(gpstRecodData->hTotalTimer == NULL)
    {
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 82;
        pstTextArg.y= 125;
        pstTextArg.xSize= 42;
        pstTextArg.ySize= -1;
        pstTextArg.text= Time;
        pstTextArg.BackdropX= 82;
        pstTextArg.BackdropY= 125;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstRecodData->hTotalTimer = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        playerTextContentArg.opercmd = TEXT_SET_CONTENT;
        playerTextContentArg.text = Time;

        GuiTask_OperWidget(gpstRecodData->hTotalTimer, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }


    Sec = (CurrTimer / 1000) % 60;
    Min = (CurrTimer / 60000) % 60;
    Hour = CurrTimer / 3600000;

    sprintf(buf, "%02d:%02d:%02d", Hour, Min, Sec);
    //printf ("CurrentTime=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        Time[i]= (Ucs2)buf[i];
    }

    Time[i] = 0;


    if(gpstRecodData->hCurrTimer == NULL)
    {
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 5;
        pstTextArg.y= 125;
        pstTextArg.xSize= 43;
        pstTextArg.ySize= -1;
        pstTextArg.text= Time;
        pstTextArg.BackdropX= 5;
        pstTextArg.BackdropY= 125;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstRecodData->hCurrTimer = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        playerTextContentArg.opercmd= TEXT_SET_CONTENT;
        playerTextContentArg.text = Time;
        GuiTask_OperWidget(gpstRecodData->hCurrTimer, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }

    if (TotalTimer== 0)return;

    pstProgressBarContent.cmd = PROGRESSBAR_SET_CONTENT;
    pstProgressBarContent.percentage = CurrTimer * 100 / TotalTimer;

    if(gpstRecodData->hTimerBar == NULL)
    {
        pstProgressArg.x= 7;
        pstProgressArg.y= 140;
        pstProgressArg.display = 1;
        pstProgressArg.level = 0;

        pstProgressArg.Backdrop= IMG_ID_MIC_BACKGROUND;
        pstProgressArg.BackdropX= 7;
        pstProgressArg.BackdropY= 140;

        pstProgressArg.Bar= IMG_ID_PROGRESS_BAR;
        pstProgressArg.percentage = pstProgressBarContent.percentage;
        gpstRecodData->hTimerBar = GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        GuiTask_OperWidget(gpstRecodData->hTimerBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: RecodTask_ButtonCallBack
** Input:uint32 evnet_type,uint32 event,void * arg,uint32 mode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:30:03
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON FUN rk_err_t RecodTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
   RECOD_ASK_QUEUE RecodAskQueue;

    if(event_type==APP_RECIVE_MSG_EVENT_KEY)
    {
        RecodAskQueue.type = RECOD_TYPE_KEY;
        RecodAskQueue.cmd  = event;

        rkos_queue_send(gpstRecodData->RecodAskQueue, &RecodAskQueue, 0);
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: RecodTask_ButtonCallBack
** Input:uint32 evnet_type,uint32 event,void * arg,uint32 mode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:30:03
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON API void RecodTask_RecordCallBack(uint32 record_state)
{
    RECOD_ASK_QUEUE RecodAskQueue;

    switch( record_state )
    {
        case RECORD_STATE_PREPARE:
            gpstRecodData->state= 0;
            break;

        case RECORD_STATE_BEING:
            gpstRecodData->state= 1;
            break;

        case RECORD_STATE_PAUSE:
            gpstRecodData->state= 2;
            break;

        case RECORD_STATE_STOP:
            break;

        case RECORD_STATE_TIME_CHANGE:
            break;

        default:
            break;
    }

    RecodAskQueue.type = RECORD_TYPE_RECORD;
    RecodAskQueue.cmd  = record_state;
    rkos_queue_send(gpstRecodData->RecodAskQueue, &RecodAskQueue, 0);

}

/*******************************************************************************
** Name: RecodTask_Enter
** Input:uint32 event
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_RECORD_RECOD_TASK_COMMON_
COMMON API rk_err_t RecodTask_KeyEventHD(uint32 event)
{
    RKGUI_TEXT_ARG TextContentArg;
    switch(event)
    {
        case KEY_VAL_ESC_SHORT_UP:

            RecordControlTask_SendCmd(RECORD_CMD_STOP,(void *)0, SYNC_MODE);

            if(RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE)!= RK_SUCCESS)
            {
                printf("delete TASK_ID_RECORDCONTROL fault\n");
                return RK_ERROR;
            }

            RecodTask_GuiDeInit();
            MainTask_TaskSwtich(TASK_ID_RECOD, 0, TASK_ID_MAIN, 0, NULL);
            while(1)
            {
                rkos_sleep(2000);
            }
            break;

        case KEY_VAL_MENU_SHORT_UP:
            TextContentArg.opercmd= TEXT_SET_CONTENT;
            if( gpstRecodData->state==1 )
            {
                RecordControlTask_SendCmd(RECORD_CMD_PAUSE, NULL, SYNC_MODE);
                TextContentArg.resource = SID_RECORD_PAUSE;
            }
            else if( gpstRecodData->state==2 )
            {
                RecordControlTask_SendCmd(RECORD_CMD_RESUME, NULL, SYNC_MODE);
                TextContentArg.resource = SID_RECORDING;
            }
            else
            {
                RecordControlTask_SendCmd(RECORD_CMD_START, NULL, SYNC_MODE);
                TextContentArg.resource = SID_RECORDING;
            }
            #ifdef _USE_GUI_
            GuiTask_OperWidget(gpstRecodData->hStatus, OPERATE_SET_CONTENT, &TextContentArg, SYNC_MODE);
            #endif
            break;

        case KEY_VAL_PLAY_SHORT_UP:
            if((Recorder_GetRecordInf(NULL) == RECORD_STATE_BEING) || (Recorder_GetRecordInf(NULL) == RECORD_STATE_PAUSE))
            {
                RecordControlTask_SendCmd(RECORD_CMD_STOP,(void *)1, SYNC_MODE);
                TextContentArg.opercmd= TEXT_SET_CONTENT;
                gpstRecodData->state = 0;
                TextContentArg.resource = SID_RECORD_PREPARE;
                #ifdef _USE_GUI_
                GuiTask_OperWidget(gpstRecodData->hStatus, OPERATE_SET_CONTENT, &TextContentArg, SYNC_MODE);
                #endif
            }
            break;

        default :
            break;
    }
    return RK_SUCCESS;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RecodTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_RECORD_RECOD_TASK_INIT_
INIT API rk_err_t RecodTask_DeInit(void *pvParameters)
{
#ifdef _USE_GUI_
    GuiTask_AppUnReciveMsg(RecodTask_ButtonCallBack);
#endif
    gpstRecodData->playerExist = 0;
    rkos_queue_delete(gpstRecodData->RecodAskQueue);
    rkos_memory_free(gpstRecodData);
    gpstRecodData = NULL;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ReCodTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_RECORD_RECOD_TASK_INIT_
INIT API rk_err_t RecodTask_Init(void *pvParameters, void *arg)
{
#ifdef _USE_GUI_
    gpstRecodData= (RECOD_TASK_DATA_BLOCK *)rkos_memory_malloc(sizeof(RECOD_TASK_DATA_BLOCK));
    if(gpstRecodData== NULL)
    {
        return RK_ERROR;
    }

    gpstRecodData->RecodAskQueue= rkos_queue_create(1, sizeof(RECOD_ASK_QUEUE));

    if( gpstRecodData->RecodAskQueue== NULL )
    {
        rkos_memory_free(gpstRecodData);
        return RK_ERROR;
    }

    gpstRecodData->hTitle = NULL;
    gpstRecodData->hBackdrop = NULL;
    gpstRecodData->hBit = NULL;
    gpstRecodData->hBitrate = NULL;
    gpstRecodData->hCurrTimer = NULL;
    gpstRecodData->hFileName = NULL;
    gpstRecodData->hMode = NULL;
    gpstRecodData->hStatus = NULL;
    gpstRecodData->hTimerBar = NULL;
    gpstRecodData->hTotalTimer = NULL;

    gpstRecodData->state= 0;
    gpstRecodData->playerExist = 0;
    GuiTask_AppReciveMsg(RecodTask_ButtonCallBack);
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
/*******************************************************************************
** Name: RecodTask_GuiInit
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_RECORD_RECOD_TASK_INIT_
COMMON FUN rk_err_t RecodTask_GuiInit(void)
{
#ifdef _USE_GUI_
    uint32 st_tim;
    int16 i;
    char buf[9];
    Ucs2 Bitrate[9];
    Ucs2 FileName[30];

    RKGUI_ICON_ARG pstIconArg;
    RKGUI_TEXT_ARG pstTextArg;
    RKGUI_PROGRESSBAR_ARG pstProgressArg;

    pstIconArg.x= 0;
    pstIconArg.y= 0;
    pstIconArg.display = 1;
    pstIconArg.resource= IMG_ID_MIC_BACKGROUND;
    pstIconArg.level= 0;
    gpstRecodData->hBackdrop= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);

    pstTextArg.cmd= TEXT_CMD_ID;
    pstTextArg.align= TEXT_ALIGN_X_CENTER;
    pstTextArg.x= 0;
    pstTextArg.y= 20;
    pstTextArg.xSize= 128;
    pstTextArg.ySize= -1;

    pstTextArg.lucency= OPACITY;
    pstTextArg.Backdrop= IMG_ID_MIC_BACKGROUND;
    pstTextArg.ForegroundR= 0;
    pstTextArg.ForegroundG= 0;
    pstTextArg.ForegroundB= 0;

    pstTextArg.BackgroundR= 255;
    pstTextArg.BackgroundG= 255;
    pstTextArg.BackgroundB= 255;
    pstTextArg.BackdropX= 0;
    pstTextArg.BackdropY= 20;

    pstTextArg.resource= SID_MIC_REC;
    pstTextArg.display = 1;
    pstTextArg.level= 0;
    gpstRecodData->hTitle= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    pstTextArg.y= 53;
    pstTextArg.BackdropY= 53;
    pstTextArg.resource = SID_RECORD_PREPARE;
    gpstRecodData->hStatus= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    pstTextArg.y= 147;
    pstTextArg.BackdropY= 147;

    if(gpstRecodData->RecordInf.channels == 2 )
    {
        pstTextArg.resource= SID_RECODING_STEREO;
    }
    else
    {
        pstTextArg.resource= SID_RECODING_MONO;
    }
    gpstRecodData->hMode= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);


    pstTextArg.align= TEXT_ALIGN_X_LEFT;
    pstTextArg.x= 100;
    pstTextArg.BackdropX= 100;
    pstTextArg.xSize = -1;


    if(gpstRecodData->RecordInf.bitpersample== 24)
    {
        pstTextArg.resource= SID_RECODING_BIT24;
    }
    else if(gpstRecodData->RecordInf.bitpersample== 16)
    {
        pstTextArg.resource= SID_RECODING_BIT16;
    }
    gpstRecodData->hBit= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
#endif
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: RecodTask_GuiDeInit
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_RECORD_RECOD_TASK_INIT_
COMMON FUN rk_err_t RecodTask_GuiDeInit(void)
{
#ifdef _USE_GUI_
    GuiTask_ScreenLock();
    if( gpstRecodData->hTitle!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hTitle);
    }
    if( gpstRecodData->hBit!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hBit);
    }
    if( gpstRecodData->hBitrate!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hBitrate);
    }
    if( gpstRecodData->hCurrTimer!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hCurrTimer);
    }
    if( gpstRecodData->hFileName!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hFileName);
    }
    if( gpstRecodData->hMode!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hMode);
    }
    if( gpstRecodData->hStatus!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hStatus);
    }
    if( gpstRecodData->hTimerBar!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hTimerBar);
    }
    if( gpstRecodData->hTotalTimer!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hTotalTimer);
    }
    if( gpstRecodData->hBackdrop!= NULL )
    {
        GuiTask_DeleteWidget(gpstRecodData->hBackdrop);
    }
    GuiTask_ScreenUnLock();
#endif
    return RK_SUCCESS;
}


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
