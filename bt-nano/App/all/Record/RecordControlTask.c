/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Record\RecordControl.c
* Owner: chad.ma
* Date: 2015.12.22
* Time: 16:52:29
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.ma     2015.12.22     16:52:29   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_RECORD_RECORDCONTROL_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "audio_globals.h"
#include "SysInfoSave.h"
#include "audio_file_access.h"

#include "rockcodec.h"
#include "codec.h"
#include "record_globals.h"
#include "RecordControlTask.h"
#include "FMControlTask.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _IN_RECORD_CONTROL

typedef  struct _RECORDCONTROL_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}RECORDCONTROL_RESP_QUEUE;
typedef  struct _RECORDCONTROL_ASK_QUEUE
{
    uint32 cmd;
    void * msg;
    uint32 mode;

}RECORDCONTROL_ASK_QUEUE;

typedef  struct _RECORDCONTROL_TASK_DATA_BLOCK
{
    pQueue      RecordControlAskQueue;
    pQueue      RecordControlRespQueue;

    HDC         hAudio;
    HDC         hFileHdl;
    HDC         hDirDev;
    HDC         hBcore;


    UINT32  RecordQuality;
    UINT32  RecordState;
    UINT32  RecordSource;

    uint32 RecordType;               //Record Mode : 0 : MIC_STERO / 1 : LINE1 / 2 : LINE2 / 3 : MIC1_MONO / 4 : MIC2_MONO
    uint32 RecordSampleRate;        //record Samplerate  8K - 192K : 8000 - 192000;
    uint32 RecordBitrate;
    uint32 RecordChannel;            //record Channels : 1 : mono / 2 : stero
    uint32 RecordDataWidth;         //Record DataWidth : 0x0F : 16bit / 0x17 : 24bit
    uint32 RecordEncodeType;        //Record EncodeType : 0 : wav / 1 : mp3 / 2 : pcm
    uint32 RecordEncodeSubType;       //Record CodecConfig : 0 : Codec no config / 1 : Codec config
    uint32 PcmDataUnusedSamples;        //
    uint8 *PcmInputBuf;        //
    RECORD_CALLBACK * pfRecordState;

    UINT32      TaskObjectID;

}RECORDCONTROL_TASK_DATA_BLOCK;

#define     RECORD_QUALITY_HIGH                 0
#define     RECORD_QUALITY_NORMAL               1

#define     RECORD_FMT_MP3                      0
#define     RECORD_FMT_WAV                      1

typedef struct
{
    UINT16      TotalFiles;     //the total file number in current directory.
    UINT16      CurrentFileNum;
    //UINT16      PlayedFileNum;
    //UINT16      DiskTotalFiles; //the total file number in all disk.

    //UINT16      Range;
    //UINT16      Mode;

    UINT16      *pExtStr;       //type of file.

    //FDT         Fdt;
    //FIND_DATA   FindData;

    //UINT16      RandomBuffer[16];
    UINT8       shortName[12];
    UINT8       Path[127];

} RECORD_SYS_FILE_INFO;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static RECORDCONTROL_TASK_DATA_BLOCK * gpstRecordControlData;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
UINT32      RecordSamples;
UINT32      RecordInputBuffer;
UINT32      RecordOutputBuffer;

UINT32      RecordFreeMemory;

UINT32      RecordTotalTime;              //total time of record.
UINT32      RecordCurrTimeSave;           //left time can be use for recording.
UINT32      RecordCurrTime;               //the current record time.


UINT16     RecordLongFileName[64]; //record file name.
RECORD_SYS_FILE_INFO   RecordFileInfo;

DMA_CFGX RecordControlDmaCfg  = {DMA_CTLL_I2S0_RX, DMA_CFGL_I2S0_RX, DMA_CFGH_I2S0_RX, 0};

//UINT8 MicPathStr[]      = {'U', ':', '\\', 'R', 'E', 'C', 'O', 'R', 'D', ' ', ' ', ' ', ' ', ' ', '\\', 'V', 'O', 'I', 'C', 'E', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
UINT8 RecordPathStr[]      = {'C', ':', '\\', 'R', 'E', 'C', 'O', 'R', 'D', '\\', '\0'};
UINT8 RecordFileNameWavStr[]  = {'R', 'E', 'C', '0', '0', '0', '0',  'W', 'A', 'V', '\0'};
UINT8 RecordFileNameMp3Str[]  = {'R', 'E', 'C', '0', '0', '0', '0',  'M', 'P', '3', '\0'};
UINT8 RecordFileNamePcmStr[]  = {'R', 'E', 'C', '0', '0', '0', '0',  'P', 'C', 'M', '\0'};
UINT8 RecordFileNameAmrStr[]  = {'R', 'E', 'C', '0', '0', '0', '0',  'A', 'M', 'R', '\0'};


#ifndef A_CORE_DECODE
RecFileHdl_t gRecordFileHdl; //file handle and other info struct
RecordBlock  gRecordBlock;   //record encord block

//UINT16   gRecEncbuf[WAV_AD_PIPO_BUFFER_SIZE];
static UINT16 RecordDmaTranferCallback;
static UINT16 DmaTranferCb;

pSemaphore osRecordEncodeOk;
HDC        hRecordDma;

_ATTR_WAVENC_DATA_ volatile unsigned int record_dma_channel;

_ATTR_WAVENC_TEXT_
static void DMATranferCb1(uint32 ch)
{
    DmaDev_RealseChannel(hRecordDma, record_dma_channel);
    rkos_semaphore_give(osRecordEncodeOk);
}
#endif

//uint8 RecordNameFlag[4096];

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint8 * RecordState2Str(uint8 recordState);
void RecordGetPCMAndProcess(void);
BOOLEAN RecordResume(void);

int16 RecordStart(void);
BOOLEAN RecordPause(void);
void RecordStop(uint32 IsNextFile);
BOOLEAN RecordEncodeProc(RECORD_CMD id, void * msg);
void RecordCodecDeInit(void);
void RecordSetType(UINT32  Type);
void RecordExitType(UINT32 Type);
void RecordCpuResume(void);
void RecordCpuInit(void);
void RecordCodecInit(void);
void RegMBoxRecFileSvc(void);
void RecordWaitBBStart(void);
void RecordWaitBBStop(void);
void RecordHWDeInit(void);
BOOLEAN RecordCreateFile(void);
__irq void RecordEncodeGetOutBuffer(void);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RecordPcmInput
** Input:uint8 * buf, uint32 size
** Return: void
** Owner:aaron.sun
** Date: 2016.3.3
** Time: 18:17:33
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API void RecordPcmInput(uint8 * buf, uint32 Samples)
{
    uint8 * pbuf = buf;
    uint32 pcmLenTemp;
    uint32 unusedPos;

    if(Samples == RecordSamples)
    {
        RecordInputBuffer = (uint32)buf;
        RecordControlTask_SendCmd(RECORD_CMD_GETPCM_PROCESS, NULL, SYNC_MODE);
    }
    else if((gpstRecordControlData->PcmDataUnusedSamples + Samples) >= RecordSamples)
    {
        //printf("S=%d > RS=%d UL=%d \n", Samples, RecordSamples, gpstRecordControlData->PcmDataUnusedSamples);
        pcmLenTemp = (((RecordSamples-gpstRecordControlData->PcmDataUnusedSamples) * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
        unusedPos = ((gpstRecordControlData->PcmDataUnusedSamples * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
        memcpy(((uint8 *)gpstRecordControlData->PcmInputBuf + unusedPos) , buf, pcmLenTemp);
        RecordInputBuffer = (uint32)gpstRecordControlData->PcmInputBuf;
        gpstRecordControlData->PcmDataUnusedSamples += Samples;
        while(1)
        {
            RecordControlTask_SendCmd(RECORD_CMD_GETPCM_PROCESS, NULL, SYNC_MODE);
            gpstRecordControlData->PcmDataUnusedSamples -= RecordSamples;
            #if 0
            if(Samples < gpstRecordControlData->PcmDataUnusedSamples)
            {
                printf(",,,error...\n");
            }
            #endif
            unusedPos = (((Samples - gpstRecordControlData->PcmDataUnusedSamples) * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
            pbuf = buf + unusedPos;
            RecordInputBuffer = (uint32)pbuf;
            if(gpstRecordControlData->PcmDataUnusedSamples < RecordSamples)
            {
                break;
            }
        }
        if(gpstRecordControlData->PcmDataUnusedSamples > 0)
        {
            //printf("US=%d ....\n", gpstRecordControlData->PcmDataUnusedSamples);
            pcmLenTemp = ((RecordSamples * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
            memset((uint8 *)gpstRecordControlData->PcmInputBuf, 0, pcmLenTemp);
            unusedPos = ((gpstRecordControlData->PcmDataUnusedSamples * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
            memcpy((uint8 *)gpstRecordControlData->PcmInputBuf , pbuf, unusedPos);
        }
    }
    else
    {
        //printf("S=%d < RS=%d  UL=%d\n", Samples, RecordSamples, gpstRecordControlData->PcmDataUnusedSamples);
        unusedPos = ((gpstRecordControlData->PcmDataUnusedSamples * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
        pcmLenTemp = ((Samples * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
        memcpy(((uint8 *)gpstRecordControlData->PcmInputBuf + unusedPos) , buf, pcmLenTemp);
        gpstRecordControlData->PcmDataUnusedSamples += Samples;
    }
}

/*******************************************************************************
** Name: Recorder_ChangeCallBack
** Input:P_RECORD_CALLBACK old, P_RECORD_CALLBACK new
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.7
** Time: 14:26:28
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API rk_err_t Recorder_ChangeCallBack(P_RECORD_CALLBACK old, P_RECORD_CALLBACK new)
{
    RECORD_CALLBACK * pCur, * pLast;

    pCur = gpstRecordControlData->pfRecordState;
    pLast = NULL;

    if(old != NULL)
    {
        while(pCur != NULL)
        {
            if(pCur->pfRecordState == old)
            {
                if(new != NULL)
                {
                    pCur->pfRecordState = new;
                    return RK_SUCCESS;
                }
                else
                {
                    if(pLast == NULL)
                    {
                        gpstRecordControlData->pfRecordState = pCur->pNext;
                    }
                    else
                    {
                        pLast->pNext = pCur->pNext;
                    }
                    rkos_memory_free(pCur);
                    return RK_SUCCESS;
                }

            }
            pLast = pCur;
            pCur = pCur->pNext;
        };
    }
    else
    {
        pCur = rkos_memory_malloc(sizeof(RECORD_CALLBACK));
        if((int32)pCur <= 0)
        {
            return RK_ERROR;
        }

        pCur->pNext = gpstRecordControlData->pfRecordState;
        pCur->pfRecordState = new;
        gpstRecordControlData->pfRecordState = pCur;
        return RK_SUCCESS;
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: Recorder_GetTotalTime
** Input:uint32 * time
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.7
** Time: 14:09:28
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API rk_err_t Recorder_GetTotalTime(uint32 * time)
{
    *time = RecordTotalTime;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: Recorder_GetCurTime
** Input:uint32 * time
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.7
** Time: 14:08:51
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API rk_err_t Recorder_GetCurTime(uint32 * time)
{
    *time = RecordCurrTime;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: Recorder_GetRecordInf
** Input:RECORD_INF * pRecordInf
** Return: void
** Owner:aaron.sun
** Date: 2016.3.7
** Time: 10:36:37
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API uint32 Recorder_GetRecordInf(RECORD_INFO * pRecordInf)
{
    if(pRecordInf != NULL)
    {
        pRecordInf->bitpersample = gpstRecordControlData->RecordDataWidth;
        pRecordInf->Bitrate = gpstRecordControlData->RecordBitrate;
        pRecordInf->channels = gpstRecordControlData->RecordChannel;
        pRecordInf->EncodeType = gpstRecordControlData->RecordEncodeType;
        memcpy((uint8 *)pRecordInf->FileName, (uint8 *)RecordFileInfo.shortName, 12);
    }

    return gpstRecordControlData->RecordState;
}

/*******************************************************************************
** Name: dumpMemoryCharA
** Input:uint8 * addr,uint32 size, uint8 data_type
** Return: void
** Owner:chad.ma
** Date: 2016.1.8
** Time: 14:29:25
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API void dumpMemoryCharA(uint8 * addr,uint32 size, uint8 data_type)
{
    uint32 index = 0;
    uint8 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            printf("\r\n");

        if (index % 16 == 0 && index != 0)
            printf("\n");

        switch ( data_type )
        {
            case PRINTF_HEX_DATA:
                printf("%02x ", *ptemp);
                break;

            case PRINTF_DEC_DATA:
                printf("%d ", *ptemp);
                break;

            case PRINTF_CHAR_DATA:
                printf("%c ", *ptemp);
                break;
        }

        ptemp++;
    }
}

/*******************************************************************************
** Name: dumpMemoryShort2Char
** Input:void* memory,uint32 size
** Return: void
** Owner:chad.ma
** Date: 2016.1.6
** Time: 16:40:04
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API void dumpMemoryShort2Char(uint16* memory,uint32 size)
{
    uint32 index = 0;
    uint8 temp;
    uint16* pTemp;
    pTemp = memory;

    for (index = 0; index < size; index++)
    {
        temp = *pTemp & 0xff;
        if(temp == 0)
            break;

        if (index == 0)
            printf("\r\n");

        if (index % 16 == 0 && index != 0)
            printf("\n");

        printf("%c", temp);

        pTemp++;
    }
    printf("\n");

}


/*******************************************************************************
** Name: RecordControlTask_SendCmd
** Input:uint32 Cmd, void * msg, uint32 Mode
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.5
** Time: 9:31:21
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API rk_err_t RecordControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode)
{
    RECORDCONTROL_ASK_QUEUE RecordControlAskQueue;
    RECORDCONTROL_RESP_QUEUE RecordControlRespQueue;

    while(RKTaskFind(TASK_ID_RECORDCONTROL, 0) == NULL)
    {
        rkos_delay(10);
    }

    RecordControlAskQueue.cmd = Cmd;
    RecordControlAskQueue.msg = msg;
    RecordControlAskQueue.mode = Mode;


    rkos_queue_send(gpstRecordControlData->RecordControlAskQueue, &RecordControlAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstRecordControlData->RecordControlRespQueue, &RecordControlRespQueue, MAX_DELAY);
        if ((RecordControlRespQueue.cmd == Cmd) && RecordControlRespQueue.status == RK_SUCCESS)
        {
            return RK_SUCCESS;
        }
        else
        {
            return RK_ERROR;
        }
    }
    else
    {
        return RK_SUCCESS;
    }
}


/*******************************************************************************
** Name: RecordControlTask_Enter
** Input:void * arg
** Return: void
** Owner:chad.ma
** Date: 2015.12.22
** Time: 16:56:56
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON API void RecordControlTask_Enter(void * arg)
{
    RECORDCONTROL_ASK_QUEUE RecordControlAskQue;
    RECORDCONTROL_RESP_QUEUE RecordControlResQue;

    rk_print_string("Record control task enter...\n");

    while(RKTaskCreate(TASK_ID_STREAMCONTROL, 0, NULL, SYNC_MODE) != RK_SUCCESS)
    {
        rkos_sleep(10);
    }

//    RecordStart();
    while(1)
    {
//        if ((RECORD_STATE_STOP == gpstRecordControlData->RecordState)||(RECORD_STATE_PAUSE == gpstRecordControlData->RecordState)
//            ||(RECORD_STATE_PREPARE == gpstRecordControlData->RecordState))
        {
            if(rkos_queue_receive(gpstRecordControlData->RecordControlAskQueue, &RecordControlAskQue, MAX_DELAY) == RK_SUCCESS)
            {
                RecordEncodeProc(RecordControlAskQue.cmd, RecordControlAskQue.msg);

                RecordControlResQue.cmd = RecordControlAskQue.cmd;
                RecordControlResQue.status = RK_SUCCESS;

                if(RecordControlResQue.cmd == RECORD_CMD_GETPCM_PROCESS)
                {
                    if((gpstRecordControlData->RecordState != RECORD_STATE_BEING) && (gpstRecordControlData->RecordState != RECORD_STATE_PAUSE))
                    {
                        continue;
                    }
                }

                if(RecordControlAskQue.mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstRecordControlData->RecordControlRespQueue, &RecordControlResQue, MAX_DELAY);
                }
            }
        }
//        else
//        {
//            if(rkos_queue_receive(gpstRecordControlData->RecordControlAskQueue, &RecordControlAskQue, 0) == RK_SUCCESS)
//            {
//                rk_printf("xxx rkos_queue_receive cmd %d",RecordControlAskQue.cmd);
//                RecordEncodeProc(RecordControlAskQue.cmd, RecordControlAskQue.msg);
//                RecordControlResQue.cmd = RecordControlAskQue.cmd;
//                RecordControlResQue.status = RK_SUCCESS;
//
//                if(RecordControlAskQue.mode == SYNC_MODE)
//                {
//                    rkos_queue_send(gpstRecordControlData->RecordControlRespQueue, &RecordControlResQue, MAX_DELAY);
//                }
//            }
//        }
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
** Name: RecordGetPCMAndProcess
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2016.1.9
** Time: 10:13:02
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordGetPCMAndProcess(void)
{
    UINT32 i, j;
    uint32 encodeDataLen, encodeDataLenlostlen, writeDataLen, encodeDataddr;
    uint32 index, index_old;

    if(gpstRecordControlData->RecordState == RECORD_STATE_BEING)
    {
        if (gpstRecordControlData->RecordChannel == RECORD_CHANNEL_MONO)      //µ¥ÉùµÀÂ¼Òô
        {
            if(gpstRecordControlData->RecordType == RECORD_TYPE_MIC_STERO)
            {
                if(RECORD_DATAWIDTH_16BIT == gpstRecordControlData->RecordDataWidth)
                {
                    uint16 * pInputBuffere16 =  (uint16 *)RecordInputBuffer;
                    uint16 * pOutputBuffere16 = (uint16 *)RecordOutputBuffer;
                    for (index = 0; index < RecordSamples; index++)
                    {
                        pOutputBuffere16[index] =   pInputBuffere16[2 * index];
                    }
                }
                else if (RECORD_DATAWIDTH_24BIT == gpstRecordControlData->RecordDataWidth)
                {
                    uint8 * pInputBuffere8=  (uint8 *)RecordInputBuffer;
                    uint8 * pOutputBuffere8 = (uint8 *)RecordOutputBuffer;
                    for (index = 0; index < RecordSamples; index++)
                    {
                        pOutputBuffere8[3 * index]     =  pInputBuffere8[index * 6];
                        pOutputBuffere8[3 * index + 1] =  pInputBuffere8[index * 6 + 1];
                        pOutputBuffere8[3 * index + 2] =  pInputBuffere8[index * 6 + 2];
                    }
                }
                else if(RECORD_DATAWIDTH_32BIT == gpstRecordControlData->RecordDataWidth)
                {
                    uint32 * pInputBuffere32=  (uint32 *)RecordInputBuffer;
                    uint32 * pOutputBuffere32 = (uint32 *)RecordOutputBuffer;
                    for (index = 0; index < RecordSamples; index++)
                    {
                        pOutputBuffere32[index]  =  pInputBuffere32[2 * index];
                    }
                }
            }
            else
            {
                //dma
                #if 0
                {

                     rk_err_t ret;
                     DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

                     ret = DmaDev_GetChannel(hRecordDma);

                     if (ret < 0)
                     {
                          memcpy( (uint8 *)RecordOutputBuffer,  (uint8 *)RecordInputBuffer ,(RecordSamples * pstRecCtrArg->RecordDataWidth ) / 16);
                     }
                     else
                     {
                         DmaDev_DmaStart(hRecordDma, (uint32)(ret), (UINT32)(RecordInputBuffer),(uint32)(RecordOutputBuffer),(RecordSamples * pstRecCtrArg->RecordDataWidth ) / 16,&DmaCfg, DMATranferCb1);
                         rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
                     }
                }
                #else
                memcpy((uint8 *)RecordOutputBuffer, (uint8 *)RecordInputBuffer ,(RecordSamples * gpstRecordControlData->RecordDataWidth) / 8);

                #endif
            }
        }
        else if ( gpstRecordControlData->RecordChannel == RECORD_CHANNEL_STERO)    //Ë«ÉùµÀÂ¼Òô
        {
            if((gpstRecordControlData->RecordType == RECORD_TYPE_MIC1_MONO)
                || (gpstRecordControlData->RecordType == RECORD_TYPE_MIC2_MONO))
            {
                if(RECORD_DATAWIDTH_16BIT == gpstRecordControlData->RecordDataWidth)
                {
                    uint16 * pInputBuffere16 =  (uint16 *)RecordInputBuffer;
                    uint16 * pOutputBuffere16 = (uint16 *)RecordOutputBuffer;
                    for (index = 0; index < RecordSamples; index++)
                    {
                        pOutputBuffere16[2 * index] =       pInputBuffere16[index];
                        pOutputBuffere16[2 * index + 1] =   pInputBuffere16[index];
                    }
                }
                else if (RECORD_DATAWIDTH_24BIT == gpstRecordControlData->RecordDataWidth)
                {
                    uint8 * pInputBuffere8=  (uint8 *)RecordInputBuffer;
                    uint8 * pOutputBuffere8 = (uint8 *)RecordOutputBuffer;
                    for (index = 0; index < RecordSamples; index++)
                    {
                        pOutputBuffere8[6 * index]     =  pInputBuffere8[index * 6 + 3];
                        pOutputBuffere8[6 * index + 1] =  pInputBuffere8[index * 6 + 4];
                        pOutputBuffere8[6 * index + 2] =  pInputBuffere8[index * 6 + 5];

                        pOutputBuffere8[6 * index + 3] =  pInputBuffere8[index * 6 + 3];
                        pOutputBuffere8[6 * index + 4] =  pInputBuffere8[index * 6 + 4];
                        pOutputBuffere8[6 * index + 5] =  pInputBuffere8[index * 6 + 5];
                    }
                }
                else if(RECORD_DATAWIDTH_32BIT == gpstRecordControlData->RecordDataWidth)
                {
                    uint32 * pInputBuffere32=  (uint32 *)RecordInputBuffer;
                    uint32 * pOutputBuffere32 = (uint32 *)RecordOutputBuffer;
                    for (index = 0; index < RecordSamples; index++)
                    {
                        pOutputBuffere32[index]  =  pInputBuffere32[index];
                    }
                }
            }
            else
            {
                //dma
                #if 0
                {

                     rk_err_t ret;
                     DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

                     ret = DmaDev_GetChannel(hRecordDma);

                     if (ret < 0)
                     {
                          memcpy((uint8 *)RecordOutputBuffer,  (uint8 *)RecordInputBuffer ,(RecordSamples * pstRecCtrArg->RecordDataWidth ) / 16);
                     }
                     else
                     {
                         DmaDev_DmaStart(hRecordDma, (uint32)(ret), (UINT32)(RecordInputBuffer),(uint32)(RecordOutputBuffer),(RecordSamples * pstRecCtrArg->RecordDataWidth ) / 16,&DmaCfg, DMATranferCb1);
                         rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
                     }
                }
                #else

                //printf("\n rec RecordSamples =%d size = %d",RecordSamples, (RecordSamples * gpstRecordControlData->RecordDataWidth) / 4);
                memcpy((uint8 *)RecordOutputBuffer, (uint8 *)RecordInputBuffer ,(RecordSamples * gpstRecordControlData->RecordDataWidth) / 4);

                #endif

            }

        }

        #ifdef A_CORE_DECODE
        CodecEncode();
        CodecGetEncBuffer((short *)&RecordOutputBuffer, &RecordSamples);
        #else
        CodecGetEncBuffer((short *)&RecordOutputBuffer, &RecordSamples);
        #endif

        RecordCurrTime += ((RecordSamples * 1000) / gpstRecordControlData->RecordSampleRate);

        if((RecordCurrTime - RecordCurrTimeSave) >= 1000)
        {
            RecordCurrTimeSave = RecordCurrTime;
            {
                RECORD_CALLBACK * pCur;

                pCur = gpstRecordControlData->pfRecordState;

                while(pCur != NULL)
                {
                    pCur->pfRecordState(RECORD_STATE_TIME_CHANGE);
                    pCur = pCur->pNext;
                }
            }
        }

    }

}

/*******************************************************************************
** Name: RecordResume
** Input:void
** Return: BOOLEAN
** Owner:chad.ma
** Date: 2015.12.25
** Time: 17:16:02
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN BOOLEAN RecordResume(void)
{

    if (gpstRecordControlData->RecordState == RECORD_STATE_BEING)
    {
        return FALSE;
    }



    if (gpstRecordControlData->RecordState != RECORD_STATE_BEING)
    {
        gpstRecordControlData->RecordState = RECORD_STATE_BEING;
        {
            RECORD_CALLBACK * pCur;

            pCur = gpstRecordControlData->pfRecordState;

            while(pCur != NULL)
            {
                pCur->pfRecordState(RECORD_STATE_BEING);
                pCur = pCur->pNext;
            }
        }
    }

    return TRUE;
}



/*******************************************************************************
** Name: RecordStart
** Input:void
** Return: int16
** Owner:chad.ma
** Date: 2015.12.24
** Time: 15:40:26
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN int16 RecordStart(void)
{
    int16 ret;
    uint32 pcmLen;

    AUDIO_DEV_ARG stAudioArg;

    if(gpstRecordControlData->RecordState == RECORD_STATE_PREPARE)
    {
        RecordCpuInit();
        RecordHWInit();
        ret = RecordCreateFile();     //build file in disk.
        if (FALSE == ret )
        {
            RecordCpuResume();
            rk_printf("[RECORD] RecordCreateFile Fail......");
            return RK_ERROR;
        }

#ifndef A_CORE_DECODE
        memset(&gRecordFileHdl, 0, sizeof(gRecordFileHdl));
        gRecordFileHdl.fileHandle =  4;
        gRecordFileHdl.codecType = CurrentEncCodec;
        FileDev_GetFileOffset(gpstRecordControlData->hFileHdl, &gRecordFileHdl.fileOffset);
        RecordWaitBBStart();
#endif
        RecordCodecInit();

        gpstRecordControlData->PcmDataUnusedSamples = 0;
        gpstRecordControlData->PcmInputBuf = NULL;
        pcmLen = ((RecordSamples * gpstRecordControlData->RecordChannel * gpstRecordControlData->RecordDataWidth) / 32) * 4;
        gpstRecordControlData->PcmInputBuf = rkos_memory_malloc(pcmLen);
        if (gpstRecordControlData->PcmInputBuf == NULL)
        {
            rk_printf("PcmInputBuf malloc error\n");
            return RK_ERROR;
        }
        memset(gpstRecordControlData->PcmInputBuf, 0, sizeof(gpstRecordControlData->PcmInputBuf));
        rk_printf("PcmInputBuf malloc %d ok",RecordSamples);

        if(RecordServiceGetTotalTime() != RK_SUCCESS)
        {
            {
                RECORD_CALLBACK * pCur;

                pCur = gpstRecordControlData->pfRecordState;

                while(pCur != NULL)
                {
                    pCur->pfRecordState(RECORD_STATE_NOT_SPACE);
                    pCur = pCur->pNext;
                }
            }
            return RK_ERROR;
        }


        gpstRecordControlData->RecordState = RECORD_STATE_BEING;
        if(gpstRecordControlData->RecordSource == 0)
        {
            if(RKTaskFind(TASK_ID_AUDIODEVICESERVICE, 0) == NULL)
            {
                rk_printf("create TASK_ID_AUDIODEVICESERVICE........\n");
                stAudioArg.Bit = gpstRecordControlData->RecordDataWidth;
                stAudioArg.RecordType = gpstRecordControlData->RecordType;
                stAudioArg.SampleRate = gpstRecordControlData->RecordSampleRate;
                stAudioArg.pfPcmCallBack = RecordPcmInput;
                stAudioArg.SamplesPerBlock = RecordSamples;
                RKTaskCreate(TASK_ID_AUDIODEVICESERVICE, 0, &stAudioArg, SYNC_MODE);
                AudioDev_ReadEnable(gpstRecordControlData->hAudio,1);
            }
        }
        {
            RECORD_CALLBACK * pCur;

            pCur = gpstRecordControlData->pfRecordState;

            while(pCur != NULL)
            {
                pCur->pfRecordState(RECORD_STATE_BEING);
                pCur = pCur->pNext;
            }
        }
    }
    return 0;
}

/*******************************************************************************
** Name: RecordPause
** Input:void
** Return: BOOLEAN
** Owner:chad.ma
** Date: 2015.12.24
** Time: 15:37:38
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN BOOLEAN RecordPause(void)
{
    uint32 timeout = 2000;
    printf("=== Record Pause in ===\n");

    if (gpstRecordControlData->RecordState == RECORD_STATE_BEING)
    {
        gpstRecordControlData->RecordState = RECORD_STATE_PAUSE;
        {
            RECORD_CALLBACK * pCur;

            pCur = gpstRecordControlData->pfRecordState;

            while(pCur != NULL)
            {
                pCur->pfRecordState(RECORD_STATE_PAUSE);
                pCur = pCur->pNext;
            }
        }
    }

    printf("=== Record Pause out ===\n");
    return TRUE;
}
/*******************************************************************************
** Name: RecordStop
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 15:35:07
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordStop(uint32 IsNextFile)
{
    UINT32  TempFileSize;
    UINT32  TempFileOffset;
    UINT32  TempSectorIndex;
    UINT32  TempFileLength;
    uint32 timeout = 2000;

    UINT32 i;
    FILE_ATTR stFileAttr;
    UINT16 pathBuff[128] = {0};

    printf("RecordStop in\n");


    if ((gpstRecordControlData->RecordState == RECORD_STATE_BEING)
        || (gpstRecordControlData->RecordState == RECORD_STATE_PAUSE)
        || (gpstRecordControlData->RecordState == RECORD_STATE_ERROR)) //if the current status had been recording status or suspend status.
    {
        gpstRecordControlData->RecordState = RECORD_STATE_STOP;
        rk_printf("Record state is ###RECORD_STATE_STOP###\n");

        RecordCodecDeInit();
        RecordHWDeInit();

        if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
        {
            uint8 *buf = rkos_memory_malloc(WAV_HEAD_BUFFER_SIZE);
            uint32 TempFileSize, TempFileLength;

            FileDev_FileSeek(gpstRecordControlData->hFileHdl, SEEK_SET, 0);
            FileDev_ReadFile(gpstRecordControlData->hFileHdl, buf, WAV_HEAD_BUFFER_SIZE);
            FileDev_GetFileSize(gpstRecordControlData->hFileHdl, &TempFileSize);
            TempFileSize    = TempFileSize - 8;
            TempFileLength  = (TempFileSize + 8) - WAV_HEAD_BUFFER_SIZE;
            if(TempFileSize > 0)
            {
                buf[4]   = (TempFileSize >> 0) &0xff;  //RIFF WAVE Chunk Size
                buf[5]   = (TempFileSize >> 8) &0xff;
                buf[6]   = (TempFileSize >> 16)&0xff;
                buf[7]   = (TempFileSize >> 24)&0xff;

                buf[4092]   = (TempFileLength >> 0) &0xff; // Data Chunk Size
                buf[4093]   = (TempFileLength >> 8) &0xff;
                buf[4094]   = (TempFileLength >> 16)&0xff;
                buf[4095]   = (TempFileLength >> 24)&0xff;
            }

            FileDev_FileSeek(gpstRecordControlData->hFileHdl, SEEK_SET, 0);

            FileDev_WriteFile(gpstRecordControlData->hFileHdl, buf, WAV_HEAD_BUFFER_SIZE);
            rkos_memory_free(buf);

        }
        FileDev_CloseFile(gpstRecordControlData->hFileHdl);
        gpstRecordControlData->hFileHdl = NULL;
        if(gpstRecordControlData->RecordSource == 0)
        {
            //printf("delete RecordStop\n");
            RKTaskDelete(TASK_ID_AUDIODEVICESERVICE, 0, SYNC_MODE);
        }

        if(IsNextFile)
        {
            RecordEncodeProc(RECORD_CMD_GETINFO, (void*)1);

            RecordCpuResume();
        }
        gpstRecordControlData->PcmDataUnusedSamples = 0;
        if(gpstRecordControlData->PcmInputBuf != NULL)
        {
            rkos_memory_free(gpstRecordControlData->PcmInputBuf);
            gpstRecordControlData->PcmInputBuf = NULL;
        }
    }
    else
    {
        //
    }
    printf("RecordStop OK\n");

}
/*******************************************************************************
** Name: RecordEncodeProc
** Input:MSG_ID id, void * msg
** Return: BOOLEAN
** Owner:chad.ma
** Date: 2015.12.24
** Time: 15:20:44
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN BOOLEAN RecordEncodeProc(RECORD_CMD id, void * msg)
{
    BOOLEAN ret = TRUE;

    switch (id)
    {

        case RECORD_CMD_GETINFO:
        {
            RecordGetInfo();
            break;
        }

        case RECORD_CMD_START:      //start record,write file head,timer interrupt start.
        {
            FREQ_EnterModule(FREQ_MAX);
            RecordStart();
            FREQ_ExitModule(FREQ_MAX);
            break;
        }

        case RECORD_CMD_PAUSE:      //record suspend,close timer interrupt,stop accept the sample value.
        {
            RecordPause();
            break;
        }

        case RECORD_CMD_RESUME:    //start timer interrupt.continue accept sample value.
        {
            RecordResume();
            break;
        }

        case RECORD_CMD_STOP:        //stop record.
        {
            RecordStop((uint32)msg);
            printf("RecordStop over\n");
            break;
        }

        case RECORD_CMD_GETPCM_PROCESS: //get pcm data and process data,transmit src pcm data to B core encode.
        {
            RecordGetPCMAndProcess();
            break;
        }

        default:
            ret = FALSE;
            break;
    }

    return ret;
}

/*******************************************************************************
** Name: RecordCodecDeInit
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 15:07:38
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordCodecDeInit(void)
{
#ifndef A_CORE_DECODE
        RecordWaitBBStop(); //B core 'close' msg cmd to write wav file tail.
#endif
}

/*******************************************************************************
** Name: RecordSetType
** Input:UINT32  Type
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 11:27:14
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordSetType(UINT32  Type)
{
    uint32 recSampRate;
    recSampRate = gpstRecordControlData->RecordSampleRate;

    switch (Type)
    {
        case RECORD_TYPE_MIC_STERO:
            Codec_SetMode(Codec_MicStero, recSampRate);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_LINEIN1:
            Codec_SetMode(Codec_Line1ADC, recSampRate);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_LINEIN2:
            Codec_SetMode(Codec_Line2ADC, recSampRate);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_MIC1_MONO:
            Codec_SetMode(Codec_Mic1Mono, recSampRate);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            break;

        case RECORD_TYPE_MIC2_MONO:
            Codec_SetMode(Codec_Mic2Mono, recSampRate);
            ACodec_Set_MIC_AnaVol(2);
            ACodec_Set_ADCMUX_Vol(5);
            ACodec_Set_ADC_DigVol(0);
            break;

        default:
            break;
    }
}
/*******************************************************************************
** Name: RecordExitType
** Input:UINT32 Type
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 11:26:33
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordExitType(UINT32 Type)
{
    switch (Type)
    {
        case RECORD_TYPE_MIC_STERO:
            Codec_ExitMode(Codec_MicStero);
            break;

        case RECORD_TYPE_LINEIN1:
            Codec_ExitMode(Codec_Line1ADC);
            break;

        case RECORD_TYPE_LINEIN2:
            Codec_ExitMode(Codec_Line2ADC);
            break;

        case RECORD_TYPE_MIC1_MONO:
            Codec_ExitMode(Codec_Mic1Mono);
            break;

        case RECORD_TYPE_MIC2_MONO:
            Codec_ExitMode(Codec_Mic2Mono);
            break;

        default:
            break;
    }
}

/*******************************************************************************
** Name: RecordCpuInit
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 11:16:11
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordCpuInit(void)
{
    if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
        FREQ_EnterModule(FREQ_RECORDXXX);
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
        FREQ_EnterModule(FREQ_RECORDAMR);
    }
    else
    {
        FREQ_EnterModule(FREQ_RECORDWAV);
    }
}

/*******************************************************************************
** Name: RecordCpuResume
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 11:16:36
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordCpuResume(void)
{
    if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
        FREQ_ExitModule(FREQ_RECORDXXX);
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
        FREQ_ExitModule(FREQ_RECORDAMR);
    }
    else
    {
        FREQ_ExitModule(FREQ_RECORDWAV);
    }
}

/*******************************************************************************
** Name: RecordCodecInit
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.24
** Time: 10:08:48
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordCodecInit(void)
{
    EncodeArg stEncArg;

    stEncArg.channel = gpstRecordControlData->RecordChannel;
    stEncArg.dataWidth = gpstRecordControlData->RecordDataWidth;
    stEncArg.encodeType = gpstRecordControlData->RecordEncodeType;
    stEncArg.encodeSubType = gpstRecordControlData->RecordEncodeSubType;
    stEncArg.sampleRate = gpstRecordControlData->RecordSampleRate;
    CodeOpenEnc((unsigned long)&stEncArg, (short *)&RecordOutputBuffer, &RecordSamples);

    CodecGetEncBitrate(&gpstRecordControlData->RecordBitrate);
}

/*******************************************************************************
** Name: RecordEncodingFileBuffer
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 19:59:32
*******************************************************************************/
//_APP_RECORD_RECORDCONTROLTASK_COMMON_
//__irq COMMON FUN void RecordEncodingFileBuffer(void)
//{
//    uint32 cmd;
//    uint32 data;
//
//    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
//    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
//    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
//
//    switch (cmd)
//    {
//        case MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL:
//            rkos_semaphore_give_fromisr(osRecordEncodeOk);
//            break;
//
//        default:
//            break;
//    }
//}
/*******************************************************************************
** Name: RegMBoxRecFileSvc
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 19:58:53
*******************************************************************************/
//_APP_RECORD_RECORDCONTROLTASK_COMMON_
//COMMON FUN void RegMBoxRecFileSvc(void)
//{
//    IntRegister(INT_ID_MAILBOX2, (void*)RecordEncodingFileBuffer);
//    IntPendingClear(INT_ID_MAILBOX2);
//    IntEnable(INT_ID_MAILBOX2);
//    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
//}

/*******************************************************************************
** Name: DeRegMBoxRecFileSvc
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 20:07:21
*******************************************************************************/
//_APP_RECORD_RECORDCONTROLTASK_COMMON_
//COMMON FUN void DeRegMBoxRecFileSvc(void)
//{
//    IntUnregister(INT_ID_MAILBOX2);
//    IntPendingClear(INT_ID_MAILBOX2);
//    IntDisable(INT_ID_MAILBOX2);
//    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_2);
//}

/*******************************************************************************
** Name: RecordWaitBBStart
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 19:40:05
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordWaitBBStart(void)
{
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_CREATE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData((uint32)&gRecordFileHdl, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    rk_printf("RecordWaitBBStart rkos_semaphore_take");
    rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
    rk_printf("RecordWaitBBStart over###");
}

/*******************************************************************************
** Name: RecordWaitBBStop
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 19:39:33
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordWaitBBStop(void)
{
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_REC_FILE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData(1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    //printf("[REC] RecordWaitBBStop rkos_semaphore_take\n");
    rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
    //printf("[REC] RecordWaitBBStop success\n");
}

/*******************************************************************************
** Name: RecordCreateFile
** Input:void
** Return: BOOLEAN
** Owner:chad.ma
** Date: 2015.12.24
** Time: 15:44:18
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN BOOLEAN RecordCreateFile(void)
{
    FILE_ATTR stFileAttr;
    UINT16 pathBuff[128] = {0};
    //UINT16 fileName[255];
    UINT32 i,ret;

    //debug_hex((char*)pathBuff,255,10);

    memset(&stFileAttr,0,sizeof(FILE_ATTR));

    stFileAttr.Path     = pathBuff;
    stFileAttr.FileName = RecordLongFileName;

    dumpMemoryShort2Char(stFileAttr.FileName, 24);

    //fileName[Ascii2Unicode(RecordFileInfo.shortName, fileName, strlen(RecordFileInfo.shortName)) / 2 ] = 0;

    for (i = 0; i < 127 ; i++)
    {
        stFileAttr.Path[i]      = RecordFileInfo.Path[i];
        if (RecordFileInfo.Path[i] == '\0')
            break;
    }

//    for (i = 0; i < 255 ; i++)
//    {
//        stFileAttr.FileName[i]     = RecordFileInfo.shortName[i];
//        if (RecordFileInfo.shortName[i] == '\0')
//            break;
//    }

    for (i = 0; i < 12 ; i++)
    {
        stFileAttr.ShortName[i] = RecordFileInfo.shortName[i];

        if (RecordFileInfo.shortName[i] == '\0')
            break;
    }

    rk_printf("[RECORD]create file path is :%s",RecordFileInfo.Path);
    rk_printf("[RECORD]create file name is :%s",stFileAttr.ShortName);


//    {
//            stFileAttr.Path = L"C:\\RECORD\\";
//            stFileAttr.FileName = L"REC0001.WAV";
//    }

    ret = FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr);

    if (ret != RK_SUCCESS)
    {
        DEBUG("===## RecordFile Create Fail ##===");
        return FALSE;
    }

    DEBUG("===## RecordFile Create Success ##===");

    gpstRecordControlData->hFileHdl = FileDev_OpenFile(FileSysHDC, NULL, NOT_CARE,&stFileAttr);
    if((rk_err_t)gpstRecordControlData->hFileHdl <= 0)
    {
        rk_printf("[RECORD]Record File Open Failure...\n");
        return FALSE;
    }

    StreamControlTask_SendFileHandle(gpstRecordControlData->hFileHdl, 4);

    return TRUE;
}

/*******************************************************************************
** Name: RecordEncodeGetOutBuffer
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 18:20:52
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
__irq COMMON FUN void RecordEncodeGetOutBuffer(void)
{
    uint32 cmd;
    uint32 data;

    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_ENCODE_OPEN_CMPL:
        {
            memcpy(&gRecordBlock, (RecordBlock *)data, sizeof(RecordBlock) - 4);
            gRecordBlock.encodeOver = 1;
            rkos_semaphore_give_fromisr(osRecordEncodeOk);
            break;
        }

        case MEDIA_MSGBOX_CMD_ENCODE_CMPL:
        {
            memcpy(&gRecordBlock, (RecordBlock *)data, sizeof(RecordBlock) - 4);
            gRecordBlock.encodeOver = 1;

            if(gRecordBlock.needencode == 1)
            {
                gRecordBlock.needencode = 0;
                rkos_semaphore_give_fromisr(osRecordEncodeOk);
            }
            break;
        }

        case MEDIA_MSGBOX_CMD_BUF_CHANGE:
        {
           memcpy(&gRecordBlock, (RecordBlock *)data, sizeof(RecordBlock) - 4);
           rkos_semaphore_give_fromisr(osRecordEncodeOk);
           break;
        }

        case MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL:
        {
            rkos_semaphore_give_fromisr(osRecordEncodeOk);
            break;
        }

        case MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_CMPL:
        {
            rkos_semaphore_give_fromisr(osRecordEncodeOk);
            break;
        }

        default:
            break;
    }
}

/*******************************************************************************
** Name: RegMBoxRecEncodeSvc
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 18:17:22
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RegMBoxRecEncodeSvc(void)
{
    IntRegister(INT_ID_MAILBOX1, (void*)RecordEncodeGetOutBuffer);
    IntPendingClear(INT_ID_MAILBOX1);
    IntEnable(INT_ID_MAILBOX1);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
}

/*******************************************************************************
** Name: DeRegMBoxRecEncodeSvc
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 18:18:10
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void DeRegMBoxRecEncodeSvc(void)
{
    IntUnregister(INT_ID_MAILBOX1);
    IntPendingClear(INT_ID_MAILBOX1);
    IntDisable(INT_ID_MAILBOX1);
    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
}

/*******************************************************************************
** Name: RecordHWInit
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 16:52:07
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordHWInit(void)
{
#ifdef A_CORE_DECODE
    if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
        FW_LoadSegment(SEGMENT_ID_XXX_ENCODE, SEGMENT_OVERLAY_ALL);
        CurrentEncCodec = CODEC_XXX_ENC;
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
    {
        FW_LoadSegment(SEGMENT_ID_WAV_ENCODE, SEGMENT_OVERLAY_ALL);
        CurrentEncCodec = CODEC_WAV_ENC;
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
        //FW_LoadSegment(SEGMENT_ID_AMR_DECODE, SEGMENT_OVERLAY_ALL);
        FW_LoadSegment(SEGMENT_ID_AMR_ENCODE, SEGMENT_OVERLAY_ALL);
        CurrentEncCodec = CODEC_AMR_ENC;
    }
#else

#ifdef BB_SYS_JTAG
    BcoreDev_Start(gpstRecordControlData->hBcore,SEGMENT_ID_BB_CODE);
#else

    if (gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
        rk_printf("RecordHWInit:load b core  XXX");
#ifdef XXX_ENC_INCLUDE
        FW_LoadSegment(SEGMENT_ID_XXX_ENCODE, SEGMENT_OVERLAY_ALL);
        BcoreDev_Start(gpstRecordControlData->hBcore,SEGMENT_ID_XXX_ENCODE_BIN);
        CurrentEncCodec = CODEC_XXX_ENC;
#endif
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
    {
#ifdef WAV_ENC_INCLUDE
        rk_printf("RecordHWInit:load b core wav");
        FW_LoadSegment(SEGMENT_ID_WAV_ENCODE, SEGMENT_OVERLAY_ALL);
        BcoreDev_Start(gpstRecordControlData->hBcore,SEGMENT_ID_WAV_ENCODE_BIN);
        CurrentEncCodec = CODEC_WAV_ENC;
#endif
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
#ifdef AMR_ENC_INCLUDE
        rk_printf("RecordHWInit:load b core AMR");
        FW_LoadSegment(SEGMENT_ID_AMR_ENCODE, SEGMENT_OVERLAY_ALL);
        BcoreDev_Start(gpstRecordControlData->hBcore,SEGMENT_ID_AMR_ENCODE_BIN);
        CurrentEncCodec = CODEC_AMR_ENC;
#endif
    }
#endif

#endif

    rk_printf("[RECORD] RecordHWInit OK");
}

/*******************************************************************************
** Name: RecordHWDeInit
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 19:38:07
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordHWDeInit(void)
{
    #ifdef A_CORE_DECODE
     if ( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef XXX_ENC_INCLUDE
        FW_RemoveSegment(SEGMENT_ID_XXX_ENCODE);
#endif
#endif
    }
    else if(gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef AMR_ENC_INCLUDE
        FW_RemoveSegment(SEGMENT_ID_AMR_ENCODE);
#endif
#endif
    }
    else
    {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef WAV_ENC_INCLUDE
        FW_RemoveSegment(SEGMENT_ID_WAV_ENCODE);
#endif
#endif
    }

    #else
    BcoreDev_ShutOff(gpstRecordControlData->hBcore);

    if ( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef XXX_ENC_INCLUDE
        FW_RemoveSegment(SEGMENT_ID_XXX_ENCODE_BIN);
        FW_RemoveSegment(SEGMENT_ID_XXX_ENCODE);
#endif
#endif
    }
    else if ( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef AMR_ENC_INCLUDE
        FW_RemoveSegment(SEGMENT_ID_AMR_ENCODE_BIN);
        FW_RemoveSegment(SEGMENT_ID_AMR_ENCODE);
#endif
#endif
    }
    else
    {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef WAV_ENC_INCLUDE
        FW_RemoveSegment(SEGMENT_ID_WAV_ENCODE_BIN);
        FW_RemoveSegment(SEGMENT_ID_WAV_ENCODE);
#endif
#endif
    }
    #endif

    printf("[RECORD] RecordHWDeInit over\n");
}

/*******************************************************************************
** Name: RecordServiceGetTotalTime
** Input:void
** Return: BOOL
** Owner:chad.ma
** Date: 2015.12.23
** Time: 15:45:05
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN BOOL RecordServiceGetTotalTime(void)
{

    VOLUME_INF stVolumeInf;

    FileDev_GetVolumeInfByLabel(FileSysHDC, 'C', &stVolumeInf);

    RecordFreeMemory = stVolumeInf.VolumeFreeSize;

    rk_printf("RecordFreeMemory = %d, bit = %d", RecordFreeMemory, gpstRecordControlData->RecordBitrate);

    if ((MIN_RECORD_RESERVE_MEMORY / 1024) > RecordFreeMemory)
    {
        return RK_ERROR;
    }

    RecordTotalTime = (((RecordFreeMemory - (MIN_RECORD_RESERVE_MEMORY /1024)) * 8)
                / (((UINT32) gpstRecordControlData->RecordBitrate) / 1000)) * 1000;


    rk_printf(" ### TOTAL TIME = %d ### ", RecordTotalTime);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: RecordGetInfo
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2015.12.23
** Time: 15:42:12
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN void RecordGetInfo(void)
{
    BOOL bStatus2;
    unsigned short usBytesPerBlock;
    unsigned short usSamplesPerBlock;
    unsigned short uBitsPerSample;

    gpstRecordControlData->RecordQuality = gSysConfig.RecordConfig.RecordQuality; //get record volume from menu or system backup.

    //DEBUG("RecordBitrate = %d",RecordBitrate);

    FREQ_EnterModule(FREQ_MAX);
    bStatus2 = RecordGetFileName();
    FREQ_ExitModule(FREQ_MAX);

    if (bStatus2 == TRUE)
    {
//        SendMsg(MSG_RECORDWIN_DISPFLAG_RECQUALITY);  //notice the record quality,time and file name.
//        SendMsg(MSG_RECORD_STATUS_PREPARE);
//        SendMsg(MSG_RECORDWIN_DISPFLAG_TIME);
//        SendMsg(MSG_RECORDWIN_DISPFLAG_FILENAME);
//        SendMsg(MSG_RECORDWIN_DISPFLAG_TOTALTIME);
    }
    else
    {
        printf("GetFileName and total time Fail!\n");
    }

    gpstRecordControlData->RecordState = RECORD_STATE_PREPARE;
    RecordCurrTime = 0;
    RecordCurrTimeSave = 0;
    gpstRecordControlData->RecordBitrate = 0;
    RecordTotalTime = 0;

    {
        RECORD_CALLBACK * pCur;

        pCur = gpstRecordControlData->pfRecordState;

        while(pCur != NULL)
        {
            pCur->pfRecordState(RECORD_STATE_PREPARE);
            pCur = pCur->pNext;
        }
    }

}

_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN static uint32 FileIndexStr2Num(uint8* fileIndexStr)
{
    uint32 fileNum = 0;

    fileNum = (fileIndexStr[0] - '0');
    fileNum = fileNum * 10;
    fileNum += (fileIndexStr[1] - '0');
    fileNum = fileNum * 10;
    fileNum += (fileIndexStr[2] - '0');
    fileNum = fileNum * 10;
    fileNum += (fileIndexStr[3] - '0');

    return fileNum;
}

_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN static void FileNum2Str(uint32 fileNum,uint8* fileNumStr)
{
    fileNumStr[3] = '0' + (fileNum % 10 );
    fileNumStr[2] = '0' + ((fileNum / 10 ) % 10);
    fileNumStr[1] = '0' + ((fileNum / 100 ) % 10);
    fileNumStr[0] = '0' + ((fileNum / 1000) % 10);
}

/*******************************************************************************
** Name: RecordGetFileName
** Input:void
** Return: BOOL
** Owner:chad.ma
** Date: 2015.12.23
** Time: 14:36:09
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_COMMON_
COMMON FUN BOOL RecordGetFileName(void)
{
    UINT32 i;
    UINT32 MaxFileNum;
    UINT8 temp;
    uint8 Buf[1024];
    UINT16 CurIndex;
    HDC hDir;
    rk_err_t ret;

    FILE_ATTR stFileAttr;
    uint8* pShortName;

    stFileAttr.Path = L"C:\\RECORD\\";
    stFileAttr.FileName = NULL;

    pShortName = (uint8*)stFileAttr.ShortName;
    memset(&RecordFileInfo,0,sizeof(RecordFileInfo));

    hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &stFileAttr);
    if ((rk_err_t)hDir <= 0)
    {
        stFileAttr.Path = L"C:\\";
        stFileAttr.FileName = L"RECORD";
        ret = FileDev_CreateDir(FileSysHDC, NULL,&stFileAttr);
        if (ret != RK_SUCCESS )
        {
            rk_print_string("#### dir create failure #### \n");
            return RK_ERROR;
        }

        stFileAttr.Path = L"C:\\RECORD\\";
        stFileAttr.FileName = NULL;
        hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &stFileAttr);
        if ((rk_err_t)hDir <= 0)
        {
            rk_print_string("dir open failure\n");
            return RK_ERROR;
        }
    }

    for (i = 0; i < 99 ; i++)
    {
        RecordFileInfo.Path[i]     = RecordPathStr[i];

        if (RecordPathStr[i] == '\0')
            break;
    }

    for (i = 0; i < 12; i ++)
    {
        if ( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
        {
            RecordFileInfo.shortName[i] = RecordFileNameMp3Str[i];
        }
        else if( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
        {
            RecordFileInfo.shortName[i] = RecordFileNameWavStr[i];
        }
        else if( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
        {
            RecordFileInfo.shortName[i] = RecordFileNameAmrStr[i];
        }
        else
        {
            RecordFileInfo.shortName[i] = RecordFileNamePcmStr[i];
        }
    }

    //RecordFileInfo.shortName[7] =  ' ';

    i           = 0;
    CurIndex    = 0;
    MaxFileNum  = 0;
    //memset(RecordNameFlag, 0, 1250); // 9999/8

    while (1)
    {
        if (FileDev_NextFile(hDir, 0, NULL, &stFileAttr) != RK_SUCCESS)
        {
            break;
        }

        if ( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
        {
            if ((pShortName[0] != 'R') || (pShortName[1] != 'E') || (pShortName[2] != 'C')
                || (pShortName[8] != 'W') || (pShortName[9] != 'A') || (pShortName[10] != 'V'))
            {
                continue;
            }
        }
        else if( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
        {
            if ((pShortName[0] != 'R') || (pShortName[1] != 'E') || (pShortName[2] != 'C')
                || (pShortName[8] != 'A') || (pShortName[9] != 'M') || (pShortName[10] != 'R'))
            {
                continue;
            }
        }
        else if( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
        {
            if ((pShortName[0] != 'R') || (pShortName[1] != 'E') || (pShortName[2] != 'C')
                || (pShortName[8] != 'M') || (pShortName[9] != 'P') || (pShortName[10] != '3'))
            {
                continue;
            }
        }
        else
        {
            if ((pShortName[0] != 'R') || (pShortName[1] != 'E') || (pShortName[2] != 'C')
                || (pShortName[8] != 'P') || (pShortName[9] != 'C') || (pShortName[10] != 'M'))
            {
                continue;
            }
        }

        if ((pShortName[3] > '9') || (pShortName[3] < '0'))
        {
            continue;
        }

        if ((pShortName[4] > '9') || (pShortName[4] < '0'))
        {
            continue;
        }

        if ((pShortName[5] > '9') || (pShortName[5] < '0'))
        {
            continue;
        }

        if ((pShortName[6] > '9') || (pShortName[6] < '0'))
        {
            continue;
        }

        memcpy(Buf, pShortName, 8);

        CurIndex = FileIndexStr2Num(&pShortName[3]);

        if(CurIndex > MaxFileNum)
            MaxFileNum = CurIndex;
    }

    if ((gSysConfig.RecordConfig.RecordIndex > 9999) || (gSysConfig.RecordConfig.RecordIndex == 0))
    {
        gSysConfig.RecordConfig.RecordIndex = 1;
        CurIndex = gSysConfig.RecordConfig.RecordIndex;
    }

    gSysConfig.RecordConfig.RecordIndex = CurIndex + 1;

    FileNum2Str(CurIndex+1,&RecordFileInfo.shortName[3]);

    for (i = 0; i < 8; i ++)
    {
        RecordLongFileName[i] = RecordFileInfo.shortName[i];
    }

    //add format str
    if ( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_XXX)
    {
        RecordLongFileName[7] =   '.';
        RecordLongFileName[8] =   'M';
        RecordLongFileName[9]  =  'P';
        RecordLongFileName[10] =  '3';
        RecordLongFileName[11] =  '\0';
    }
    else if( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_WAV)
    {
        RecordLongFileName[7] =   '.';
        RecordLongFileName[8] =   'W';
        RecordLongFileName[9] =   'A';
        RecordLongFileName[10] =  'V';
        RecordLongFileName[11] =  '\0';
    }
    else if( gpstRecordControlData->RecordEncodeType == RECORD_ENCODE_TYPE_AMR)
    {
        RecordLongFileName[7] =   '.';
        RecordLongFileName[8] =   'A';
        RecordLongFileName[9] =   'M';
        RecordLongFileName[10] =  'R';
        RecordLongFileName[11] =  '\0';
    }
    else
    {
        RecordLongFileName[7] =   '.';
        RecordLongFileName[8] =   'P';
        RecordLongFileName[9] =   'C';
        RecordLongFileName[10] =  'M';
        RecordLongFileName[11] =  '\0';
    }

    FileDev_CloseDir(hDir);

    //dump reocrd file name.
    dumpMemoryShort2Char(RecordLongFileName,64);

    debug_hex((uint8*)RecordLongFileName,64,16);

    return TRUE;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RecordControlTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:chad.ma
** Date: 2015.12.22
** Time: 16:56:56
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_INIT_
INIT API rk_err_t RecordControlTask_DeInit(void *pvParameters)
{

    if(gpstRecordControlData->RecordState != RECORD_STATE_STOP)
    {
        RecordStop(0);
    }

    DeRegMBoxRecEncodeSvc();

    rkos_queue_delete(gpstRecordControlData->RecordControlAskQueue);
    rkos_queue_delete(gpstRecordControlData->RecordControlRespQueue);

    if(hRecordDma != NULL)
    {
        RKDev_Close(hRecordDma);
    }

    if(osRecordEncodeOk != NULL)
    {
       rkos_semaphore_delete(osRecordEncodeOk);
    }
    gpstRecordControlData->PcmDataUnusedSamples = 0;
    if(gpstRecordControlData->PcmInputBuf != NULL)
    {
        rkos_memory_free(gpstRecordControlData->PcmInputBuf);
        gpstRecordControlData->PcmInputBuf = NULL;
    }

    rkos_memory_free(gpstRecordControlData);
    gpstRecordControlData = NULL;

    RKTaskDelete(TASK_ID_STREAMCONTROL, 0, ASYNC_MODE);
    rk_printf("record de init ok");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: RecordControlTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:chad.ma
** Date: 2015.12.22
** Time: 16:56:56
*******************************************************************************/
_APP_RECORD_RECORDCONTROLTASK_INIT_
INIT API rk_err_t RecordControlTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS*   pRecordControlTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_RECORDCONTROL_ARG * pArg = (RK_TASK_RECORDCONTROL_ARG *)arg;
    RECORDCONTROL_TASK_DATA_BLOCK*  pRecordControlTaskData;
    if (pRecordControlTask == NULL)
    {
        rk_printf("record task para error\n");
        return RK_PARA_ERR;
    }

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_RECORDCONTROL_TASK, SEGMENT_OVERLAY_CODE);
    #endif

    pRecordControlTaskData = rkos_memory_malloc(sizeof(RECORDCONTROL_TASK_DATA_BLOCK));
    if(pRecordControlTaskData == NULL)
    {
        rk_printf("pAudioControlTaskData == null\n");
    }
    memset(pRecordControlTaskData, NULL, sizeof(RECORDCONTROL_TASK_DATA_BLOCK));

    pRecordControlTaskData->RecordControlAskQueue   = rkos_queue_create(1, sizeof(RECORDCONTROL_ASK_QUEUE));
    pRecordControlTaskData->RecordControlRespQueue  = rkos_queue_create(1, sizeof(RECORDCONTROL_RESP_QUEUE));
    pRecordControlTaskData->TaskObjectID            = pRecordControlTask->TaskObjectID;

    if(pArg->pfRecordState != NULL)
    {
        pRecordControlTaskData->pfRecordState = rkos_memory_malloc(sizeof(RECORD_CALLBACK));
        pRecordControlTaskData->pfRecordState->pNext = NULL;
        pRecordControlTaskData->pfRecordState->pfRecordState = pArg->pfRecordState;
    }
    else
    {
        pRecordControlTaskData->pfRecordState = NULL;
    }

    pRecordControlTaskData->hDirDev = RKDev_Open(DEV_CLASS_DIR, 0 ,NOT_CARE);
    if ((pRecordControlTaskData->hDirDev == NULL)
        || (pRecordControlTaskData->hDirDev == (HDC)RK_ERROR)
        || (pRecordControlTaskData->hDirDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\n audio dir device open failure");
        return RK_ERROR;
    }

    pRecordControlTaskData->hBcore = RKDev_Open(DEV_CLASS_BCORE, 0, NOT_CARE);
    if ((pRecordControlTaskData->hBcore == NULL)
        || (pRecordControlTaskData->hBcore == (HDC)RK_ERROR)
        || (pRecordControlTaskData->hBcore == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\n Bcore device open failure");
        return RK_ERROR;
    }


    pRecordControlTaskData->hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);

    if ((pRecordControlTaskData->hAudio == NULL)
        || (pRecordControlTaskData->hAudio == (HDC)RK_ERROR)
        || (pRecordControlTaskData->hAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("Audio device open failure");
        return RK_ERROR;
    }


    hRecordDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    if (hRecordDma == NULL)
    {
        rk_print_string("\n DMA RKDev_Open failure");
        return RK_ERROR;
    }

    pRecordControlTaskData->RecordType              = pArg->RecordType;
    pRecordControlTaskData->RecordEncodeSubType       = pArg->RecordEncodeSubType;
    pRecordControlTaskData->RecordSampleRate        = pArg->RecordSampleRate;
    pRecordControlTaskData->RecordChannel           = pArg->RecordChannel;
    pRecordControlTaskData->RecordDataWidth         = pArg->RecordDataWidth;
    pRecordControlTaskData->RecordEncodeType        = pArg->RecordEncodeType;
    pRecordControlTaskData->RecordSource            = pArg->RecordSource;
    gpstRecordControlData->RecordState = RECORD_STATE_STOP;

    gpstRecordControlData   = pRecordControlTaskData;

    RecordEncodeProc(RECORD_CMD_GETINFO, (void*)1);

#ifndef A_CORE_DECODE
    osRecordEncodeOk = rkos_semaphore_create(1, 0);
    RegMBoxRecEncodeSvc();
#endif

    rk_printf("RecordControlTask_Init  out...\n");

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
