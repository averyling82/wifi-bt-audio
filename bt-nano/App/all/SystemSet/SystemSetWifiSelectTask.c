/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\SystemSet\systemSetWifiSelectTask.c
* Owner: cjh
* Date: 2015.11.17
* Time: 15:30:07
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.11.17     15:30:07   1.0c
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "effect.h"
#include "..\Resource\ImageResourceID.h"
#include "wiced_wifi.h"
#include "wiced_result.h"
#include "wifithread.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _SYSTEMSETWIFISELECT_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}SYSTEMSETWIFISELECT_RESP_QUEUE;

typedef  struct _SYSTEMSETWIFISELECT_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;
    uint32 offset;
    void *wifiMsg;
}SYSTEMSETWIFISELECT_ASK_QUEUE;


typedef struct _GUI_SSID_
{
    uint8_t length;                    /**< SSID length */
    uint16 value[ SSID_NAME_SIZE ];    /**< SSID name Unicode (AP name)  */
} GUI_SSID;


typedef  struct _WIFI_SSID_MSG
{
    struct _WIFI_SSID_MSG * next;
    GUI_SSID   SSID;
    int16_t signal_strength;
    wiced_security_t  security;         //< Security type>

}WIFI_SSID_MSG;


typedef  struct _SYSTEMSETWIFISELECT_TASK_DATA_BLOCK
{
    GUI_CONTROL_CLASS pfControl;
    pQueue  systemSetWifiSelectAskQueue;
    pQueue  systemSetWifiSelectRespQueue;
    pSemaphore systemSetWifiListOprReqSem;
    HGC     hGc;;
    WIFI_SSID_MSG *pWifiSSIDList;
    WIFI_SSID_MSG curWifiSSIDMsg;
    uint16  wifiNum;
    uint16  wifiScaneOk;
    int     deleteTaskFlag;
    uint8   QueueFull;


}SYSTEMSETWIFISELECT_TASK_DATA_BLOCK;

#define SCAN_LIST_LENGTH        (30)

#define WIFI_LIST_ADD            1
#define WIFI_LIST_DEL            2
#define WIFI_LIST_DEL_ALL        3

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SYSTEMSETWIFISELECT_TASK_DATA_BLOCK * gpstsystemSetWifiSelectData;



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
rk_err_t SystemSetWifiTask_UpdateSSIDToList(void* arg, int cmd);
rk_err_t SystemSetWifiTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc);
rk_err_t SystemSetWifiTask_ListFullSceen(void);
rk_err_t SystemSetWifiTask_ListSetOneLineIcon(WIFI_SSID_MSG * pWifiListTemp, void *arg);
wiced_result_t SystemSetWifiTask_WifiScan(wiced_scan_handler_result_t* malloced_scan_result);
rk_err_t SystemSetWifiTask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SystemSetWifiSelectTask_Delete
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.21
** Time: 16:59:46
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON API rk_err_t SystemSetWifiSelectTask_Delete(void)
{
    SYSTEMSETWIFISELECT_RESP_QUEUE systemSetWifiSelectRespQueue;
#ifdef _DRIVER_WIFI__
    rk_wifi_scan_stop();
#endif
    systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
    rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);

    return RKTaskDelete(TASK_ID_SYSTEMSETWIFISELECT, 0, ASYNC_MODE);
}
/*******************************************************************************
** Name: systemSetWifiSelectTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.17
** Time: 15:35:49
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON API rk_err_t SystemSetWifiSelectTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: systemSetWifiSelectTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.17
** Time: 15:35:49
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON API rk_err_t SystemSetWifiSelectTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: systemSetWifiSelectTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2015.11.17
** Time: 15:35:49
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON API void SystemSetWifiSelectTask_Enter(void * arg)
{
#ifdef _USE_GUI_
#ifdef _DRIVER_WIFI__
    SYSTEMSETWIFISELECT_ASK_QUEUE systemSetwifiSelectAskQueue;
    RKGUI_SELECT_ARG stSelectArg;
    uint8 ssidValue[32];
    rk_err_t ret;

    //GuiTask_AppReciveMsg(SystemSetWifiTask_ButtonCallBack);
    if(MainTask_GetStatus(MAINTASK_WIFI_OPEN_OK) == 1)
    {
        gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length = 0;
        rkwifi_get_ssid(ssidValue, (int *)(&(gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length)));
        Ascii2Unicode(ssidValue, (uint16 *)gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.value,
            gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length);
        *(gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.value + gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length) = 0x0000;
        //printf("\n*...*cur_ssid =%s length=%d\n",ssidValue,gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length);
    }
    SystemSetWifiTask_ListFullSceen();
    //rk_printf("SystemSetWifiSelectTask_Enter...");
    gpstsystemSetWifiSelectData->wifiScaneOk = 0;

    rk_wifi_scan((void *)SystemSetWifiTask_WifiScan);
    //rk_printf("  rk_wifi_scan");
    while (1)
    {
        ret = rkos_queue_receive(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetwifiSelectAskQueue, 500);
        if(ret == RK_SUCCESS)
        {
            //rk_printf("!!wifi list event_type = 0x%x event =0x%x off =%d\n",systemSetwifiSelectAskQueue.event_type,systemSetwifiSelectAskQueue.event,systemSetwifiSelectAskQueue.offset);
            switch(systemSetwifiSelectAskQueue.event_type)
            {
                case SYSTEM_WIFI_RECIVE_KEY:
                    switch(systemSetwifiSelectAskQueue.event)
                    {
                        case KEY_VAL_PLAY_PRESS:
                            MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                            break;

                        case KEY_VAL_ESC_SHORT_UP:
                            SYSTEMSETWIFISELECT_RESP_QUEUE systemSetWifiSelectRespQueue;
                            rk_printf("esc scan");
#ifdef _DRIVER_WIFI__
                            rk_wifi_scan_stop();
#endif
                            systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
                            rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);

#ifdef __APP_SYSTEMSET_SYSTEMSETTASK_C__
                            SystemSetTask_UpData(SYSTEM_SET_UPDATA, systemSetwifiSelectAskQueue.event_type, NULL, SYNC_MODE);
                            RKTaskDelete(TASK_ID_SYSTEMSETWIFISELECT, 0, ASYNC_MODE);
                            while(1)
                            {
                                rkos_sleep(2000);
                                rk_printf("wifi TASK_ID_MAIN\n");
                            }
#endif
                            break;

                        case KEY_VAL_FFW_SHORT_UP:
                            break;

                        case KEY_VAL_FFD_SHORT_UP:
                            break;

                        case KEY_VAL_MENU_SHORT_UP:
                            break;

                       default:

                            break;
                    }
                    gpstsystemSetWifiSelectData->QueueFull = 0;
                    break;

               case SYSTEM_WIFI_NEW_SSID:
                    //rk_printf(" .... cjhNEW add ssid num=%d\n",gpstsystemSetWifiSelectData->wifiNum);
                    //rkos_semaphore_take(gpstsystemSetWifiSelectData->systemSetWifiListOprReqSem, MAX_DELAY);
                    SystemSetWifiTask_UpdateSSIDToList(systemSetwifiSelectAskQueue.wifiMsg, WIFI_LIST_ADD);
                    //rkos_semaphore_give(gpstsystemSetWifiSelectData->systemSetWifiListOprReqSem);
                    break;

               default:

                    break;
            }
        }
        else
        {
            if(MainTask_GetStatus(MAINTASK_WIFI_SUSPEND) == 0)
            {
                gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length = 0;
                rkwifi_get_ssid(ssidValue, (int *)(&(gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length)));
                Ascii2Unicode(ssidValue, (uint16 *)gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.value,
                gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length);
                SystemSetWifiTask_ListFullSceen();
                rk_wifi_scan((void *)SystemSetWifiTask_WifiScan);
            }
        }
    }
#endif
#endif
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _USE_GUI_
/*******************************************************************************
** Name: SystemSetWifiTask_UpdateSSIDToList
** Input:WIFI_SSID_MSG * pWifiListTemp, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.20
** Time: 11:57:35
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON FUN rk_err_t SystemSetWifiTask_UpdateSSIDToList(void* arg, int cmd)
{
    wiced_scan_handler_result_t* malloced_scan_result = (wiced_scan_handler_result_t*)arg;

    uint32 wifiLen;
    SYSTEMSETWIFISELECT_RESP_QUEUE systemSetWifiSelectRespQueue;
    WIFI_SSID_MSG * pWifiSSIDTemp=NULL;
    WIFI_SSID_MSG * pWifiListTemp=NULL;
    WIFI_SSID_MSG * pPreWifiListTemp=NULL;
    WIFI_SSID_MSG * pExisteWifiListTemp=NULL;
    WIFI_SSID_MSG * pLastWifiListTemp=NULL;
    uint16 tempWifiName[SSID_NAME_SIZE],i,j;
    int16_t tempSignalStrength, tempSignalStrengthBackup;
    rk_err_t ret = 0;

    switch(cmd)
    {
        case WIFI_LIST_ADD:
        #if 1
            wifiLen = malloced_scan_result->ap_details.SSID.length;
            if((malloced_scan_result->status == WICED_SCAN_COMPLETED_SUCCESSFULLY) || (malloced_scan_result->status == WICED_SCAN_ABORTED))
            {
                SystemSetWifiTask_ListFullSceen();//-1

                systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
                rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);
                return RK_SUCCESS;
            }
            else if((wifiLen == 0) || (wifiLen > SSID_NAME_SIZE))
            {
                systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
                rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);
                return RK_SUCCESS;
            }

            pExisteWifiListTemp = NULL;
            Ascii2Unicode(malloced_scan_result->ap_details.SSID.value, (uint16 *)tempWifiName, wifiLen);
            if(wifiLen == gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length)
            {
                if(StrCmpA((uint8 *)tempWifiName, (uint8 *)gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.value, wifiLen*2) == RK_SUCCESS)
                {
                    //printf("777 ii send RespQueue \n");
                    gpstsystemSetWifiSelectData->curWifiSSIDMsg.signal_strength = malloced_scan_result->ap_details.signal_strength;
                    systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
                    rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);
                    //printf("777 oo send RespQueue \n");
                    return RK_SUCCESS;
                }
            }
#if 0
            pWifiListTemp =  gpstsystemSetWifiSelectData->pWifiSSIDList;
            for(i = 0; i < gpstsystemSetWifiSelectData->wifiNum; i++)
            {
                printf("Q_n[%d]=0x%x  ",i,pWifiListTemp);
                pWifiListTemp = pWifiListTemp->next;
            }
            printf("Q_n[%d]=0x%x  ",i,pWifiListTemp);
#endif

            pPreWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;
            pWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;
            for(i = 0; i < gpstsystemSetWifiSelectData->wifiNum; i++)
            {
                if(pWifiListTemp != NULL)
                {
                    if(wifiLen == pWifiListTemp->SSID.length)
                    {
                        if(StrCmpA((uint8 *)tempWifiName, (uint8 *)pWifiListTemp->SSID.value, wifiLen*2) == RK_SUCCESS)//StrCmpA(uint8 * pstr1, uint8 * pstr2, uint32 len)
                        {
                            // signal_strength upData
#if 1
                            pExisteWifiListTemp = pWifiListTemp;
                            //printf("\n----1 pExisteWifiListTemp = 0x%x nameExist = %s, newstrength=%d",pExisteWifiListTemp, malloced_scan_result->ap_details.SSID.value,malloced_scan_result->ap_details.signal_strength);
                            //del from list
                            if(i != 0)//pPreWifiListTemp != pWifiListTemp
                            {
                                pPreWifiListTemp->next = pPreWifiListTemp->next->next;//pWifiListTemp->next;
                            }
                            else
                            {
                                gpstsystemSetWifiSelectData->pWifiSSIDList = pPreWifiListTemp->next;
                                //pPreWifiListTemp = pPreWifiListTemp->next;
                            }
                            gpstsystemSetWifiSelectData->wifiNum--;
#endif
                            break;
                        }
                    }
                    pPreWifiListTemp = pWifiListTemp;
                    pWifiListTemp = pWifiListTemp->next;
                }
                else
                {
                    while(1)
                    {
                        rk_printf("while(1) pWifiListTemp NULL i=%d wifiNum=%d", i, gpstsystemSetWifiSelectData->wifiNum);
                    }
                }
            }

            if(gpstsystemSetWifiSelectData->wifiNum < SCAN_LIST_LENGTH)
            {
                if(pExisteWifiListTemp == NULL)
                {
                    pWifiSSIDTemp = rkos_memory_malloc(sizeof(WIFI_SSID_MSG));
                    if(pWifiSSIDTemp == NULL)
                    {
                        systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
                        rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);
                        return RK_ERROR;
                    }
                    memcpy(pWifiSSIDTemp->SSID.value, tempWifiName, wifiLen*2);
                    *(pWifiSSIDTemp->SSID.value + wifiLen) = 0x0000;
                    pWifiSSIDTemp->SSID.length = wifiLen;
                    //printf("++++1 malloc = 0x%x \n",pWifiSSIDTemp);
                }
                else
                {
                    pWifiSSIDTemp = pExisteWifiListTemp;
                    //printf("+1 exist = 0x%x \n",pWifiSSIDTemp);
                }

                pWifiSSIDTemp->next = NULL;
                pWifiSSIDTemp->signal_strength = malloced_scan_result->ap_details.signal_strength;
                pWifiSSIDTemp->security = malloced_scan_result->ap_details.security;

                if (gpstsystemSetWifiSelectData->pWifiSSIDList == NULL)
                {
                    gpstsystemSetWifiSelectData->pWifiSSIDList = pWifiSSIDTemp;
                    //printf("HEAD ");
                }
                else
                {
                    pWifiListTemp =  gpstsystemSetWifiSelectData->pWifiSSIDList;
                    tempSignalStrength = malloced_scan_result->ap_details.signal_strength;
                    pPreWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;

                    for(i = 0; i < gpstsystemSetWifiSelectData->wifiNum; i++)
                    {
                        if(pWifiListTemp == NULL)
                        {
                            //printf("iNULL");
                            pPreWifiListTemp->next = pWifiSSIDTemp;
                            break;
                        }
                        else if(tempSignalStrength > pWifiListTemp->signal_strength)
                        {
                            //printf("QM[%d]",i);
                            pWifiSSIDTemp->next = pWifiListTemp;
                            if(i == 0)//pPreWifiListTemp == pWifiListTemp
                            {
                                //pPreWifiListTemp = pWifiSSIDTemp;
                                //printf(" 0+");
                                gpstsystemSetWifiSelectData->pWifiSSIDList = pWifiSSIDTemp;
                            }
                            else
                            {
                                //printf(" 1+");
                                pPreWifiListTemp->next = pWifiSSIDTemp;
                            }
                            break;
                        }
                        else if(i == gpstsystemSetWifiSelectData->wifiNum-1)
                        {
                            //printf("HM[%d]",i+1);
                            pWifiListTemp->next = pWifiSSIDTemp;
                            break;
                        }

                        pPreWifiListTemp = pWifiListTemp;
                        pWifiListTemp = pWifiListTemp->next;
                    }
                }
                gpstsystemSetWifiSelectData->wifiNum++;
                //printf("$$ insert i=%d \n ",i);
            }
            else
            {
                //printf("wifiNum > 30\n");
                pWifiListTemp =  gpstsystemSetWifiSelectData->pWifiSSIDList;
                tempSignalStrength = malloced_scan_result->ap_details.signal_strength;
                pPreWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;
                i = SCAN_LIST_LENGTH;
                while(i--)
                {
                    if((pWifiListTemp->next == NULL) || (i == 0))
                    {
                        pPreWifiListTemp->next = NULL;
                        pLastWifiListTemp = pWifiListTemp;
                        //gpstsystemSetWifiSelectData->wifiNum--;
                        //printf("> 30 i=%d",i);
                        break;
                    }
                    pPreWifiListTemp = pWifiListTemp;
                    pWifiListTemp = pWifiListTemp->next;
                }

                if(tempSignalStrength > pWifiListTemp->signal_strength)
                {
                    memcpy(pLastWifiListTemp->SSID.value, tempWifiName, SSID_NAME_SIZE*2);
                    pLastWifiListTemp->SSID.length = wifiLen;
                    pLastWifiListTemp->signal_strength = malloced_scan_result->ap_details.signal_strength;
                    pLastWifiListTemp->security = malloced_scan_result->ap_details.security;

                    pWifiListTemp =  gpstsystemSetWifiSelectData->pWifiSSIDList;
                    tempSignalStrength = malloced_scan_result->ap_details.signal_strength;
                    pPreWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;

                    if (gpstsystemSetWifiSelectData->pWifiSSIDList == NULL)
                    {
                        gpstsystemSetWifiSelectData->pWifiSSIDList = pLastWifiListTemp;
                    }
                    else
                    {
                        for(i = 0; i < gpstsystemSetWifiSelectData->wifiNum; i++)
                        {
                            if(pWifiListTemp == NULL)
                            {
                                //printf("30iNULL");
                                pPreWifiListTemp->next = pLastWifiListTemp;
                                break;
                            }
                            else if(tempSignalStrength > pWifiListTemp->signal_strength)
                            {
                                //printf("30QM[%d]",i);
                                pLastWifiListTemp->next = pWifiListTemp;
                                if(i == 0)//pPreWifiListTemp == pWifiListTemp
                                {
                                    //pPreWifiListTemp = pWifiSSIDTemp;
                                    //printf(" 30 0+");
                                    gpstsystemSetWifiSelectData->pWifiSSIDList = pLastWifiListTemp;
                                }
                                else
                                {
                                    //printf(" 30 1+");
                                    pPreWifiListTemp->next = pLastWifiListTemp;
                                }
                                break;
                            }
                            else if(i == gpstsystemSetWifiSelectData->wifiNum-1)
                            {
                                //printf("30 HM[%d]",i+1);
                                pWifiListTemp->next = pLastWifiListTemp;
                                break;
                            }

                            pPreWifiListTemp = pWifiListTemp;
                            pWifiListTemp = pWifiListTemp->next;
                        }
                    }
                    //gpstsystemSetWifiSelectData->wifiNum++;
                    //printf("$$ 30 insert i=%d  wifiNum =%d\n ",i,gpstsystemSetWifiSelectData->wifiNum);
                }
                else
                {
                    pPreWifiListTemp->next = pLastWifiListTemp;
                }
            }

            if((gpstsystemSetWifiSelectData->wifiScaneOk == 1) || (gpstsystemSetWifiSelectData->wifiNum%8 == 0) || (gpstsystemSetWifiSelectData->wifiNum == 1))
            {
                SystemSetWifiTask_ListFullSceen();//-1
            }
            #endif
#if 0
            pWifiListTemp =  gpstsystemSetWifiSelectData->pWifiSSIDList;
            for(i = 0; i < gpstsystemSetWifiSelectData->wifiNum; i++)
            {
                printf("h_n[%d]=0x%x  ",i,pWifiListTemp);
                pWifiListTemp = pWifiListTemp->next;
            }
            printf("h_n[%d]=0x%x  ",i,pWifiListTemp);

#endif
            break;

        case WIFI_LIST_DEL:

            break;

        case WIFI_LIST_DEL_ALL:

            break;

        default:
            break;
    }
    systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
    rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SystemSetWifiTask_ButtonCallBack
** Input:APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.24
** Time: 16:44:55
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON FUN rk_err_t SystemSetWifiTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    SYSTEMSETWIFISELECT_ASK_QUEUE systemSetWifiAskQueue;
    SYSTEMSETWIFISELECT_ASK_QUEUE systemSetWifiAskQueue_tmp;
    SYSTEMSETWIFISELECT_RESP_QUEUE systemSetWifiSelectRespQueue;

    if ((gpstsystemSetWifiSelectData->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP) && ((event & KEY_STATUS_SHORT_UP) != KEY_STATUS_SHORT_UP))
    {
        rk_printf("lose key wifiListQueueFull=%d\n",gpstsystemSetWifiSelectData->QueueFull);
        return RK_SUCCESS;
    }

    if(gpstsystemSetWifiSelectData->deleteTaskFlag != 1)
    {
        switch(event_type)
        {
            case APP_RECIVE_MSG_EVENT_KEY:
            {
                gpstsystemSetWifiSelectData->QueueFull = 1;
                systemSetWifiAskQueue.event = event;
                systemSetWifiAskQueue.event_type = SYSTEM_WIFI_RECIVE_KEY;
                systemSetWifiAskQueue.offset = NULL;

                ret = rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
                if (ret == RK_ERROR)
                {
                    if((event==KEY_VAL_FFW_PRESS)||(event==KEY_VAL_FFD_PRESS)||(event==KEY_VAL_HOLD_PRESS))
                    {
                        gpstsystemSetWifiSelectData->QueueFull = 0;
                        return RK_SUCCESS;
                    }
                    rkos_queue_receive(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue_tmp, 0);
                    if(systemSetWifiAskQueue.event == KEY_VAL_ESC_SHORT_UP)
                    {
                        if(systemSetWifiAskQueue_tmp.event_type == SYSTEM_WIFI_NEW_SSID)
                        {
                            systemSetWifiSelectRespQueue.cmd = RK_SUCCESS;
                            rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 0);
                        }
                        rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
                    }
                    else if(systemSetWifiAskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                    {
                        rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue_tmp, 0);
                        rkos_queue_receive(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue_tmp, 0);
                        if(systemSetWifiAskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                        {
                            rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue_tmp, 0);
                            gpstsystemSetWifiSelectData->QueueFull = 0;
                        }
                        else
                        {
                            ret = rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
                            if (ret == RK_ERROR)
                            {
                                rk_printf("Send Key Failure 0\n");
                                gpstsystemSetWifiSelectData->QueueFull = 0;
                            }
                        }
                    }
                    else
                    {
                        ret = rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
                        if (ret == RK_ERROR)
                        {
                            rk_printf("Send Key Failure 1\n");
                            gpstsystemSetWifiSelectData->QueueFull = 0;
                        }
                    }

                }
            }
            break;

            case APP_RECIVE_MSG_EVENT_WARING:
            {
                systemSetWifiAskQueue.event =  event;
                systemSetWifiAskQueue.event_type = event_type;
                systemSetWifiAskQueue.offset = NULL;
                //gpstsystemSetWifiSelectData->QueueFull = 1;
                ret = rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
                if(ret == RK_ERROR)
                {
                    rk_printf("-------lose WifiList_MsgBoxCallBack 0\n");
                    rkos_queue_receive(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue_tmp, 0);//MAX_DELAY
                    rk_printf ("-------lose WifiList_MsgBoxCallBack 1\n");
                    rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
                }


            }
            break;

            case APP_RECIVE_MSG_EVENT_DIALOG:
            {
                systemSetWifiAskQueue.event = event;
                systemSetWifiAskQueue.event_type = event_type;
                systemSetWifiAskQueue.offset = NULL;
                ret = rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
            }
            break;

            default:
            break;
        }
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SystemSetWifiTask_SSIDFilter
** Input:(void)
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.15
** Time: 9:40:41
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON FUN rk_err_t SystemSetWifiTask_ListFullSceen(void)
{
    uint16 i, temp;
    RKGUI_SELECT_ARG stSelectArg;
    RKGUI_SELECT_CONTENT stContentArg;
    WIFI_SSID_MSG * pWifiListTemp;
    uint8 tempWifiName[SSID_NAME_SIZE];
    uint16 wifiTotalNum;
    uint16 tempOffset;

    if((gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length > 0))
    {
        wifiTotalNum = gpstsystemSetWifiSelectData->wifiNum + 1;
    }
    else
    {
        wifiTotalNum = gpstsystemSetWifiSelectData->wifiNum;
    }

    temp = wifiTotalNum>8 ? 8:wifiTotalNum;
    pWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;
    {
        if(gpstsystemSetWifiSelectData->hGc == NULL)
        {
            stSelectArg.x = 0;
            stSelectArg.y = 20;
            stSelectArg.xSize = 128;
            stSelectArg.ySize = 137;
            stSelectArg.display = 1;
            stSelectArg.level = 0;

            stSelectArg.Background = IMG_ID_BROWSER_BACKGROUND;
            stSelectArg.SeekBar = IMG_ID_BROWSER_SCOLL2;
            stSelectArg.Cursor= 0;
            stSelectArg.ItemStartOffset = 0;
            stSelectArg.itemNum = wifiTotalNum;
            printf("\n\n first to num =%d\n\n", stSelectArg.itemNum);

            stSelectArg.MaxDisplayItem = MAX_DISP_ITEM_NUM;
            stSelectArg.CursorStyle = IMG_ID_SEL_ICON;
            stSelectArg.IconBoxSize = 18;

            stSelectArg.pReviceMsg = SystemSetWifiTask_SelectCallBack;

            gpstsystemSetWifiSelectData->hGc = GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
            if(gpstsystemSetWifiSelectData->hGc == NULL)
            {
                rk_printf("error:pGc == NULL\n");
            }
        }
        else
        {
            stContentArg.itemNum = wifiTotalNum;
            GuiTask_OperWidget(gpstsystemSetWifiSelectData->hGc, OPERATE_SET_CONTENT, &stContentArg, SYNC_MODE);
        }
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SystemSetWifiTask_WifiIconShow
** Input:int16 singleStr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.14
** Time: 20:13:52
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON FUN rk_err_t SystemSetWifiTask_ListSetOneLineIcon(WIFI_SSID_MSG * pWifiListTemp, void *arg)
{
    RKGUI_SELECT_ITEM *pItem = (RKGUI_SELECT_ITEM *)arg;

    if(WICED_SECURITY_OPEN == pWifiListTemp->security)
    {
        if(pWifiListTemp->signal_strength >= -45)//-30--- -120
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI4;
        }
        else if(pWifiListTemp->signal_strength >= -55)
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI3;
        }
        else if(pWifiListTemp->signal_strength > -65)
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI2;
        }
        else if(pWifiListTemp->signal_strength > -85)
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI1;
        }
        else
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI0;
        }
    }
    else
    {
        if(pWifiListTemp->signal_strength >= -45)//-30--- -120
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI_LOCK4;
        }
        else if(pWifiListTemp->signal_strength >= -55)
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI_LOCK3;
        }
        else if(pWifiListTemp->signal_strength > -65)
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI_LOCK2;
        }
        else if(pWifiListTemp->signal_strength > -85)
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI_LOCK1;
        }
        else
        {
            pItem->sel_icon = -1;
            pItem->unsel_icon = IMG_ID_WIFI_LOCK0;
        }
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SystemSetTadk_WifiScan
** Input:viod
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.1
** Time: 11:13:15
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON FUN wiced_result_t SystemSetWifiTask_WifiScan(wiced_scan_handler_result_t* malloced_scan_result)
{
    SYSTEMSETWIFISELECT_ASK_QUEUE systemSetWifiSelectAskQueue;
    SYSTEMSETWIFISELECT_RESP_QUEUE systemSetWifiSelectRespQueue;
    rk_err_t ret;

    gpstsystemSetWifiSelectData->wifiScaneOk = 0;
    if((malloced_scan_result->status == WICED_SCAN_COMPLETED_SUCCESSFULLY) || (malloced_scan_result->status == WICED_SCAN_ABORTED))
    {
        gpstsystemSetWifiSelectData->wifiScaneOk = 1;
        //vPortFree(malloced_scan_result);
        //return RK_SUCCESS;
    }

    systemSetWifiSelectAskQueue.event_type = SYSTEM_WIFI_NEW_SSID;
    systemSetWifiSelectAskQueue.event = NULL;
    systemSetWifiSelectAskQueue.offset = NULL;
    systemSetWifiSelectAskQueue.wifiMsg = (void *)malloced_scan_result;
    //printf("!!!! lyb ssid = %s len=%d... \n",malloced_scan_result->ap_details.SSID.value , malloced_scan_result->ap_details.SSID.length);
    ret = rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiSelectAskQueue, 0);
    if(ret == RK_ERROR)
    {
        printf("....send fail..\n");
        rkos_memory_free(malloced_scan_result);
        return RK_SUCCESS;
    }
    rkos_queue_receive(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue, &systemSetWifiSelectRespQueue, 300);

    rkos_memory_free(malloced_scan_result);
    return RK_SUCCESS;
}




/*******************************************************************************
** Name: SystemSetWifiTask_SelectCallBack
** Input:uint32 event_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.18
** Time: 15:10:30
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_
COMMON FUN rk_err_t SystemSetWifiTask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset)
{
    SYSTEMSETWIFISELECT_ASK_QUEUE systemSetWifiAskQueue;
    WIFI_SSID_MSG * pWifiListTemp;
    RKGUI_SELECT_ITEM * item;
    uint8 tempWifiName[SSID_NAME_SIZE];
    uint16 i;
    uint16 wifiTotalNum;
    uint16 tempOffset;

    switch(event_type)
    {
        case SELECT_ENVEN_ENTER:
            systemSetWifiAskQueue.event = (uint32)arg;
            systemSetWifiAskQueue.event_type = SYSTEM_WIFI_LIST_ENTER;
            systemSetWifiAskQueue.offset = offset;
            rkos_queue_send(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue, &systemSetWifiAskQueue, 0);
            break;

        case SELECT_ENVEN_UPDATA:
            item = (RKGUI_SELECT_ITEM *)arg;
            item->cmd= TEXT_CMD_BUF;

            if((gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length > 0))
            {
                wifiTotalNum = gpstsystemSetWifiSelectData->wifiNum + 1;
                if(offset == 0)
                {
                    SystemSetWifiTask_ListSetOneLineIcon(&gpstsystemSetWifiSelectData->curWifiSSIDMsg, item);
                    memcpy((uint8 *)item->text, gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.value,(gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length)*2);
                    *(item->text + gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID.length) = 0x0000;
                    return RK_SUCCESS;
                }
                else
                {
                    tempOffset = offset - 1;
                }
            }
            else
            {
                wifiTotalNum = gpstsystemSetWifiSelectData->wifiNum;
                tempOffset = offset;
            }

            pWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;
            if(tempOffset < wifiTotalNum)
            {
                for(i = 0; i< tempOffset; i++)
                {
                    pWifiListTemp = pWifiListTemp->next;
                    if(pWifiListTemp == NULL)
                    {
                        printf("wifi list NULL..\n");
                        return RK_ERROR;
                    }
                }
            }

            SystemSetWifiTask_ListSetOneLineIcon(pWifiListTemp, item);
            memcpy((uint8 *)item->text, pWifiListTemp->SSID.value,(pWifiListTemp->SSID.length)*2);
            *(item->text + pWifiListTemp->SSID.length) = 0x0000;
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
** Name: systemSetWifiSelectTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.17
** Time: 15:35:49
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_INIT_
INIT API rk_err_t SystemSetWifiSelectTask_DeInit(void *pvParameters)
{
#ifdef _USE_GUI_
    WIFI_SSID_MSG * pWifiListTemp = gpstsystemSetWifiSelectData->pWifiSSIDList;;
    WIFI_SSID_MSG * curPWifiListTemp = NULL;
    if (gpstsystemSetWifiSelectData->hGc != NULL)
    {
        GuiTask_DeleteWidget(gpstsystemSetWifiSelectData->hGc);
        gpstsystemSetWifiSelectData->hGc = NULL;
    }
    GuiTask_AppUnReciveMsg(SystemSetWifiTask_ButtonCallBack);

    curPWifiListTemp = pWifiListTemp;
    while (curPWifiListTemp != NULL)
    {
        curPWifiListTemp = pWifiListTemp->next;
        rkos_memory_free(pWifiListTemp);
        pWifiListTemp = curPWifiListTemp;
    }
    rkos_semaphore_delete(gpstsystemSetWifiSelectData->systemSetWifiListOprReqSem);
    rkos_queue_delete(gpstsystemSetWifiSelectData->systemSetWifiSelectAskQueue);
    rkos_queue_delete(gpstsystemSetWifiSelectData->systemSetWifiSelectRespQueue);
    rkos_memory_free(gpstsystemSetWifiSelectData);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_SYSTEMSETWIFISELECT_TASK);
#endif
    gpstsystemSetWifiSelectData = NULL;
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: systemSetWifiSelectTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2015.11.17
** Time: 15:35:49
*******************************************************************************/
_APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_INIT_
INIT API rk_err_t SystemSetWifiSelectTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS*   psystemSetWifiSelectTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_SYSTEMSETWIFISELECT_ARG * pArg = (RK_TASK_SYSTEMSETWIFISELECT_ARG *)arg;
    SYSTEMSETWIFISELECT_TASK_DATA_BLOCK*  psystemSetWifiSELECTTaskData;
    printf("SystemSetWifiSelectTask_Init\n");
    if (psystemSetWifiSelectTask == NULL)
        return RK_PARA_ERR;

    psystemSetWifiSELECTTaskData = rkos_memory_malloc(sizeof(SYSTEMSETWIFISELECT_TASK_DATA_BLOCK));
    if(psystemSetWifiSELECTTaskData == NULL)
    {
        printf("pSystemSetTaskData NULL\n");
        return RK_ERROR;
    }
    memset(psystemSetWifiSELECTTaskData, NULL, sizeof(SYSTEMSETWIFISELECT_TASK_DATA_BLOCK));
    psystemSetWifiSELECTTaskData->systemSetWifiListOprReqSem = rkos_semaphore_create(1,1);
    psystemSetWifiSELECTTaskData->systemSetWifiSelectAskQueue = rkos_queue_create(30, sizeof(SYSTEMSETWIFISELECT_ASK_QUEUE));
    psystemSetWifiSELECTTaskData->systemSetWifiSelectRespQueue = rkos_queue_create(1, sizeof(SYSTEMSETWIFISELECT_RESP_QUEUE));

    if((psystemSetWifiSELECTTaskData->systemSetWifiListOprReqSem == 0) ||
        (psystemSetWifiSELECTTaskData->systemSetWifiSelectAskQueue == 0) ||
        (psystemSetWifiSELECTTaskData->systemSetWifiSelectRespQueue == 0))
    {
        rkos_semaphore_delete(psystemSetWifiSELECTTaskData->systemSetWifiListOprReqSem);
        rkos_queue_delete(psystemSetWifiSELECTTaskData->systemSetWifiSelectAskQueue);
        rkos_queue_delete(psystemSetWifiSELECTTaskData->systemSetWifiSelectRespQueue);
        rkos_memory_free(psystemSetWifiSELECTTaskData);
        return RK_ERROR;
    }

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_SYSTEMSETWIFISELECT_TASK, SEGMENT_OVERLAY_ALL);
#endif
    gpstsystemSetWifiSelectData = psystemSetWifiSELECTTaskData;
    //gpstsystemSetWifiSelectData->curWifiSSIDMsg.security = NULL;
    //gpstsystemSetWifiSelectData->curWifiSSIDMsg.SSID = NULL;
    //gpstsystemSetWifiSelectData->curWifiSSIDMsg.signal_strength = NULL;
    //gpstsystemSetWifiSelectData->wifiNum = 0;
    //gpstsystemSetWifiSelectData->wifiScaneOk = 0;
    //gpstsystemSetWifiSelectData->curOffset = 0;

    gpstsystemSetWifiSelectData->pWifiSSIDList = NULL;
#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(SystemSetWifiTask_ButtonCallBack);
    printf("systemSetWifiSELECTTask_Init Success\n");
#endif
    gpstsystemSetWifiSelectData->wifiScaneOk = 1;
    gpstsystemSetWifiSelectData->deleteTaskFlag = 0;
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
