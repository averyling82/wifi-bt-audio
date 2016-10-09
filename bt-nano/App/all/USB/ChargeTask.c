/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\USB\ChargeTask.c
* Owner: aaron.sun
* Date: 2015.11.16
* Time: 15:17:31
* Version: 1.0
* Desc: charge window
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.11.16     15:17:31   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_USB_CHARGETASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "effect.h"




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _CHARGE_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}CHARGE_RESP_QUEUE;
typedef  struct _CHARGE_ASK_QUEUE
{
    uint32 cmd;

}CHARGE_ASK_QUEUE;
typedef  struct _CHARGE_TASK_DATA_BLOCK
{
    pQueue  ChargeAskQueue;
    pQueue  ChargeRespQueue;

}CHARGE_TASK_DATA_BLOCK;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static CHARGE_TASK_DATA_BLOCK * gpstChargeData;



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
rk_err_t GuiCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ChargeTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 15:18:18
*******************************************************************************/
_APP_USB_CHARGETASK_COMMON_
COMMON API rk_err_t ChargeTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: ChargeTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 15:18:18
*******************************************************************************/
_APP_USB_CHARGETASK_COMMON_
COMMON API rk_err_t ChargeTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: ChargeTask_Enter
** Input:void * arg
** Return: void
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 15:18:18
*******************************************************************************/
_APP_USB_CHARGETASK_COMMON_
COMMON API void ChargeTask_Enter(void * arg)
{
    CHARGE_ASK_QUEUE ChargeAskQueue;
    HGC hGc;
    RKGUI_CHAIN_ARG pstChainArg;


    rk_printf("charge task enter...");

#ifdef _USE_GUI_
    pstChainArg.x= 0;
    pstChainArg.y= 0;
    pstChainArg.resource = IMG_ID_CHARGELEVEL0;
    pstChainArg.num= 5;
    pstChainArg.delay= 20;
    pstChainArg.blurry= 0;

    pstChainArg.level= 0;
    pstChainArg.display= 1;
    hGc= GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
#endif
#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(GuiCallBack);
#else
    DeviceTask_SystemReset();
#endif
    while (1)
    {
        rkos_queue_receive(gpstChargeData->ChargeAskQueue, &ChargeAskQueue, MAX_DELAY);
        switch (ChargeAskQueue.cmd)
        {
            case KEY_VAL_ESC_SHORT_UP:
                break;

            case KEY_VAL_FFW_SHORT_UP:
                break;

            case KEY_VAL_FFD_SHORT_UP:
                break;

            case KEY_VAL_UP_SHORT_UP:
                break;

            case KEY_VAL_DOWN_SHORT_UP:
                break;

            case KEY_VAL_MENU_SHORT_UP:
                break;

            case KEY_VAL_PLAY_PRESS_START:
#ifdef _USE_GUI_
                GuiTask_DeleteWidget(hGc);
                GuiTask_AppUnReciveMsg(GuiCallBack);
#endif
                DeviceTask_SystemReset();
                break;

            default:
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
** Name: GuiCallBack
** Input:uint32 evnet_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 16:45:45
*******************************************************************************/
_APP_USB_CHARGETASK_COMMON_
COMMON FUN rk_err_t GuiCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc)
{
    if(evnet_type==APP_RECIVE_MSG_EVENT_KEY)
    {
        CHARGE_ASK_QUEUE ChargeAskQueue;
        ChargeAskQueue.cmd = event;
        rkos_queue_send(gpstChargeData->ChargeAskQueue, &ChargeAskQueue, 0);
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
** Name: ChargeTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 15:18:18
*******************************************************************************/
_APP_USB_CHARGETASK_INIT_
INIT API rk_err_t ChargeTask_DeInit(void *pvParameters)
{

    rkos_queue_delete(gpstChargeData->ChargeAskQueue);
    rkos_queue_delete(gpstChargeData->ChargeRespQueue);
    rkos_memory_free(gpstChargeData);
}
/*******************************************************************************
** Name: ChargeTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.16
** Time: 15:18:18
*******************************************************************************/
_APP_USB_CHARGETASK_INIT_
INIT API rk_err_t ChargeTask_Init(void *pvParameters, void *arg)
{

    RK_TASK_CLASS*   pChargeTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_CHARGE_ARG * pArg = (RK_TASK_CHARGE_ARG *)arg;
    CHARGE_TASK_DATA_BLOCK*  pChargeTaskData;
    if (pChargeTask == NULL)
        return RK_PARA_ERR;

    pChargeTaskData = rkos_memory_malloc(sizeof(CHARGE_TASK_DATA_BLOCK));
    memset(pChargeTaskData, NULL, sizeof(CHARGE_TASK_DATA_BLOCK));

    pChargeTaskData->ChargeAskQueue = rkos_queue_create(1, sizeof(CHARGE_ASK_QUEUE));
    if(pChargeTaskData->ChargeAskQueue == NULL)
    {
        return RK_ERROR;
    }

    pChargeTaskData->ChargeRespQueue = rkos_queue_create(1, sizeof(CHARGE_RESP_QUEUE));

    if(pChargeTaskData->ChargeRespQueue == NULL)
    {
        rkos_queue_delete(pChargeTaskData->ChargeAskQueue);
        return RK_ERROR;
    }

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_CHARGE_TASK, SEGMENT_OVERLAY_ALL);
    #endif

    gpstChargeData = pChargeTaskData;

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
