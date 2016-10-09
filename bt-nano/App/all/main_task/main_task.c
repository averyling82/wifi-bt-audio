/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\main_task\main_task.c
* Owner: aaron.sun
* Date: 2015.8.20
* Time: 17:28:49
* Version: 1.0
* Desc: main task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.8.20     17:28:49   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_MAIN_TASK_MAIN_TASK_C__

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

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _MAIN_RESP_QUEUE
{
    uint32 err_code;

}MAIN_RESP_QUEUE;

typedef  struct _MAIN_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;


}MAIN_ASK_QUEUE;

typedef  struct _MAIN_TASK_DATA_BLOCK
{
    pQueue  MainAskQueue;
    pQueue  MainRespQueue;
    pSemaphore osMainOperSem;
    pSemaphore osMainSysEventSem;
    pSemaphore osMainKeySem;
    int32   MenuId;

    //MainTask Control Params
    uint8   keycontrol;
    uint8   shutdown;
    HDC     hMsg;
    #ifndef _USE_GUI_
    HDC     hKey;
    uint32  keyvalue;
    rk_err_t (*pKeyFunc)(uint32);
    #ifdef NOSCREEN_USE_LED
    uint8   greenled_state;
    uint8   redled_state;
    #endif
    #endif

    //GUI Handle
    HGC     hBackGround;
    HGC     hplayerIcon;
    HGC     hchargeIcon;
    HGC     hbatteryIcon;
    HGC     hbtIcon;
    HGC     hwifiIcon;
    HGC     hwifiConnect;
    HGC     hSysStatus;
    HGC     hPlayState;
    HGC     hMusicName;
    HGC     hWelcom;
    HGC     hMainMenu;

    TASK_SWTICH_ARG  stSwtichs;

}MAIN_TASK_DATA_BLOCK;

typedef enum _MAIN_TASK_SUB_THREAD
{
    MUSIC_SUB_THREAD,
    VEDIO_SUB_THREAD,
    FM_SUB_THREAD,
    PHOTO_SUB_THREAD,
    BOOK_SUB_THREAD,
    RECOD_SUB_THREAD,
    BROWSER_SUB_THREAD,
    GAME_SUB_THREAD,
    SETMENU_SUB_THREAD,
    SUB_THREAD_MAX

}MAIN_TASK_SUB_THREAD;


//#define MAINTASK_DEBUG(format,...) printf("\n[Main_task.c]:(Line=%d)--------"format, __LINE__, ##__VA_ARGS__)
#define MAINTASK_DEBUG(format,...)
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static MAIN_TASK_DATA_BLOCK * gpstMainData;

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
rk_err_t  MainTask_Resume(HTC hTask);
rk_err_t  MainTask_Suspend(HTC hTask, uint32 Level);
rk_err_t MainTaskCheckIdle(HTC hTask);
#if 1
int32 MainTask_StartSubThread (uint32 MenuId);
int32 MainTask_SetWindow (int32 MenuId);
int32 MainTask_SetName (int32 MenuId);
int32 MainTask_DeleteAllApp(void);
int32 MainTask_DiaplayElectricPower (int flag);
int32 MainTask_ShutDown (int flag);
int32 MainTask_GuiInit(void);
int32 MainTask_GuiDeInit(void);
int32 MainTask_Animation ();
int32 MainTask_SetTopPrompt (int params);
int32 MainTask_HandleSysEvent(uint32 event);
int32 MainTask_HandleKey(uint32 event);
int32 MainTask_HandleAudioEvent(uint32 event);
int32 MainTask_NoScreenHandleKey(uint32 keyvalue);

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MainTask_GetStatus
** Input:uint32 cmd
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API uint8 MainTask_GetStatus (uint32 StatusID)
{
    rk_err_t ret;

    ret = MsgDev_CheckMsg(gpstMainData->hMsg, StatusID);
    if(ret == RK_SUCCESS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*******************************************************************************
** Name: MainTask_SetStatus
** Input:uint32 cmd, uint8 status
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API uint8 MainTask_SetStatus (uint32 StatusID, uint8 status)
{

    if(status)
    {
        MainTask_SetTopIcon(StatusID);
        MsgDev_SendMsg(gpstMainData->hMsg, StatusID);
    }
    else
    {
        MsgDev_ClearMsg(gpstMainData->hMsg, StatusID);
        MainTask_DeleteTopIcon(StatusID);
    }



    return RK_SUCCESS;
}




/*******************************************************************************
** Name: MainTask_SysEventCallBack
** Input:uint32 event, void*arg
** Return: rk_err_t
** Owner:wrm
** Date: 2015.10.15
** Time: 17:30:03
** Function: Other Window Switch  to Main Task
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API rk_err_t MainTask_SysEventCallBack(uint32 event, void*arg)
{
    HGC pGc;
    MAIN_ASK_QUEUE EventAskQueue;
    MAIN_ASK_QUEUE EventAskQueue_tmp;
    rk_err_t ret;
    int i = 0;
    rkos_semaphore_take(gpstMainData->osMainSysEventSem, MAX_DELAY);
    EventAskQueue.event = event;//MAINTASK_USBCREATE_CMD
    EventAskQueue.event_type = MAINTASK_SYS_EVENT;//MAINTASK_EVENT
    //shutdown
    if ((gpstMainData->shutdown == 1)&&(event == MAINTASK_SHUTDOWN))
    {
        rkos_semaphore_give(gpstMainData->osMainSysEventSem);
        return RK_SUCCESS;
    }
    else if ((gpstMainData->shutdown == 0)&&(event == MAINTASK_SHUTDOWN))
    {
        gpstMainData->shutdown == 1;
    }
    //usb
    if ((event == MAINTASK_USBCREATE)||(event == MAINTASK_USBDELETE))
    {
        for (i=0; i<5; i++)
        {
            ret = rkos_queue_receive(gpstMainData->MainAskQueue, &EventAskQueue_tmp, 0);
            if (ret == RK_ERROR)
            {
                continue;
            }
            if ((EventAskQueue_tmp.event == MAINTASK_USBCREATE)||(EventAskQueue_tmp.event == MAINTASK_USBDELETE))
            {
                ret = rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue, 0);
                if (ret == RK_ERROR)
                {
                    MAINTASK_DEBUG("A\n");
                }
                //MAINTASK_DEBUG("A BREAK:event=%d \n",EventAskQueue_tmp.event);
                break;
            }
            else
            {
                //MAINTASK_DEBUG ("----event=%d event_type=%d\n",EventAskQueue_tmp.event,EventAskQueue_tmp.event_type);
                ret = rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue_tmp, 0);
                if (ret == RK_ERROR)
                {
                    MAINTASK_DEBUG("B\n");
                }
            }
        }
        if (i==5)
        {
            ret = rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue, 0);
            if (ret == RK_ERROR)
            {
                MAINTASK_DEBUG("C\n");
            }
        }
        rkos_semaphore_give(gpstMainData->osMainSysEventSem);
        return RK_SUCCESS;
    }
    //MAINTASK_DEBUG ("----event=%d event_type=%d\n",EventAskQueue.event,EventAskQueue.event_type);
    ret = rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue, 0);
    if (ret == RK_ERROR)
    {
        rkos_queue_receive(gpstMainData->MainAskQueue, &EventAskQueue_tmp, 0);
        MAINTASK_DEBUG ("0----type=%d event=%d\n",EventAskQueue_tmp.event_type,EventAskQueue_tmp.event);
        //EventAskQueue.event_type = MAINTASK_EVENT;//MAINTASK_EVENT
        if (EventAskQueue_tmp.event_type == GUI_EVENT_KEY)
        {
            //丢按键
            rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue, 0);
            MAINTASK_DEBUG ("1----type=%d evnet=%d\n",EventAskQueue.event_type,EventAskQueue.event);
        }
        else
        {
            //丢本次事件
            rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue_tmp, 0);
            MAINTASK_DEBUG ("2----type=%d cmd=%d\n",EventAskQueue.event_type,EventAskQueue.event);
        }
    }
    rkos_semaphore_give(gpstMainData->osMainSysEventSem);
    return RK_SUCCESS;
}

#ifndef _USE_GUI_
/*******************************************************************************
** Name: MainTask_RegisterKey
** Input:pKeyFunc func
** Return: rk_err_t
** Owner:wrm
** Date: 2016.3.22
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API rk_err_t MainTask_RegisterKey(pKeyFunc func)
{
    gpstMainData->pKeyFunc = func;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_UnRegisterKey
** Input:void
** Return: rk_err_t
** Owner:wrm
** Date: 2016.3.22
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API rk_err_t MainTask_UnRegisterKey(void)
{
    gpstMainData->pKeyFunc = NULL;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_KeyCallBack
** Input:uint32 keyvalue
** Return: rk_err_t
** Owner:wrm
** Date: 2016.3.22
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API rk_err_t MainTask_KeyCallBack(uint32 keyvalue)
{
    rk_err_t ret;
    MAIN_ASK_QUEUE MAskQueue;

    if(gpstMainData->pKeyFunc != NULL)
    {
        //printf ("--------keyvalue -> app:\n");
        gpstMainData->pKeyFunc(keyvalue);
        return RK_SUCCESS;
    }

    rkos_semaphore_take(gpstMainData->osMainKeySem, MAX_DELAY);
    if (keyvalue==0)
    {
        if (gpstMainData->keyvalue == 0)
        {
            rkos_semaphore_give(gpstMainData->osMainKeySem);
            return RK_SUCCESS;
        }
        else
        {
            goto NEXT;
        }
    }


    gpstMainData->keyvalue = keyvalue;
    if (gpstMainData->keycontrol == 1)
    {
        rkos_semaphore_give(gpstMainData->osMainKeySem);
        return RK_SUCCESS;
    }

NEXT:
    MAskQueue.event = gpstMainData->keyvalue;
    MAskQueue.event_type = MAINTASK_KEY_EVENT;
    gpstMainData->keyvalue = 0;
    gpstMainData->keycontrol = 1;
    ret = rkos_queue_send(gpstMainData->MainAskQueue, &MAskQueue, 0);
    if (ret == RK_ERROR)
    {
        gpstMainData->keycontrol = 0;
    }

    rkos_semaphore_give(gpstMainData->osMainKeySem);

    return RK_SUCCESS;
}
#ifdef NOSCREEN_USE_LED
/*******************************************************************************
** Name: MainTask_SetLED
** Input:int8 led,int8 led_state
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API uint8 MainTask_SetLED (int8 led,int8 led_state)
{
    if (led == MAINTASK_LED1)//red led
    {
        switch (led_state)
        {
            case MAINTASK_LED_ENABLE:
                #if 1
                Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin5, 0); // led 1(红灯)
                Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin5, GPIO_OUT);
                Gpio_SetPinLevel(GPIO_CH2, GPIOPortB_Pin5, GPIO_HIGH); //red off
                Grf_GPIO_SetPinPull(GPIO_CH2, GPIOPortB_Pin5, DISABLE);
                #else
                printf ("<-----------------red led init-------------->\n");
                #endif
                break;
            case MAINTASK_LED_ON:
                if (gpstMainData->redled_state != 1)
                {
                    #if 1
                    Gpio_SetPinLevel(GPIO_CH2, GPIOPortB_Pin5, GPIO_LOW); //red on
                    Grf_GPIO_SetPinPull(GPIO_CH2, GPIOPortB_Pin5, DISABLE);
                    #else
                    printf ("<-----------------red led on-------------->\n");
                    #endif
                }
                gpstMainData->redled_state = 1;
                break;

            case MAINTASK_LED_OFF:
                if (gpstMainData->redled_state != 0)
                {
                    #if 1
                    Gpio_SetPinLevel(GPIO_CH2, GPIOPortB_Pin5, GPIO_HIGH); //red off
                    Grf_GPIO_SetPinPull(GPIO_CH2, GPIOPortB_Pin5, DISABLE);
                    #else
                    printf ("<-----------------red led off-------------->\n");
                    #endif
                }
                gpstMainData->redled_state = 0;
                break;
            default:
                break;

        }
        return RK_SUCCESS;
    }
    if (led == MAINTASK_LED2)//green led
    {
        switch (led_state)
        {
            case MAINTASK_LED_ENABLE:
                #if 1
                Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin4, 0); // led 2(绿灯灯)
                Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin4, GPIO_OUT);
                Gpio_SetPinLevel(GPIO_CH2, GPIOPortB_Pin4, GPIO_LOW); //green off
                Grf_GPIO_SetPinPull(GPIO_CH2, GPIOPortB_Pin4, DISABLE);
                #else
                printf ("<-----------------green led init-------------->\n");
                #endif
                break;
            case MAINTASK_LED_ON:
                if (gpstMainData->greenled_state != 1)
                {
                    #if 1
                    Gpio_SetPinLevel(GPIO_CH2, GPIOPortB_Pin4, GPIO_HIGH); //green off
                    Grf_GPIO_SetPinPull(GPIO_CH2, GPIOPortB_Pin4, DISABLE);
                    #else
                    printf ("<-----------------green led on-------------->\n");
                    #endif
                }
                gpstMainData->greenled_state = 1;
                break;

            case MAINTASK_LED_OFF:
                if (gpstMainData->greenled_state != 0)
                {
                    #if 1
                    Gpio_SetPinLevel(GPIO_CH2, GPIOPortB_Pin4, GPIO_LOW); //green off
                    Grf_GPIO_SetPinPull(GPIO_CH2, GPIOPortB_Pin4, DISABLE);
                    #else
                    printf ("<-----------------green led off-------------->\n");
                    #endif
                }
                gpstMainData->greenled_state = 0;
                break;
            default:
                break;

        }
        return RK_SUCCESS;
    }
}

#endif //NOSCREEN_USE_LED end

#endif //No _USE_GUI_ end
/*******************************************************************************
** Name: MainTask_GuiCallBack
** Input:uint32 event_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
COMMON API rk_err_t MainTask_GuiCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, uint32 mode)
{
    HGC pGc;
    MAIN_ASK_QUEUE GuiAskQueue;
    MAIN_ASK_QUEUE GuiAskQueue_tmp;
    rk_err_t ret;

    if (gpstMainData->keycontrol == 1)
    {
        //MAINTASK_DEBUG ("---------lose ButtonCallBack Because EventBack\n");
        return RK_SUCCESS;
    }

    gpstMainData->keycontrol = 1;
    if (event_type != APP_RECIVE_MSG_EVENT_KEY)
    {
        gpstMainData->keycontrol = 0;
        return RK_ERROR;
    }

    GuiAskQueue.event = event;
    GuiAskQueue.event_type = MAINTASK_KEY_EVENT;
    ret = rkos_queue_send(gpstMainData->MainAskQueue, &GuiAskQueue, 0);
    if (ret == RK_ERROR)
    {
        gpstMainData->keycontrol = 0;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MainTask_AudioCallBack
** Input:uint32 audio_state
** Return: void
** Owner:wrm
** Date: 2015.10.15
** Time: 17:30:03
** Function: Other Window Switch  to Main Task
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API void MainTask_AudioCallBack(uint32 audio_state)
{
    rk_err_t ret;
    MAIN_ASK_QUEUE MainAskQueue;

    switch(audio_state)
    {
        case AUDIO_STATE_ERROR:
        case AUDIO_STATE_CUSTOM:
        case AUDIO_STATE_MUSIC_CHANGE:
        case AUDIO_STATE_PAUSE:
        case AUDIO_STATE_PLAY:
        case AUDIO_STATE_FFD:
        case AUDIO_STATE_FFW:
        case AUDIO_STATE_DELETE:
        {
            MainAskQueue.event_type = MAINTASK_AUDIO_EVENT;
            MainAskQueue.event = audio_state;
            rkos_queue_send(gpstMainData->MainAskQueue, &MainAskQueue, MAX_DELAY);
        }
        break;
    }

    return ;
}


/*******************************************************************************
** Name: MainTask_TaskSwtich
** Input:void
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API int32 TaskSwtich (void * arg)
{
    TASK_SWTICH_ARG * pstTaskS = (TASK_SWTICH_ARG * )arg;

    //rk_printf("task swtich id = %d",  pstTaskS->TargetClassId);

    if(pstTaskS->CurClassId == TASK_ID_MAIN)
    {
        #ifdef _USE_GUI_
        GuiTask_AppUnReciveMsg(MainTask_GuiCallBack);
        #endif
        MainTask_GuiDeInit();
        FW_RemoveSegment(SEGMENT_ID_MAIN_MENU);
    }
    else
    {
        RKTaskDelete(pstTaskS->CurClassId, pstTaskS->CurObjectId, SYNC_MODE);
    }

    if(pstTaskS->TargetClassId == TASK_ID_MAIN)
    {
        //rk_printf("task swtich id");
        FW_LoadSegment(SEGMENT_ID_MAIN_MENU, SEGMENT_OVERLAY_ALL);
        MainTask_GuiInit();
        MainTask_SetWindow(gpstMainData->MenuId);
        #ifdef _USE_GUI_
        GuiTask_AppReciveMsg(MainTask_GuiCallBack);
        #endif
    }
    else
    {
        RKTaskCreate(pstTaskS->TargetClassId, pstTaskS->TargetObjectId, pstTaskS->targ, SYNC_MODE);
    }

    if(pstTaskS->targ != NULL)
    {
        rkos_memory_free(pstTaskS->targ);
    }
}


/*******************************************************************************
** Name: MainTask_TaskSwtich
** Input:void
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API int32 MainTask_TaskSwtich (uint32 CurClassId, uint32 CurObjectId, uint32 TargetClassId, uint32 TargetObjectId,  void *targ)
{
     MAIN_ASK_QUEUE EventAskQueue;
     rk_err_t ret;
     EventAskQueue.event = NULL;
     EventAskQueue.event_type = MAINTASK_TASK_SWTICH;
     gpstMainData->stSwtichs.CurClassId = CurClassId;
     gpstMainData->stSwtichs.CurObjectId = CurObjectId;
     gpstMainData->stSwtichs.TargetClassId = TargetClassId;
     gpstMainData->stSwtichs.TargetObjectId = TargetObjectId;

     if(targ != NULL)
     {
         gpstMainData->stSwtichs.targ = rkos_memory_malloc(sizeof(RK_TASK_ARG));
         memcpy(gpstMainData->stSwtichs.targ, targ, sizeof(RK_TASK_ARG));
     }
     else
     {
        gpstMainData->stSwtichs.targ = NULL;
     }
     ret = rkos_queue_send(gpstMainData->MainAskQueue, &EventAskQueue, MAX_DELAY);
     return ret;
}


/*******************************************************************************
** Name: MainTask_SetTopIcon
** Input:int cmd
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API int32 MainTask_SetTopIcon (uint32 StatusID)
{
    RKGUI_ICON_ARG pGcArg;
    RKGUI_CHAIN_ARG pstChainArg;
    RKGUI_TEXT_ARG pstTextArg;

    uint32 Icon[6] = {IMG_ID_RK_LOCALL_PLAYER, IMG_ID_RK_DLNA_PLAYER,IMG_ID_RK_BT_PLAYER,
        IMG_ID_RK_XXX_PLAYER,IMG_ID_RK_CHANNEL_PLAYER,IMG_ID_RK_RECORD};

    rkos_semaphore_take(gpstMainData->osMainOperSem, MAX_DELAY);

    switch (StatusID)
    {
        case MAINTASK_APP_LOCAL_PLAYER:
        case MAINTASK_APP_DLNA_PLAYER:
        case MAINTASK_APP_BT_PLAYER:
        case MAINTASK_APP_XXX_PLAYER:
        case MAINTASK_APP_CHANNEL_PLAYER:
        case MAINTASK_APP_RECORD:
            #ifdef _USE_GUI_
            if(gpstMainData->hplayerIcon == NULL)
            {
                pGcArg.x= 2;
                pGcArg.y= 1;
                pGcArg.display = 1;
                pGcArg.level= 1;
                pGcArg.resource = Icon[StatusID - MAINTASK_APP_LOCAL_PLAYER];
                gpstMainData->hplayerIcon= GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
                MAINTASK_DEBUG("Create hplayerIcon 0\n");
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hplayerIcon, OPERATE_SET_CONTENT, (void*)Icon[StatusID - MAINTASK_APP_LOCAL_PLAYER], SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hplayerIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
                MAINTASK_DEBUG("Create hplayerIcon 1\n");
            }
            #endif
            break;

        case MAINTASK_CHARGE:
            #ifdef _USE_GUI_
            if(gpstMainData->hchargeIcon == NULL)
            {
                pstChainArg.x= 100;
                pstChainArg.y= 1;
                pstChainArg.resource = IMG_ID_BATTERY01;
                pstChainArg.num= 5;
                pstChainArg.delay= 20;
                pstChainArg.level= 1;
                pstChainArg.display = 1;
                pstChainArg.blurry= 0;
                gpstMainData->hchargeIcon = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
            }
            else
            {
                MAINTASK_DEBUG("chargeIcon Display\n");
                GuiTask_OperWidget(gpstMainData->hchargeIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;
        case MAINTASK_BATTERY:
            #ifdef _USE_GUI_
            if(gpstMainData->hbatteryIcon == NULL)
            {
                pGcArg.x= 100;
                pGcArg.y= 1;
                pGcArg.display = 1;
                pGcArg.level= 1;
                pGcArg.resource = IMG_ID_BATTERY01 + gSysConfig.battery_level;
                gpstMainData->hbatteryIcon = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
            }
            else
            {
                pGcArg.resource = IMG_ID_BATTERY01 + gSysConfig.battery_level;
                GuiTask_OperWidget(gpstMainData->hbatteryIcon, OPERATE_SET_CONTENT, (void*)(pGcArg.resource), SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hbatteryIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_CONNECT_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiIcon == NULL)
            {
                pGcArg.x= 80;
                pGcArg.y= 1;
                pGcArg.display = 1;
                pGcArg.level= 1;
                pGcArg.resource = IMG_ID_STATE_WIFI0 + 4;
                gpstMainData->hwifiIcon = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_CONTENT, (void *)(IMG_ID_STATE_WIFI0 + 4), SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_AP_CONNECT_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiIcon == NULL)
            {
                pGcArg.x= 80;
                pGcArg.y= 1;
                pGcArg.display = 1;
                pGcArg.level= 1;
                pGcArg.resource = IMG_ID_AP;
                gpstMainData->hwifiIcon = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_CONTENT, (void *)IMG_ID_AP, SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_CONNECTING:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiConnect == NULL)
            {
                pstChainArg.x= 80;
                pstChainArg.y= 1;
                pstChainArg.resource = IMG_ID_STATE_WIFI0;
                pstChainArg.num = 5;
                pstChainArg.delay= 20;
                pstChainArg.level= 1;
                pstChainArg.display = 1;
                pstChainArg.blurry= 0;
                gpstMainData->hwifiConnect = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
            }
            else
            {
                 GuiTask_OperWidget(gpstMainData->hwifiConnect, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_OPEN_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiIcon == NULL)
            {
                pGcArg.x= 80;
                pGcArg.y= 1;
                pGcArg.display = 1;
                pGcArg.level= 1;
                pGcArg.resource = IMG_ID_STATE_WIFI0;
                gpstMainData->hwifiIcon = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_CONTENT, (void *)(IMG_ID_STATE_WIFI0), SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_AP_OPEN_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiIcon == NULL)
            {
                pGcArg.x= 80;
                pGcArg.y= 1;
                pGcArg.display = 1;
                pGcArg.level= 1;
                pGcArg.resource = IMG_ID_AP1;
                gpstMainData->hwifiIcon = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_CONTENT, (void *)(IMG_ID_AP1), SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
            }
            #endif
            break;



        case MAINTASK_BT_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hbtIcon == NULL)
            {
                 pGcArg.x= 80;
                 pGcArg.y= 1;
                 pGcArg.display = 1;
                 pGcArg.level= 1;
                 pGcArg.resource = IMG_ID_BT;
                 gpstMainData->hbtIcon = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
                 MAINTASK_DEBUG("Create hBtIcon 0\n");
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hbtIcon, OPERATE_SET_DISPLAY, (void*)1, SYNC_MODE);
                MAINTASK_DEBUG("Create hBtIcon 1\n");
            }
            #endif
            break;

        case MAINTASK_SYS_MEDIA_UPDATE:
        case MAINTASK_SYS_READY_SD:
            #ifdef _USE_GUI_
            if(gpstMainData->hSysStatus == NULL)
            {
                //rk_printf("display status");

                pstTextArg.lucency= OPACITY;
                pstTextArg.Backdrop= IMG_ID_STATE_BACKGROUND;
                pstTextArg.ForegroundR= 255;
                pstTextArg.ForegroundG= 255;
                pstTextArg.ForegroundB= 255;

                pstTextArg.BackgroundR= 0;
                pstTextArg.BackgroundG= 0;
                pstTextArg.BackgroundB= 0;
                pstTextArg.BackdropX= 0;
                pstTextArg.BackdropY= 0;

                pstTextArg.align= TEXT_ALIGN_X_CENTER;
                pstTextArg.cmd= TEXT_CMD_ID;
                pstTextArg.x= 0;
                pstTextArg.y= 2;
                pstTextArg.xSize= 90;
                pstTextArg.ySize= -1;
                pstTextArg.resource = (StatusID == MAINTASK_SYS_MEDIA_UPDATE)? SID_UPDATE_MEDIALIB:SID_READY_SD_CARD;
                pstTextArg.display = 1;
                pstTextArg.level= 1;

                gpstMainData->hSysStatus = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            }
            else
            {
                pstTextArg.opercmd = TEXT_SET_CONTENT;
                pstTextArg.resource = (StatusID == MAINTASK_SYS_MEDIA_UPDATE)? SID_UPDATE_MEDIALIB:SID_READY_SD_CARD;
                GuiTask_OperWidget(gpstMainData->hSysStatus, OPERATE_SET_CONTENT, &pstTextArg, SYNC_MODE);
            }
            #endif

        default:
            break;
    }

    rkos_semaphore_give(gpstMainData->osMainOperSem);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MainTask_SetTopIcon
** Input:int cmd
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API int32 MainTask_DeleteTopIcon (uint32 StatusID)
{
    rkos_semaphore_take(gpstMainData->osMainOperSem, MAX_DELAY);

    RKGUI_ICON_ARG pGcArg;

    switch (StatusID)
    {
        case MAINTASK_APP_LOCAL_PLAYER:
        case MAINTASK_APP_DLNA_PLAYER:
        case MAINTASK_APP_BT_PLAYER:
        case MAINTASK_APP_XXX_PLAYER:
        case IMG_ID_RK_CHANNEL_PLAYER:
        case IMG_ID_RK_RECORD:
            #ifdef _USE_GUI_
            MAINTASK_DEBUG("Delete player0\n");
            if(gpstMainData->hplayerIcon != NULL)
            {
                MAINTASK_DEBUG("Delete player1\n");
                GuiTask_OperWidget(gpstMainData->hplayerIcon, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_CHARGE:
            #ifdef _USE_GUI_
            MAINTASK_DEBUG("chargeIcon not Display\n");
            if(gpstMainData->hchargeIcon != NULL)
            {
                GuiTask_OperWidget(gpstMainData->hchargeIcon, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_BATTERY:
            #ifdef _USE_GUI_
            if(gpstMainData->hbatteryIcon != NULL)
            {
                GuiTask_OperWidget(gpstMainData->hbatteryIcon, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_CONNECT_OK:
        case MAINTASK_WIFI_AP_CONNECT_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiIcon != NULL)
            {
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_CONTENT, (void *)(IMG_ID_STATE_WIFI0), SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_CONNECTING:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiConnect != NULL)
            {
                GuiTask_OperWidget(gpstMainData->hwifiConnect, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_WIFI_OPEN_OK:
        case MAINTASK_WIFI_AP_OPEN_OK:
            #ifdef _USE_GUI_
            if(gpstMainData->hwifiIcon != NULL)
            {
                GuiTask_OperWidget(gpstMainData->hwifiIcon, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_BT_OK:
            #ifdef _USE_GUI_
            MAINTASK_DEBUG("Delete hBtIcon 0\n");
            if(gpstMainData->hbtIcon != NULL)
            {
                MAINTASK_DEBUG("hbtIcon not Display\n");
                GuiTask_OperWidget(gpstMainData->hbtIcon, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        case MAINTASK_SYS_MEDIA_UPDATE:
        case MAINTASK_SYS_READY_SD:
            #ifdef _USE_GUI_
            if(gpstMainData->hSysStatus != NULL)
            {
                GuiTask_OperWidget(gpstMainData->hSysStatus, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
            }
            #endif
            break;

        default:
            break;
    }

    rkos_semaphore_give(gpstMainData->osMainOperSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_Enter
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON API void MainTask_Enter(void)
{
    MAIN_ASK_QUEUE MainTaskAskQueue;
    HTC hSelf;
    hSelf = RKTaskGetRunHandle();


    MainTask_Animation(0);
    MainTask_GuiInit();

    MainTask_SetWindow(gpstMainData->MenuId);
#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(MainTask_GuiCallBack);
#else
    #ifdef NOSCREEN_USE_LED
    {
        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ENABLE);
        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ENABLE);
    }
    #endif
    gpstMainData->hKey = RKDev_Open(DEV_CLASS_KEY, 0, NOT_CARE);
    if(gpstMainData->hKey!= NULL)
    {
        KeyDev_Register(gpstMainData->hKey, KeySendMsg);
    }
    gpstMainData->pKeyFunc = NULL; //按键传送给MainTask
#endif

#ifndef _USE_GUI_
    if (Grf_CheckVbus()==0)
    {
        #ifdef _BLUETOOTH_
            MainTask_StartSubThread(FM_SUB_THREAD); //打开音乐线程
        #endif

        #ifdef _DRIVER_WIFI__
            MainTask_StartSubThread(VEDIO_SUB_THREAD); //打开WIFI音乐线程
        #endif
    }
#endif

    while (1)
    {
        rkos_queue_receive(gpstMainData->MainAskQueue, &MainTaskAskQueue, MAX_DELAY);
        //printf ("\nMainTask:2----event=%x event_type=%d\n",MainTaskAskQueue.event,MainTaskAskQueue.event_type);
        if(MainTaskCheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }
        switch (MainTaskAskQueue.event_type)
        {
            case MAINTASK_SYS_EVENT:
                MainTask_HandleSysEvent(MainTaskAskQueue.event);
                break;

            case MAINTASK_KEY_EVENT:
                #ifdef _USE_GUI_
                    if (gpstMainData->hMainMenu != NULL)
                    {
                        //MAINTASK_DEBUG ("KEY_EVENT\n");
                        MainTask_HandleKey(MainTaskAskQueue.event);
                    }
                    gpstMainData->keycontrol = 0;
                #else
                    MainTask_NoScreenHandleKey(MainTaskAskQueue.event);
                    gpstMainData->keycontrol = 0;
                    MainTask_KeyCallBack(0);
                #endif
                break;
            case MAINTASK_AUDIO_EVENT:
                MainTask_HandleAudioEvent(MainTaskAskQueue.event);
                break;

            case MAINTASK_TASK_SWTICH:
                TaskSwtich(&gpstMainData->stSwtichs);
                break;

            default:
                MAINTASK_DEBUG ("default\n");
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
** Name: MainTaskResume
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN rk_err_t MainTaskResume(HTC hTask)
{
   RK_TASK_CLASS*   pMainTask = (RK_TASK_CLASS*)hTask;
   gpstMainData->hMsg = RKDev_Open(DEV_CLASS_MSG, 0, NOT_CARE);
   pMainTask->State = TASK_STATE_WORKING;
   return RK_SUCCESS;
}
/*******************************************************************************
** Name: MainTaskSuspend
** Input: HTC hTask, uint32  Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN rk_err_t MainTaskSuspend(HTC hTask, uint32  Level)
{
    RK_TASK_CLASS*   pMainTask = (RK_TASK_CLASS*)hTask;

    if(Level == TASK_STATE_IDLE1)
    {
        pMainTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pMainTask->State = TASK_STATE_IDLE2;
    }
    RKDev_Close(gpstMainData->hMsg);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTaskCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 19:32:40
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN rk_err_t MainTaskCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        if(MainTaskResume(hTask) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

#ifndef _USE_GUI_
/*******************************************************************************
** Name: MainTask_NoScreenHandleKey
** Input:uint32 event
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON int32 MainTask_NoScreenHandleKey(uint32 keyvalue)
{
#ifdef _USB_
    if (RKTaskFind(TASK_ID_USBSERVER, 0)!=NULL)
    {
        //rk_printf ("USB exit,No Work Other Module");
        return RK_ERROR;
    }
#endif

    return RK_SUCCESS;
}
#endif
/*******************************************************************************
** Name: MainTask_HandleKey
** Input:uint32 event
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
int32 MainTask_HandleKey(uint32 event)
{
    uint32 MenuIdMax = 8;
    switch (event)
    {
        case KEY_VAL_PLAY_PRESS_START: //ShutDown
            MAINTASK_DEBUG ("----关机---\n");
            //Delete all application....
            MainTask_DeleteAllApp();
            MAINTASK_DEBUG ("RKOS Shutdown................\n");
            MainTask_ShutDown(0);
            //Shutdown
            DeviceTask_System_PowerOff();
            break;


        case KEY_VAL_MENU_SHORT_UP:
            MainTask_StartSubThread(gpstMainData->MenuId);
            break;

        case KEY_VAL_FFW_SHORT_UP:
            gpstMainData->MenuId++;
            #if 1 //wrm-add linshi
            if ((gpstMainData->MenuId>7)&&(gpstMainData->MenuId<MenuIdMax))
            {
                gpstMainData->MenuId = MenuIdMax;
            }
            #endif
            if (gpstMainData->MenuId > MenuIdMax)
            {
                gpstMainData->MenuId = 0;
            }
            #ifdef _USE_GUI_
            MainTask_SetWindow(gpstMainData->MenuId);
            #endif
            break;

        case KEY_VAL_FFD_SHORT_UP:
            gpstMainData->MenuId--;
            #if 1 //wrm-add linshi
            if ((gpstMainData->MenuId>7)&&(gpstMainData->MenuId<MenuIdMax))
            {
                gpstMainData->MenuId = 7;
            }
            #endif
            if (gpstMainData->MenuId < 0 )
            {
                gpstMainData->MenuId = MenuIdMax;
            }
            MainTask_SetWindow(gpstMainData->MenuId);
            break;

        case KEY_VAL_FFD_PRESS:
            //MAINTASK_DEBUG ("FFD LONG PRESS \n");
            gpstMainData->MenuId--;
            #if 1 //wrm-add linshi
            if ((gpstMainData->MenuId>7)&&(gpstMainData->MenuId<MenuIdMax))
            {
                gpstMainData->MenuId = 7;
            }
            #endif
            if (gpstMainData->MenuId < 0 )
            {
                gpstMainData->MenuId = MenuIdMax;
            }
            MainTask_SetWindow(gpstMainData->MenuId);
            break;
        case KEY_VAL_FFD_PRESS_START:
            //MAINTASK_DEBUG ("FFD LONG PRESS start\n");
            break;

        case KEY_VAL_FFD_LONG_UP:
            //MAINTASK_DEBUG ("FFD LONG PRESS end\n");
            break;

        case KEY_VAL_FFW_PRESS:
            gpstMainData->MenuId++;
            #if 1 //wrm-add linshi
            if ((gpstMainData->MenuId>7)&&(gpstMainData->MenuId<MenuIdMax))
            {
                gpstMainData->MenuId = MenuIdMax;
            }
            #endif
            if (gpstMainData->MenuId > MenuIdMax)
            {
                gpstMainData->MenuId = 0;
            }
            MainTask_SetWindow(gpstMainData->MenuId);
            break;
        case KEY_VAL_FFW_PRESS_START:
            //MAINTASK_DEBUG ("FFW LONG PRESS start\n");
            break;
        case KEY_VAL_FFW_LONG_UP:
            //MAINTASK_DEBUG ("FFW LONG PRESS end\n");
            break;
        default:
            break;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_HandleSysEvent
** Input:uint32 event ,int params
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
int32 MainTask_HandleSysEvent(uint32 event)
{

    switch (event)
    {
        case MAINTASK_USBCREATE:
            #ifdef _USB_
            {
                #ifndef _USE_GUI_
                if(RKTaskFind(TASK_ID_MUSIC_PLAY_MENU, 0) != NULL)
                {
                    //RKTaskDelete(TASK_ID_MUSIC_PLAY_MENU, 0, SYNC_MODE);
                    MainTask_SetStatus(MAINTASK_APP_PLAYMENU,0);
                    //SaveSysInformation(1);
                    DeviceTask_SystemReset();
                }
                #endif
                MAINTASK_DEBUG ("Create Usb Server\n");
                if(MainTask_GetStatus(MAINTASK_APP_USB_START) == 0)
                {
                    MainTask_SetStatus(MAINTASK_APP_USB_START, 1);
                    MAINTASK_DEBUG ("Create Usb Server 1\n");

                    //Delete all application....
                    MainTask_DeleteAllApp();

                    //USB Create
                    MainTask_TaskSwtich(gpstMainData->stSwtichs.TargetClassId, 0, TASK_ID_USBSERVER, 0, NULL);
                }
            }
            #endif

            break;
        case MAINTASK_USBDELETE:
            #ifdef _USB_
            {
                MAINTASK_DEBUG ("Delete Usb Server\n");
                if(MainTask_GetStatus(MAINTASK_APP_USB_OK) == 1)
                {
                    MAINTASK_DEBUG ("Delete Usb Server 1\n");

                    UsbMscDev_IntDisalbe();
                    #ifdef _USE_GUI_
                    UsbService_DeleteUsb(1);
                    #else
                    UsbService_DeleteUsb(2);
                    #endif
                }
            }
            #endif

            break;
        case MAINTASK_SHUTDOWN: //关机
            {
                rk_printf("get a power off");

                MainTask_ShutDown(0);

                #ifndef _USE_GUI_
                KeyDev_UnRegister(gpstMainData->hKey);
                RKDev_Close(gpstMainData->hKey);
                #endif
                //Delete all application....
                MainTask_DeleteAllApp();

                MAINTASK_DEBUG ("RKOS Shutdown................\n");

                rkos_sleep(1000);
                //Shutdown
                DeviceTask_System_PowerOff();
                while(1)
                {
                    rkos_sleep(100);
                }
            }
            break;

        case MAINTASK_SUSPEND_WIFI:
        {

            #ifdef _WIFI_
            MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 1);
            rk_wifi_deinit();
            RKTaskDelete(TASK_ID_WIFI_APPLICATION,0,SYNC_MODE);
            #endif
            break;
        }

        case MAINTASK_RESUME_WIFI:
        {
            #ifdef _WIFI_
            RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_STA, SYNC_MODE);
            MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 0);
            #endif
            break;
        }

        default:
            MAINTASK_DEBUG ("Defalut(sys event error)\n");
            break;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MainTask_HandleAudioEvent
** Input:uint32 event ,int params
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
int32 MainTask_HandleAudioEvent(uint32 event)
{
    switch (event)
    {
        AUDIO_INFO stAudioInfo;
        uint32 playstate;
        uint32 PlayerState;
        RKGUI_ICON_ARG pstIconArg;
        RKGUI_TEXT_ARG pstTextArg;



        case AUDIO_STATE_MUSIC_CHANGE:
        {
            playstate = AudioPlayer_GetAudioInfo(&stAudioInfo);
            #ifdef _USE_GUI_
            if(gpstMainData->hMusicName == NULL)
            {
                pstTextArg.lucency= OPACITY;
                pstTextArg.Backdrop= IMG_ID_STATE_BACKGROUND;
                pstTextArg.ForegroundR= 255;
                pstTextArg.ForegroundG= 255;
                pstTextArg.ForegroundB= 255;

                pstTextArg.BackgroundR= 0;
                pstTextArg.BackgroundG= 0;
                pstTextArg.BackgroundB= 0;
                pstTextArg.BackdropX= 0;
                pstTextArg.BackdropY= 0;

                pstTextArg.align= TEXT_ALIGN_X_CENTER;
                pstTextArg.cmd= TEXT_CMD_BUF;
                pstTextArg.x= 48;
                pstTextArg.y= 2;
                pstTextArg.xSize= 32;
                pstTextArg.ySize= -1;
                pstTextArg.text = stAudioInfo.path;
                pstTextArg.display = 1;
                pstTextArg.level= 1;

                gpstMainData->hMusicName = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
                GuiTask_OperWidget(gpstMainData->hMusicName, OPERATE_START_MOVE, (void*)10, SYNC_MODE);
            }
            else
            {
                pstTextArg.opercmd = TEXT_SET_CONTENT;
                pstTextArg.text = stAudioInfo.path;
                GuiTask_OperWidget(gpstMainData->hMusicName, OPERATE_SET_CONTENT, &pstTextArg, SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hMusicName, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
            }
            #endif
        }

        case AUDIO_STATE_PAUSE:
        case AUDIO_STATE_PLAY:
        case AUDIO_STATE_FFW:
        case AUDIO_STATE_FFD:
        case AUDIO_STATE_STOP:
        {

            playstate = AudioPlayer_GetAudioInfo(&stAudioInfo);
#ifdef _USE_GUI_
            if (playstate == AUDIO_STATE_PAUSE)
            {
                PlayerState = IMG_ID_MUSIC_PLAY_STATE;
            }
            else if (playstate == AUDIO_STATE_PLAY)
            {
                PlayerState = IMG_ID_MUSIC_PAUSE_STATE;
            }
            else if (playstate == AUDIO_STATE_FFW)
            {
                PlayerState = IMG_ID_MUSIC_FFW_STATE;
            }
            else if (playstate == AUDIO_STATE_FFD)
            {
                PlayerState = IMG_ID_MUSIC_FFD_STATE;
            }
            else if (playstate == AUDIO_STATE_STOP)
            {
                PlayerState = IMG_ID_MUSIC_STOP_STATE;
            }

            if(gpstMainData->hPlayState == NULL)
            {
                pstIconArg.resource = PlayerState;//IMG_ID_MUSIC_PLAY_STATE
                pstIconArg.x= 24;
                pstIconArg.y= 0;
                pstIconArg.level = 1;
                pstIconArg.display= 1;
                gpstMainData->hPlayState= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
            }
            else
            {
                GuiTask_OperWidget(gpstMainData->hPlayState, OPERATE_SET_CONTENT, (void *)PlayerState, SYNC_MODE);
                GuiTask_OperWidget(gpstMainData->hPlayState, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
            }
#endif
            break;
        }

        case AUDIO_STATE_CUSTOM:
        case AUDIO_STATE_DELETE:
        {
            #ifdef _USE_GUI_
            if(gpstMainData->hPlayState != NULL)
            {
               GuiTask_OperWidget(gpstMainData->hPlayState, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
            }

            if(gpstMainData->hMusicName != NULL)
            {
               GuiTask_OperWidget(gpstMainData->hMusicName, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
            }
            #endif

            break;
        }

        default:
            break;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MainTask_Animation
** Input:int params
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
COMMON FUN int32 MainTask_Animation ()
{
#ifdef _USE_GUI_
    int i;
    HGC hGc;

    RKGUI_CHAIN_ARG  pstChainArg;

    MAINTASK_DEBUG("Boot animation display...\n");


    pstChainArg.x= 0;
    pstChainArg.y= 0;
    pstChainArg.resource = IMG_ID_POWERON0;
    pstChainArg.num= 23;
    pstChainArg.delay= 1;
    pstChainArg.level= 0;
    pstChainArg.display = 1;
    pstChainArg.blurry= 0;
    gpstMainData->hWelcom = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
    rkos_sleep(2000);
    GuiTask_DeleteWidget(gpstMainData->hWelcom);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_GuiInit
** Input:int flag
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
COMMON FUN int32 MainTask_GuiInit ()
{
#ifdef _USE_GUI_
    HGC hGc;
    RKGUI_ICON_ARG pGcArg;

    GuiTask_ScreenLock();
    //Window Init
    pGcArg.resource = IMG_ID_MAINMENU_MUSIC + gpstMainData->MenuId;
    pGcArg.x= -1;
    pGcArg.y= -1;
    pGcArg.display = 1;
    pGcArg.level= 0;
    gpstMainData->hMainMenu = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);

    GuiTask_ScreenUnLock();
#endif
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MainTask_GuiDeInit
** Input:int flag
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
COMMON FUN int32 MainTask_GuiDeInit ()
{
#ifdef _USE_GUI_
    if(gpstMainData->hMainMenu != NULL)
    {
         GuiTask_DeleteWidget(gpstMainData->hMainMenu);
         gpstMainData->hMainMenu = NULL;
    }
#endif
}


/*******************************************************************************
** Name: MainTask_ShutDown
** Input:int flag
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN int32 MainTask_ShutDown (int flag)
{
#ifdef _USE_GUI_
    int i;
    HGC hGc;

    RKGUI_CHAIN_ARG  pstChainArg;

    MAINTASK_DEBUG("Boot animation display...\n");


    pstChainArg.x= 0;
    pstChainArg.y= 0;
    pstChainArg.resource = IMG_ID_POWEROFF0;
    pstChainArg.num= 11;
    pstChainArg.delay= 1;
    pstChainArg.level= 0;
    pstChainArg.display = 1;
    pstChainArg.blurry= 0;
    gpstMainData->hWelcom = GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_DiaplayElectricPower
** Input:int flag
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN int32 MainTask_DiaplayElectricPower (int flag)
{
#ifdef _USE_GUI_
    HGC hGc;
    RKGUI_ICON_ARG pGcArg;
    pGcArg.x= 100;
    pGcArg.y= 1;
    pGcArg.display = 1;
    pGcArg.level= 1;
    pGcArg.resource = IMG_ID_BATTERY01+4;
    hGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
    GuiTask_DeleteWidget(hGc);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_DeleteAllApp
** Input:void
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN int32 MainTask_DeleteAllApp (void)
{
    MAINTASK_DEBUG ("-----------Delete AllApp\n");
#ifdef _USE_GUI_
    //delete player
    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_LOCAL_PLAYER) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete LocalPlayer\n");
        MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);
        RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_MEDIA_BROWSER) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete MediaBrowser\n");
        MainTask_SetStatus(MAINTASK_APP_MEDIA_BROWSER, 0);
        RKTaskDelete(TASK_ID_MEDIA_BROWSER, 0, SYNC_MODE);
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_DLNA_PLAYER) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete DLNAPlayer\n");
        MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
        #ifdef __WIFI_DLNA_C__
        rk_dlna_end();
        RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
        #endif
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_XXX_PLAYER) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete XXX\n");
        MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
        #ifdef __WIFI_XXX_C__
        while (XXX_init_state() == 0)
        {
            rkos_sleep(1000);
        }
        XXX_end();
        RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
        MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER,0);
        #endif
        return RK_SUCCESS;
    }

    //delete playmenuapp
    if(RKTaskFind(TASK_ID_MUSIC_PLAY_MENU, 0) != NULL)
    {
        RKTaskDelete(TASK_ID_MUSIC_PLAY_MENU, 0, SYNC_MODE);
    }

    //delete systemset
    if(MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_SYSTEMSET) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete System Setting\n");
        RKTaskDelete(TASK_ID_SYSTEMSET,0,SYNC_MODE);
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_BT_START) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete BTPlayer -----0\n");
        #ifdef _BLUETOOTH_
        while (gSysConfig.BtOpened != 1)
        {
            rkos_sleep(100);
        }
        #endif
        MAINTASK_DEBUG ("Delete BTPlayer -----1\n");
        MainTask_SetStatus(MAINTASK_BT_OK, 0);
        MainTask_SetStatus(MAINTASK_BT_START, 0);
        #ifdef _BLUETOOTH_
        bluetooth_stop();
        FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
        #endif
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_WIFI_OPEN_OK) == RK_SUCCESS)
    {
        #ifdef _DRIVER_WIFI__
        MAINTASK_DEBUG ("Delete wifi\n");
        while(wifi_init_flag() == 0)
        {
            rkos_sleep(100);
        }

        rk_wifi_deinit();
        RKTaskDelete(TASK_ID_WIFI_APPLICATION, 0, SYNC_MODE);
        MAINTASK_DEBUG ("Delete wifi OK\n");
        MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 0);
        #endif
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_BROWER) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete browser\n");
        RKTaskDelete(TASK_ID_BROWSERUI, 0, SYNC_MODE);
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_LINE_IN) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete line in\n");
        RKTaskDelete(TASK_ID_LINEIN, 0, SYNC_MODE);
    }

    if (MsgDev_GetMsg(gpstMainData->hMsg, MAINTASK_APP_FMUI) == RK_SUCCESS)
    {
        MAINTASK_DEBUG ("Delete fm\n");
        RKTaskDelete(TASK_ID_FMUI, 0, SYNC_MODE);
    }

#else
    #ifdef _BLUETOOTH_
    {
        //if (gSysConfig.BtOpened==1)
        if (gSysConfig.BtControl==1)
        {
            #ifdef _BLUETOOTH_
            bluetooth_stop();
            FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
            gSysConfig.BtControl = 0;
            #endif
        }
        else
        {
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
            }
        }
    }
    #endif

    #ifdef _WIFI_
    {
        if(RKTaskFind(TASK_ID_DLNA, 0) != NULL)
        {
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
            #ifdef __WIFI_DLNA_C__
            rk_dlna_end();
            RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
            #endif
        }

        if(RKTaskFind(TASK_ID_XXX, 0) != NULL)
        {
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
            #ifdef __WIFI_XXX_C__
            while (XXX_init_state() == 0)
            {
                rkos_sleep(1000);
            }

            XXX_end();
            RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
            #endif
        }

        if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) != NULL)
        {
            MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 0);
            rk_wifi_deinit();
            RKTaskDelete(TASK_ID_WIFI_APPLICATION, 0, SYNC_MODE);
            MAINTASK_DEBUG ("Delete wifi OK\n");
        }

    }
    #endif
#endif


    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_SetName
** Input:uint32 MenuId
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
COMMON FUN int32 MainTask_SetName (int32 MenuId)
{
#ifdef _USE_GUI_
    RKGUI_TEXT_ARG pstTextArg;
    RKGUI_TEXT_ARG playerTextContentArg;
    HGC pHgc;

    pstTextArg.cmd= TEXT_CMD_ID;
    pstTextArg.level = 0;
    pstTextArg.display = 1;

#if 1
    pstTextArg.lucency= OPACITY;
    pstTextArg.Backdrop= MenuId + IMG_ID_MAINMENU_MUSIC;
    pstTextArg.ForegroundR= 0;
    pstTextArg.ForegroundG= 0;
    pstTextArg.ForegroundB= 0;
    pstTextArg.BackgroundR= 255;
    pstTextArg.BackgroundG= 255;
    pstTextArg.BackgroundB= 255;

    pstTextArg.align= TEXT_ALIGN_X_CENTER;
    pstTextArg.xSize= 128;
    pstTextArg.ySize= -1;
    pstTextArg.BackdropX= 0;
    pstTextArg.BackdropY= 120;
    pstTextArg.x= 0;
    pstTextArg.y= 120;
#endif

    switch(MenuId)
    {
        case 0:
            pstTextArg.resource = SID_MUSIC;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 1:
            pstTextArg.resource = SID_DLNA;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 2:
            pstTextArg.resource = SID_MAINMENU_BLUETOOTH;

            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 3:
            #ifdef __WIFI_XXX_C__
            pstTextArg.resource = SID_XXX;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            #else
            pstTextArg.resource = SID_PHOTO;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            #endif
            break;
        case 4:
            pstTextArg.resource = SID_LINE_IN;//SID_5_1_CHANNEL;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 5:
            pstTextArg.resource = SID_RECORD;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 6:
            pstTextArg.resource = SID_EXPLORER;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 7:
            pstTextArg.resource = SID_RADIO;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        case 8: //Setting
            pstTextArg.resource = SID_MAIN_SETTINGS;
            pHgc = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
            break;
        default:
            playerTextContentArg.opercmd= TEXT_SET_CONTENT;
            playerTextContentArg.text = TEXT("No Name");
            return RK_SUCCESS;
    }
    GuiTask_DeleteWidget(pHgc);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MainTask_SetWindow
** Input:uint32 MenuId
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_MENU_
COMMON FUN int32 MainTask_SetWindow (int32 MenuId)
{
#ifdef _USE_GUI_
    //MAINTASK_DEBUG("SetWindow hMainMenu=0x%x\n",gpstMainData->hMainMenu);
    GuiTask_OperWidget(gpstMainData->hMainMenu,
                       OPERATE_SET_CONTENT,
                       (void *)(MenuId + IMG_ID_MAINMENU_MUSIC),
                       SYNC_MODE);
    MainTask_SetName(MenuId);
#endif
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MainTask_StartSubThread
** Input:uint32 MenuId
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN int32 MainTask_StartSubThread (uint32 MenuId)
{
    RK_TASK_PLAYMENU_ARG stTaskPlayer;
    RK_TASK_BROWSER_ARG Arg;
    RK_TASK_FM_ARG FmUiArg;

    uint8 ret;

    switch (MenuId)
    {
        case MUSIC_SUB_THREAD: //本地播放器
#ifdef __APP_MEDIA_LIBRARY_C__    // 测试媒体库功能实现，媒体库功能实现完全后开启。
            RK_TASK_MEDIALIB_ARG MediaLibArg;
            MediaLibArg.CurId = 0;
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_MEDIA_LIBRARY,0, &MediaLibArg);
#else
            if(MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER))
            {
                stTaskPlayer.FileNum = 0;
                stTaskPlayer.ucSelPlayType = SOURCE_FROM_CUR_AUDIO;
                memcpy(stTaskPlayer.filepath, L"C:\\MUSIC\\", 20);
                MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer);
            }
            else
            {
                #ifdef __APP_BROWSER_BROWSERUITASK_C__
                memcpy(Arg.filepath, L"C:\\", 8);
                Arg.SelSouceType = SOURCE_FROM_FILE_BROWSER;
                Arg.Flag = 0;
                MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_BROWSERUI, 0, &Arg);
                #endif
            }
#endif
            //RKTaskCreate(TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer, SYNC_MODE);
            //记录正在播放的播放器类型
            break;

        case VEDIO_SUB_THREAD: //DLNA播放器
            #ifdef __WIFI_DLNA_C__
            //本次播放器类型
            stTaskPlayer.ucSelPlayType = SOURCE_FROM_DLNA;

            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer);
            //RKTaskCreate(TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer, SYNC_MODE);
            #endif
            break;

        case FM_SUB_THREAD:    //BT播放器
            #ifdef _BLUETOOTH_
            //本次播放器类型
            stTaskPlayer.ucSelPlayType = SOURCE_FROM_BT;
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer);
            //RKTaskCreate(TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer, SYNC_MODE);
            #endif
            break;

        case PHOTO_SUB_THREAD: //SOURCE_FROM_XXX
            #ifdef __WIFI_XXX_C__
            //本次播放器类型
            stTaskPlayer.ucSelPlayType = SOURCE_FROM_XXX;

            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer);
            //RKTaskCreate(TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer, SYNC_MODE);
            #endif
            break;

        //SystemSetTak
        case SETMENU_SUB_THREAD:
#ifdef __APP_SYSTEMSET_SYSTEMSETTASK_C__
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_SYSTEMSET,0, NULL);
            //RKTaskCreate(TASK_ID_SYSTEMSET,0, NULL, ASYNC_MODE);
#endif
            //RKTaskDelete(TASK_ID_MAIN, 0, ASYNC_MODE);
            break;

        case RECOD_SUB_THREAD:
#ifdef __APP_RECORD_RECOD_TASK_C__
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_RECOD,0, NULL);
#endif
            break;

        case BROWSER_SUB_THREAD:
#ifdef __APP_BROWSER_BROWSERUITASK_C__
            memcpy(Arg.filepath, L"C:\\", 8);
            Arg.SelSouceType = SOURCE_FROM_FILE_BROWSER;
            Arg.Flag = 0;
            //debug_hex((char *)Arg.filepath, 8, 8);
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_BROWSERUI, 0, &Arg);
#endif
            //RKTaskDelete(TASK_ID_MAIN, 0, ASYNC_MODE);
            break;

        case GAME_SUB_THREAD:
#ifdef __APP_FM_FMUITASK_C__
            FmUiArg.source = 1;//RECORD_TYPE_LINEIN1
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_FMUI,0, &FmUiArg);
#endif
            break;
        case BOOK_SUB_THREAD:
#ifdef __APP_LINE_IN_LINEINTASK_C__
            RK_TASK_LINEIN_ARG lineInArg;
            lineInArg.source = 2;//RECORD_TYPE_LINEIN2
            MainTask_TaskSwtich(TASK_ID_MAIN, 0, TASK_ID_LINEIN, 0, &lineInArg);
            break;
#endif
        default:
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
** Name: MainTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_INIT_
INIT API rk_err_t MainTask_DeInit(void *pvParameters)
{
    #ifdef _WIFI_
    lwip_socket_deinit();
    FW_RemoveSegment(SEGMENT_ID_LWIP_API);
    #endif
#ifdef _USE_GUI_
    if(gpstMainData->hBackGround != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hBackGround);
        gpstMainData->hBackGround = NULL;
    }

    if(gpstMainData->hplayerIcon != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hplayerIcon);
        gpstMainData->hplayerIcon = NULL;
    }

    if(gpstMainData->hSysStatus != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hSysStatus);
        gpstMainData->hSysStatus = NULL;
    }

    if(gpstMainData->hPlayState != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hPlayState);
        gpstMainData->hSysStatus = NULL;
    }

    if(gpstMainData->hMusicName != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hMusicName);
        gpstMainData->hMusicName = NULL;
    }

    if(gpstMainData->hchargeIcon != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hchargeIcon);
        gpstMainData->hchargeIcon = NULL;
    }

    if(gpstMainData->hbatteryIcon != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hbatteryIcon);
        gpstMainData->hbatteryIcon = NULL;
    }

    if(gpstMainData->hbtIcon != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hbtIcon);
        gpstMainData->hbtIcon = NULL;
    }

    if(gpstMainData->hwifiIcon != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hwifiIcon);
        gpstMainData->hwifiIcon = NULL;
    }

    if(gpstMainData->hwifiConnect != NULL)
    {
        GuiTask_DeleteWidget(gpstMainData->hwifiConnect);
        gpstMainData->hwifiConnect = NULL;
    }

    if(gpstMainData->hMainMenu != NULL)
    {
         GuiTask_DeleteWidget(gpstMainData->hMainMenu);
         gpstMainData->hMainMenu = NULL;
    }

    if(gpstMainData->hWelcom != NULL)
    {
         printf("hWelcom");
         GuiTask_DeleteWidget(gpstMainData->hWelcom);
         gpstMainData->hWelcom = NULL;
    }

    GuiTask_AppUnReciveMsg(MainTask_GuiCallBack);
#endif
    rkos_queue_delete(gpstMainData->MainAskQueue);
    rkos_queue_delete(gpstMainData->MainRespQueue);
    rkos_semaphore_delete(gpstMainData->osMainOperSem);
    rkos_semaphore_delete(gpstMainData->osMainSysEventSem);
    rkos_semaphore_delete(gpstMainData->osMainKeySem);

    RKDev_Close(gpstMainData->hMsg);

    rkos_memory_free(gpstMainData);
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_MAIN_TASK);
#endif
    gpstMainData = NULL;

    rk_printf("main task deinit ok");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: MainTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_INIT_
INIT API rk_err_t MainTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS*   pMainTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_MAIN_ARG * pArg = (RK_TASK_MAIN_ARG *)arg;
    MAIN_TASK_DATA_BLOCK*  pMainTaskData;
    RKGUI_ICON_ARG pGcArg;

    if (pMainTask == NULL)
        return RK_PARA_ERR;

    pMainTask->Idle1EventTime = 10 * PM_TIME;
    pMainTask->TaskSuspendFun = MainTaskSuspend;
    pMainTask->TaskResumeFun = MainTaskResume;

    pMainTaskData = rkos_memory_malloc(sizeof(MAIN_TASK_DATA_BLOCK));
    memset(pMainTaskData, NULL, sizeof(MAIN_TASK_DATA_BLOCK));

    pMainTaskData->hMsg = RKDev_Open(DEV_CLASS_MSG, 0, NOT_CARE);
    if((int)pMainTaskData->hMsg <= 0)
    {
        rkos_memory_free(pMainTaskData);
        rk_print_string("\nmsg device open failue");
        return RK_ERROR;
    }

    pMainTaskData->MainAskQueue = rkos_queue_create(5, sizeof(MAIN_ASK_QUEUE));
    pMainTaskData->MainRespQueue = rkos_queue_create(1, sizeof(MAIN_RESP_QUEUE));
    pMainTaskData->osMainOperSem  = rkos_semaphore_create(1,1);
    pMainTaskData->osMainSysEventSem  = rkos_semaphore_create(1,1);
    pMainTaskData->osMainKeySem  = rkos_semaphore_create(1,1);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_MAIN_TASK, SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_MAIN_MENU, SEGMENT_OVERLAY_ALL);
#endif

    gpstMainData = pMainTaskData;
#ifdef _USE_GUI_
    //Top Icon Init
    pGcArg.x= 0;
    pGcArg.y= 0;
    pGcArg.display = 1;
    pGcArg.level= 1;
    pGcArg.resource = IMG_ID_STATE_BACKGROUND;
    gpstMainData->hBackGround = GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
#else
    MainTask_SetStatus(MAINTASK_WIFICONFIG,0);
    MainTask_SetStatus(MAINTASK_APP_PLAYMENU,0);
    #ifdef NOSCREEN_USE_LED
    gpstMainData->greenled_state = 0;
    gpstMainData->redled_state = 0;
    #endif
#endif
    //Task
    gpstMainData->stSwtichs.TargetClassId = TASK_ID_MAIN;
    gpstMainData->keycontrol = 0;
    gpstMainData->MenuId = 0;
    gpstMainData->shutdown = 0;

    #ifdef _WIFI_
    //open net interface;
    FW_LoadSegment(SEGMENT_ID_LWIP_API, SEGMENT_OVERLAY_ALL);
    lwip_socket_init();
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
