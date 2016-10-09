/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Gui\GUITask.c
* Owner: zhuzhe
* Date: 2014.5.30
* Time: 17:14:35
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    zhuzhe     2014.5.30     17:14:35   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __GUI_GUITASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "..\Resource\MenuResourceID.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "GUIIcon.h"
#include "GUIImage.h"
#include "GUIText.h"
#include "GUISelect.h"
#include "KeyDevice.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define GUI_ASK_QUEUE_LENTH         1
#define GUI_ASK_QUEUE_SIZE          sizeof(GUI_ASK_QUEUE)

#define GUI_RESP_QUEUE_LENTH        1
#define GUI_RESP_QUEUE_SIZE         sizeof(GUI_RESP_QUEUE)

#define GUI_KEY_QUEUE_LENTH         1
#define GUI_KEY_QUEUE_SIZE          sizeof(GUI_KEY_QUEUE)

typedef  struct _GUI_ASK_QUEUE
{
    uint32 event_type;
    uint32 event;
    uint32 mode;
    void*  arg;
}GUI_ASK_QUEUE;

typedef  struct _GUI_KEY_QUEUE
{
    uint32 event_type;
    uint32 event;
    uint32 mode;
    void*  arg;
}GUI_KEY_QUEUE;

typedef  struct _GUI_APP_RECIVE_MSG
{
    struct _GUI_APP_RECIVE_MSG * next;
    P_APP_RECIVE_MSG pfAppReciveMsg;
}GUI_APP_RECIVE_MSG;

typedef  struct _GUI_GC_RECIVE_MSG
{
    struct _GUI_GC_RECIVE_MSG * next;
    P_APP_RECIVE_MSG pfAppReciveMsg;
    HGC pGc;

}GUI_GC_RECIVE_MSG;

typedef  struct _GUI_CLASS_TIMER
{
    struct _GUI_CLASS_TIMER * next;
    HGC pGc;
    P_TEXT_RECIVE pfTimerReciveMsg;
    int start;
}GUI_CLASS_TIMER;

typedef  struct _GUI_TASK_CTRL_BLOCK
{
    pQueue  pAskQueue;
    pQueue  pRespQueue;
    pQueue  pKeyQueue;

    GUI_APP_RECIVE_MSG * pfAppCallBack;
    GUI_GC_RECIVE_MSG * pfGcFocus;
    GUI_GC_RECIVE_MSG * pfActiveFocus;

    int TimerLock;
    GUI_CLASS_TIMER * pfTimerList;

    pSemaphore ApiReqSem;

    int ScreenLock;

    HGC Background;
    HGC StateBackground;

    HDC hKey;
}GUI_TASK_CTRL_BLOCK;

#ifdef _SPI_BOOT_
SPI_BOOT_RESOURCE gstSpiResoure;
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
GUI_TASK_CTRL_BLOCK * gpstGUICtrlBlock = NULL;
GUI_CONTROL_CLASS *pfGcList= NULL;
GUI_CONTROL_CLASS *pfGcListTop= NULL;
GUI_CONTROL_CLASS *pfUpdataGcList= NULL;
HDC hDisplay0;
HDC hDisplay1;

extern GUI_CONTROL_CLASS * GUI_GcCreate(eGc eGcId, void* arg);
extern rk_err_t GUI_GcDelete(HGC hGc);
extern rk_err_t GUI_DisplayWidge(HGC hGc);

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
rk_err_t GUITaskResume(HTC hTask);
rk_err_t GUITaskSuspend(HTC hTask, uint32 Level);
rk_err_t GUITaskCheckIdle(HTC hTask);
rk_err_t GuiShellPcb(HDC dev, uint8 * pstr);
rk_err_t GUITaskShellTest(HDC dev, uint8 * pstr);
rk_err_t GUITaskShellHelp(HDC dev, uint8 * pstr);
rk_err_t GUITaskShellDel(HDC dev, uint8 * pstr);
rk_err_t GUITaskShellMc(HDC dev, uint8 * pstr);
rk_err_t GUITaskShellPcb(HDC dev, uint8 * pstr);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FW_GetPicInfoWithIDNum
** Input:UINT16 pictureIDNum, PICTURE_INFO_STRUCT *psPictureInfo
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:44:29
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON FUN rk_err_t FW_GetPicInfoWithIDNum(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *psPictureInfo)
{
    UINT32 flashAddr;
    flashAddr= gstFwInf.ImageLogicAddress + 32 + ((UINT32)pictureIDNum * 16);

#ifdef _SPI_BOOT_
    if(gstSpiResoure.Image== NULL)
    {
        //printf("gstSpiResoure.Image fault\n");
        return RK_ERROR;
    }
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);

    FileDev_FileSeek(gstSpiResoure.Image, SEEK_SET, flashAddr);
    FileDev_ReadFile(gstSpiResoure.Image, (UINT8 *)psPictureInfo, 16);
    rkos_semaphore_give(gstSpiResoure.osReadSource);


#endif

    //32是固件中图片资源头信息的长度 IMAGE_RESOURCE_INFO_SIZE 是每个图片信息结构体所占用的字节数(16)
#ifdef _EMMC_BOOT_
    int8  hMainFile;
    FW_ReadFirmwaveByByte(flashAddr, (UINT8*)psPictureInfo, 16);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_GetMenuInfoWithIDNum
** Input:UINT32 menuTextID,MENU_TEXT_INFO_STRUCT *pMenuTextInfo
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:43:22
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t FW_GetMenuInfoWithIDNum(UINT32 menuTextID,MENU_TEXT_INFO_STRUCT *pMenuTextInfo)
{
    UINT32   CharInNFAddr;
    UINT32   languegeOffsetAddr;

//    languegeOffsetAddr = gstFwInf.MenuLogicAddress + gSysConfig.SysLanguage*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM-9)+TOTAL_LANAUAGE_NUM*4+2;
//    if(gSysConfig.FMEnable)
//    {
        languegeOffsetAddr = gstFwInf.MenuLogicAddress + gSysConfig.SysLanguage*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM)+TOTAL_LANAUAGE_NUM*4+2;
//    }

    CharInNFAddr = languegeOffsetAddr + menuTextID * MENU_ITEM_LENGTH;
#ifdef _SPI_BOOT_
    if(gstSpiResoure.Menu == NULL)return RK_ERROR;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Menu, SEEK_SET, CharInNFAddr);
    FileDev_ReadFile(gstSpiResoure.Menu, (UINT8*)pMenuTextInfo, sizeof(MENU_TEXT_INFO_STRUCT));
    rkos_semaphore_give(gstSpiResoure.osReadSource);
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(CharInNFAddr, (UINT8*)pMenuTextInfo, sizeof(MENU_TEXT_INFO_STRUCT));
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_GetPicResource
** Input:uint32 FlashAddr, uint8 *pData, uint16 length
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:41:52
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t FW_GetPicResource(uint32 FlashAddr, uint8 *pData, uint16 length)
{
#ifdef _SPI_BOOT_
    if(gstSpiResoure.Image== NULL)return RK_ERROR;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Image, SEEK_SET, FlashAddr);
    FileDev_ReadFile(gstSpiResoure.Image, pData, length);
    rkos_semaphore_give(gstSpiResoure.osReadSource);
    return RK_SUCCESS;
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(gstFwInf.ImageLogicAddress+FlashAddr,pData,length);
    return RK_SUCCESS;
#endif

}

/*******************************************************************************
** Name: FW_GetMenuResource
** Input:UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:40:12
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t FW_GetMenuResource(UINT16 menuTextID , UINT16 *pMenuStr ,UINT16 StrLen)
{
    UINT32   CharInNFAddr;
    UINT32   languegeOffsetAddr;

//    languegeOffsetAddr = gstFwInf.MenuLogicAddress +gSysConfig.SysLanguage*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM)+TOTAL_LANAUAGE_NUM*4+2;
//    if(gSysConfig.FMEnable)
//    {
        languegeOffsetAddr = gstFwInf.MenuLogicAddress + gSysConfig.SysLanguage*MENU_ITEM_LENGTH*(TOTAL_MENU_ITEM)+TOTAL_LANAUAGE_NUM*4+2;
//    }
    CharInNFAddr = languegeOffsetAddr + menuTextID*MENU_ITEM_LENGTH+MENU_CONTENT_OFFSET;
#ifdef _SPI_BOOT_
    if(gstSpiResoure.Menu== NULL)return RK_ERROR;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Menu, SEEK_SET, CharInNFAddr);
    FileDev_ReadFile(gstSpiResoure.Menu, (UINT8*)pMenuStr, StrLen);
    rkos_semaphore_give(gstSpiResoure.osReadSource);
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(CharInNFAddr, (UINT8*)pMenuStr, StrLen);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUITask_SendMsg
** Input:void
** Return: void
** Owner:Benjo.lei
** Date: 2014.6.9
** Time: 9:20:38
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUITask_SendMsg(uint32 EVENT_type, uint32 event, void * arg, uint32 mode)
{
    GUI_KEY_QUEUE KeyQueue;
    GUI_KEY_QUEUE KeyQueueTemp;

    KeyQueue.event_type = EVENT_type;
    KeyQueue.event = event;
    KeyQueue.arg = arg;
    KeyQueue.mode = ASYNC_MODE;

    if(gpstGUICtrlBlock== NULL)return RK_ERROR;
    rkos_queue_receive(gpstGUICtrlBlock->pKeyQueue,&KeyQueueTemp, 0);
    rkos_queue_send(gpstGUICtrlBlock->pKeyQueue,&KeyQueue, 0);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUITask_PingPongMsg
** Input:uint32 EVENT_type, uint32 event, void * arg, uint32 mode, GUI_RESP_QUEUE * RespQueue
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 9:20:38
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUITask_PingPongMsg(uint32 EVENT_type, uint32 event, void * arg, uint32 mode, GUI_RESP_QUEUE * RespQueue)
{
    GUI_ASK_QUEUE AskQueue;

    AskQueue.event_type = EVENT_type;
    AskQueue.event = event;
    AskQueue.arg = arg;
    AskQueue.mode = mode;

    if(gpstGUICtrlBlock == NULL) return RK_ERROR;
    rkos_queue_send(gpstGUICtrlBlock->pAskQueue,&AskQueue, MAX_DELAY);

    if (mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstGUICtrlBlock->pRespQueue,RespQueue, MAX_DELAY);
        return RespQueue->error_code;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GuiTask_DeleteWidget
** Input:
** Return:
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_DeleteWidget(HGC pGc)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(GUI_EVENT_DELETE, 0, pGc, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GuiTask_OperWidget
** Input:HGC pGc, eICON_OPERATE_CMD cmd, void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_OperWidget(HGC pGc, eRKGUI_OPERATE_CMD cmd, void *arg, uint32 mode)
{
    rk_err_t ret;
    RKGUI_OPERATE_ARG pOperArg;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    pOperArg.pGc= pGc;
    pOperArg.arg= arg;
    ret= GUITask_PingPongMsg(GUI_EVENT_OPERATE, cmd, &pOperArg, mode, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GuiTask_UnloadResource
** Input:
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_UnloadResource(void)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(GUI_OPERATE_RESOURCE, 0, (void *)0, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GuiTask_LoadResource
** Input:
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_LoadResource(void)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(GUI_OPERATE_RESOURCE, 0, (void *)1, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GUITask_CreateWidget
** Input:eGc ClassID, void * arg
** Return: HGC
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API HGC GUITask_CreateWidget(eGc ClassID, void * arg)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(GUI_EVENT_CREATE, ClassID, arg, SYNC_MODE, &RespQueue);
    if ( ret!= RK_SUCCESS )
    {
        rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
        return NULL;
    }

    if ( RespQueue.arg== NULL )
    {
        rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
        return NULL;
    }
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return (HGC)RespQueue.arg;
}

/*******************************************************************************
** Name: GuiTask_ScreenLock
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_ScreenLock(void)
{
    rkos_enter_critical();
    gpstGUICtrlBlock->ScreenLock++;
    rkos_exit_critical();
}

/*******************************************************************************
** Name: GuiTask_ScreenUnLock
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_ScreenUnLock(void)
{
    rkos_enter_critical();
    if(--gpstGUICtrlBlock->ScreenLock< 0) gpstGUICtrlBlock->ScreenLock= 0;
    rkos_exit_critical();
}

/*******************************************************************************
** Name: GuiTask_ScreenLock
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_TimerLock(void)
{
    rkos_enter_critical();
    gpstGUICtrlBlock->TimerLock++;
    rkos_exit_critical();
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GuiTask_ScreenUnLock
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_TimerUnLock(void)
{
    rkos_enter_critical();
    if(--gpstGUICtrlBlock->TimerLock< 0)gpstGUICtrlBlock->TimerLock= 0;
    rkos_exit_critical();
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GuiTask_Setfocus
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_Setfocus(HGC pGc)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(APP_REQUEST_FOCUS, 0, pGc, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GuiTask_Delfocus
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_Delfocus(HGC pGc)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(APP_REQUEST_FOCUS, 1, pGc, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GuiTask_AppReciveMsg
** Input:P_APP_RECIVE_MSG callback
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_AppReciveMsg(P_APP_RECIVE_MSG pApp)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(APP_REQUEST_RECIVE_MSG, 0, pApp, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GuiTask_AppUnReciveMsg
** Input:
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.19
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GuiTask_AppUnReciveMsg(P_APP_RECIVE_MSG pApp)
{
    rk_err_t ret;
    GUI_RESP_QUEUE RespQueue;

    rkos_semaphore_take(gpstGUICtrlBlock->ApiReqSem, MAX_DELAY);
    ret= GUITask_PingPongMsg(APP_REQUEST_RECIVE_MSG, 1, pApp, SYNC_MODE, &RespQueue);
    rkos_semaphore_give(gpstGUICtrlBlock->ApiReqSem);
    return ret;
}

/*******************************************************************************
** Name: GUITask_Enter
** Input:void
** Return: void
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API void GUITask_Enter(void)
{
    GUI_TASK_CTRL_BLOCK*  pGUITaskControlBlock = gpstGUICtrlBlock;
    GUI_ASK_QUEUE AskQueue;
    GUI_RESP_QUEUE RespQueue;
    GUI_KEY_QUEUE KeyQueue;
    rk_err_t ret;
    HTC hSelf;
    hSelf = RKTaskGetRunHandle();

    {
        RKGUI_IMAGE_ARG pstImageArg;
        pstImageArg.cmd= IMAGE_CMD_RECT;
        pstImageArg.R= 0;
        pstImageArg.G= 0;
        pstImageArg.B= 0;
        pstImageArg.transparency= 0;
        pstImageArg.x= 0;
        pstImageArg.y= 0;
        pstImageArg.xSize= 128;
        pstImageArg.ySize= 160;
        pstImageArg.display = 1;
        pstImageArg.level= 0;
        gpstGUICtrlBlock->Background= GUI_ImageCreate(&pstImageArg);
        GcRelevanceRegister(gpstGUICtrlBlock->Background);

        pstImageArg.xSize= 128;
        pstImageArg.ySize= 20;
        pstImageArg.level= 1;
        gpstGUICtrlBlock->StateBackground= GUI_ImageCreate(&pstImageArg);
        GcRelevanceRegister(gpstGUICtrlBlock->StateBackground);
    }

    while (1)
    {
        if(pGUITaskControlBlock->ScreenLock==0)
        {
            if(CheckSystemIdle() != RK_SUCCESS)
            {
                DisplayDev_ScreenUpdate();
            }
        }


        if( rkos_queue_receive(pGUITaskControlBlock->pAskQueue,&AskQueue, 2) == RK_SUCCESS)
        {
            if(AskQueue.event_type != 0)
            {
                if(GUITaskCheckIdle(hSelf) != RK_SUCCESS)
                {
                    continue;
                }

                switch (AskQueue.event_type)
                {
                    case GUI_EVENT_CREATE:
                        GUI_CONTROL_CLASS * pGc;

                        pGc= GUI_GcCreate(AskQueue.event,AskQueue.arg);
                        RespQueue.arg= (void *)pGc;
                        ret= RK_SUCCESS;
                        break;

                    case GUI_EVENT_DELETE:
                        pGc= (GUI_CONTROL_CLASS *)AskQueue.arg;
                        ret= GUI_GcDelete(pGc);
                        break;

                    case GUI_EVENT_OPERATE:
                        {
                            RKGUI_OPERATE_ARG * pArg= (RKGUI_OPERATE_ARG *)AskQueue.arg;
                            ret= GUI_OperWidge(pArg->pGc, AskQueue.event, pArg->arg);
                        }
                        break;

                    case APP_REQUEST_RECIVE_MSG:
                        if ( AskQueue.event )
                            ret= GUI_AppUnReciveMsg(AskQueue.arg);
                        else
                            ret= GUI_AppReciveMsg(AskQueue.arg);
                        break;

                    case APP_REQUEST_FOCUS:
                        if ( AskQueue.event )
                            ret= GUI_Deletefocus(AskQueue.arg);
                        else
                            ret= GUI_Setfocus(AskQueue.arg);
                        break;

                    case GUI_OPERATE_RESOURCE:

                    #ifdef _SPI_BOOT_
                    if(AskQueue.arg)
                    {
                        FILE_ATTR stFileAttr;

                        stFileAttr.FileName = NULL;
                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"A:\\Font12.bin";
                        #else
                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"B:\\Font12.bin";
                        #else
                        stFileAttr.Path = L"C:\\Font12.bin";
                        #endif
                        #endif

                        gstSpiResoure.Font12= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
                        if(gstSpiResoure.Font12== (HDC)RK_ERROR)
                        {
                            printf("open Font12.bin fault\n");
                            gstSpiResoure.Font12= NULL;
                        }

                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"A:\\Font16.bin";
                        #else
                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"B:\\Font16.bin";
                        #else
                        stFileAttr.Path = L"C:\\Font16.bin";
                        #endif
                        #endif
                        gstSpiResoure.Font16= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
                        if(gstSpiResoure.Font16== (HDC)RK_ERROR)
                        {
                            printf("open Font16.bin fault\n");
                            gstSpiResoure.Font16= NULL;
                        }

                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"A:\\RkNanoImage.uis";
                        #else
                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"B:\\RkNanoImage.uis";
                        #else
                        stFileAttr.Path = L"C:\\RkNanoImage.uis";
                        #endif
                        #endif
                        gstSpiResoure.Image= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
                        if(gstSpiResoure.Image== (HDC)RK_ERROR)
                        {
                            printf("open RkNanoImage.uis fault\n");
                            gstSpiResoure.Image= NULL;
                        }


                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"A:\\menu.res";
                        #else
                        #ifdef _HIDDEN_DISK1_
                        stFileAttr.Path = L"B:\\menu.res";
                        #else
                        stFileAttr.Path = L"C:\\menu.res";
                        #endif
                        #endif
                        gstSpiResoure.Menu= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
                        if(gstSpiResoure.Menu== (HDC)RK_ERROR)
                        {
                            printf("open menu.res fault\n");
                            gstSpiResoure.Menu= NULL;
                        }

                    }
                    else
                    {
                        if(gstSpiResoure.Font12!= NULL)
                        {
                            FileDev_CloseFile(gstSpiResoure.Font12);
                            gstSpiResoure.Font12= NULL;
                        }
                        if(gstSpiResoure.Font16!= NULL)
                        {
                            FileDev_CloseFile(gstSpiResoure.Font16);
                            gstSpiResoure.Font16= NULL;
                        }
                        if(gstSpiResoure.Image!= NULL)
                        {
                            FileDev_CloseFile(gstSpiResoure.Image);
                            gstSpiResoure.Image= NULL;
                        }
                        if(gstSpiResoure.Menu!= NULL)
                        {
                            FileDev_CloseFile(gstSpiResoure.Menu);
                            gstSpiResoure.Menu= NULL;
                        }
                    }
                    #endif
                    ret= RK_SUCCESS;
                    break;

                    default :
                        break;
                }

                RespQueue.error_code = ret;
                if (AskQueue.mode == SYNC_MODE)
                {
                    rkos_queue_send(pGUITaskControlBlock->pRespQueue,&RespQueue, MAX_DELAY);
                }
            }
        }

        if(rkos_queue_receive(pGUITaskControlBlock->pKeyQueue,&KeyQueue, 0)== RK_SUCCESS)
        {
             if(KeyQueue.event_type != 0)
            {
                if(GUITaskCheckIdle(hSelf) != RK_SUCCESS)
                {
                    continue;
                }

                switch (KeyQueue.event_type)
                {
                    case GUI_EVENT_KEY:
                    {
                        GUI_GC_RECIVE_MSG * pfGcFocusTemp = pGUITaskControlBlock->pfActiveFocus;
                        if(pfGcFocusTemp != NULL)
                        {
                            ret= pfGcFocusTemp->pfAppReciveMsg(KeyQueue.event_type ,KeyQueue.event, KeyQueue.arg, pfGcFocusTemp->pGc);
                            if(ret != RK_SUCCESS)
                            {
                                ret= GUI_CallBackApp(APP_RECIVE_MSG_EVENT_KEY, KeyQueue.event, KeyQueue.arg, NULL);
                            }
                            break;

                        }

                        ret= GUI_CallBackApp(APP_RECIVE_MSG_EVENT_KEY, KeyQueue.event, KeyQueue.arg, NULL);
                        break;
                    }

                    default :

                        break;
                }

            }

        }

        if(AskQueue.event_type != 0)
        {
            AskQueue.event_type = 0;
            continue;
        }

        if(CheckSystemIdle() != RK_SUCCESS)
        {
            GUI_CLASS_TIMER * pTimerTemp= pGUITaskControlBlock->pfTimerList;

            while (pTimerTemp != NULL && ((pGUITaskControlBlock->TimerLock == 0) || ((GUI_CONTROL_CLASS *)(pTimerTemp->pGc))->level))
            {
                if (pTimerTemp->pGc!= NULL)
                {
                    if(pTimerTemp->start)
                    {
                        pTimerTemp->pfTimerReciveMsg(pTimerTemp->pGc);
                    }
                }
                pTimerTemp= pTimerTemp->next;
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
** Name: GUITaskResume
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 14:37:37
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON FUN rk_err_t GUITaskResume(HTC hTask)
{
   RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;

   hDisplay0= RKDev_Open(DEV_CLASS_DISPLAY, 0, NULL);
#ifdef DISPLAY_DOUBLE_LAYER
   hDisplay1= RKDev_Open(DEV_CLASS_DISPLAY, 1, NULL);
#endif

    pShellTask->State = TASK_STATE_WORKING;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUITaskSuspend
** Input:HTC hTask, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 14:36:55
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON FUN rk_err_t GUITaskSuspend(HTC hTask, uint32 Level)
{
    RK_TASK_CLASS*   pGuiTask = (RK_TASK_CLASS*)hTask;

    if(Level == TASK_STATE_IDLE1)
    {
        pGuiTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pGuiTask->State = TASK_STATE_IDLE2;
    }

    RKDev_Close(hDisplay0);
    RKDev_Close(hDisplay1);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUITaskCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 18:46:32
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON FUN rk_err_t GUITaskCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        if(GUITaskResume(hTask) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_CallBackApp
** Input:uint32 EVENT_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GUI_CallBackApp(uint32 EVENT_type, uint32 event, void * arg, HGC pGc)
{
    GUI_APP_RECIVE_MSG * pfAppReciTemp= gpstGUICtrlBlock->pfAppCallBack;

    if(pfAppReciTemp!= NULL)
    {
        pfAppReciTemp->pfAppReciveMsg(EVENT_type, event, arg, pGc);
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}


/*******************************************************************************
** Name: GUI_AppReciveMsg
** Input:P_APP_RECIVE_MSG pApp
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GUI_AppReciveMsg(P_APP_RECIVE_MSG pApp)
{
    GUI_APP_RECIVE_MSG * pAppTemp = rkos_memory_malloc(sizeof(GUI_APP_RECIVE_MSG));
    GUI_APP_RECIVE_MSG * pstAppReciTemp;

    if (pAppTemp== NULL)
    {
        return RK_ERROR;
    }

    pAppTemp->pfAppReciveMsg= pApp;
    pAppTemp->next= gpstGUICtrlBlock->pfAppCallBack;
    gpstGUICtrlBlock->pfAppCallBack= pAppTemp;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_AppUnReciveMsg
** Input:P_APP_RECIVE_MSG pApp
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GUI_AppUnReciveMsg(P_APP_RECIVE_MSG pApp)
{
    GUI_APP_RECIVE_MSG *pAppTemp = gpstGUICtrlBlock->pfAppCallBack;
    GUI_APP_RECIVE_MSG *pLastAppTemp = NULL;

    while(pAppTemp!= NULL)
    {
        if (pAppTemp->pfAppReciveMsg== pApp)
        {
            if (pLastAppTemp== NULL)
            {
                gpstGUICtrlBlock->pfAppCallBack= gpstGUICtrlBlock->pfAppCallBack->next;
            }
            else
            {
                pLastAppTemp->next= pAppTemp->next;
            }
            rkos_memory_free(pAppTemp);
            return RK_SUCCESS;
        }
        pLastAppTemp= pAppTemp;
        pAppTemp= pAppTemp->next;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_Createfocus
** Input:P_APP_RECIVE_MSG pApp
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GUI_Createfocus(P_APP_RECIVE_MSG pApp, HGC pGc)
{
    #if 0
    //windows focus

    GUI_GC_RECIVE_MSG * pNewTemp= rkos_memory_malloc(sizeof(GUI_GC_RECIVE_MSG));
    GUI_GC_RECIVE_MSG * pGcTemp= gpstGUICtrlBlock->pfGcFocus;
    GUI_GC_RECIVE_MSG * pGcLastTemp= NULL;

    if( pNewTemp== NULL )
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(gpstGUICtrlBlock->FocusReqSem, MAX_DELAY);

    while(pGcTemp!= NULL)
    {
        pGcLastTemp= pGcTemp;
        pGcTemp= pGcTemp->next;
    }

    pNewTemp->pfAppReciveMsg= pApp;
    pNewTemp->pGc= pGc;
    pNewTemp->next= NULL;
    pNewTemp->active= 0;
    if(pGcLastTemp== NULL)
    {
        gpstGUICtrlBlock->pfGcFocus= pNewTemp;
    }
    else
    {
        pGcLastTemp->next= pNewTemp;
    }
    rkos_semaphore_give(gpstGUICtrlBlock->FocusReqSem);
    return RK_SUCCESS;
    #else

    //gc focus
    GUI_GC_RECIVE_MSG * pNewTemp = rkos_memory_malloc(sizeof(GUI_GC_RECIVE_MSG));
    GUI_CONTROL_CLASS * pGuiControl = (GUI_CONTROL_CLASS *)pGc;

    if( pNewTemp == NULL )
    {
        return RK_ERROR;
    }

    if(pGuiControl == NULL)
    {
        return RK_ERROR;
    }

    pNewTemp->pfAppReciveMsg = pApp;
    pNewTemp->pGc = pGc;
    pNewTemp->next = NULL;
    pGuiControl->focus = pNewTemp;

    gpstGUICtrlBlock->pfActiveFocus = pNewTemp;

    //printf("\ncreate focus = %x", pNewTemp);

    if(gpstGUICtrlBlock->pfGcFocus == NULL)
    {
        gpstGUICtrlBlock->pfGcFocus = pNewTemp;
    }
    else
    {
        pNewTemp->next = gpstGUICtrlBlock->pfGcFocus;
        gpstGUICtrlBlock->pfGcFocus = pNewTemp;
    }

    return RK_SUCCESS;

    #endif


}

/*******************************************************************************
** Name: GUI_Deletefocus
** Input:P_APP_RECIVE_MSG pApp
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GUI_Deletefocus(HGC pGc)
{
    #if 0
    //windows focus
    GUI_GC_RECIVE_MSG * pGcTemp= gpstGUICtrlBlock->pfGcFocus;
    GUI_GC_RECIVE_MSG * pLastTemp = NULL;
    if(pGc == NULL)
    {
        return RK_ERROR;
    }

    while(pGcTemp!= NULL)
    {
        if(pGcTemp->pGc == pGc)
        {
            if (pLastTemp == NULL)
            {
                gpstGUICtrlBlock->pfGcFocus = gpstGUICtrlBlock->pfGcFocus->next;
            }
            else
            {
                pLastTemp->next = pGcTemp->next;
            }
            rkos_memory_free(pGcTemp);
            rkos_semaphore_give(gpstGUICtrlBlock->FocusReqSem);
            return RK_SUCCESS;
        }
        pLastTemp= pGcTemp;
        pGcTemp= pGcTemp->next;
    }

    return RK_SUCCESS;
    #else
    GUI_CONTROL_CLASS * pGuiControl = (GUI_CONTROL_CLASS *)pGc;
    GUI_GC_RECIVE_MSG * pGcTemp = gpstGUICtrlBlock->pfGcFocus;
    GUI_GC_RECIVE_MSG * pLastTemp = NULL;
    if(pGc == NULL)
    {
        return RK_ERROR;
    }

    if(pGuiControl == NULL)
    {
        return RK_ERROR;
    }

    while(pGcTemp!= NULL)
    {
        if(pGcTemp->pGc == pGc)
        {
            if (pLastTemp == NULL)
            {
                gpstGUICtrlBlock->pfGcFocus = gpstGUICtrlBlock->pfGcFocus->next;
            }
            else
            {
                pLastTemp->next = pGcTemp->next;
            }
            rkos_memory_free(pGcTemp);
            //printf("\ndelete focus = %x", pGcTemp);
            break;
        }
        pLastTemp= pGcTemp;
        pGcTemp= pGcTemp->next;
    }

    pGuiControl->focus = NULL;

    gpstGUICtrlBlock->pfActiveFocus = gpstGUICtrlBlock->pfGcFocus;
    //printf("\n cur focus =  %x", gpstGUICtrlBlock->pfActiveFocus);

    return RK_SUCCESS;
    #endif
}

/*******************************************************************************
** Name: GUI_Setfocus
** Input:P_APP_RECIVE_MSG pApp
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GUI_Setfocus(HGC pGc)
{
    #if 0
    //windows focus

    GUI_GC_RECIVE_MSG * pGcTemp= gpstGUICtrlBlock->pfGcFocus;
    GUI_GC_RECIVE_MSG * pLastTemp = NULL;

    if(pGc == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(gpstGUICtrlBlock->FocusReqSem, MAX_DELAY);
    while(pGcTemp != NULL)
    {
        if (pGcTemp->pGc == pGc)
        {
            if( pLastTemp->next== NULL )
            {
                gpstGUICtrlBlock->pfGcFocus= pGcTemp;
                pGcTemp->next= NULL;
                gpstGUICtrlBlock->pfGcFocus->active= 1;
            }
            else
            {
                pLastTemp->next= pGcTemp->next;
                pGcTemp->next= gpstGUICtrlBlock->pfGcFocus->next;
                gpstGUICtrlBlock->pfGcFocus= pGcTemp;
                gpstGUICtrlBlock->pfGcFocus->active= 1;
            }
            break;
        }
        pLastTemp= pGcTemp;
        pGcTemp= pGcTemp->next;
    }

    rkos_semaphore_give(gpstGUICtrlBlock->FocusReqSem);
    return RK_SUCCESS;
    #else

    //gc focus
    GUI_CONTROL_CLASS * pGuiControl = (GUI_CONTROL_CLASS *)pGc;
    if(pGuiControl == NULL)
    {
        return RK_ERROR;
    }
    else
    {
        gpstGUICtrlBlock->pfActiveFocus = (GUI_GC_RECIVE_MSG *)pGuiControl->focus;
    }
    return RK_SUCCESS;

    #endif
}

/*******************************************************************************
** Name: rkgui_start_timer
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t rkgui_start_timer(HGC pGc)
{
    GUI_CLASS_TIMER * pTimerTemp= gpstGUICtrlBlock->pfTimerList;
    while (pTimerTemp!= NULL)
    {
        if (pTimerTemp->pGc== pGc)
        {
            pTimerTemp->start= 1;
            return RK_SUCCESS;
        }
        pTimerTemp= pTimerTemp->next;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: rkgui_stop_timer
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t rkgui_stop_timer(HGC pGc)
{
    GUI_CLASS_TIMER * pTimerTemp= gpstGUICtrlBlock->pfTimerList;
    while (pTimerTemp!= NULL)
    {
        if ( pTimerTemp->pGc== pGc )
        {
            pTimerTemp->start= 0;
        }
        pTimerTemp= pTimerTemp->next;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: rkgui_create_timer
** Input:HGC pGc, P_TEXT_RECIVE pfTimerReciveMsg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t rkgui_create_timer(HGC pGc, P_TEXT_RECIVE pfTimerReciveMsg)
{
    GUI_CLASS_TIMER * pTimerTemp= rkos_memory_malloc(sizeof(GUI_CLASS_TIMER));
    GUI_CLASS_TIMER * pstTimer;

    if (pTimerTemp== NULL)
    {
        printf("rkgui_create_timer: malloc GUI_CLASS_TIMER fault\n");
        return RK_ERROR;
    }

    pTimerTemp->pGc= pGc;
    pTimerTemp->pfTimerReciveMsg= pfTimerReciveMsg;
    pTimerTemp->next= NULL;
    pTimerTemp->start= 0;

    if (gpstGUICtrlBlock->pfTimerList== NULL)
    {
        gpstGUICtrlBlock->pfTimerList= pTimerTemp;
    }
    else
    {
        pstTimer= gpstGUICtrlBlock->pfTimerList;
        while (pstTimer->next!= NULL)
        {
            pstTimer= pstTimer->next;
        }
        pstTimer->next= pTimerTemp;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: rkgui_delete_timer
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t rkgui_delete_timer(HGC pGc)
{
    GUI_CLASS_TIMER *pTimerTemp = gpstGUICtrlBlock->pfTimerList;
    GUI_CLASS_TIMER *pLastTimerTemp = NULL;

    while (pTimerTemp != NULL)
    {
        if (pTimerTemp->pGc== pGc)
        {
            if (pLastTimerTemp== NULL)
            {
                gpstGUICtrlBlock->pfTimerList= gpstGUICtrlBlock->pfTimerList->next;
            }
            else
            {
                pLastTimerTemp->next= pTimerTemp->next;
            }
            rkos_memory_free(pTimerTemp);
            return RK_SUCCESS;
        }
        pLastTimerTemp= pTimerTemp;
        pTimerTemp= pTimerTemp->next;
    }
}


/*******************************************************************************
** Name:UpdataRegister
** Input:GUI_CONTROL_CLASS *pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t UpdataRegister(GUI_CONTROL_CLASS *pGc)
{
    GUI_CONTROL_CLASS *pstGcList;
    GUI_CONTROL_CLASS *pGcTemp;

    if (pfUpdataGcList == NULL)
    {
        pfUpdataGcList = pGc;
        pGc->updata= NULL;
    }
    else
    {
        pGcTemp= pfUpdataGcList;
        while (pGcTemp->updata != NULL)
        {
            if( pGcTemp== pGc )
            {
                return RK_SUCCESS;
            }
            pGcTemp= pGcTemp->updata;
        }

        pGcTemp->updata = pGc;
        pGc->updata= NULL;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name:UpdataRegister
** Input:GUI_CONTROL_CLASS *pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t TraversalUpdataList(GUI_CONTROL_CLASS *list)
{
    GUI_CONTROL_CLASS * pstGcList = list;

    while(pstGcList != NULL)
    {
        if(pstGcList->level == 0)
        {
            GcUpdataRegister(pfGcList, pstGcList);
        }
        else
        {
            GcUpdataRegister(pfGcListTop, pstGcList);
        }
        pstGcList= pstGcList->updata;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name:GcUpdataRegister
** Input:GUI_CONTROL_CLASS * pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GcUpdataRegister(GUI_CONTROL_CLASS * list, GUI_CONTROL_CLASS * pGc)
{
    uint32 flag;

    GUI_CONTROL_CLASS * pstGcList= list;

    flag = 0;

    while (pstGcList!= NULL)
    {
        //printf("\n gui gc = %x", pstGcList);

        if(pstGcList == pGc)
        {
            flag = 1;
            pstGcList = pstGcList->next;
            continue;
        }

        if((pGc->lucency == OPACITY) && flag)
        {
            pGc->flag = 1;
            return RK_SUCCESS;
        }

        if(pstGcList->display == 0)
        {
            pstGcList = pstGcList->next;
            //printf("\n shade");
            continue;
        }

        if(pstGcList->flag == 1)
        {
            pstGcList = pstGcList->next;
            //printf("\already change");
            continue;
        }

        #if 0
        if(pGc->translucence)
        {
            if( pGc->level==0 )
            {
                pstGcList= pfGcList;
            }
            else if( pGc->level==1 )
            {
                pstGcList= pfGcListTop;
            }

            while(pstGcList!= NULL)
            {
                pstGcList->flag= 1;
                pstGcList= pstGcList->next;
            }

            pfUpdataGcList= NULL;
            return RK_SUCCESS;
        }
        #endif

        if( (pGc->x0 > (pstGcList->xSize + pstGcList->x0))
            || ((pGc->x0 + pGc->xSize) < pstGcList->x0)
            || (pGc->y0> (pstGcList->ySize+ pstGcList->y0))
            || ((pGc->y0 + pGc->ySize) < pstGcList->y0))
        {
            //printf("\n not x0 = %d, %d, y0 = %d, %d, xSize = %d, %d, ySize = %d, %d", pGc->x0,pstGcList->x0,pGc->y0,pstGcList->y0,
            //    pGc->xSize,pstGcList->xSize, pGc->ySize,pstGcList->ySize);

            pstGcList = pstGcList->next;
            continue;
        }

        //printf("\nlevel = %d, gui update = %x", pGc->level, pstGcList);

        pstGcList->flag = 1;
        UpdataRegister(pstGcList);
        pstGcList = pstGcList->next;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name:GcRelevanceOper
** Input:eRKGUI_OPERATE_CMD cmd, void * arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GcRelevanceOper(GUI_CONTROL_CLASS * pGc)
{
    GUI_CONTROL_CLASS * pList;

    if(pGc==NULL)
    {
        return RK_ERROR;
    }

    if(pGc->level==0)
    {
        pList= pfGcList;
    }
    else
    {
        pList= pfGcListTop;
    }

    if(pList== NULL)
    {
        return RK_SUCCESS;
    }

    pfUpdataGcList = NULL;

    GcUpdataRegister(pList, pGc);
    TraversalUpdataList(pfUpdataGcList);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name:GcDisplay
** Input:GUI_CONTROL_CLASS * pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GcDisplay(GUI_CONTROL_CLASS * pGc)
{
    rk_err_t ret= RK_SUCCESS;
    GUI_CONTROL_CLASS * pList, * pLast;

    if(pGc->level==0)
    {
        pList = pfGcList;
        pLast = pList;
    }
    else
    {
        pList= pfGcListTop;
        pLast = pList;
    }

    while(pList != NULL)
    {
         pLast = pList;
         pList= pList->next;
    }

    while (pLast != NULL)
    {
        //printf("\n leve = %d, list = %x", pGc->level,  pLast);

        if((pLast->flag) && (pLast->display))
        {
            //printf("\t\ndisplay = %x", pLast);
            if( GUI_DisplayWidge(pLast)!= RK_SUCCESS )
            {
                ret= RK_ERROR;
            }
        }
        pLast->flag = 0;
        pLast= pLast->prev;
    }

    return ret;
}

/*******************************************************************************
** Name:GcRelevanceRegister
** Input:GUI_CONTROL_CLASS *pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GcRelevanceRegister(GUI_CONTROL_CLASS *pGc)
{
    GUI_CONTROL_CLASS *pstGcList;
    GUI_CONTROL_CLASS *pGcTemp;

    if( pGc== NULL )
    {
        return RK_ERROR;
    }

    if( pGc->level==0 )
    {
        pstGcList= pfGcList;
    }
    else
    {
        pstGcList= pfGcListTop;
    }

    //printf("\nlevel = %d, gui create  = %x xSize = %d", pGc->level, pGc, pGc->xSize);
    if (pstGcList == NULL)
    {
        if( pGc->level==0 )
        {
            pfGcList = pGc;
        }
        else
        {
            pfGcListTop= pGc;
        }

        pGc->next= NULL;
        pGc->prev = NULL;
    }
    else
    {

        if( pGc->level == 0 )
        {
            pfGcList->prev = pGc;
            pGc->next = pfGcList;
            pfGcList = pGc;
        }
        else
        {
            pfGcListTop->prev = pGc;
            pGc->next = pfGcListTop;
            pfGcListTop = pGc;
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name:GcRelevanceUnRegister
** Input:GUI_CONTROL_CLASS *pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
rk_err_t GcRelevanceUnRegister(GUI_CONTROL_CLASS *pGc)
{
    GUI_CONTROL_CLASS *pGcTemp;
    GUI_CONTROL_CLASS *pLastGcTemp = NULL;

    if( pGc== NULL)
    {
        return RK_ERROR;
    }

    if( pGc->level==0 )
    {
        pGcTemp= pfGcList;
        pLastGcTemp = NULL;
    }
    else
    {
        pGcTemp= pfGcListTop;
        pLastGcTemp = NULL;
    }

    //printf("\ngui delete  = %x", pGc);

    while (pGcTemp != NULL)
    {
        if (pGcTemp == pGc)
        {
            if(pGc->level == 0)
            {
                if(pLastGcTemp == NULL)
                {
                    pGc->next->prev = NULL;
                    pfGcList = pGc->next;
                }
                else
                {
                    pGc->next->prev = pLastGcTemp;
                    pLastGcTemp->next = pGc->next;
                }
            }
            else
            {
                if(pLastGcTemp == NULL)
                {
                    pGc->next->prev = NULL;
                    pfGcListTop = pGc->next;
                }
                else
                {
                    pGc->next->prev = pLastGcTemp;
                    pLastGcTemp->next = pGc->next;
                }
            }

            return RK_SUCCESS;
        }

        pLastGcTemp = pGcTemp;
        pGcTemp = pGcTemp->next;
    }
    return RK_ERROR;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GUITask_DevInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_INIT_
INIT API rk_err_t GUITask_DeInit(void *pvParameters)
{
    RK_TASK_CLASS* TaskHandler = (RK_TASK_CLASS*)pvParameters;

    if(gpstGUICtrlBlock->StateBackground!= NULL)
    {
        GUI_ImageDelete(gpstGUICtrlBlock->StateBackground);
        GcRelevanceUnRegister(gpstGUICtrlBlock->StateBackground);
    }
    if(gpstGUICtrlBlock->Background!= NULL)
    {
        GUI_ImageDelete(gpstGUICtrlBlock->Background);
        GcRelevanceUnRegister(gpstGUICtrlBlock->Background);
    }

#ifdef _SPI_BOOT_
    if(gstSpiResoure.Font12!= NULL)FileDev_CloseFile(gstSpiResoure.Font12);
    if(gstSpiResoure.Font16!= NULL)FileDev_CloseFile(gstSpiResoure.Font16);
    if(gstSpiResoure.Image!= NULL)FileDev_CloseFile(gstSpiResoure.Image);
    if(gstSpiResoure.Menu!= NULL)FileDev_CloseFile(gstSpiResoure.Menu);
#endif
    RKDev_Close(hDisplay0);
#ifdef DISPLAY_DOUBLE_LAYER
    RKDev_Close(hDisplay1);
#endif

    rkos_queue_delete(gpstGUICtrlBlock->pAskQueue);
    rkos_queue_delete(gpstGUICtrlBlock->pRespQueue);
    rkos_queue_delete(gpstGUICtrlBlock->pKeyQueue);
    rkos_memory_free(gpstGUICtrlBlock);
    rk_printf("gui de init");
    return RK_SUCCESS;

exit:
    return RK_ERROR;

}

/*******************************************************************************
** Name: GUITask_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.30
** Time: 17:15:15
*******************************************************************************/
_GUI_GUITASK_INIT_
INIT API rk_err_t GUITask_Init(void *pvParameters, void *arg)
{
    int i;
    RK_TASK_CLASS*   pGUITask = (RK_TASK_CLASS*)pvParameters;
    GUI_TASK_CTRL_BLOCK*  pGUITaskControlBlock;
    if (pGUITask == NULL)
    {
        return RK_PARA_ERR;
    }

    pGUITask->Idle1EventTime = 10 * PM_TIME;
    pGUITask->TaskResumeFun = GUITaskResume;
    pGUITask->TaskSuspendFun = GUITaskSuspend;

    pGUITaskControlBlock = (GUI_TASK_CTRL_BLOCK*) rkos_memory_malloc(sizeof(GUI_TASK_CTRL_BLOCK));
    if (pGUITaskControlBlock == NULL)
    {
        return RK_ERROR;
    }

    memset(pGUITaskControlBlock, NULL, sizeof(GUI_TASK_CTRL_BLOCK));

    pGUITaskControlBlock->pAskQueue = rkos_queue_create(GUI_ASK_QUEUE_LENTH,GUI_ASK_QUEUE_SIZE);
    pGUITaskControlBlock->pRespQueue = rkos_queue_create(GUI_RESP_QUEUE_LENTH,GUI_RESP_QUEUE_SIZE);
    pGUITaskControlBlock->pKeyQueue = rkos_queue_create(GUI_KEY_QUEUE_LENTH,GUI_KEY_QUEUE_SIZE);

    pGUITaskControlBlock->pfAppCallBack= NULL;
    pGUITaskControlBlock->pfGcFocus= NULL;
    pGUITaskControlBlock->TimerLock= 0;
    pGUITaskControlBlock->pfTimerList= NULL;

    pfGcList= NULL;
    pfGcListTop= NULL;
    pGUITaskControlBlock->ApiReqSem= rkos_semaphore_create(1,1);

    pGUITaskControlBlock->ScreenLock= 0;
    hDisplay0= RKDev_Open(DEV_CLASS_DISPLAY, 0, NULL);
#ifdef DISPLAY_DOUBLE_LAYER
    hDisplay1= RKDev_Open(DEV_CLASS_DISPLAY, 1, NULL);
#endif

    pGUITaskControlBlock->Background= NULL;
    pGUITaskControlBlock->StateBackground= NULL;

#ifdef _SPI_BOOT_
    FILE_ATTR stFileAttr;

    gstSpiResoure.osReadSource = rkos_semaphore_create(1, 1);
    if(gstSpiResoure.osReadSource != NULL)
    {
        stFileAttr.FileName = NULL;
        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"A:\\Font12.bin";
        #else
        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"B:\\Font12.bin";
        #else
        stFileAttr.Path = L"C:\\Font12.bin";
        #endif
        #endif
        gstSpiResoure.Font12= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if(gstSpiResoure.Font12== (HDC)RK_ERROR)
        {
            printf("open Font12.bin fault\n");
            gstSpiResoure.Font12= NULL;
        }

        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"A:\\Font16.bin";
        #else
        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"B:\\Font16.bin";
        #else
        stFileAttr.Path = L"C:\\Font16.bin";
        #endif
        #endif

        gstSpiResoure.Font16= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if(gstSpiResoure.Font16== (HDC)RK_ERROR)
        {
            printf("open Font16.bin fault\n");
            gstSpiResoure.Font16= NULL;
        }

        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"A:\\RkNanoImage.uis";
        #else
        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"B:\\RkNanoImage.uis";
        #else
        stFileAttr.Path = L"C:\\RkNanoImage.uis";
        #endif
        #endif

        gstSpiResoure.Image= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if(gstSpiResoure.Image== (HDC)RK_ERROR)
        {
            printf("open RkNanoImage.uis fault\n");
            gstSpiResoure.Image= NULL;
        }

        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"A:\\menu.res";
        #else
        #ifdef _HIDDEN_DISK1_
        stFileAttr.Path = L"B:\\menu.res";
        #else
        stFileAttr.Path = L"C:\\menu.res";
        #endif
        #endif


        gstSpiResoure.Menu= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if(gstSpiResoure.Menu== (HDC)RK_ERROR)
        {
            printf("open menu.res fault\n");
            gstSpiResoure.Menu= NULL;
        }
    }
    else
    {
        gstSpiResoure.Font12= NULL;
        gstSpiResoure.Font16= NULL;
        gstSpiResoure.Image = NULL;
        gstSpiResoure.Menu= NULL;
    }
#endif

    gpstGUICtrlBlock = pGUITaskControlBlock;

    KeyDev_Register(hKey, KeySendMsgToGUI);

    FREQ_EnterModule(FREQ_BLON);

    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _GUI_SHELL_
static SHELL_CMD ShellGuiName[] =
{
    "test",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "pcb",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",                         // the end
};


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GuiShell
** Input:HDC dev
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.29
** Time: 10:43:17
*******************************************************************************/
_GUI_GUITASK_SHELL_
rk_err_t GuiShell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellGuiName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    switch (i)
    {
        case 0x00:  //test
            ret = GuiShellTest(dev,pItem);
            break;

        case 0x01:  //help
            ret = GuiShellHelp(dev,pItem);
            break;

        case 0x02:
            ret = GuiShellPcb(dev, pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GuiShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.1.6
** Time: 10:16:36
*******************************************************************************/
_GUI_GUITASK_SHELL_
SHELL FUN rk_err_t GuiShellPcb(HDC dev, uint8 * pstr)
{
    uint32 TaskID;
    GUI_TASK_CTRL_BLOCK * pstGuiTask;
    uint32 i;

#ifdef SHELL_HELP
    pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA((uint8 *)pstr, "help", 4) == 0)
        {
            rk_print_string("gui.pcb : gui pcb info cmd.\r\n");
            return RK_SUCCESS;
        }
    }
    pstr++;
#endif
    // TODO:
    //add other code below:
    //...


    TaskID = String2Num(pstr);

    #if 0
    if(TaskID >= LUN_MAX_NUM)
    {
        return RK_ERROR;
    }
    #endif

    pstGuiTask = gpstGUICtrlBlock;

    if(pstGuiTask == NULL)
    {
        rk_printf("gui%d in not exist", TaskID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstGUICtrlBlock");
    rk_printf_no_time("    .pAskQueue = %08x",  pstGuiTask->pAskQueue);
    rk_printf_no_time("    .pRespQueue = %08x", pstGuiTask->pRespQueue);
    rk_printf_no_time("    .pKeyQueue = %08x", pstGuiTask->pKeyQueue);
    rk_printf_no_time("    .pfAppCallBack = %08x", pstGuiTask->pfAppCallBack);
    rk_printf_no_time("    .pfGcFocus = %08x",pstGuiTask->pfGcFocus);
    rk_printf_no_time("    .pfActiveFocus = %08x",pstGuiTask->pfActiveFocus);
    rk_printf_no_time("    .TimerLock = %d",pstGuiTask->TimerLock);
    rk_printf_no_time("    .pfTimerList = %08x",pstGuiTask->pfTimerList);
    rk_printf_no_time("    .ScreenLock = %08x",pstGuiTask->ScreenLock);
    rk_printf_no_time("    .Background = %d",pstGuiTask->Background);
    rk_printf_no_time("    .StateBackground = %d",pstGuiTask->StateBackground);

    {
        GUI_GC_RECIVE_MSG * pFocus;
        pFocus = pstGuiTask->pfGcFocus;
        while(pFocus != NULL)
        {
            rk_printf_no_time(".pfGcFocus = %x", pFocus);
            pFocus = pFocus->next;
        }
    }

    rk_printf_no_time("All level 0 class:");
    {
        GUI_CONTROL_CLASS *pGcList= pfGcList;
        while(pGcList != NULL)
        {
            rk_printf_no_time("    .pGcList = %x    display = %d", pGcList, pGcList->display);
            pGcList = pGcList->next;
        }
    }

    rk_printf_no_time("All level 1 class:");
    {
        GUI_CONTROL_CLASS *pTopGcList= pfGcListTop;
        while(pTopGcList != NULL)
        {
            rk_printf_no_time("    .pTopGcList = %x    display = %d", pTopGcList, pTopGcList->display);
            pTopGcList = pTopGcList->next;
        }
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: GuiTask_MsgBoxCallBack
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_SHELL_
COMMON API rk_err_t GuiTask_MsgBoxCallBack(uint32 EVENT_type, uint32 event, void * arg, HGC pGc)
{
    if(EVENT_type==APP_RECIVE_MSG_EVENT_DIALOG)
    {
        printf("on_off:%d\n",(int)arg);
    }
    else if(EVENT_type==APP_RECIVE_MSG_EVENT_KEY)
    {
        printf("APP_RECIVE_MSG_EVENT_KEY:%x\n",event);
    }
}

/*******************************************************************************
** Name: MainTask_SelectCallBack
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_SHELL_
COMMON API rk_err_t GuiTask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE EVENT_type, RKGUI_SELECT_ITEM * item, int offset)
{
    switch(EVENT_type)
    {
        case SELECT_ENVEN_UPDATA:
//            item->cmd= TEXT_CMD_ID;
//            item->text_id= offset;
            item->cmd= TEXT_CMD_BUF;
            item->text[0]= 0x0031+offset;
            item->text[1]= 0x0031+offset;
            item->text[2]= 0x0031+offset;
            item->text[3]= 0x0031+offset;
            item->text[4]= 0x0031+offset;
            item->text[5]= 0x0031+offset;
            item->text[6]= 0x0031+offset;
            item->text[7]= 0x0031+offset;
            item->text[8]= 0x0031+offset;
            item->text[9]= 0x0031+offset;
            item->text[10]= 0x0031+offset+ 1;
            item->text[11]= 0x0031+offset;
            item->text[12]= 0x0031+offset;
            item->text[13]= 0x0031+offset;
            item->text[14]= 0x0031+offset;
            item->text[15]= 0x0031+offset;
            item->text[16]= 0x0031+offset;
            item->text[17]= 0x0031+offset;
            item->text[18]= 0x0031+offset;
            item->text[19]= 0x0031+offset;
            item->text[20]= 0x0031+offset;
            item->text[21]= 0x0031+offset;
            item->text[22]= 0x0031+offset;
            item->text[23]= 0x0031+offset;
            item->text[24]= 0;
            item->sel_icon= IMG_ID_SETMENU_ICON_SEL;
            item->unsel_icon= IMG_ID_SETMENU_ICON_NOSEL;
            break;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GuiShellHelp
** Input:HDC dev
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.29
** Time: 10:43:17
*******************************************************************************/
_GUI_GUITASK_SHELL_
rk_err_t GuiShellHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if ( StrLenA( pstr) != 0)
        return RK_ERROR;

    printf("test      测试gui命令    \r\n");
    printf("help      显示gui命令帮助信息  \r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GuiShellTest
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.29
** Time: 10:43:17
*******************************************************************************/
_GUI_GUITASK_SHELL_
rk_err_t GuiShellTest(HDC dev,  uint8 * pstr)
{
    GUI_CONTROL_CLASS * hGc, * hGc1, * hGc2, * hGc3;

    RKGUI_ICON_ARG pstIconArg;
    RKGUI_IMAGE_ARG pstImageArg;
    RKGUI_TEXT_ARG pstTextArg;
    RKGUI_CHAIN_ARG pstChainArg;
    RKGUI_MSGBOX_ARG pstMsgBoxArg;
    RKGUI_PROGRESSBAR_ARG pstProgressArg;
    RKGUI_SPECTRUM_ARG pstSpectrumArg;

    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    RKGUI_TEXT_ARG pstTextContent;
    RKGUI_SPECTRUM_ARG pstSpectrumContent;

//    pstIconArg.x= 0;
//    pstIconArg.y= 0;
//    pstIconArg.display = 1;
//    pstIconArg.resource= IMG_ID_MUSIC_BACKGROUND;
//    pstIconArg.level= 0;
//    hGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    uint8 buf[12]= {35,35,35,35,35,35,35,35,35,35,35,35};
//    uint8 buf2[12]= {15,15,15,15,15,15,15,15,15,15,15,15};

//    pstSpectrumArg.x= 23;
//    pstSpectrumArg.y= 29;
//    pstSpectrumArg.level= 0;
//    pstSpectrumArg.display= 1;
//    pstSpectrumArg.StripNum= 12;
//    pstSpectrumArg.StripStyle= IMG_ID_MUSIC_SPECTRUM20;
//    pstSpectrumArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
//    pstSpectrumArg.BackdropX= 23;
//    pstSpectrumArg.BackdropY= 29;
//    pstSpectrumArg.StripDat= buf;
//    hGc= GUITask_CreateWidget(GUI_CLASS_SPECTRUM, &pstSpectrumArg);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(1000);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(1000);

//    DelayMs(2000);

//    while(1)
//    {
//        pstSpectrumContent.cmd= SPECTRUM_SET_CONTENT;
//        pstSpectrumContent.StripDat= buf;
//        Gpio_SetPinLevel(GPIO_CH2, GPIOPortA_Pin3, 0);
//        GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstSpectrumContent, SYNC_MODE);

//        pstSpectrumContent.StripDat= buf2;
//        Gpio_SetPinLevel(GPIO_CH2, GPIOPortA_Pin3, 1);
//        GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstSpectrumContent, SYNC_MODE);
//    }

//    {
////        char *buf;
//        PICTURE_INFO_STRUCT psPictureInfo;
////        if( FW_GetPicInfoWithIDNum(0, &psPictureInfo)!= RK_SUCCESS )
////        {
////            return RK_ERROR;
////        }
////        buf= (uint8 *)rkos_memory_malloc(6000);
////
//        while(1)
//        {
//            Gpio_SetPinLevel(GPIO_CH2, GPIOPortA_Pin3, 0);
//            FW_GetPicResource(psPictureInfo.offsetAddr, buf, 6000);
////            FW_GetPicInfoWithIDNum(0, &psPictureInfo);
//
//            Gpio_SetPinLevel(GPIO_CH2, GPIOPortA_Pin3, 1);
//            FW_GetPicResource(psPictureInfo.offsetAddr, buf, 6000);
////            FW_GetPicInfoWithIDNum(0, &psPictureInfo);
//        }
//    }


//    pstImageArg.cmd= IMAGE_CMD_RECT;
//    pstImageArg.R= 0;
//    pstImageArg.G= 0xFF;
//    pstImageArg.B= 0;
//    pstImageArg.transparency= 0;
//    pstImageArg.x= 100;
//    pstImageArg.y= 30;
//    pstImageArg.xSize= 5;
//    pstImageArg.ySize= 90;
//    pstImageArg.display = 1;
//    pstImageArg.level= 0;
//    pstImageArg.lucency= OPACITY;

//    hGc= GUITask_CreateWidget(GUI_CLASS_IMAGE, &pstImageArg);
//    DelayMs(500);

//    pstProgressArg.x= 7;
//    pstProgressArg.y= 88;
//    pstProgressArg.display = 1;
//    pstProgressArg.level= 0;

//    pstProgressArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
//    pstProgressArg.BackdropX= 7;
//    pstProgressArg.BackdropY= 78;

//    pstProgressArg.Bar= IMG_ID_PROGRESS_BAR;
//    pstProgressArg.percentage= 10000;
//    hGc= GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
//    DelayMs(500);

//    pstImageArg.cmd= IMAGE_CMD_RECT;
//    pstImageArg.R= 0xFF;
//    pstImageArg.G= 0;
//    pstImageArg.B= 0;
//    pstImageArg.transparency= 0;
//    pstImageArg.x= 90;
//    pstImageArg.y= 30;
//    pstImageArg.xSize= 5;
//    pstImageArg.ySize= 90;
//    pstImageArg.display = 1;
//    pstImageArg.level= 0;
//    pstImageArg.lucency= OPACITY;

//    hGc1= GUITask_CreateWidget(GUI_CLASS_IMAGE, &pstImageArg);
//    DelayMs(500);

//    pstProgressBarContent.percentage= 5000;
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    pstIconArg.x= 0;
//    pstIconArg.y= 0;
//    pstIconArg.display= 1;
//    pstIconArg.resource= IMG_ID_STATE_BACKGROUND;
//    pstIconArg.level= 1;
//    hGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);

//    pstIconArg.x= 0;
//    pstIconArg.y= 0;
//    pstIconArg.display = 1;
//    pstIconArg.resource= 200;
//    pstIconArg.level= 1;
//    hGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    pstImageArg.cmd= IMAGE_CMD_RECT;
//    pstImageArg.R= 0;
//    pstImageArg.G= 0xFF;
//    pstImageArg.B= 0;
//    pstImageArg.transparency= 0;
//    pstImageArg.x= 30;
//    pstImageArg.y= 30;
//    pstImageArg.xSize= 50;
//    pstImageArg.ySize= 50;
//    pstImageArg.display = 1;
//    pstImageArg.level= 0;
//    pstImageArg.lucency= OPACITY;

//    hGc= GUITask_CreateWidget(GUI_CLASS_IMAGE, &pstImageArg);
//    DelayMs(500);

//    pstImageArg.cmd= IMAGE_CMD_RECT;
//    pstImageArg.R= 0xFF;
//    pstImageArg.G= 0;
//    pstImageArg.B= 0;
//    pstImageArg.transparency= 0;
//    pstImageArg.x= 50;
//    pstImageArg.y= 30;
//    pstImageArg.xSize= 50;
//    pstImageArg.ySize= 50;
//    pstImageArg.display = 1;
//    pstImageArg.level= 0;
//    pstImageArg.lucency= OPACITY;

//    hGc1= GUITask_CreateWidget(GUI_CLASS_IMAGE, &pstImageArg);
//    DelayMs(500);

//    pstImageArg.cmd= IMAGE_CMD_RECT;
//    pstImageArg.R= 0;
//    pstImageArg.G= 0;
//    pstImageArg.B= 0xFF;
//    pstImageArg.transparency= 0;
//    pstImageArg.x= 40;
//    pstImageArg.y= 40;
//    pstImageArg.xSize= 50;
//    pstImageArg.ySize= 50;
//    pstImageArg.display = 1;
//    pstImageArg.level= 0;
//    pstImageArg.lucency= OPACITY;

//    hGc2= GUITask_CreateWidget(GUI_CLASS_IMAGE, &pstImageArg);
//    DelayMs(500);

//    pstImageArg.cmd= IMAGE_CMD_RECT;
//    pstImageArg.R= 0;
//    pstImageArg.G= 0;
//    pstImageArg.B= 0xFF;
//    pstImageArg.transparency= 0;
//    pstImageArg.x= 40;
//    pstImageArg.y= 92;
//    pstImageArg.xSize= 20;
//    pstImageArg.ySize= 20;
//    pstImageArg.display = 1;
//    pstImageArg.level= 0;
//    pstImageArg.lucency= OPACITY;

//    hGc3= GUITask_CreateWidget(GUI_CLASS_IMAGE, &pstImageArg);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc1, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc1, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc2, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc2, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc3, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc3, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    pstImageSize.x= 10;
//    pstImageSize.y= 50;
//    GuiTask_OperWidget(hGc1, OPERATE_SET_SIZE, &pstImageSize, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc2, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc2, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    pstIconArg.x= 35;
//    pstIconArg.y= 30;
//    pstIconArg.display = 1;
//    pstIconArg.resource= 234;
//    pstIconArg.level= 0;
//    hGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, (void *)200, SYNC_MODE);
//    DelayMs(500);

//    pstChainArg.x= 20;
//    pstChainArg.y= 40;
//    pstChainArg.resource = IMG_ID_WAIT0;
//    pstChainArg.num = 16;
//    pstChainArg.delay = 10;
//    pstChainArg.level = 0;
//    pstChainArg.display = 1;
//    pstChainArg.blurry= 0;
//    hGc= GUITask_CreateWidget(GUI_CLASS_CHAIN, &pstChainArg);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_AppReciveMsg(GuiTask_MsgBoxCallBack);
//    pstMsgBoxArg.cmd= MSGBOX_CMD_WARNING;
//    pstMsgBoxArg.x= 4;
//    pstMsgBoxArg.y= 20;
//    pstMsgBoxArg.display = 1;
//    pstMsgBoxArg.level= 0;
//    pstMsgBoxArg.title= SID_WARNING;
//    pstMsgBoxArg.text_cmd= TEXT_CMD_ID;
//    pstMsgBoxArg.text=  (void *)0;
//    pstMsgBoxArg.Button_On= SID_YES;
//    pstMsgBoxArg.Button_Off= SID_NO;
//    pstMsgBoxArg.align= TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;
//    hGc= GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);
//    GuiTask_DeleteWidget(hGc);

//    pstTextArg.cmd= TEXT_CMD_BUF;
//    pstTextArg.align= TEXT_ALIGN_X_LEFT;
//    pstTextArg.x= 20;
//    pstTextArg.y= 50;
//    pstTextArg.xSize= 100;
//    pstTextArg.ySize= 50;

//    pstTextArg.lucency= OPACITY;
//    pstTextArg.Backdrop= IMG_ID_POWERON0;
//    pstTextArg.ForegroundR= 0;
//    pstTextArg.ForegroundG= 0;
//    pstTextArg.ForegroundB= 0;

//    pstTextArg.BackgroundR= 255;
//    pstTextArg.BackgroundG= 255;
//    pstTextArg.BackgroundB= 255;
//    pstTextArg.BackdropX= 20;
//    pstTextArg.BackdropY= 50;

//    pstTextArg.text= TEXT("\u663e\u793a\u5185\u5bb9");
//    pstTextArg.display = 1;
//    pstTextArg.level= 0;
//    hGc= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_RIGHT;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_CENTER;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_LEFT| TEXT_ALIGN_Y_DOWN;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_RIGHT| TEXT_ALIGN_Y_DOWN;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_CENTER| TEXT_ALIGN_Y_DOWN;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_LEFT| TEXT_ALIGN_Y_CENTER;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_RIGHT| TEXT_ALIGN_Y_CENTER;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_CENTER| TEXT_ALIGN_Y_CENTER;
//    pstTextContent.txt= TEXT("\u663e\u793a\u5185\u5bb9");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(1000);

//    pstTextContent.align= TEXT_ALIGN_X_CENTER| TEXT_ALIGN_Y_CENTER;
//    pstTextContent.txt= TEXT("\u6eda\u52a8\u663e\u793a\u6d4b\u8bd5\u0031\u0032\u0033\u0034\u0035\u0036\u0037\u0038\u0039\u0030");
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_START_MOVE, (void *)10, SYNC_MODE);
//    DelayMs(5000);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    pstTextArg.cmd= TEXT_CMD_BUF;
//    pstTextArg.align= TEXT_ALIGN_X_RIGHT;
//    pstTextArg.x= 20;
//    pstTextArg.y= 50;
//    pstTextArg.xSize= 100;
//    pstTextArg.ySize= -1;

//    pstTextArg.lucency= OPACITY;
//    pstTextArg.Backdrop= -1;
//    pstTextArg.ForegroundR= 0;
//    pstTextArg.ForegroundG= 0;
//    pstTextArg.ForegroundB= 0;

//    pstTextArg.BackgroundR= 255;
//    pstTextArg.BackgroundG= 255;
//    pstTextArg.BackgroundB= 255;
//    pstTextArg.BackdropX= 20;
//    pstTextArg.BackdropY= 50;

//    pstTextArg.text= TEXT("\u663e\u793a\u5185\u5bb9");
//    pstTextArg.display = 1;
//    pstTextArg.level= 0;
//    hGc= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
//    DelayMs(500);

//    pstTextArg.cmd= TEXT_CMD_ID;
//    pstTextArg.align= TEXT_ALIGN_X_RIGHT;
//    pstTextArg.x= 20;
//    pstTextArg.y= 50;
//    pstTextArg.xSize= 100;
//    pstTextArg.ySize= -1;

//    pstTextArg.lucency= LUCENCY;
//    pstTextArg.ForegroundR= 0;
//    pstTextArg.ForegroundG= 0;
//    pstTextArg.ForegroundB= 0;

//    pstTextArg.resource= 0;
//    pstTextArg.display = 1;
//    pstTextArg.level= 0;
//    hGc= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
//    DelayMs(500);

//    pstTextContent.align= TEXT_ALIGN_X_RIGHT;
//    pstTextContent.resource= 1;
//    GuiTask_OperWidget(hGc, OPERATE_SET_CONTENT, &pstTextContent, SYNC_MODE);
//    DelayMs(500);

//    GuiTask_OperWidget(hGc, OPERATE_START_MOVE, (void *)10, SYNC_MODE);
//    DelayMs(5000);

//    GuiTask_SetDisplayStatusBar(1);
//    DelayMs(500);
//    GuiTask_SetDisplayStatusBar(0);
//    DelayMs(500);

//    {
//        HGC pGc;
//        RKGUI_SELECT_ARG stSelectArg;
//        RKGUI_SELECT_CONTENT parameter;

//        stSelectArg.x= 0;
//        stSelectArg.y= 20;
//        stSelectArg.xSize= 128;
//        stSelectArg.ySize= 137;
//        stSelectArg.display = 1;
//        stSelectArg.level= 0;

//        stSelectArg.Background= IMG_ID_BROWSER_BACKGROUND;
//        stSelectArg.SeekBar= IMG_ID_BROWSER_SCOLL2;
//        stSelectArg.Cursor= 1;
//        stSelectArg.ItemStartOffset= 0;
//        stSelectArg.itemNum= 14;
//        stSelectArg.MaxDisplayItem= 8;
//        stSelectArg.IconBoxSize= 18;
//        stSelectArg.pReviceMsg= GuiTask_SelectCallBack;
//        stSelectArg.CursorStyle= IMG_ID_SEL_ICON;

//        pGc= GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
//        DelayMs(3000);

//        parameter.itemNum= 19;
//        parameter.icon_sel= IMG_ID_SETMENU_ICON_SEL;
//        GuiTask_OperWidget(pGc, OPERATE_SET_CONTENT, &parameter, 0);
//        DelayMs(3000);

//        parameter.itemNum= 4;
//        parameter.icon_sel= IMG_ID_SETMENU_ICON_SEL;
//        GuiTask_OperWidget(pGc, OPERATE_SET_CONTENT, &parameter, 0);
//        DelayMs(3000);

//        parameter.itemNum= 29;
//        parameter.icon_sel= IMG_ID_SETMENU_ICON_SEL;
//        GuiTask_OperWidget(pGc, OPERATE_SET_CONTENT, &parameter, 0);
//        DelayMs(3000);
//    }
    return RK_SUCCESS;
}
#endif
#endif
