/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: system\usbserver\usbservice.c
* Owner: wrm
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: usb server
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm        2015.8.20     17:29:00   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __SYSTEM_USBSERVER_USBSERVICE_C__

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

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _USBSERVICE_RESP_QUEUE
{
    uint32 err_code;

}USBSERVICE_RESP_QUEUE;

typedef  struct _USBSERVICE_ASK_QUEUE
{
    uint32 cmd;

}USBSERVICE_ASK_QUEUE;

typedef  struct _USBSERVICE_DATA_BLOCK
{
    pQueue  UsbServiceAskQueue;
    pQueue  UsbServiceRespQueue;

    //GUI Handlle
    HGC hUsbBackGroud;


}USBSERVICE_DATA_BLOCK;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static USBSERVICE_DATA_BLOCK * gpstUsbService;


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
rk_err_t UsbServiceCheckIdle(HTC hTask);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UsbService_DeleteGuiHandle
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON FUN void UsbService_DeleteGuiHandle(void)
{
#ifdef _USE_GUI_
    if (gpstUsbService->hUsbBackGroud != NULL)
    {
        GuiTask_DeleteWidget(gpstUsbService->hUsbBackGroud);
        gpstUsbService->hUsbBackGroud = NULL;
    }
#endif
}
/*******************************************************************************
** Name: UsbMscDeviceServer
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.14
** Time: 17:18:49
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON FUN int UsbMscDeviceServer(void)
{
    uint8 usbenum_ok = 0;
    int i;
    rk_err_t ret = RK_ERROR;
    #ifdef _USE_GUI_
    //GUI 显示
    RKGUI_ICON_ARG pUsbIconArg;
    #endif

    if (Grf_CheckVbus() == 0)
    {
        printf ("Vbus Disconnect\n");
        MainTask_TaskSwtich(TASK_ID_USBSERVER, 0, TASK_ID_MAIN, 0, NULL);
        return 0;
    }

    //创建USBMSC Device
    ret = DeviceTask_CreateDeviceList(DEVICE_LIST_USBDEVICE_MSC, NULL, SYNC_MODE);
    if (ret == RK_ERROR)
    {
        printf ("\nusbservive create usbmsc device failure\n");
        ChargeEnable();
        MainTask_TaskSwtich(TASK_ID_USBSERVER, 0, TASK_ID_MAIN, 0, NULL);
        while(1)
        {
            rkos_sleep(1000);
        }
    }

    //rkos_sleep(1500);
    for (i=0;i<200;i++)
    {
        //获取枚举状态
        usbenum_ok = UsbOtgDev_GetEnumStatus(NULL);
        if (usbenum_ok)
        {
            break;
        }
        rkos_sleep(10);
    }


    if (usbenum_ok) //连接者为电脑,手机等
    {
        //printf ("\nusbdevice connect with Host\n");
        #ifdef _USE_GUI_
        pUsbIconArg.resource = IMG_ID_USB_BACKGROUND;
        pUsbIconArg.x = 0;
        pUsbIconArg.y = 0;
        pUsbIconArg.level = 0;
        pUsbIconArg.display = 1;
        gpstUsbService->hUsbBackGroud = GUITask_CreateWidget(GUI_CLASS_ICON, &pUsbIconArg);
        #endif
        {
            uint32 i;

            for(i = 0; gLun[i] != 0; i++)
            {
                if(gLun[i] == 2)
                {
                    #ifndef _HIDDEN_DISK1_
                    #ifndef _HIDDEN_DISK2_
                    FW_CodePageUnLoad();
                    #ifdef _USE_GUI_
                    GuiTask_UnloadResource();
                    #endif
                    #endif
                    #endif

                    #ifdef _EMMC_BOOT_
                    DeviceTask_RemoveFs(DEVICE_LIST_EMMC_FS1,SYNC_MODE);
                    #endif

                    #ifdef _SPI_BOOT_
                    DeviceTask_RemoveFs(DEVICE_LIST_SPI_FS1,SYNC_MODE);
                    #endif

                }
                #ifdef _HIDDEN_DISK1_
                else if(gLun[i] == 3)
                {
                    FW_CodePageUnLoad();
                    #ifdef _USE_GUI_
                    GuiTask_UnloadResource();
                    #endif

                    #ifdef _EMMC_BOOT_
                    DeviceTask_RemoveFs(DEVICE_LIST_EMMC_FS2,SYNC_MODE);
                    #endif

                    #ifdef _SPI_BOOT_
                    DeviceTask_RemoveFs(DEVICE_LIST_SPI_FS2,SYNC_MODE);
                    #endif
                }
                #endif
                #ifdef _HIDDEN_DISK2_
                else if(gLun[i] == 4)
                {
                    #ifndef _HIDDEN_DISK1_
                    FW_CodePageUnLoad();
                    #ifdef _USE_GUI_
                    GuiTask_UnloadResource();
                    #endif
                    #endif

                    #ifdef _EMMC_BOOT_
                    DeviceTask_RemoveFs(DEVICE_LIST_EMMC_FS3,SYNC_MODE);
                    #endif

                    #ifdef _SPI_BOOT_
                    DeviceTask_RemoveFs(DEVICE_LIST_SPI_FS3,SYNC_MODE);
                    #endif
                }
                #endif
            }
        }
        ChargeEnable();
        //Notification MainTask USB already Open
        MainTask_SetStatus(MAINTASK_APP_USB_OK,1);

    }
    else //连接者为充电器
    {
        printf ("\nusbdevice not connect with Host\n");
        UsbMscDev_IntDisalbe();
        ChargeEnable();
        UsbService_DeleteUsb(3);
        while(1)
        {
        	ChargeEnable();//jjjhhh 20161110
            rkos_sleep(1000);
        }
    }
    return 0;
}
/*******************************************************************************
** Name: UsbServiceCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 14:17:07
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON FUN rk_err_t UsbServiceCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        pTask->State = TASK_STATE_WORKING;
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MainTask_NoScreenUSBKey
** Input:uint32 event
** Return: int32
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
** Function:Delete all application
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON int32 UsbService_NoScreenUSBKey(uint32 keyvalue)
{
#if 1
    USBSERVICE_ASK_QUEUE UsbAskQueue;
    UsbAskQueue.cmd = keyvalue;
    rkos_queue_send(gpstUsbService->UsbServiceAskQueue, &UsbAskQueue, 0);
    return RK_SUCCESS;
#else
    printf ("--------app is: USB\n");
    switch (keyvalue)
    {
        case KEY_VAL_ESC_SHORT_UP:
            //MainTask_SysEventCallBack(MAINTASK_USBDELETE);
            #ifdef _USB_
            {
                MAINTASK_DEBUG ("Delete Usb Server\n");
                if(MainTask_GetStatus(MAINTASK_APP_USB_OK) == 1)
                {
                    MAINTASK_DEBUG ("Delete Usb Server 1\n");

                    UsbMscDev_IntDisalbe();
                    UsbService_DeleteUsb(2);
                }
            }
            #endif
            break;
        default:
            break;
    }
    return RK_SUCCESS;
#endif
}


/*******************************************************************************
** Name: UsbService_ButtonCallBack
** Input:uint32 evnet_type,uint32 event,void * arg,uint32 mode
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
static COMMON API rk_err_t UsbService_ButtonCallBack(uint32 event_type,
                                                            uint32 event,
                                                            void * arg,
                                                            uint32 mode)
{
    USBSERVICE_ASK_QUEUE UsbAskQueue;
    UsbAskQueue.cmd = event;
    rkos_queue_send(gpstUsbService->UsbServiceAskQueue, &UsbAskQueue, 0);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbService_DeleteUsb
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API void UsbService_DeleteUsb(int status)
{
    //Delete USBOTG和USBMSC
    DeviceTask_DeleteDeviceList(DEVICE_LIST_USBDEVICE_MSC, NULL, SYNC_MODE);

    if (status != 3) //status ==3 表示文件系统未卸载，故无需再加载
    {
       {
            uint32 i;

            for(i = 0; gLun[i] != 0; i++)
            {
                if(gLun[i] == 2)
                {
                    #ifdef _EMMC_BOOT_
                    DeviceTask_LoadFs(DEVICE_LIST_EMMC_FS1,SYNC_MODE);
                    #endif

                    #ifdef _SPI_BOOT_
                    DeviceTask_LoadFs(DEVICE_LIST_SPI_FS1,SYNC_MODE);
                    #endif

                    #ifndef _HIDDEN_DISK1_
                    #ifndef _HIDDEN_DISK2_
                    FW_CodePageLoad();
                    #ifdef _USE_GUI_
                    GuiTask_LoadResource();
                    #endif
                    #endif
                    #endif
                }
                #ifdef _HIDDEN_DISK1_
                else if(gLun[i] == 3)
                {
                    #ifdef _EMMC_BOOT_
                    DeviceTask_LoadFs(DEVICE_LIST_EMMC_FS2,SYNC_MODE);
                    #endif

                    #ifdef _SPI_BOOT_
                    DeviceTask_LoadFs(DEVICE_LIST_SPI_FS2,SYNC_MODE);
                    #endif

                    FW_CodePageLoad();
                    #ifdef _USE_GUI_
                    GuiTask_LoadResource();
                    #endif
                }
                #endif
                #ifdef _HIDDEN_DISK2_
                else if(gLun[i] == 4)
                {
                    #ifdef _EMMC_BOOT_
                    DeviceTask_LoadFs(DEVICE_LIST_EMMC_FS3,SYNC_MODE);
                    #endif

                    #ifdef _SPI_BOOT_
                    DeviceTask_LoadFs(DEVICE_LIST_SPI_FS3,SYNC_MODE);
                    #endif

                    #ifndef _HIDDEN_DISK1_
                    FW_CodePageLoad();
                    #ifdef _USE_GUI_
                    GuiTask_LoadResource();
                    #endif
                    #endif
                }
                #endif
            }
        }
    }
    else
    {
        status = 1;
    }

    ChargeDisable();

    UsbService_DeleteGuiHandle();

    if (status == 1) //go to Maintask Window
    {
#ifdef _MEDIA_MODULE_
        if(gSysConfig.MedialibPara.MediaUpdataFlag)
        {
            printf("\n -----media update-----\n");
            RKTaskCreate(TASK_ID_MEDIA_UPDATE, 0, NULL, SYNC_MODE);
        }
#endif

        MainTask_TaskSwtich(TASK_ID_USBSERVER, 0, TASK_ID_MAIN, 0, NULL);
    }
    else if (status == 2) //go to PLayMenu Window
    {
        #ifndef _USE_GUI_
        rkos_sleep(1000);
        MainTask_TaskSwtich(TASK_ID_USBSERVER, 0, TASK_ID_MUSIC_PLAY_MENU, 0, NULL);
        #endif
    }
}


/*******************************************************************************
** Name: UsbService_Enter
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API void UsbService_Enter(void)
{
    USBSERVICE_ASK_QUEUE UsbAskQueue;
    //printf ("\nUsb Service Enter\n");

    HTC hSelf;
    hSelf = RKTaskGetRunHandle();


    UsbMscDeviceServer();

    while (1)
    {
        rkos_queue_receive(gpstUsbService->UsbServiceAskQueue, &UsbAskQueue, MAX_DELAY);
        if(UsbServiceCheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

        switch (UsbAskQueue.cmd)
        {
            case KEY_VAL_ESC_SHORT_UP:
                #ifdef _USE_GUI_
                UsbMscDev_IntDisalbe();
                //printf ("\nUSB KEY ESC\n");

                UsbService_DeleteUsb(1);
                //RKTaskDelete(TASK_ID_USBSERVER, 0, ASYNC_MODE);
                while(1)
                {
                    rkos_sleep(1000);
                }
                #else
                if(MainTask_GetStatus(MAINTASK_APP_USB_OK) == 1)
                {
                    printf ("Delete Usb Server\n");

                    UsbMscDev_IntDisalbe();
                    UsbService_DeleteUsb(2);
                }
                #endif
                break;
            case KEY_VAL_PLAY_PRESS_START://关机
                #if 0
                #if 1
                #ifdef _EMMC_BOOT_
                DeviceTask_LoadFs(DEVICE_LIST_SPI_FS1,SYNC_MODE);
                #endif

                #ifdef _SPI_BOOT_
                DeviceTask_LoadFs(DEVICE_LIST_SPI_FS1,SYNC_MODE);
                #endif
                //rkos_sleep(1000);
                GuiTask_LoadResource();
                #endif
                MainTask_SysEventCallBack(MAINTASK_SHUTDOWN_CMD, NULL);
                #endif
                break;
            default:
                break;
        }
    }
}


/*******************************************************************************
** Name: UsbService_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t UsbService_DeInit(void *pvParameters)
{
    //Notification MainTask USB already Deleted
    MainTask_SetStatus(MAINTASK_APP_USB_START,0);
    MainTask_SetStatus(MAINTASK_APP_USB_OK,0);
#ifdef _USE_GUI_
    GuiTask_AppUnReciveMsg(UsbService_ButtonCallBack);
#else
    MainTask_UnRegisterKey();
#endif
    rkos_queue_delete(gpstUsbService->UsbServiceAskQueue);
    rkos_queue_delete(gpstUsbService->UsbServiceRespQueue);
    rkos_memory_free(gpstUsbService);
    gpstUsbService = NULL;
    FREQ_ExitModule(FREQ_MAX);
    //printf("Delete UsbService Success\n");

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_USBSERVER_TASK);
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: UsbService_CallBack
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API void UsbService_CallBack(void)
{
    USBSERVICE_ASK_QUEUE UsbServiceAskQueue;
    rkos_queue_send(gpstUsbService->UsbServiceAskQueue, &UsbServiceAskQueue, MAX_DELAY);
}


/*******************************************************************************
** Name: UsbService_Resume
** Input:void
** Return: rk_err_t
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t UsbService_Resume(void)
{

}

/*******************************************************************************
** Name: UsbService_Suspend
** Input:void
** Return: rk_err_t
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t UsbService_Suspend(void)
{
    return 0;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UsbService_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_INIT_
INIT API rk_err_t UsbService_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS*   pMainTask = (RK_TASK_CLASS*)pvParameters;
    USBSERVICE_ARG * pArg = (USBSERVICE_ARG *)arg;
    USBSERVICE_DATA_BLOCK*  pUsbServiceData;

    if (pMainTask == NULL)
        return RK_PARA_ERR;

    FREQ_EnterModule(FREQ_MAX);

    pUsbServiceData = rkos_memory_malloc(sizeof(USBSERVICE_DATA_BLOCK));
    memset(pUsbServiceData, NULL, sizeof(USBSERVICE_DATA_BLOCK));

    pUsbServiceData->UsbServiceAskQueue = rkos_queue_create(1, sizeof(USBSERVICE_ASK_QUEUE));
    pUsbServiceData->UsbServiceRespQueue = rkos_queue_create(1, sizeof(USBSERVICE_RESP_QUEUE));

    gpstUsbService = pUsbServiceData;


#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_USBSERVER_TASK, SEGMENT_OVERLAY_ALL);
#endif

    #ifdef _USE_GUI_
    GuiTask_AppReciveMsg(UsbService_ButtonCallBack);
    #else
    MainTask_RegisterKey(UsbService_NoScreenUSBKey);
    #endif

    return 0;
}
#endif

