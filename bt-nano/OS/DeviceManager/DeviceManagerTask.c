/*
********************************************************************************************
*
*                Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\OS\Plugin\DevicePlugin.c
* Owner: Aaron.sun
* Date: 2014.4.9
* Time: 8:56:53
* Desc: device manager task
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.4.9     8:56:53   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __OS_PLUGIN_DEVICEPLUGIN_C__

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
#include "FwAnalysis.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _DEVICE_TASK_DATA_BLOCK
{
    uint32 DeviceList[DEVICE_LIST_NUM + MAX_D_DEV_LIST][MAX_DEV_PER_LIST + 1];
    pQueue  DeviceAskQueue;
    pQueue  DeviceRespQueue;

}DEVICE_TASK_DATA_BLOCK;

typedef struct _DEVICE_ASK_QUEUE
{
    uint32 cmd;
    uint32 DeviceListID;
    uint32 Mode;
    void * arg;

}DEVICE_ASK_QUEUE;

typedef struct _DEVICE_RESP_QUEUE
{
    uint32 cmd;
    uint32 Status;

}DEVICE_RESP_QUEUE;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static DEVICE_TASK_DATA_BLOCK *   gpstDeviceTaskDataBlock;
uint32 * DeviceList;


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
rk_err_t DeviceTaskCheckIdle(HTC hTask);
rk_err_t DeviceResume(uint32 ClassID, uint32 ObjectID);
rk_err_t DeleteDeviceList(uint32 DeviceListID, void * arg);
void SystemReset(uint32 maskrom);
void SystemPowerOff(void);
rk_err_t CreateDeviceList(uint32 DeviceListID, void * arg);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DeviceTask_ResumeDevice
** Input:uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.29
** Time: 15:12:35
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_ResumeDevice(uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DEVICE_RESP_QUEUE DeviceRespQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_RESUME;
    DeviceAskQueue.DeviceListID = (ClassID << 16) | ObjectID;
    DeviceAskQueue.arg = NULL;
    DeviceAskQueue.Mode = Mode;

    if(Mode == DIRECT_MODE)
    {
        return RK_ERROR;
    }

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
        if (DeviceRespQueue.Status == RK_SUCCESS)
        {
            return RK_SUCCESS;
        }
        else
        {
            return RK_ERROR;
        }
    }
    else if(Mode == ASYNC_MODE)
    {
        return RK_SUCCESS;
    }

}


/*******************************************************************************
** Name: DeviceTask_DeleteDevice
** Input:uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.29
** Time: 15:11:56
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_DeleteDevice(uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode)
{

}

/*******************************************************************************
** Name: DeviceTask_CreateDevice
** Input:uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.29
** Time: 15:09:45
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_CreateDevice(uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode)
{

}

/*******************************************************************************
** Name: DeviceTask_DevIdleTick
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.28
** Time: 14:01:14
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_DevIdleTick(void)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_IDLE_TICK;
    DeviceAskQueue.Mode = ASYNC_MODE;

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, 0);
}

#ifdef _FS_
/*******************************************************************************
** Name: DeviceTask_LoadFs
** Input:uint32 DeviceListID, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.31
** Time: 16:39:59
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_LoadFs(uint32 DeviceListID, uint32 Mode)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DEVICE_RESP_QUEUE DeviceRespQueue;
    HDC hLun;
    DeviceAskQueue.cmd = DEVICE_CMD_LOAD_FS;
    DeviceAskQueue.DeviceListID = DeviceListID;
    DeviceAskQueue.arg = NULL;
    DeviceAskQueue.Mode = Mode;

    if(Mode == DIRECT_MODE)
    {
        return RK_ERROR;
    }

    hLun = RKDev_Open(DEV_CLASS_LUN, 2, NOT_CARE);
    RKDev_Close(hLun);

    hLun = RKDev_Open(DEV_CLASS_LUN, 3, NOT_CARE);
    RKDev_Close(hLun);

    hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);
    RKDev_Close(hLun);

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
        if (DeviceRespQueue.Status == RK_SUCCESS)
        {
            return RK_SUCCESS;
        }
        else
        {
            return RK_ERROR;
        }
    }
    else if(Mode == ASYNC_MODE)
    {
        return RK_SUCCESS;
    }

}

/*******************************************************************************
** Name: DeviceTask_RemoveFs
** Input:uint32 DeviceListID, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.31
** Time: 16:38:56
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_RemoveFs(uint32 DeviceListID, uint32 Mode)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DEVICE_RESP_QUEUE DeviceRespQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_REMOVE_FS;
    DeviceAskQueue.DeviceListID = DeviceListID;
    DeviceAskQueue.arg = NULL;
    DeviceAskQueue.Mode = Mode;

    if(Mode == DIRECT_MODE)
    {
        return RK_ERROR;
    }

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
        if (DeviceRespQueue.Status == RK_SUCCESS)
        {
            return RK_SUCCESS;
        }
        else
        {
            return RK_ERROR;
        }
    }
    else if(Mode == ASYNC_MODE)
    {
        return RK_SUCCESS;
    }

}
#endif

/*******************************************************************************
** Name: DeviceTask_DeleteDeviceList
** Input:uint32 DeviceListID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.20
** Time: 19:55:22
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_DeleteDeviceList(uint32 DeviceListID, void * arg, uint32 Mode)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DEVICE_RESP_QUEUE DeviceRespQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_DELETE_LIST;
    DeviceAskQueue.DeviceListID = DeviceListID;
    DeviceAskQueue.arg = arg;
    DeviceAskQueue.Mode = Mode;

    if(Mode == DIRECT_MODE)
    {
        return RK_ERROR;
    }

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
        if (DeviceRespQueue.Status == RK_SUCCESS)
        {
            return RK_SUCCESS;
        }
        else
        {
            return RK_ERROR;
        }
    }
    else if(Mode == ASYNC_MODE)
    {
        return RK_SUCCESS;
    }

}

/*******************************************************************************
** Name: DeviceTask_System_PowerOff
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.17
** Time: 18:54:03
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_System_PowerOff(void)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_SYS_POWER_OFF;
    DeviceAskQueue.Mode = ASYNC_MODE;

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

}
/*******************************************************************************
** Name: DeviceTask_SystemReset
** Input:void
** Return: rk_err_T
** Owner:aaron.sun
** Date: 2015.11.17
** Time: 18:52:01
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_SystemReset(uint32 maskrom)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_SYS_RESET;
    DeviceAskQueue.Mode = ASYNC_MODE;
    DeviceAskQueue.arg = (void *)maskrom;

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);
}

/*******************************************************************************
** Name: DeviceTask_CreateDeviceList
** Input:uint32 DeviceListID
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.9
** Time: 17:55:46
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API rk_err_t DeviceTask_CreateDeviceList(uint32 DeviceListID, void *arg, uint32 Mode)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DEVICE_RESP_QUEUE DeviceRespQueue;
    DeviceAskQueue.cmd = DEVICE_CMD_CREATE_LIST;
    DeviceAskQueue.DeviceListID = DeviceListID;
    DeviceAskQueue.arg = arg;
    DeviceAskQueue.Mode = Mode;

    if(Mode == DIRECT_MODE)
    {
        return RK_ERROR;
    }

    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
        if (DeviceRespQueue.Status == RK_SUCCESS)
        {
            return RK_SUCCESS;
        }
        else
        {
            return RK_ERROR;
        }
    }
    else if(Mode == ASYNC_MODE)
    {
        return RK_SUCCESS;
    }

}

/*******************************************************************************
** Name: DeviceTask
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.9
** Time: 10:11:27
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON API void DeviceTask(void)
{
    DEVICE_ASK_QUEUE DeviceAskQueue;
    DEVICE_RESP_QUEUE DeviceRespQueue;
    rk_err_t ret;
    HTC hSelf;
    hSelf = RKTaskGetRunHandle();

    SystemInit();
    FREQ_ExitModule(FREQ_INIT);

    while (1)
    {
        rkos_queue_receive(gpstDeviceTaskDataBlock->DeviceAskQueue, &DeviceAskQueue, MAX_DELAY);

        if(DeviceTaskCheckIdle(hSelf) != RK_SUCCESS)
        {
            rk_printf("dm resume fail");
            continue;
        }

        switch (DeviceAskQueue.cmd)
        {
            case DEVICE_CMD_CREATE_LIST:
            {
                ret = CreateDeviceList(DeviceAskQueue.DeviceListID, DeviceAskQueue.arg);
                if (ret == RK_SUCCESS)
                {
                    DeviceRespQueue.Status = RK_SUCCESS;
                }
                else
                {
                    DeviceRespQueue.Status = RK_ERROR;
                }

                if(DeviceAskQueue.Mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
                }
                break;
            }

            case DEVICE_CMD_DELETE_LIST:
            {
                ret = DeleteDeviceList(DeviceAskQueue.DeviceListID, DeviceAskQueue.arg);
                if (ret == RK_SUCCESS)
                {
                    DeviceRespQueue.Status = RK_SUCCESS;
                }
                else
                {
                    DeviceRespQueue.Status = RK_ERROR;
                }

                if(DeviceAskQueue.Mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
                }
                break;
            }

            #ifdef _FS_
            case DEVICE_CMD_REMOVE_FS:
            {
                ret = DeviceListRemoveFs(DeviceAskQueue.DeviceListID, ((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DeviceAskQueue.DeviceListID]));

                if (ret == RK_SUCCESS)
                {
                    DeviceRespQueue.Status = RK_SUCCESS;
                }
                else
                {
                    DeviceRespQueue.Status = RK_ERROR;
                }

                if(DeviceAskQueue.Mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
                }
                break;
            }

            case DEVICE_CMD_LOAD_FS:
            {
                ret = DeviceListLoadFs(DeviceAskQueue.DeviceListID, ((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DeviceAskQueue.DeviceListID]));

                if (ret == RK_SUCCESS)
                {
                    DeviceRespQueue.Status = RK_SUCCESS;
                }
                else
                {
                    DeviceRespQueue.Status = RK_ERROR;
                }

                if(DeviceAskQueue.Mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
                }
                break;
            }
            #endif

            case DEVICE_CMD_SYS_RESET:
                SystemReset((uint32)DeviceAskQueue.arg);
                break;

            case DEVICE_CMD_SYS_POWER_OFF:
                SystemPowerOff();
                break;

            case DEVICE_CMD_IDLE_TICK:
                RKDev_IdleCheck();
                break;

            case DEVICE_CMD_RESUME:
                ret = DeviceResume(DeviceAskQueue.DeviceListID >> 16, DeviceAskQueue.DeviceListID & 0x0000ffff);

                if (ret == RK_SUCCESS)
                {
                    DeviceRespQueue.Status = RK_SUCCESS;
                }
                else
                {
                    DeviceRespQueue.Status = RK_ERROR;
                }

                if(DeviceAskQueue.Mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstDeviceTaskDataBlock->DeviceRespQueue, &DeviceRespQueue, MAX_DELAY);
                }
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
** Name: DeviceTaskCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 18:30:24
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON FUN rk_err_t DeviceTaskCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        pTask->State = TASK_STATE_WORKING;
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DeviceResume
** Input:uint32 ClassID, uint32 ObjectID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.29
** Time: 18:54:48
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON FUN rk_err_t DeviceResume(uint32 ClassID, uint32 ObjectID)
{
    int i, j, k;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 flag;

    pDev = (DEVICE_CLASS *)RKDeviceFind(ClassID, ObjectID);
    if(pDev->State == DEV_STATE_WORKING)
    {
        return RK_SUCCESS;
    }

    flag = 1;

    for(i = 0; i <(DEVICE_LIST_NUM + MAX_D_DEV_LIST); i++)
    {
        for(j = MAX_DEV_PER_LIST; j >= 0; j--)
        {
            if((((gpstDeviceTaskDataBlock->DeviceList[i][j] >> 8) & 0x000000ff) == ClassID)
                && ((gpstDeviceTaskDataBlock->DeviceList[i][j] & 0x000000ff) == ObjectID))
            {
                flag = 0;
                rk_printf("find dev %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                break;
            }
        }

        if(j > 0)
        {
            for(k = 0; k <= j; k++)
            {
                ret = RKDeviceFind(((gpstDeviceTaskDataBlock->DeviceList[i][k] >> 8) & 0x000000ff)
                    , (gpstDeviceTaskDataBlock->DeviceList[i][k] & 0x000000ff));
                if(ret == RK_ERROR)
                {
                    return RK_ERROR;
                }
                else
                {
                    pDev = (DEVICE_CLASS *)ret;
                    if(pDev->State != DEV_STATE_WORKING)
                    {
                        if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                        {
                            if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                            {
                                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                                FW_LoadSegment(DevInfo_Table[pDev->DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                                #endif
                            }
                        }

                        if(pDev->resume(pDev) != RK_SUCCESS)
                        {
                            rk_printf("resume fail = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                        }

                        if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                        {
                            if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                            {
                                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                                FW_RemoveSegment(DevInfo_Table[pDev->DevClassID].SegmentID);
                                #endif
                            }
                        }

                        DevTotalSuspendCnt--;

                        rk_printf("resume ok = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }
                    else
                    {
                        rk_printf("resume ok = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }
                }


            }
        }
    }

    if(flag)
    {
        pDev = (DEVICE_CLASS *)RKDeviceFind(ClassID, ObjectID);
        if(pDev->State != DEV_STATE_WORKING)
        {
            if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
            {
                if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                {
                    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                    FW_LoadSegment(DevInfo_Table[pDev->DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                    #endif
                }
            }

            if(pDev->resume(pDev) != RK_SUCCESS)
            {
                rk_printf("resume fail = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
            }

            if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
            {
                if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                {
                    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                    FW_RemoveSegment(DevInfo_Table[pDev->DevClassID].SegmentID);
                    #endif

                    DevTotalSuspendCnt--;
                }
            }

            rk_printf("resume ok = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
        }
        else
        {
            rk_printf("resume ok = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
        }
    }
}

/*******************************************************************************
** Name: DeleteDeviceList
** Input:uint32 DeviceListID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.20
** Time: 9:14:11
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON FUN rk_err_t DeleteDeviceList(uint32 DeviceListID, void * arg)
{
    rk_err_t ret;

    switch (DeviceListID)
    {
#ifdef _EMMC_BOOT_

        case DEVICE_LIST_EMMC_BOOT:
        {
            return DeleteDeviceListEmmcBoot((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_BOOT]);
        }
        case DEVICE_LIST_EMMC_DATABASE:
        {
            return DeleteDeviceListEmmcDataBase((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_DATABASE]);
        }
        #ifdef _FS_
        case DEVICE_LIST_EMMC_FS1:
        {
            return DeleteDeviceListEmmcFs1((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_FS1]);
        }
        #ifdef _HIDDEN_DISK1_
        case DEVICE_LIST_EMMC_FS2:
        {
            return DeleteDeviceListEmmcFs2((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_FS2]);
        }
        #endif
        #ifdef _HIDDEN_DISK2_
        case DEVICE_LIST_EMMC_FS3:
        {
            return DeleteDeviceListEmmcFs3((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_FS3]);
        }
        #endif
        #endif
 #endif

 #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_BOOT:
        {
            return DeleteDeviceListSpiBoot((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_BOOT]);
        }
        case DEVICE_LIST_SPI_DATABASE:
        {
            return DeleteDeviceListSpiDataBase((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_DATABASE]);
        }
        #ifdef _FS_
        case DEVICE_LIST_SPI_FS1:
        {
            return DeleteDeviceListSpiFs1((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_FS1]);
        }
        #ifdef _HIDDEN_DISK1_
        case DEVICE_LIST_SPI_FS2:
        {
            return DeleteDeviceListSpiFs2((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_FS2]);
        }
        #endif
        #ifdef _HIDDEN_DISK2_
        case DEVICE_LIST_SPI_FS3:
        {
            return DeleteDeviceListSpiFs3((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_FS3]);
        }
        #endif
        #endif
 #endif

        case DEVICE_LIST_ADUIO_PLAY:
        {
             return DeleteDeviceListAudioPlay((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_ADUIO_PLAY]);
        }


        case DEVICE_LIST_SDIO:
        {
             return DeleteDeviceListSdio((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SDIO]);
        }

        #ifdef _USB_
        case DEVICE_LIST_USBDEVICE_MSC: //Device USBMSC
        {
             return DeleteDeviceListUsbDeviceMsc((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_USBDEVICE_MSC]);
        }
        #endif
#ifdef _USE_GUI_
        case DEVICE_LIST_DISPLAY:
        {
             return DeleteDeviceListDisplay((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_DISPLAY]);
        }
#endif

        case DEVICE_LIST_ADC_KEY:
        {
             return DeleteDeviceListKey((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_ADC_KEY]);
        }

        #ifdef _USB_
        case DEVICE_LIST_HOST_USBMSC:
        {
            return DeleteDeviceListUsbHostMsc((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_HOST_USBMSC]);
        }
        #endif

        #ifdef _FS_
        case DEVICE_LIST_DIR:
        {
            return DeleteDeviceListDir((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_DIR]);
        }
        #endif

        case DEVICE_LIST_FIFO:
        {
            return DeleteDeviceListFIFO((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_ADC_KEY], arg);
        }
        case DEVICE_LIST_FM:
        {
            #ifdef __DRIVER_FM_FMDEVICE_C__
            DeleteDeviceListFm((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_FM], arg);
            #endif
            return RK_SUCCESS;
        }
    }

err:

    return RK_ERROR;

}

/*******************************************************************************
** Name: SystemReset
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.18
** Time: 10:50:53
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON FUN void SystemReset(uint32 maskrom)
{
    rk_printf("power reset");
    FW_LoadSegment(SEGMENT_ID_INIT, SEGMENT_OVERLAY_ALL);
    SystemDeInit();
    if(maskrom)
    {
        System_Reset(SYS_RESET_MASKROM);
    }
    else
    {
        System_Reset(SYS_RESET_RESTART);
    }
}
/*******************************************************************************
** Name: SystemPowerOff
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.11.18
** Time: 10:41:43
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON FUN void SystemPowerOff(void)
{
    rk_printf("power off");
    FW_LoadSegment(SEGMENT_ID_INIT, SEGMENT_OVERLAY_ALL);
    SystemDeInit();
    System_Power_On(0);
    if(Grf_CheckVbus())
    {
        PmuSetSysRegister(0, 0);
        System_Reset(SYS_RESET_RESTART);
    }
    while(1);
}
/*******************************************************************************
** Name: CreateDeviceList
** Input:uint32 DeviceListID
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.9
** Time: 11:37:28
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_
COMMON FUN rk_err_t CreateDeviceList(uint32 DeviceListID, void * arg)
{
    rk_err_t ret;

    switch (DeviceListID)
    {
#ifdef _EMMC_BOOT_

        case DEVICE_LIST_EMMC_BOOT:
        {
            return CreateDeviceListEmmcBoot((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_BOOT]);
        }
        case DEVICE_LIST_EMMC_DATABASE:
        {
            return CreateDeviceListEmmcDataBase((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_DATABASE]);
        }
        #ifdef _FS_
        case DEVICE_LIST_EMMC_FS1:
        {
            return CreateDeviceListEmmcFs1((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_FS1]);
        }
        #ifdef _HIDDEN_DISK1_
        case DEVICE_LIST_EMMC_FS2:
        {
            return CreateDeviceListEmmcFs2((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_FS2]);
        }
        #endif

        #ifdef _HIDDEN_DISK2_
        case DEVICE_LIST_EMMC_FS3:
        {
            return CreateDeviceListEmmcFs3((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_EMMC_FS3]);
        }
        #endif
        #endif
 #endif

 #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_BOOT:
        {
            return CreateDeviceListSpiBoot((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_BOOT]);
        }
        case DEVICE_LIST_SPI_DATABASE:
        {
            return CreateDeviceListSpiDataBase((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_DATABASE]);
        }
        #ifdef _FS_
        case DEVICE_LIST_SPI_FS1:
        {
            return CreateDeviceListSpiFs1((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_FS1]);
        }
        #ifdef _HIDDEN_DISK1_
        case DEVICE_LIST_SPI_FS2:
        {
            return CreateDeviceListSpiFs2((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_FS2]);
        }
        #endif
        #ifdef _HIDDEN_DISK2_
        case DEVICE_LIST_SPI_FS3:
        {
            return CreateDeviceListSpiFs3((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SPI_FS3]);
        }
        #endif
        #endif
 #endif

        case DEVICE_LIST_ADUIO_PLAY:
        {
             return CreateDeviceListAudioPlay((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_ADUIO_PLAY]);
        }


        case DEVICE_LIST_SDIO:
        {
             return CreateDeviceListSdio((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_SDIO]);
        }

        #ifdef _USB_
        case DEVICE_LIST_USBDEVICE_MSC: //Device USBMSC
        {
             return CreateDeviceListUsbDeviceMsc((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_USBDEVICE_MSC]);
        }
        #endif
#ifdef _USE_GUI_
        case DEVICE_LIST_DISPLAY:
        {
             return CreateDeviceListDisplay((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_DISPLAY]);
        }
#endif

        case DEVICE_LIST_ADC_KEY:
        {
             return CreateDeviceListKey((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_ADC_KEY]);
        }

        #ifdef _USB_
        case DEVICE_LIST_HOST_USBMSC:
        {
            return CreateDeviceListUsbHostMsc((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_HOST_USBMSC]);
        }
        #endif
        #ifdef _FS_
        case DEVICE_LIST_DIR:
        {
            return CreateDeviceListDir((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_DIR]);
        }
        #endif

        case DEVICE_LIST_FIFO:
        {
            return CreateDeviceListFIFO((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_ADC_KEY], arg);
        }

        case DEVICE_LIST_FM:
        {
         #ifdef __DRIVER_FM_FMDEVICE_C__
            return CreateDeviceListFm((uint32 *)gpstDeviceTaskDataBlock->DeviceList[DEVICE_LIST_FM], arg);
          #endif
            return RK_SUCCESS;
        }

    }

err:

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
** Name: DeviceTaskDeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.9
** Time: 9:32:33
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_INIT_
INIT API rk_err_t DeviceTaskDeInit(void *pvParameters)
{
    rkos_queue_delete(gpstDeviceTaskDataBlock->DeviceAskQueue);
    rkos_queue_delete(gpstDeviceTaskDataBlock->DeviceRespQueue);
}

/*******************************************************************************
** Name: DeviceTaskInit
** Input:void *
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.9
** Time: 9:18:26
*******************************************************************************/
_OS_DEVICEMANAGER_DEVICEMANAGERTASK_INIT_
INIT API rk_err_t DeviceTaskInit(void *pvParameters)
{
    RK_TASK_CLASS*   pDeviceTask = (RK_TASK_CLASS*)pvParameters;
    DEVICE_TASK_DATA_BLOCK*  pDeviceTaskDataBlock;
    uint32 i;

    if (pDeviceTask == NULL)
        return RK_PARA_ERR;

    pDeviceTask->Idle1EventTime = 10 * PM_TIME;

    pDeviceTaskDataBlock = rkos_memory_malloc(sizeof(DEVICE_TASK_DATA_BLOCK));
    memset(pDeviceTaskDataBlock, 0, sizeof(DEVICE_TASK_DATA_BLOCK));
    pDeviceTaskDataBlock->DeviceAskQueue = rkos_queue_create(1, sizeof(DEVICE_ASK_QUEUE));
    pDeviceTaskDataBlock->DeviceRespQueue = rkos_queue_create(1, sizeof(DEVICE_RESP_QUEUE));


    gpstDeviceTaskDataBlock = pDeviceTaskDataBlock;
    DeviceList = (uint32*)gpstDeviceTaskDataBlock;
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

