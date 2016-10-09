/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\App\Audio\AudioTask.c
* Owner: Aaron.sun
* Date: 2014.5.7
* Time: 17:41:42
* Desc: Media Task
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.5.7     17:41:42   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __APP_MEDIA_MEDIATASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "bt_config.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum _MEDIA_STATUS
{
    MEDIA_STATUS_IDLE,
    MEDIA_STATUS_PLAY,
    MEDIA_STATUS_STOP,
    MEDIA_STATUS_FFW,
    MEDIA_STATUS_FFD,
    MEDIA_STATUS_SET_A,
    MEDIA_STATUS_AB,
    MEDIA_STATUS_PAUSE,
    MEDIA_STATUS_NUM

}MEDIA_STATUS;

typedef  struct _MEDIA_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}MEDIA_RESP_QUEUE;


typedef  struct _MEDIA_ASK_QUEUE
{
    uint32 cmd;

}MEDIA_ASK_QUEUE;


typedef  struct _MEDIA_TASK_DATA_BLOCK
{
    HDC hAudio;
    uint32 BitRate;
    uint32 bps;
    uint32 SampleRate;
    uint32 Channel;
    uint32 currentPlayTime;
    uint32 TotalPlayTime;
    uint32 outptr;
    uint32 OutLength;
    pQueue  MediaAskQueue;
    pQueue  MediaRespQueue;
    uint32 TaskObjectID;

}MEDIA_TASK_DATA_BLOCK;


typedef  struct _AUDIO_FUN
{
    rk_err_t (* pfCodeOpen)(void);

}AUDIO_FUN;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static MEDIA_TASK_DATA_BLOCK * gpstMediaDataBlock;
static HDC hAudio, hFifo;


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
rk_err_t MediaTaskShellHelp(HDC dev,  uint8 * pstr);
rk_err_t MediaTaskShellTest(HDC dev, uint8 * pstr);
rk_err_t MediaTaskShellDel(HDC dev, uint8 * pstr);
rk_err_t MediaTaskShellMc(HDC dev, uint8 * pstr);
rk_err_t MediaTaskShellPcb(HDC dev, uint8 * pstr);

extern int32 BTPhoneDataSend(uint8 *data, uint16 len);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MediaTask_Enter
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 17:23:43
*******************************************************************************/
_APP_MEDIA_MEDIATASK_COMMON_
COMMON API void MediaTask_Enter(void)
{
    //HDC hAudio, hFifo;

    rk_err_t ret;

    uint8 Buf[8192];
    uint32 Bank;
    uint32 totalsize;

    rk_print_string("MediaTask_Enter");
    hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);
    hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);

    if ((hAudio == NULL) || (hAudio == (HDC)RK_ERROR) || (hAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("Audio device open failure");
        while(1);
    }

    if ((hFifo == NULL) || (hFifo == (HDC)RK_ERROR) || (hFifo == (HDC)RK_PARA_ERR))
    {
        rk_print_string("fifo device open failure");
        RKDev_Close(hAudio);
        while(1);
    }

    Bank = 0;

    ret = AudioDev_GetMainTrack(hAudio);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("Audio main track Get Failure");
        goto over;
    }

    AudioDev_SetChannel(hAudio, 0, 2);
    AudioDev_SetSampleRate(hAudio, 0, CodecFS_8000Hz);
    AudioDev_SetTrackLen(hAudio, 2048);
    AudioDev_SetBit(hAudio, 0, 16);

    AudioDev_SetVol(hAudio, 25);

    fifoDev_SetFIFOLevel(hFifo, 4096, 2048);

    fifoDev_GetTotalSize(hFifo, &totalsize);

    while(fifoDev_Read(hFifo, Buf + Bank * 4096, 2048, 0, SYNC_MODE, NULL) == 2048)
    {
        ret = AudioDev_Write(hAudio, 0, Buf + Bank * 4096);
        if(ret != RK_SUCCESS)
        {
            goto over;
        }
        Bank = Bank?0:1;
#ifdef _AEC_DECODE
        BTPhoneVoiceStreamNofity(Buf + Bank * 4096, 2048);
#endif
    }
    AudioDev_RealseMainTrack(hAudio);
    rk_print_string("play ok");

over:
    rk_printf("play error:%d\n", ret);
    RKDev_Close(hAudio);
    RKDev_Close(hFifo);
    RKTaskDelete(TASK_ID_MEDIA, gpstMediaDataBlock->TaskObjectID, ASYNC_MODE);
    while(1);

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
** Name: MediaTask_DevInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 17:23:43
*******************************************************************************/
_APP_MEDIA_MEDIATASK_INIT_
INIT API rk_err_t MediaTask_DevInit(void *pvParameters)
{
    rkos_queue_delete(gpstMediaDataBlock->MediaAskQueue);
    rkos_queue_delete(gpstMediaDataBlock->MediaRespQueue);
    rkos_memory_free(gpstMediaDataBlock);
    AudioDev_RealseMainTrack(hAudio);
    RKDev_Close(hAudio);
    RKDev_Close(hFifo);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaTask_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 17:23:43
*******************************************************************************/
_APP_MEDIA_MEDIATASK_INIT_
INIT API rk_err_t MediaTask_Init(void *pvParameters)
{
    RK_TASK_CLASS *   pDeviceTask = (RK_TASK_CLASS*) pvParameters;
    MEDIA_TASK_DATA_BLOCK*  pMediaDataBlock;

    uint32 i;

    if(pDeviceTask == NULL)
        return RK_PARA_ERR;

    pMediaDataBlock = rkos_memory_malloc(sizeof(MEDIA_TASK_DATA_BLOCK));

    if(pMediaDataBlock == NULL)
    {
        return RK_ERROR;
    }


    memset(pMediaDataBlock, 0, sizeof(MEDIA_TASK_DATA_BLOCK));


    pMediaDataBlock->MediaAskQueue = rkos_queue_create(1, sizeof(MEDIA_ASK_QUEUE));
    pMediaDataBlock->MediaRespQueue = rkos_queue_create(1, sizeof(MEDIA_RESP_QUEUE));
    pMediaDataBlock->TaskObjectID = pDeviceTask->TaskObjectID;

    gpstMediaDataBlock = pMediaDataBlock;

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_MEDIA_TASK, SEGMENT_OVERLAY_ALL);
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

