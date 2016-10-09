
/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Web\dlna\dlna.c
* Owner: linyb
* Date: 2015.7.21
* Time: 18:10:48
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    linyb     2015.7.21     18:10:48   1.0
********************************************************************************************
*/





/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "BspConfig.h"
#ifdef __WIFI_DLNA_C__
#define NOT_INCLUDE_OTHER

#include "typedef.h"
#include "RKOS.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "DeviceManagerTask.h"
#include "ShellTask.h"
#include "FwAnalysis.h"
#include "ModuleInfoTab.h"
#include "PowerManager.h"
#include "DriverInclude.h"
#include "DeviceInclude.h"
#include "dlna.h"
#include "Bsp.h"

extern void dlna_main(void const * argument);
extern void stop_dlna_device(void);
extern void start_dlna_device(void);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _DLNA_ASK_QUEUE
{
    uint8 cmd;

}DLNA_ASK_QUEUE;

typedef struct _DLNA_RESP_QUEUE
{
    uint8 cmd;
}DLNA_RESP_QUEUE;

typedef struct _DLNA_TASK_DATA_BLOCK
{
   pQueue DLNAAPI_ASK;
   pQueue DLNAAPI_RESP;
}DLNA_TASK_DATA_BLOCK;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static DLNA_TASK_DATA_BLOCK *gpstDLNAData;


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
_DLNA_SHELL_
static SHELL_CMD ShellDLnaName[] =
{
    "start",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

_DLNA_SHELL_
rk_err_t dlna_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        printf("\n StrCnt = 0 \n");
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellDLnaName, pItem, StrCnt);
    if (ret < 0)
    {
        printf("\n ret < 0 \n");
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item
    switch (i)
    {
        case 0x00:
            RKTaskCreate(TASK_ID_DLNA,0, NULL, SYNC_MODE);
            rk_dlna_start();
            break;

        case 0x01:
            rk_dlna_end();
            RKTaskDelete(TASK_ID_DLNA,0,SYNC_MODE);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    ret = RK_SUCCESS;

    return ret;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DLNATask_Resume
** Input:void
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
//_WEB_DLNA_DLNA_COMMON_
COMMON API rk_err_t DLNATask_Resume(void)
{

}
/*******************************************************************************
** Name: DLNATask_Suspend
** Input:void
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
//_WEB_DLNA_DLNA_COMMON_
COMMON API rk_err_t DLNATask_Suspend(void)
{

}

void rk_dlna_start()
{
    DLNA_ASK_QUEUE dlna_ask;
    DLNA_RESP_QUEUE dlna_resp;

    dlna_ask.cmd = DLNA_CREATE;

    rkos_queue_send(gpstDLNAData->DLNAAPI_ASK, &dlna_ask, MAX_DELAY);
    rkos_queue_receive(gpstDLNAData->DLNAAPI_RESP, &dlna_resp, MAX_DELAY);

    return;
}

rk_err_t rk_dlna_end()
{
    DLNA_ASK_QUEUE dlna_ask;
    DLNA_RESP_QUEUE dlna_resp;


    dlna_ask.cmd = DLNA_DESTORY;

    rkos_queue_send(gpstDLNAData->DLNAAPI_ASK, &dlna_ask, MAX_DELAY);
  //  printf("rk_dlna_end 11 \n");
    rkos_queue_receive(gpstDLNAData->DLNAAPI_RESP, &dlna_resp, MAX_DELAY);
    return dlna_resp.cmd;
}


/*******************************************************************************
** Name: DLNATask_Enter
** Input:void
** Return: void
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
//_WEB_DLNA_DLNA_COMMON_
COMMON API void DLNATask_Enter(void)
{
    DLNA_ASK_QUEUE dlna_ask;
    DLNA_RESP_QUEUE dlna_resp;
    FIFO_DEV_ARG stFifoArg;
    FILE_ATTR stFileAttr;
    rk_err_t ret;

    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = L"RkFifiDevice.buf";


    stFifoArg.hReadFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    if ((int)stFifoArg.hReadFile <= 0)
    {
        ret = FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr);
        if (ret != RK_SUCCESS)
        {
            rk_print_string("file create failure");
            while(1)
            {
                rkos_sleep(1000);
            }
        }

        stFifoArg.hReadFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if ((int)stFifoArg.hReadFile <= 0)
        {
            while(1)
            {
                rkos_sleep(1000);
            }
        }
    }

    stFifoArg.BlockCnt = 200000;
    //stFifoArg.BlockCnt = 50;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 1;
    stFifoArg.ObjectId = 0;


    if (DeviceTask_CreateDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE) != RK_SUCCESS)
    {
        printf("fifo  device list create fail\n");
    }

    while(1)
    {
      rkos_queue_receive(gpstDLNAData->DLNAAPI_ASK, &dlna_ask, MAX_DELAY);
     // printf("dlna_asl cmd = %d\n", dlna_ask.cmd);
      switch(dlna_ask.cmd)
      {
         case DLNA_CREATE:
             //dlna_main(NULL);
             start_dlna_device();
             dlna_resp.cmd = RK_SUCCESS;
             rkos_queue_send(gpstDLNAData->DLNAAPI_RESP, &dlna_resp, MAX_DELAY);
             break;
         case DLNA_DESTORY:
             printf("dlna destory\n");
             stop_dlna_device();
             DeviceTask_DeleteDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE);
             if(stFifoArg.hReadFile)
             {
                FileDev_CloseFile(stFifoArg.hReadFile);
             }

             dlna_resp.cmd = RK_SUCCESS;
             rkos_queue_send(gpstDLNAData->DLNAAPI_RESP, &dlna_resp, MAX_DELAY);
             break;
         default:
             printf("dlna task cmd err\n");
             break;
      }

    }

}

//overlay xml
rk_err_t Cg_Get_Xml(DEVICE_XML type, char **buf,  uint32 *buf_len)
{
    SEGMENT_INFO_T stCodeInfo;
    uint32 page_addr;
    uint32 fwsize;
    uint32 offset = 0;

    switch(type)
    {
       case XML_DMR_DEVICE_SCRIPTION:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_DEVICE_DATA, &stCodeInfo);
            break;
       case XML_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_CONNECTIONMANAGER_SERVICE_DATA, &stCodeInfo);
            break;
       case XML_DMR_RENDERINGCONTROL_SERVICE_DESCRIPTION:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_RENDERINGCONTROL_SERVICE_DATA, &stCodeInfo);
            break;
       case XML_DMR_AVTRANSPORT_SERVICE_DESCRIPTION:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_AVTRANSPORT_SERVICE_DATA, &stCodeInfo);
            break;
       case XML_DMR_NAMINGBITMAP_DATA:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_NAMINGBITMAP_DATA,  &stCodeInfo);
            break;
       case XML_DMR_NMSTRTPAGES_DATA:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_NMSTRTPAGES_DATA, &stCodeInfo);
            break;
       case XML_DMR_NAMEPAGES_DATA:
            FW_GetSegmentInfo(SEGMENT_ID_DMR_NAMEPAGES_DATA, &stCodeInfo);
            break;
       default:
            printf("no xml\n");
            return RK_ERROR;
    }
    page_addr = stCodeInfo.CodeLoadBase;
    fwsize = stCodeInfo.CodeImageLength;
    offset = 0;

    *buf = malloc(fwsize);
     if(NULL == *buf)
     {
        printf("xml get malloc fail fwsize = %d\n", fwsize);
        return RK_ERROR;
     }
     FW_ReadFirmwaveByByte(page_addr, *buf, fwsize);
    *buf_len = fwsize;

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DLNATask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
_DLNA_TASK_INIT_
INIT API rk_err_t DLNATask_DeInit(void *pvParameters)
{
    rkos_queue_delete(gpstDLNAData->DLNAAPI_ASK);
    rkos_queue_delete(gpstDLNAData->DLNAAPI_RESP);
    rkos_memory_free(gpstDLNAData);
    FW_RemoveSegment(SEGMENT_ID_DLNA);
    FW_RemoveSegment(SEGMENT_ID_UPNP);
    FW_RemoveSegment(SEGMENT_ID_XML);
    //FW_RemoveSegment(SEGMENT_ID_DLNA_TASK_API);
    FW_RemoveSegment(SEGMENT_ID_HTTP);
    FREQ_ExitModule(FREQ_DLNA);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DLNATask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
_DLNA_TASK_INIT_
INIT API rk_err_t DLNATask_Init(void *pvParameters, void *arg)
{
    DLNA_TASK_DATA_BLOCK *mDlnaTaskData = NULL;

    mDlnaTaskData = rkos_memory_malloc(sizeof(DLNA_TASK_DATA_BLOCK));
    if(NULL == mDlnaTaskData)
    {
        rk_printf("dlna task init mdlnatask data malloc fail\n");
        return RK_ERROR;
    }

    FREQ_EnterModule(FREQ_DLNA);

    mDlnaTaskData->DLNAAPI_ASK = rkos_queue_create(1, sizeof(DLNA_ASK_QUEUE));
    mDlnaTaskData->DLNAAPI_RESP = rkos_queue_create(1, sizeof(DLNA_RESP_QUEUE));

    FW_LoadSegment(SEGMENT_ID_DLNA,SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_UPNP,SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_XML,SEGMENT_OVERLAY_ALL);
    //FW_LoadSegment(SEGMENT_ID_DLNA_TASK_API,SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_HTTP, SEGMENT_OVERLAY_ALL);


    gpstDLNAData = mDlnaTaskData;

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


