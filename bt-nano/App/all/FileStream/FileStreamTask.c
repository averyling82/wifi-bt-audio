/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\App\FileStream\FileStreamTask.c
* Owner: Aaron.sun
* Date: 2014.5.7
* Time: 17:43:17
* Desc: File Stream
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.5.7     17:43:17   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __APP_FILESTREAM_FILESTREAMTASK_C__

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
#include "AppInclude.h"

#include "opt.h"
#include "api.h"
#include "tcpip.h"
#include "memp.h"
#include "ip.h"
#include "raw.h"
#include "udp.h"
#include "tcp.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _FILESTREAM_TASK_DATA_BLOCK
{

    uint8 * buf;
    uint32 len;
    uint32 front;
    uint32 back;
    uint32 ShuffleMode;
    uint32 LoopMode;

    pQueue  FileStreamAskQueue;
    pQueue  FileStreamRespQueue;
    pSemaphore FileSycnSem;
    uint32 FileSycnCmd;
    uint32 TaskObjectID;
    HDC hFifo;

}FILESTREAM_TASK_DATA_BLOCK;

typedef  struct _FILESTREAM_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}FILESTREAM_RESP_QUEUE;

typedef  struct _FILESTREAM_ASK_QUEUE
{
    uint32 cmd;

}FILESTREAM_ASK_QUEUE;


typedef struct _FILE_READ_st
{
    uint8 * buf;
    uint32  blocksize;

}F_READ_PARAM,*pF_READ_PARAM;

typedef struct _HIFI_FILE_SEEK_st
{
    uint8 Whence;
    int32 SecNum;

}HF_SEEK_PARAM,*pHF_SEEK_PARAM;


uint32 gReadCmd;
uint32 gReadData;
F_READ_PARAM gReadParam;

uint32 gHifiFlag1;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static FILESTREAM_TASK_DATA_BLOCK * gpstFileStreamDataBlock;



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
void UdpRecive(void *arg, struct udp_pcb *pcb, struct pbuf *p,ip_addr_t *addr, u16_t port);
rk_err_t FileStreamTaskShellHelp(HDC dev, uint8 * pstr);
rk_err_t FileStreamTaskShellTest(HDC dev, uint8 * pstr);
rk_err_t FileStreamTaskShellDel(HDC dev, uint8 * pstr);
rk_err_t FileStreamTaskShellMc(HDC dev, uint8 * pstr);
rk_err_t FileStreamTaskShellPcb(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: FileStreamTask_Enter
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.26
** Time: 19:17:01
*******************************************************************************/
_APP_FILESTREAM_FILESTREAMTASK_COMMON_
COMMON API void FileStreamTask_Enter(void)
{
    HDC hFifo;
    HDC hFile;
    FILE_ATTR stFileAttr;
    rk_err_t ret;
    uint8 Buf[4096];
    uint32 Bank, LBA;
    struct udp_pcb * udp;

    struct netif * netif;


    netif = netif_find("wl0");
    if(netif == NULL)
    {
        rk_printf("not find network");
    }


    hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
    gpstFileStreamDataBlock->hFifo = hFifo;

#if 1
    fifoDev_SetTotalSize(hFifo, 0xffffffff);


    udp = udp_new();

    ip_set_option(udp, SOF_BROADCAST);

    ret = udp_bind(udp, &netif->ip_addr, 6789);
    if(ret != ERR_OK)
    {
        rk_printf("udp bind error code = %d", ret);
    }

    //ret = udp_connect(udp, IP_ADDR_ANY, 0);

    //if(ret != ERR_OK)
    {
    //    rk_printf("udp connect err code = %d", ret);
    }

    udp_recv(udp, UdpRecive, NULL);

    rk_printf("udp connect ok");



    rkos_semaphore_take(gpstFileStreamDataBlock->FileSycnSem, MAX_DELAY);
#endif


#if 0
    stFileAttr.Path = L"C:\\I love you.pcm";
    stFileAttr.FileName = NULL;

    hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
    if((rk_err_t)hFile <= 0)
    {
        rk_print_string("File Open Failure");

        RKDev_Close(hFifo);
        while(1);
    }

    rk_printf("!!!!stFileAttr.FileSize = 0x%08x!!!\n",stFileAttr.FileSize);



    fifoDev_SetTotalSize(hFifo, stFileAttr.FileSize);

    LBA = 0;

    while(FileDev_ReadFile(hFile, Buf,  4096) == 4096)
    {
        LBA += 1;
        //rk_printf("LBA = %d", LBA);
        ret = fifoDev_Write(hFifo, Buf, 4096, SYNC_MODE, NULL);

        if(ret != 4096)
        {
            goto over;
        }

    }
    rk_print_string("file end");

#endif

over:

    FileDev_CloseFile(hFile);
    RKDev_Close(hFifo);
    RKTaskDelete(TASK_ID_FILESTREAM, gpstFileStreamDataBlock->TaskObjectID, ASYNC_MODE);

    while(1);

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: UdpRecive
** Input:void *arg, struct udp_pcb *pcb, struct pbuf *p,
** Return: void
** Owner:aaron.sun
** Date: 2015.9.17
** Time: 9:49:09
*******************************************************************************/
_APP_FILESTREAM_FILESTREAMTASK_COMMON_
COMMON FUN void UdpRecive(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    uint8 buf[2048], i;
    rk_printf("recive a udp packet.... = %d", p->len);


    if(p->len == 1024)
    {

        for(i = 0; i < 1024; i++)
        {
           buf[2 * i] = ((uint8 *)(p->payload))[i];
           buf[2 * i + 1] = ((uint8 *)(p->payload))[i];
        }
        fifoDev_Write(gpstFileStreamDataBlock->hFifo, buf, 2048, SYNC_MODE, NULL);
    }

    pbuf_free(p);
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FileStreamTask_DevInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.26
** Time: 19:17:01
*******************************************************************************/
_APP_FILESTREAM_FILESTREAMTASK_INIT_
INIT API rk_err_t FileStreamTask_DevInit(void *pvParameters)
{
    rkos_queue_delete(gpstFileStreamDataBlock->FileStreamAskQueue);
    rkos_queue_delete(gpstFileStreamDataBlock->FileStreamRespQueue);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FileStreamTask_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.26
** Time: 19:17:01
*******************************************************************************/
_APP_FILESTREAM_FILESTREAMTASK_INIT_
INIT API rk_err_t FileStreamTask_Init(void *pvParameters)
{
    RK_TASK_CLASS*   pDeviceTask = (RK_TASK_CLASS*)pvParameters;
    FILESTREAM_TASK_DATA_BLOCK*  pFileStreamDataBlock;

    uint32 i;

    if(pDeviceTask == NULL)
        return RK_PARA_ERR;


    pFileStreamDataBlock = rkos_memory_malloc(sizeof(FILESTREAM_TASK_DATA_BLOCK));
    if(pFileStreamDataBlock == NULL)
    {
        return RK_ERROR;
    }

    memset(pFileStreamDataBlock, 0, sizeof(FILESTREAM_TASK_DATA_BLOCK));


    pFileStreamDataBlock->FileStreamAskQueue = rkos_queue_create(1, sizeof(FILESTREAM_ASK_QUEUE));
    pFileStreamDataBlock->FileStreamRespQueue = rkos_queue_create(1, sizeof(FILESTREAM_RESP_QUEUE));
    pFileStreamDataBlock->TaskObjectID = pDeviceTask->TaskObjectID;

    pFileStreamDataBlock->FileSycnSem = rkos_semaphore_create(1, 0);
    gpstFileStreamDataBlock = pFileStreamDataBlock;

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_FILESTREAM_TASK, SEGMENT_OVERLAY_ALL);
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
#endif

