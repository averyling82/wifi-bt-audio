/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\SystemSet\SystemSetTask.c
* Owner: cjh
* Date: 2015.11.16
* Time: 10:13:38
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.11.16     10:13:38   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __APP_SYSTEMSET_SYSTEMSETTASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "effect.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "wiced_wifi.h"
#include "wiced_result.h"
#include "wifithread.h"

/*
*----------- ----------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _SYSTEMSET_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}SYSTEMSET_RESP_QUEUE;

typedef  struct _SYSTEMSET_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;
    uint32 offset;
}SYSTEMSET_ASK_QUEUE;

typedef struct
{
    uint16 *PWifiName;
}P_wifi_LIST;

typedef  struct _SYSTEMSET_TASK_DATA_BLOCK
{
    GUI_CONTROL_CLASS pfControl;
    pQueue  SystemSetAskQueue;
    pQueue  SystemSetRespQueue;
    HGC     hSelect;
    //HGC     hShowText;
    HGC     hWait;
    HGC     hMsgBox;

    MENU_TEXT_INFO_STRUCT SetMenuTxtInfo;

    uint32  curSelectID;
    uint32  curCmd;
    uint8   lastFlag;
    uint8   wifiListTaskCreateFlag;

    int     smartConfigStart;
    int     deleteTaskFlag;
    uint8   QueueFull;

    //wrm-add
    uint8   blevel_set;
    uint8   btime_set;
    uint8   eqmode_set;
}SYSTEMSET_TASK_DATA_BLOCK;

#define AP_CONFIGON   1;
#define AP_CONFIGOFF  0;

#define SELECTSHOWNUM   8;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SYSTEMSET_TASK_DATA_BLOCK * gpstSystemSetData;

_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
uint8 EqMode_Value[SID_EQ_CUSTOM - SID_EQ_NONE + 1] =
{
    EQ_NOR,
    EQ_BASS,
    EQ_HEAVY,
    EQ_POP,
    EQ_JAZZ,
    EQ_UNIQUE,
    EQ_USER,
};



_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
uint8 BackLight_LevelValue[SID_LUM_5-SID_LUM_1+1] =
{
    BL_LEVEL_1_VALUE,
    BL_LEVEL_2_VALUE,
    BL_LEVEL_3_VALUE,
    BL_LEVEL_4_VALUE,
    BL_LEVEL_5_VALUE,
};

_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
uint32 BackLight_Time[SID_SCREEN_OFF_30_MIN-SID_SCREEN_OFF_15_SEC+1] =
{
    BL_OFF_TIME_1,
    BL_OFF_TIME_2,
    BL_OFF_TIME_3,
    BL_OFF_TIME_4,
    BL_OFF_TIME_5,
    BL_OFF_TIME_6,
};

_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
UINT32 MultipleLanguages[SID_TURKISH-SID_CHINESE_S+1] =
{
    LANGUAGE_CHINESE_S,
    LANGUAGE_CHINESE_T,
    LANGUAGE_ENGLISH,
    LANGUAGE_KOREAN,
    LANGUAGE_JAPANESE,
    LANGUAGE_FRENCH,
    LANGUAGE_GERMAN,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_RUSSIAN,
    LANGUAGE_SWEDISH,
    LANGUAGE_THAI,
    LANGUAGE_POLAND,
    LANGUAGE_DENISH,
    LANGUAGE_DUTCH,
    LANGUAGE_HELLENIC,
    LANGUAGE_CZECHIC,
    LANGUAGE_TURKIC,
    //LANGUAGE_RABBINIC,
    //LANGUAGE_ARABIC
};

_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
UINT32 LanguagesLocation[LANGUAGE_TURKIC+1] =
{
    0,//(LANGUAGE_CHINESE_S - LANGUAGE_CHINESE_S),
    1,//(LANGUAGE_CHINESE_T - LANGUAGE_CHINESE_S),
    2,//(LANGUAGE_ENGLISH - LANGUAGE_CHINESE_S),
    3,//(LANGUAGE_KOREAN - LANGUAGE_CHINESE_S),
    4,//(LANGUAGE_JAPANESE - LANGUAGE_CHINESE_S),
    5,//(LANGUAGE_FRENCH - LANGUAGE_CHINESE_S),
    6,//(LANGUAGE_GERMAN - LANGUAGE_CHINESE_S),
    7,//(LANGUAGE_PORTUGUESE - LANGUAGE_CHINESE_S),
    8,//(LANGUAGE_RUSSIAN - LANGUAGE_CHINESE_S),
    0,//LANGUAGE_SPAISH
    0,//LANGUAGE_ITALIAN
    9,//(LANGUAGE_SWEDISH - LANGUAGE_CHINESE_S),
    10,//(LANGUAGE_THAI - LANGUAGE_CHINESE_S),
    11,//(LANGUAGE_POLAND - LANGUAGE_CHINESE_S),
    12,//(LANGUAGE_DENISH - LANGUAGE_CHINESE_S),
    0,//LANGUAGE_DUTCH
    13,//(LANGUAGE_HELLENIC - LANGUAGE_CHINESE_S),
    14,//(LANGUAGE_CZECHIC - LANGUAGE_CHINESE_S),
    15,//(LANGUAGE_TURKIC - LANGUAGE_CHINESE_S),
    //(LANGUAGE_RABBINIC- LANGUAGE_CHINESE_S),
    //(LANGUAGE_ARABIC- LANGUAGE_CHINESE_S),
};


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
rk_err_t System_GoToFatherList(void);
rk_err_t SystemSetTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc);
rk_err_t System_MsgBoxCallBack(HGC pGc, void * arg);
rk_err_t SystemTask_SetPopupWindow(void * text, uint16 text_cmd);
rk_err_t SystemTask_EvnetOper(uint32 menuTextID);
rk_err_t SystemTask_ChangeSelectList(uint32 SelectID, uint32 CursorID);
rk_err_t SystemTask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, RKGUI_SELECT_ITEM * arg, int offset);
rk_err_t System_SelectEventMove(int32 offset);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*******************************************************************************
** Name: MainTask_UpDataCallBack
** Input:uint32 event_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:cjh
** Date: 2015.10.26
** Time: 09:30:03
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON API rk_err_t SystemSetTask_UpData(uint32 event_type, uint32 event, void * arg, uint32 mode)
{
    SYSTEMSET_ASK_QUEUE SystemSetAskQueue;
    rk_err_t ret = 0;

#ifdef __GUI_GUITASK_C__
    if(event_type != SYSTEM_SET_UPDATA)return RK_ERROR;
    rk_printf("updata SystemSetTask....... 0x%x",event);
    SystemSetAskQueue.event_type = event_type;//SYSTEM_SET_UPDATA
    SystemSetAskQueue.event = event;
    rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &SystemSetAskQueue, MAX_DELAY);
    if (ret == RK_ERROR)
    {
        rk_printf("---------lose SystemSetTask_UpData");
    }
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SystemSetTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.16
** Time: 10:14:20
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON API rk_err_t SystemSetTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: SystemSetTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.16
** Time: 10:14:20
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON API rk_err_t SystemSetTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: SystemSetTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2015.11.16
** Time: 10:14:20
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON API void SystemSetTask_Enter(void * arg)
{
    //uint32 msg;
    SYSTEMSET_ASK_QUEUE systemSetAskQueue;
    RKGUI_SELECT_CONTENT parameter;
    RKGUI_OPERATE_ARG pOperArg;
    rk_err_t ret;
    uint32 scanNum = 0;
    RKGUI_TEXT_ARG pstTextArg;

    uint16 textLen;
    UINT16 text[256],i;
    uint16 listCount;
    uint16 menuTextID;
    uint16 CurSorID;
#ifdef __GUI_GUITASK_C__
    MainTask_SetStatus(MAINTASK_APP_SYSTEMSET,1);
    #ifdef _USE_GUI_
    SystemTask_ChangeSelectList(SID_SETTINGS, 0);
    #endif
    while (1)
    {
        ret = rkos_queue_receive(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 50);
        if(ret == RK_SUCCESS)
        {
            //rk_printf("!!enter!system event_type = 0x%x event =0x%x off =%d\n",systemSetAskQueue.event_type,systemSetAskQueue.event,systemSetAskQueue.offset);
            switch(systemSetAskQueue.event_type)
            {
                case APP_RECIVE_MSG_EVENT_KEY:
                    switch(systemSetAskQueue.event)
                    {
                        case KEY_VAL_PLAY_PRESS:
                            gpstSystemSetData->curCmd = KEY_VAL_PLAY_PRESS;
                            MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                            break;

                        case KEY_VAL_ESC_SHORT_UP:
                            {
                                if(gpstSystemSetData->hWait != NULL)
                                {
                                    if(gpstSystemSetData->smartConfigStart)
                                    {
                                        gpstSystemSetData->curCmd = KEY_VAL_ESC_SHORT_UP;
                                        rk_printf("!rk_easy_smartconfig_stop\n");
                                        #ifdef _DRIVER_WIFI__
                                        rk_easy_smartconfig_stop();
                                        #endif
                                        gpstSystemSetData->smartConfigStart = AP_CONFIGOFF;
                                        rk_printf("!ESC hWait Del\n");
                                        GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                                        GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                                        gpstSystemSetData->hWait = NULL;
                                    }
                                    else
                                    {

                                    }
                                }
                                else
                                {
                                    if(gpstSystemSetData->SetMenuTxtInfo.FatherID != 0xFFFF)
                                    {
                                        rk_printf("!!!!!!!!FatherID = 0x%x curIDinFather = 0x%x\n", gpstSystemSetData->SetMenuTxtInfo.FatherID,gpstSystemSetData->SetMenuTxtInfo.CurItemInFatherID);
                                        menuTextID = gpstSystemSetData->SetMenuTxtInfo.FatherID;
                                        if(gpstSystemSetData->SetMenuTxtInfo.CurItemInFatherID != 0xFFFF)
                                        {
                                            CurSorID = gpstSystemSetData->SetMenuTxtInfo.CurItemInFatherID;
                                        }
                                        else
                                        {
                                            CurSorID = 0;
                                        }
                                        SystemTask_ChangeSelectList(menuTextID, CurSorID);
                                    }
                                    else
                                    {
#ifdef __APP_SYSTEMSET_SYSTEMSETTASK_C__
                                        MainTask_TaskSwtich(TASK_ID_SYSTEMSET, 0,  TASK_ID_MAIN, 0,  NULL);
                                        //RKTaskDelete(TASK_ID_SYSTEMSET, 0, ASYNC_MODE);
#endif
                                        while(1)
                                        {
                                            rkos_sleep(2000);
                                            rk_printf("TASK_ID_SYSTEMSET\n");
                                        }
                                    }
                                }
                            }
                            //rk_printf("ESC OVER\n");
                            break;

                        default:
                            break;
                    }
                    gpstSystemSetData->QueueFull = 0;
                    break;

                case APP_RECIVE_MSG_EVENT_WARING:
                    gpstSystemSetData->curCmd = APP_RECIVE_MSG_EVENT_WARING;
                    if(gpstSystemSetData->hMsgBox != NULL)
                    {
                        rk_printf("gpstSystemSetData->hMsgBox delete\n");

                        GuiTask_OperWidget(gpstSystemSetData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstSystemSetData->hMsgBox);
                        gpstSystemSetData->hMsgBox = NULL;
                        /*
                        if(gpstSystemSetData->SetMenuTxtInfo.SelfID != 0xFFFF)
                        {
                            menuTextID = gpstSystemSetData->SetMenuTxtInfo.SelfID;
                            SystemTask_ChangeSelectList(menuTextID, 0);
                        }
                        else
                        {
                            rk_printf("FatherID: -1\n");
                            SystemTask_ChangeSelectList(SID_SETTINGS, 0);
                        }*/
                    }
                    gpstSystemSetData->QueueFull = 0;
                    break;

                case SYSTEM_SET_ENTER:
                    rk_printf("ENTER enter\n");
                    gpstSystemSetData->curCmd = SYSTEM_SET_ENTER;

                    if(gpstSystemSetData->SetMenuTxtInfo.ChildID != 0xFFFF)
                    {
                        menuTextID = gpstSystemSetData->SetMenuTxtInfo.ChildIDArray[systemSetAskQueue.offset];
                        //rk_printf("!!!cv SelectID =0x%x Location=%d\n",gpstSystemSetData->SetMenuTxtInfo.SelfID , LanguagesLocation[gSysConfig.SysLanguage]);
                        if(menuTextID == SID_LANGUAGE)
                        {
                            SystemTask_ChangeSelectList(menuTextID, LanguagesLocation[gSysConfig.SysLanguage]);
                        }
                        else
                        {
                            SystemTask_ChangeSelectList(menuTextID, 0);
                        }
                    }
                    break;

                case SYSTEM_SET_UPDATA:
                    gpstSystemSetData->wifiListTaskCreateFlag = 0;
                    gpstSystemSetData->curCmd = SYSTEM_SET_UPDATA;
                    if(gpstSystemSetData->SetMenuTxtInfo.SelfID != 0xFFFF)
                    {
                        menuTextID = gpstSystemSetData->SetMenuTxtInfo.SelfID;
                        SystemTask_ChangeSelectList(menuTextID, 0);
                    }
                    else
                    {
                        rk_printf("FatherID: -1\n");
                        SystemTask_ChangeSelectList(SID_SETTINGS, 0);
                    }
                    break;

                case SYSTEM_SET_DLETE_MSGBOX:
                    rk_printf("DLETE_MSGBOX: \n");
                    gpstSystemSetData->curCmd = SYSTEM_SET_DLETE_MSGBOX;
                    if(gpstSystemSetData->hMsgBox != NULL)
                    {
                        GuiTask_OperWidget(gpstSystemSetData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstSystemSetData->hMsgBox);
                        gpstSystemSetData->hMsgBox=NULL;
                    }
                    break;
                default :
                    break;
            }
        }
        else
        {
            if((gpstSystemSetData->curCmd == SID_WIFICONFIG_ON_AP)
                || (gpstSystemSetData->curCmd == SID_WIFICONFIG_ON_STA)
                || (gpstSystemSetData->curCmd == SID_APCONFIG)
                || (gpstSystemSetData->curCmd == SID_BLUETOOTH_ON))
            {
                //rk_printf("..");
                SystemTask_EvnetOper(gpstSystemSetData->curCmd);
            }
        }
    }
#endif
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: System_SelectEventMove
** Input:int32 offset
** Return: rk_err_t
** Owner:wrm
** Date: 2016.1.25
** Time: 10:45:22
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t System_SelectEventMove(int32 offset)
{
    rk_err_t ret;
    HDC hPwmDev = NULL;

    if (gpstSystemSetData->eqmode_set == 1)
    {
        if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
        {
            ret = AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EqMode_Value[offset], SYNC_MODE);
            if(ret != RK_SUCCESS)
            {
                rk_printf("set SHUFFLE_ON  fail");
            }
        }
    }

    if (gpstSystemSetData->blevel_set == 1)
    {
        hPwmDev = RKDev_Open(DEV_CLASS_PWM, 0, NOT_CARE);
        if (hPwmDev != NULL)
        {
            PwmDev_SetRate(hPwmDev,BackLight_LevelValue[offset]);
            RKDev_Close(hPwmDev);
        }
        else
        {
            rk_printf("Open PWM 0 fail");
        }
    }
    if (gpstSystemSetData->btime_set == 1)
    {

    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: System_GoToFatherList
** Input:uint32 menuID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.25
** Time: 10:45:22
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t System_GoToFatherList(void)
{
    uint32 CursorID;
    uint32 MenuID;
    if(gpstSystemSetData->SetMenuTxtInfo.FatherID != 0xFFFF)
    {
        rk_printf("!!FatherID = 0x%x curIDinFather = 0x%x\n", gpstSystemSetData->SetMenuTxtInfo.FatherID,gpstSystemSetData->SetMenuTxtInfo.CurItemInFatherID);
        MenuID = gpstSystemSetData->SetMenuTxtInfo.FatherID;
        if(gpstSystemSetData->SetMenuTxtInfo.CurItemInFatherID != 0xFFFF)
        {
            CursorID = gpstSystemSetData->SetMenuTxtInfo.CurItemInFatherID;
        }
        else
        {
            CursorID = 0;
        }
        #ifdef _USE_GUI_
        SystemTask_ChangeSelectList(MenuID, CursorID);
        #endif
    }
}
#ifdef __GUI_GUITASK_C__
/*******************************************************************************
** Name: SystemSetTask_ButtonCallBack
** Input:(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.24
** Time: 16:01:09
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t SystemSetTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    SYSTEMSET_ASK_QUEUE systemSetAskQueue;
    SYSTEMSET_ASK_QUEUE systemSetAskQueue_tmp;
    if(gpstSystemSetData->deleteTaskFlag != 1)
    {
        if((gpstSystemSetData->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP) && ((event & KEY_STATUS_SHORT_UP) != KEY_STATUS_SHORT_UP))
        {
            rk_printf("lose key QueueFull=%d\n",gpstSystemSetData->QueueFull);
            return RK_SUCCESS;
        }
        switch(event_type)
        {
            case APP_RECIVE_MSG_EVENT_KEY:
            {
                if(gpstSystemSetData->wifiListTaskCreateFlag != 1)
                {
                    gpstSystemSetData->QueueFull = 1;
                    systemSetAskQueue.event = event;
                    systemSetAskQueue.event_type = event_type;
                    systemSetAskQueue.offset = NULL;
                    ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
                    if(ret == RK_ERROR)
                    {
                        if((event==KEY_VAL_FFW_PRESS)||(event==KEY_VAL_FFD_PRESS)||(event==KEY_VAL_HOLD_PRESS))
                        {
                            gpstSystemSetData->QueueFull = 0;
                            return RK_SUCCESS;
                        }
                        rkos_queue_receive(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue_tmp, 0);
                        if (systemSetAskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                        {
                            rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue_tmp, 0);
                            rkos_queue_receive(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue_tmp, 0);
                            if (systemSetAskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                            {
                                rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue_tmp, 0);
                                gpstSystemSetData->QueueFull = 0;
                            }
                            else
                            {
                                ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
                                if (ret == RK_ERROR)
                                {
                                    rk_printf("Send Key Failure 0\n");
                                    gpstSystemSetData->QueueFull = 0;
                                }
                            }
                        }
                        else
                        {
                            ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
                            if (ret == RK_ERROR)
                            {
                                rk_printf("Send Key Failure 1\n");
                                gpstSystemSetData->QueueFull = 0;
                            }
                        }

                    }
                }
            }
            break;

            case APP_RECIVE_MSG_EVENT_WARING:
            {
                systemSetAskQueue.event = event;
                systemSetAskQueue.event_type = event_type;
                systemSetAskQueue.offset = NULL;
                gpstSystemSetData->QueueFull = 1;
                ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
                if(ret == RK_ERROR)
                {
                    rk_printf("-------lose SystemSet_MsgBoxCallBack 0\n");
                    rkos_queue_receive(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue_tmp, 0);//MAX_DELAY
                    rk_printf ("-------lose SystemSet_MsgBoxCallBack 1\n");
                    rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
                }

            }
            break;

            case APP_RECIVE_MSG_EVENT_DIALOG:
            {
                systemSetAskQueue.event = event;
                systemSetAskQueue.event_type = event_type;
                systemSetAskQueue.offset = NULL;
                ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
            }
            break;

            default:
            break;
        }
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SetPopupWindow
** Input:int type
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.22
** Time: 10:15:31
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t SystemTask_SetPopupWindow(void * text, uint16 text_cmd)
{
    RKGUI_MSGBOX_ARG pstMsgBoxArg;

    pstMsgBoxArg.cmd = MSGBOX_CMD_WARNING;
    pstMsgBoxArg.x = 4;
    pstMsgBoxArg.y = 30;
    pstMsgBoxArg.display = 1;
    pstMsgBoxArg.level = 0;
    pstMsgBoxArg.title = SID_WARNING;
    pstMsgBoxArg.text_cmd = text_cmd;//TEXT_CMD_BUF
    pstMsgBoxArg.text = text;
    pstMsgBoxArg.align= TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;

    if(gpstSystemSetData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstSystemSetData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstSystemSetData->hMsgBox);
        gpstSystemSetData->hMsgBox = NULL;
    }

    if(gpstSystemSetData->hMsgBox == NULL)
    {
        gpstSystemSetData->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: System_MsgBoxCallBack
** Input:HGC pGc, void * arg
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.22
** Time: 13:42:56
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t System_MsgBoxCallBack(HGC pGc, void * arg)
{
    rk_err_t ret;
    SYSTEMSET_ASK_QUEUE systemSetAskQueue;
    SYSTEMSET_ASK_QUEUE systemSetAskQueueTemp;
    {
        printf("on_off:%d\n",(int)arg);
        systemSetAskQueue.event = NULL;
        systemSetAskQueue.offset = NULL;
        systemSetAskQueue.event_type = SYSTEM_SET_DLETE_MSGBOX;
        ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
        if(ret == RK_ERROR)
        {
            rkos_queue_receive(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueueTemp, 0);//MAX_DELAY
            rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
        }
    }
    //else
    //{
    //    printf("on_off 1:%d\n",(int)arg);
   // }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SystemTask_EvenOpr
** Input:(uint32 MID);
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.9
** Time: 15:04:28
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t SystemTask_EvnetOper(uint32 menuTextID)
{
    rk_err_t ret;
    uint32 CurEqMode;
    RKGUI_SELECT_CONTENT bl_parameter;

    HDC hPwmDev = NULL;
    //void * text;

    switch (menuTextID)
    {
#ifdef  _BLUETOOTH_
        case SID_BLUETOOTH_ON:
        #ifdef _DRIVER_WIFI__
            if(MainTask_GetStatus(MAINTASK_WIFI_OPEN_OK) == 1)
            {
                if(gpstSystemSetData->curCmd != SID_BLUETOOTH_ON)
                {
                    //text = TEXT("Please Close Wifi");
                    SystemTask_SetPopupWindow((void *)SID_WIFI_TURN_OFF, TEXT_CMD_ID);
                }
                gpstSystemSetData->curCmd = SID_BLUETOOTH_ON;
                return RK_SUCCESS;
            }
        #endif
            {
                if(MainTask_GetStatus(MAINTASK_BT_OK) != 1)
                {

                    if(gpstSystemSetData->curCmd != SID_BLUETOOTH_ON)
                    {
                        rk_printf ("BT Starting....\n");
                        MainTask_SetStatus(MAINTASK_BT_START, 1);
                        FW_LoadSegment(SEGMENT_ID_BLUETOOTH, SEGMENT_OVERLAY_ALL);
                        if(bluetooth_start() == RK_SUCCESS)
                        {
                            RKGUI_CHAIN_ARG pstChainArg;
                            pstChainArg.x= 20;
                            pstChainArg.y= 40;
                            pstChainArg.resource = IMG_ID_WAIT0;
                            pstChainArg.num = 16;
                            pstChainArg.delay = 10;
                            pstChainArg.level = 0;
                            pstChainArg.display = 1;
                            pstChainArg.blurry = 1;

                            if(gpstSystemSetData->hWait == NULL)
                            {
                                gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                            }
                        }
                        else
                        {
                            MainTask_SetStatus(MAINTASK_BT_START, 0);
                            bluetooth_stop();
                            FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
                            //text = TEXT("BT Start Fail");
                            rk_printf ("BT Start--------Fail\n");
                            SystemTask_SetPopupWindow((void *)SID_BT_TURNON_FAILED, TEXT_CMD_ID);
                        }
                        gpstSystemSetData->curCmd = SID_BLUETOOTH_ON;
                    }

                    if(gSysConfig.BtOpened == 1)
                    {
                        rk_printf ("BT Open---Success\n");
                        MainTask_SetStatus(MAINTASK_BT_OK, 1);
                        if(gpstSystemSetData->hWait != NULL)
                        {
                            GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                            gpstSystemSetData->hWait = NULL;
                        }
                        gpstSystemSetData->curCmd = 0xFF;
                    }
                    else if(gSysConfig.BtOpened == -1)
                    {
                        bluetooth_stop();
                        FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
                        if(gpstSystemSetData->hWait != NULL)
                        {
                            GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                            gpstSystemSetData->hWait = NULL;
                        }
                        //text = TEXT("BT Start Fail");
                        gpstSystemSetData->curCmd = 0xFF;
                        rk_printf ("BT Open--Fail\n");
                        SystemTask_SetPopupWindow((void *)SID_BT_TURNON_FAILED, TEXT_CMD_ID);

                        MainTask_SetStatus(MAINTASK_BT_OK, 0);
                        MainTask_SetStatus(MAINTASK_BT_START, 0);
                    }
                    else
                    {
                        rk_printf ("BT Starting....\n");
                    }
                }
            }

            break;
        case SID_BLUETOOTH_OFF:
            gpstSystemSetData->curCmd = SID_BLUETOOTH_OFF;
            //if(gSysConfig.BtEnabled == 1)
            {
                if(MainTask_GetStatus(MAINTASK_BT_OK) == 1)
                {
                    RKGUI_CHAIN_ARG pstChainArg;

                    pstChainArg.x= 20;
                    pstChainArg.y= 40;
                    pstChainArg.resource = IMG_ID_WAIT0;
                    pstChainArg.num = 16;
                    pstChainArg.delay = 10;
                    pstChainArg.level = 0;
                    pstChainArg.display = 1;
                    pstChainArg.blurry = 1;

                    if(gpstSystemSetData->hWait == NULL)
                    {
                        gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                    }

                    bluetooth_stop();
                    FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
                    if(gpstSystemSetData->hWait != NULL)
                    {
                        GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                        GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                        gpstSystemSetData->hWait = NULL;
                    }
                    MainTask_SetStatus(MAINTASK_BT_OK, 0);
                    MainTask_SetStatus(MAINTASK_BT_START, 0);
                    rk_printf("$$$$bluetooth_stop....\n");
                }
            }

            break;
#endif

#ifdef _DRIVER_WIFI__
        case SID_WIFICONFIG_ON_STA:
            #ifdef  _BLUETOOTH_
            if(MainTask_GetStatus(MAINTASK_BT_OK) == 1)
            {
                if(gpstSystemSetData->curCmd != SID_WIFICONFIG_ON_STA)
                {
                    //text = TEXT("please close BT");
                    SystemTask_SetPopupWindow((void *)SID_BT_TURN_OFF, TEXT_CMD_ID);
                }
                gpstSystemSetData->curCmd = 0xFF;
                return RK_SUCCESS;
            }
            #endif

            {
                //gSysConfig.SelPlayType = SOURCE_FROM_NET;
                if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) == NULL )
                {
                    if(gpstSystemSetData->curCmd != SID_WIFICONFIG_ON_STA)
                    {
                        if(gpstSystemSetData->hWait == NULL)
                        {
                            RKGUI_CHAIN_ARG pstChainArg;
                            pstChainArg.x= 20;
                            pstChainArg.y= 40;
                            pstChainArg.resource = IMG_ID_WAIT0;
                            pstChainArg.num = 16;
                            pstChainArg.delay = 10;
                            pstChainArg.level = 0;
                            pstChainArg.display = 1;
                            pstChainArg.blurry = 1;
                            gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                        }
                        rk_printf("create hWait = 0x%x, gpstSystemSetData->curCmd = %d",gpstSystemSetData->hWait,gpstSystemSetData->curCmd);
                        gpstSystemSetData->curCmd = SID_WIFICONFIG_ON_STA;
                    }

                    RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_STA, SYNC_MODE);
                }


            }

            {
                if(wifi_init_flag() == WICED_TRUE)
                {
#if 1
                    #if 0
                    if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK) != 1)
                    {
                        if(wifi_join_flag() == WICED_TRUE)
                        {
                            rk_printf("...smart config connect ok\n");
                            gpstSystemSetData->curCmd = 0xFF;
                        }
                        else if(wifi_join_flag() == WICED_ERR)
                        {
                            gpstSystemSetData->curCmd = 0xFF;
                            //text = TEXT("connect wifi fail");
                            //SystemTask_SetPopupWindow((void *)SID_WIFI_CONNECT_FAILED, TEXT_CMD_ID);
                        }
                        else if(wifi_join_flag() == WICED_WAIT)
                        {
                            //rk_printf("connect.....");
                            gpstSystemSetData->curCmd = 0xFF;
                            //return RK_SUCCESS;
                        }
                        else
                        {
                            //rk_printf("wifi join start.....");
                            return RK_SUCCESS;
                        }
                    }
                    #endif

                    //printf("connect_flag=%d",wifi_join_flag());
#ifdef __APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_C__
                    if(RKTaskFind(TASK_ID_SYSTEMSETWIFISELECT, 0) == NULL)
                    {
                        gpstSystemSetData->wifiListTaskCreateFlag = 1;
                        if(gpstSystemSetData->hWait != NULL)
                        {
                            printf("hWait del...\n");
                            GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                            gpstSystemSetData->hWait = NULL;
                        }
                        if(gpstSystemSetData->hMsgBox != NULL)
                        {
                            GuiTask_OperWidget(gpstSystemSetData->hMsgBox, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                            GuiTask_DeleteWidget(gpstSystemSetData->hMsgBox);
                            gpstSystemSetData->hMsgBox = NULL;
                        }

                        if(gpstSystemSetData->hSelect != NULL)
                        {
                            GuiTask_DeleteWidget(gpstSystemSetData->hSelect);
                            gpstSystemSetData->hSelect = NULL;
                        }
                        gpstSystemSetData->curCmd = 0xFF;
                        RKTaskCreate(TASK_ID_SYSTEMSETWIFISELECT, 0, NULL, SYNC_MODE);
                    }
                    else
                    {
                        printf("WIFISELECT  exist...");
                        gpstSystemSetData->curCmd = SID_WIFICONFIG_ON_STA;
                    }
#endif
#endif
                }
            }

            break;


        case SID_WIFICONFIG_ON_AP:
            #ifdef  _BLUETOOTH_
            if(MainTask_GetStatus(MAINTASK_BT_OK) == 1)
            {
                if(gpstSystemSetData->curCmd != SID_WIFICONFIG_ON_STA)
                {
                    //text = TEXT("please close BT");
                    SystemTask_SetPopupWindow((void *)SID_BT_TURN_OFF, TEXT_CMD_ID);
                }
                gpstSystemSetData->curCmd = 0xFF;
                return RK_SUCCESS;
            }
            #endif

            {
                //gSysConfig.SelPlayType = SOURCE_FROM_NET;
                if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) == NULL )
                {
                    if(gpstSystemSetData->curCmd != SID_WIFICONFIG_ON_AP)
                    {
                        if(gpstSystemSetData->hWait == NULL)
                        {
                            RKGUI_CHAIN_ARG pstChainArg;
                            pstChainArg.x= 20;
                            pstChainArg.y= 40;
                            pstChainArg.resource = IMG_ID_WAIT0;
                            pstChainArg.num = 16;
                            pstChainArg.delay = 10;
                            pstChainArg.level = 0;
                            pstChainArg.display = 1;
                            pstChainArg.blurry = 1;
                            gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                        }
                        gpstSystemSetData->curCmd = SID_WIFICONFIG_ON_AP;
                    }

                    RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_AP, SYNC_MODE);
                }
            }

            if(wifi_init_flag() == WICED_TRUE)
            {

                if(gpstSystemSetData->hWait != NULL)
                {
                    printf("hWait del...\n");
                    GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                    GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                    gpstSystemSetData->hWait = NULL;
                }

                if(gpstSystemSetData->hMsgBox != NULL)
                {
                    GuiTask_OperWidget(gpstSystemSetData->hMsgBox, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                    GuiTask_DeleteWidget(gpstSystemSetData->hMsgBox);
                    gpstSystemSetData->hMsgBox = NULL;
                }
                gpstSystemSetData->curCmd = 0xFF;
            }
            break;


        case SID_WIFICONFIG_OFF:
            gpstSystemSetData->curCmd = SID_WIFICONFIG_OFF;
            gpstSystemSetData->smartConfigStart = AP_CONFIGOFF;
#if 0
#ifdef __WIFI_DLNA_C__
            if(RKTaskFind(TASK_ID_DLNA, 0) != NULL )
            {
                rk_dlna_end();
                RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER,0);
            }
#endif

#ifdef __WIFI_XXX_C__
            if(RKTaskFind(TASK_ID_XXX, 0) != NULL )
            {
                RKTaskDelete(TASK_ID_XXX, 0, SYNC_MODE);
            }
#endif

#endif
            if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) != NULL )
            {
                rk_wifi_deinit();
                RKTaskDelete(TASK_ID_WIFI_APPLICATION,0,SYNC_MODE);
                gSysConfig.SelPlayType = NULL;
                rk_printf("$$$$Wifi_stop....\n");
            }

            break;

        case SID_APCONFIG:
            //gSysConfig.SmartConfigEnabled = 1;
            //if(gSysConfig.SmartConfigEnabled)
            if(MainTask_GetStatus(MAINTASK_WIFI_OPEN_OK))
            {
                gpstSystemSetData->curCmd = SID_APCONFIG;
                //printf("... easy_setup = %d...\n",wifi_easy_setup_flag() );
                if(wifi_init_flag() == WICED_TRUE)
                {
                    if(gpstSystemSetData->smartConfigStart != 1)
                    {
                        if(gpstSystemSetData->hWait == NULL)
                        {
                            RKGUI_CHAIN_ARG pstChainArg;
                            pstChainArg.x= 20;
                            pstChainArg.y= 40;
                            pstChainArg.resource = IMG_ID_WAIT0;
                            pstChainArg.num = 16;
                            pstChainArg.delay = 10;
                            pstChainArg.level = 0;
                            pstChainArg.display = 1;
                            pstChainArg.blurry = 1;
                            gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                        }
                        gpstSystemSetData->smartConfigStart = AP_CONFIGON;
                        #ifdef _DRIVER_WIFI__
                        rk_wifi_smartconfig();
                        #endif

                        //printf("\n\n...smartconfig start....\n\n");
                    }
                    else
                    {
                        if(wifi_easy_setup_flag() == WICED_TRUE)
                        {
                            //printf("wifi_easy_setup_flag OK connect=%d...\n",wifi_join_flag());
                            if(wifi_join_flag() == WICED_TRUE)
                            {
#if 1
                                if(gpstSystemSetData->hWait != NULL)
                                {
                                    GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                                    GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                                    gpstSystemSetData->hWait = NULL;
                                    #ifdef _DRIVER_WIFI__
                                    rk_easy_smartconfig_stop();
                                    #endif
                                    gpstSystemSetData->smartConfigStart = AP_CONFIGOFF;
                                }
#endif
                                //gSysConfig.SelPlayType = SOURCE_FROM_HTTP;
                                rk_printf("11smart config connect ok\n");
                                gpstSystemSetData->curCmd = 0xFF;
                            }
                            else if(wifi_join_flag() == WICED_ERR)
                            {
#if 1
                                if(gpstSystemSetData->hWait != NULL)
                                {
                                    GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                                    GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                                    gpstSystemSetData->hWait = NULL;
                                    #ifdef _DRIVER_WIFI__
                                    rk_easy_smartconfig_stop();
                                    #endif
                                    gpstSystemSetData->smartConfigStart = AP_CONFIGOFF;
                                }
#endif
                                //text = TEXT("connect wifi fail");
                                gpstSystemSetData->curCmd = 0xFF;
                                SystemTask_SetPopupWindow((void *)SID_WIFI_CONNECT_FAILED, TEXT_CMD_ID);
                            }
                        }
                        else if(wifi_easy_setup_flag() == WICED_FALSE)
                        {
                            if(gpstSystemSetData->hWait != NULL)
                            {
                                GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
                                GuiTask_DeleteWidget(gpstSystemSetData->hWait);
                                gpstSystemSetData->hWait = NULL;
                                #ifdef _DRIVER_WIFI__
                                rk_easy_smartconfig_stop();
                                #endif
                                gpstSystemSetData->smartConfigStart = AP_CONFIGOFF;
                            }

                            gpstSystemSetData->curCmd = 0xFF;
                            //SystemTask_SetPopupWindow((void *)SID_WIFI_CONNECT_FAILED, TEXT_CMD_ID);

                        }
                        else
                        {
                            if(gpstSystemSetData->hWait == NULL)
                            {
                                RKGUI_CHAIN_ARG pstChainArg;
                                pstChainArg.x= 20;
                                pstChainArg.y= 40;
                                pstChainArg.resource = IMG_ID_WAIT0;
                                pstChainArg.num = 16;
                                pstChainArg.delay = 10;
                                pstChainArg.level = 0;
                                pstChainArg.display = 1;
                                pstChainArg.blurry = 1;
                                gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                            }
                        }
                    }
                }
                else
                {
                    if(gpstSystemSetData->hWait == NULL)
                    {
                        RKGUI_CHAIN_ARG pstChainArg;
                        pstChainArg.x= 20;
                        pstChainArg.y= 40;
                        pstChainArg.resource = IMG_ID_WAIT0;
                        pstChainArg.num = 16;
                        pstChainArg.delay = 10;
                        pstChainArg.level = 0;
                        pstChainArg.display = 1;
                        pstChainArg.blurry = 1;
                        gpstSystemSetData->hWait = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
                    }
                }
            }
            else
            {
                gpstSystemSetData->curCmd = 0xFF;
                //text = TEXT("please open wifi");
                SystemTask_SetPopupWindow((void *)SID_WIFI_TURN_ON, TEXT_CMD_ID);
            }
            break;

#endif
        case SID_MUSIC_FOLDER_ONCE:
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                ret = AudioControlTask_SendCmd(AUDIO_CMD_REPEATSET, (void *)AUDIO_FOLDER_ONCE, SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set repeat mod : REPEAT_ONE fail");
                }
            }
            gSysConfig.MusicConfig.RepeatMode = AUDIO_FOLDER_ONCE;
            System_GoToFatherList();
            break;

        case SID_MUSIC_FOLDER_REPEAT:
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                ret = AudioControlTask_SendCmd(AUDIO_CMD_REPEATSET, (void *)AUIDO_FOLDER_REPEAT, SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set repeat mod : FOLDER_ONCE fail");
                }
            }
            gSysConfig.MusicConfig.RepeatMode = AUIDO_FOLDER_REPEAT;
            System_GoToFatherList();
            break;

        case SID_MUSIC_REPEAT_ONE:
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                ret = AudioControlTask_SendCmd(AUDIO_CMD_REPEATSET, (void *)AUDIO_REPEAT, SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set repeat mod : FOLDER_REPEAT fail");
                }
            }
            gSysConfig.MusicConfig.RepeatMode = AUDIO_REPEAT;
            System_GoToFatherList();
            break;

        case SID_MUSIC_SHUFFLE_OFF:
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                ret = AudioControlTask_SendCmd(AUDIO_CMD_SHUFFLESET, (void *)AUDIO_INTURN, SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set SHUFFLE_OFF fail");
                }
            }
            gSysConfig.MusicConfig.PlayOrder = AUDIO_INTURN;
            System_GoToFatherList();
            break;

        case SID_MUSIC_SHUFFLE_ON:
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                ret = AudioControlTask_SendCmd(AUDIO_CMD_SHUFFLESET, (void *)AUDIO_RAND, SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set SHUFFLE_ON  fail");
                }
            }
            gSysConfig.MusicConfig.PlayOrder = AUDIO_RAND;
            System_GoToFatherList();
            break;

        case SID_EQ_NONE:
        case SID_EQ_BASS:
        case SID_EQ_HEAVY:
        case SID_EQ_POP:
        case SID_EQ_JAZZ:
        case SID_EQ_UNIQUE:
        case SID_EQ_CUSTOM:
            CurEqMode = menuTextID - SID_EQ_NONE;
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                ret = AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EqMode_Value[CurEqMode], SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set SHUFFLE_ON  fail");
                }
            }

            #ifdef _RK_EQ_
            gSysConfig.MusicConfig.Eq.Mode = EqMode_Value[CurEqMode];
            #endif
            /*System_GoToFatherList();*/
            bl_parameter.itemNum = SID_EQ_CUSTOM - SID_EQ_NONE+1;//7
            GuiTask_OperWidget(gpstSystemSetData->hSelect, OPERATE_SET_CONTENT, &bl_parameter, SYNC_MODE);
            break;

        case SID_SCREEN_OFF_15_SEC:
        case SID_SCREEN_OFF_30_SEC:
        case SID_SCREEN_OFF_1_MIN:
        case SID_SCREEN_OFF_3_MIN:
        case SID_SCREEN_OFF_5_MIN:
        case SID_SCREEN_OFF_30_MIN:
            gSysConfig.BLtime = menuTextID-SID_SCREEN_OFF_15_SEC+1;
            gSysConfig.SysIdle1EventTime = BackLight_Time[gSysConfig.BLtime-1];
            /*System_GoToFatherList();*/
            bl_parameter.itemNum = SID_SCREEN_OFF_30_MIN-SID_SCREEN_OFF_15_SEC+1;//6;
            GuiTask_OperWidget(gpstSystemSetData->hSelect, OPERATE_SET_CONTENT, &bl_parameter, SYNC_MODE);
            break;

        case SID_LUM_1:
        case SID_LUM_2:
        case SID_LUM_3:
        case SID_LUM_4:
        case SID_LUM_5:
            hPwmDev = RKDev_Open(DEV_CLASS_PWM, 0, NOT_CARE);
            if (hPwmDev != NULL)
            {
                gSysConfig.BLevel = menuTextID-SID_LUM_1+1;
                PwmDev_SetRate(hPwmDev,BackLight_LevelValue[gSysConfig.BLevel-1]);
                RKDev_Close(hPwmDev);
                /*
                gpstSystemSetData->blevel_set = 0;
                System_GoToFatherList();
                */
                bl_parameter.itemNum = SID_LUM_5-SID_LUM_1+1;//5
                GuiTask_OperWidget(gpstSystemSetData->hSelect, OPERATE_SET_CONTENT, &bl_parameter, SYNC_MODE);
            }
            else
            {
                rk_printf("Open PWM 0 fail");
            }
            break;

        case SID_CHINESE_S:
        case SID_CHINESE_T:
        case SID_ENGLISH:
        case SID_KOREAN:
        case SID_JAPANESE:
        case SID_FRENCH:
        case SID_GERMAN:
        case SID_PORTUGUESE:
        case SID_RUSSIAN:
        case SID_SWEDISH:
        case SID_THAI:
        case SID_POLISH:
        case SID_DENISH:
        case SID_HOLLANDS:
        case SID_GREEK:
        case SID_CZECHIC:
        case SID_TURKISH:
        //case SID_HEBREW:
        //case SID_ARABIC:
            gSysConfig.SysLanguage = MultipleLanguages[menuTextID - SID_CHINESE_S];
            System_GoToFatherList();
            break;

        default:
            break;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SystemTask_ChangeSelectList
** Input:uint32 SelectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.4
** Time: 17:49:12
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t SystemTask_ChangeSelectList(uint32 SelectID, uint32 CursorID)
{
    MENU_TEXT_INFO_STRUCT SetMenuTxtInfo;
    RKGUI_SELECT_ARG stSelectArg;
    rk_err_t ret;
    HDC hPwmDev = NULL;

    gpstSystemSetData->curSelectID = SelectID;

    {
        if(FW_GetMenuInfoWithIDNum(SelectID,&SetMenuTxtInfo) != RK_SUCCESS)
        {
            rk_printf("!!! FW read error\n");
            return RK_SUCCESS;
        }
    }

    if(SetMenuTxtInfo.ChildID == 0xFFFF)
    {
        //rk_printf("!!!will callback SelectID =0x%x \n",SelectID);
        SystemTask_EvnetOper(SelectID);
        return RK_SUCCESS;
    }
    memcpy(&gpstSystemSetData->SetMenuTxtInfo, &SetMenuTxtInfo, sizeof(MENU_TEXT_INFO_STRUCT));

    stSelectArg.x = 0;
    stSelectArg.y = 20;
    stSelectArg.xSize = 128;
    stSelectArg.ySize = 137;
    stSelectArg.display = 1;
    stSelectArg.level = 0;

    stSelectArg.Background = IMG_ID_BROWSER_BACKGROUND;
    stSelectArg.SeekBar = IMG_ID_BROWSER_SCOLL2;

    if(CursorID > 7)
    {
        stSelectArg.Cursor = 7;
        stSelectArg.ItemStartOffset = CursorID-7;
        //printf("......CursorOffset = %d\n", stSelectArg.CursorOffset);
    }
    else
    {
        stSelectArg.Cursor = CursorID;
        stSelectArg.ItemStartOffset= 0;
    }


    if(SelectID == SID_BLUETOOTH)
    {
        if(MainTask_GetStatus(MAINTASK_BT_OK))
        {
            stSelectArg.Cursor = 0;
        }
        else
        {
            stSelectArg.Cursor = 1;
        }
    }
    else if(SelectID == SID_WIFICONFIG)
    {
        if(MainTask_GetStatus(MAINTASK_WIFI_OPEN_OK))
        {
            stSelectArg.Cursor = 0;
        }
        else if(MainTask_GetStatus(MAINTASK_WIFI_AP_OPEN_OK))
        {
            stSelectArg.Cursor = 1;
        }
        else
        {
            stSelectArg.Cursor = 2;
        }
    }
    else if(SelectID == SID_MUSIC_REPEAT_MODE)
    {
        if(gSysConfig.MusicConfig.RepeatMode < SID_MUSIC_REPEAT_ONE - SID_MUSIC_REPEAT_MODE)
        {
            stSelectArg.Cursor = gSysConfig.MusicConfig.RepeatMode;
        }
        else
        {
            stSelectArg.Cursor = 0;
        }
    }
    else if(SelectID == SID_MUSIC_SHUFFLE)
    {
        if(gSysConfig.MusicConfig.PlayOrder < SID_MUSIC_SHUFFLE_ON - SID_MUSIC_SHUFFLE)
        {
            stSelectArg.Cursor = gSysConfig.MusicConfig.PlayOrder;
        }
        else
        {
            stSelectArg.Cursor = 1;
        }
    }
    else if(SelectID == SID_EQ_SELECT)
    {
        gpstSystemSetData->eqmode_set = 1;
    }
    else if (SelectID == SID_SETTING_MUSIC)
    {
        if (gpstSystemSetData->eqmode_set == 1)
        {
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                #ifdef RK_EQ_
                ret = AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)gSysConfig.MusicConfig.Eq.Mode, SYNC_MODE);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("set SHUFFLE_ON  fail");
                }
                #endif
            }
        }
        gpstSystemSetData->eqmode_set = 0;
    }
    else if (SelectID == SID_COMMON_BRIGHTNESS)
    {
        //entry backlight level set
        gpstSystemSetData->blevel_set = 1;
        stSelectArg.Cursor = gSysConfig.BLevel - 1;
    }
    else if (SelectID == SID_COMMON_SCREEN_OFF_TIMER)
    {
        //entry screen off time set
        gpstSystemSetData->btime_set = 1;
        stSelectArg.Cursor = gSysConfig.BLtime - 1;
    }
    else if (SelectID == SID_SETTING_COMMON)
    {
        if (gpstSystemSetData->blevel_set == 1)
        {
            hPwmDev = RKDev_Open(DEV_CLASS_PWM, 0, NOT_CARE);
            if (hPwmDev != NULL)
            {
                PwmDev_SetRate(hPwmDev,BackLight_LevelValue[gSysConfig.BLevel-1]);
                RKDev_Close(hPwmDev);
            }
            else
            {
                rk_printf("Open PWM 0 fail");
            }
        }
        gpstSystemSetData->blevel_set = 0;

        if (gpstSystemSetData->btime_set == 1)
        {

        }
        gpstSystemSetData->btime_set = 0;
    }

    stSelectArg.itemNum = gpstSystemSetData->SetMenuTxtInfo.ChildNum;
    stSelectArg.MaxDisplayItem= MAX_DISP_ITEM_NUM;

    stSelectArg.pReviceMsg = SystemTask_SelectCallBack;
    stSelectArg.CursorStyle= IMG_ID_SEL_ICON;
    stSelectArg.IconBoxSize= 16;

    gpstSystemSetData->lastFlag = 0;
    //if(SetMenuTxtInfo.ChildID != 0xFFFF)
    {
        if(FW_GetMenuInfoWithIDNum(gpstSystemSetData->SetMenuTxtInfo.ChildIDArray[0],&SetMenuTxtInfo) != RK_SUCCESS)
        {
            rk_printf("!!! FW read error\n");
            return RK_ERROR;
        }
        if(SetMenuTxtInfo.ChildID == 0xFFFF)
        {
            gpstSystemSetData->lastFlag = 1;
        }
    }
    #ifdef _USE_GUI_
    //rk_printf("###cur itemNum = %d\n",stSelectArg.itemNum);
    if(gpstSystemSetData->hSelect)
    {
        if(GuiTask_DeleteWidget(gpstSystemSetData->hSelect) != RK_SUCCESS)
        {
           rk_printf("\n -- hSelect dele NG... \n");
        }
    }
    gpstSystemSetData->hSelect = GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
    if(gpstSystemSetData->hSelect == NULL)
    {
        rk_printf("error:pGc == NULL\n");
    }
    #endif
    //GuiTask_OperWidget(gpstSystemSetData->hSelect, OPERATE_DISPLAY, NULL, SYNC_MODE);
    //GuiTask_DeleteWidget(gpstSystemSetData->hSelect);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: SystemTask_SelectCallBack
** Input:uint32 event_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.16
** Time: 16:15:34
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_COMMON_
COMMON FUN rk_err_t SystemTask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, RKGUI_SELECT_ITEM * arg, int offset)
{
    RKGUI_SELECT_ITEM * item;
    rk_err_t ret = 0;
    SYSTEMSET_ASK_QUEUE systemSetAskQueue;
    SYSTEMSET_ASK_QUEUE systemSetAskQueue_tmp;
    int Cursor;

    //printf("system offset = %d event_type=0x%x arg=0x%x\n",offset,event_type, (uint32)arg);
    switch(event_type)
    {
        case SELECT_ENVEN_ENTER:
        {
            systemSetAskQueue.event = NULL;
            systemSetAskQueue.event_type = SYSTEM_SET_ENTER;
            systemSetAskQueue.offset = offset;
            //printf("send offset %d 0x%x\n",systemSetAskQueue.offset,systemSetAskQueue.event_type);
            ret = rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
            if(ret == RK_ERROR)
            {
                printf("send enter enven\n");
                rkos_queue_receive(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue_tmp, 0);
                rkos_queue_send(gpstSystemSetData->SystemSetAskQueue, &systemSetAskQueue, 0);
            }
        }
        break;

        case SELECT_ENVEN_UPDATA:
        {
            item = (RKGUI_SELECT_ITEM *)arg;

            item->cmd = TEXT_CMD_ID;
            item->text_id = gpstSystemSetData->SetMenuTxtInfo.ChildIDArray[offset];
            //eq mode
            if (gpstSystemSetData->eqmode_set == 1)
            {
                #ifdef _RK_EQ_
                if(gSysConfig.MusicConfig.Eq.Mode == EQ_NOR)
                {
                    Cursor = 0;
                }
                else if(gSysConfig.MusicConfig.Eq.Mode == EQ_BASS)
                {
                    Cursor = 1;
                }
                else if(gSysConfig.MusicConfig.Eq.Mode == EQ_HEAVY)
                {
                    Cursor = 2;
                }
                else if(gSysConfig.MusicConfig.Eq.Mode == EQ_POP)
                {
                    Cursor = 3;
                }
                else if(gSysConfig.MusicConfig.Eq.Mode == EQ_JAZZ)
                {
                    Cursor = 4;
                }
                else if(gSysConfig.MusicConfig.Eq.Mode == EQ_UNIQUE)
                {
                    Cursor = 5;
                }
                else if(gSysConfig.MusicConfig.Eq.Mode == EQ_USER)
                {
                    Cursor = 6;
                }
                #endif

                rk_printf ("Cursor=%d offset=%d\n",Cursor,offset);
                if (offset == Cursor)
                {
                    item->sel_icon = IMG_ID_ICON_SEL;
                    item->unsel_icon = IMG_ID_ICON_SEL;
                }
                else
                {
                    item->sel_icon = IMG_ID_PONIT_NOSEL;
                    item->unsel_icon = IMG_ID_PONIT_NOSEL;
                }

                break;
            }

            //backlight level
            if (gpstSystemSetData->blevel_set == 1)
            {
                if (offset+1==gSysConfig.BLevel)
                {
                    item->sel_icon = IMG_ID_ICON_SEL;
                    item->unsel_icon = IMG_ID_ICON_SEL;
                }
                else
                {
                    item->sel_icon = IMG_ID_PONIT_NOSEL;
                    item->unsel_icon = IMG_ID_PONIT_NOSEL;
                }
                break;
            }
            //screen off time
            if (gpstSystemSetData->btime_set == 1)
            {
                if (offset+1==gSysConfig.BLtime)
                {
                    item->sel_icon = IMG_ID_ICON_SEL;
                    item->unsel_icon = IMG_ID_ICON_SEL;
                }
                else
                {
                    item->sel_icon = IMG_ID_PONIT_NOSEL;
                    item->unsel_icon = IMG_ID_PONIT_NOSEL;
                }
                break;
            }
            //normal
            if((offset == 1) && (gpstSystemSetData->SetMenuTxtInfo.SelfID == SID_WLAN))
            {
                item->sel_icon = IMG_ID_ICON_SEL;
                item->unsel_icon = IMG_ID_PONIT_NOSEL;
            }
            else if(gpstSystemSetData->lastFlag == 1)
            {
                item->sel_icon = IMG_ID_ICON_SEL;
                item->unsel_icon = IMG_ID_PONIT_NOSEL;
            }
            else
            {
                item->sel_icon = -1;
                item->unsel_icon = IMG_ID_SETMENU_ICON_NOSEL;
            }
        }
        break;

        case SELECT_ENVEN_MOVE:
            System_SelectEventMove(offset);
            break;

        default:
            break;
    }

    return RK_SUCCESS;
}
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SystemSetTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.16
** Time: 10:14:20
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_INIT_
INIT API rk_err_t SystemSetTask_DeInit(void *pvParameters)
{
    rk_err_t ret;
    gpstSystemSetData->deleteTaskFlag = 1;
    if(gpstSystemSetData->smartConfigStart)
    {
        gpstSystemSetData->smartConfigStart = AP_CONFIGOFF;
        #ifdef _DRIVER_WIFI__
        rk_easy_smartconfig_stop();
        #endif
    }

#ifdef __APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_C__
    if(RKTaskFind(TASK_ID_SYSTEMSETWIFISELECT, 0) != NULL)
    {
        SystemSetWifiSelectTask_Delete();
    }
    #endif
#ifdef _USE_GUI_
    if(gpstSystemSetData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstSystemSetData->hMsgBox, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstSystemSetData->hMsgBox);
        gpstSystemSetData->hMsgBox = NULL;
    }
    if(gpstSystemSetData->hWait != NULL)
    {
        GuiTask_OperWidget(gpstSystemSetData->hWait, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstSystemSetData->hWait);
        gpstSystemSetData->hWait = NULL;
    }
    if(gpstSystemSetData->hSelect != NULL)
    {
        //GuiTask_OperWidget(gpstSystemSetData->hSelect, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstSystemSetData->hSelect);
        gpstSystemSetData->hSelect = NULL;
    }
    MainTask_SetStatus(MAINTASK_APP_SYSTEMSET,0);
    GuiTask_AppUnReciveMsg(SystemSetTask_ButtonCallBack);

    rkos_queue_delete(gpstSystemSetData->SystemSetAskQueue);
    rkos_queue_delete(gpstSystemSetData->SystemSetRespQueue);
    rkos_memory_free(gpstSystemSetData);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_SYSTEMSET_TASK);
#endif
    gpstSystemSetData = NULL;
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SystemSetTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.16
** Time: 10:14:20
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETTASK_INIT_
INIT API rk_err_t SystemSetTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_SYSTEMSET_ARG * pArg = (RK_TASK_SYSTEMSET_ARG *)arg;
    RK_TASK_CLASS* pSystemSetTask = (RK_TASK_CLASS*)pvParameters;
    SYSTEMSET_TASK_DATA_BLOCK*  pSystemSetTaskData;

    if(pSystemSetTask == NULL)
    {
        return RK_PARA_ERR;
    }

    pSystemSetTaskData = rkos_memory_malloc(sizeof(SYSTEMSET_TASK_DATA_BLOCK));
    if(pSystemSetTaskData == NULL)
    {
        return RK_ERROR;
    }
    memset(pSystemSetTaskData, NULL, sizeof(SYSTEMSET_TASK_DATA_BLOCK));
    pSystemSetTaskData->SystemSetAskQueue = rkos_queue_create(1, sizeof(SYSTEMSET_ASK_QUEUE));
    pSystemSetTaskData->SystemSetRespQueue = rkos_queue_create(1, sizeof(SYSTEMSET_RESP_QUEUE));


#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_SYSTEMSET_TASK, SEGMENT_OVERLAY_ALL);
#endif

    gpstSystemSetData = pSystemSetTaskData;
    gpstSystemSetData->hSelect = NULL;
    gpstSystemSetData->hWait = NULL;
    gpstSystemSetData->smartConfigStart = 0;
    gpstSystemSetData->deleteTaskFlag = 0;
    gpstSystemSetData->hMsgBox = NULL;
    gpstSystemSetData->curCmd = 0XFF;
    gpstSystemSetData->blevel_set = 0;
    gpstSystemSetData->btime_set = 0;
    gpstSystemSetData->eqmode_set = 0;
#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(SystemSetTask_ButtonCallBack);
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
