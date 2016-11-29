/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Line_In\LineInControlTask.c
* Owner: cjh
* Date: 2016.5.11
* Time: 9:01:57
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.11     9:01:57   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_LINE_IN_LINEINCONTROLTASK_C__

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
#include "record_globals.h"
#include "Spectrum.h"
#include "LineInControlTask.h"
#include "Fade.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define RadioOutputVol  (gSysConfig.OutputVolume)

#ifdef _RK_SPECTRUM_
#define SPECTRUM_LINE_M      12     // 10跟频谱线
//#define SpectrumLoopTime     3
#endif

typedef  struct _LINEINCONTROL_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}LINEINCONTROL_RESP_QUEUE;

typedef  struct _LINEINCONTROL_ASK_QUEUE
{
    uint32 cmd;
    void * msg;
    uint32 mode;
}LINEINCONTROL_ASK_QUEUE;

typedef  struct _LINEINCONTROL_TASK_DATA_BLOCK
{
    pQueue  LineInControlAskQueue;
    pQueue  LineInControlRespQueue;
    pSemaphore LineInControlReqSem;
    HDC     hAudio;
    LINEIN_APP_CALLBACK * pLineInStateForAPP;
    P_REC_RECIVE_MSG pfPcmCallBack;

    UINT32    PInputPcmBuf;
    uint8 *   POutPcmBuf[2];
    UINT32    OutPcmLen;
    uint32    OutPcmIndex;
    uint32    LRChannel;
    uint32    SamplesPerBlock;
    uint32    LineInInputType;
    uint32    Bit;
    uint32    SampleRate;
    uint32    EqMode;
    uint32    playVolume;
    uint32    LineInPlayerState; //Play/pause/record
    uint32    LineInRecordState; //Play/pause
#ifdef _RK_SPECTRUM_
    uint8  SpectrumOut[SPECTRUM_LINE_M];//存放转换过后的M根谱线的能量
    uint32 SpectrumEn;
    uint32 CurrentTimeMsBk;
#endif

}LINEINCONTROL_TASK_DATA_BLOCK;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static LINEINCONTROL_TASK_DATA_BLOCK * gpstLineInControlData;



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
rk_err_t LineIn_AudioSetVolume(UINT32 Volume);
rk_err_t LineInControlTask_StartAudioServer(void);
rk_err_t LineIn_Process(UINT32 id, void *msg);
void LineInControlGetPCMAndProcess(uint8 *buf, uint32 SamplesPerBlock);
void LineInControlPcmInput(uint8 * buf, uint32 SamplesPerBlock);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LineInControlTask_SetStateChangeFunc
** Input:P_LINEIN_APP_CALLBACK old, P_LINEIN_APP_CALLBACK new
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 16:29:30
*******************************************************************************/
_APP_LINE_IN_LINEINTASK_COMMON_
COMMON API rk_err_t LineInControlTask_SetStateChangeFunc(P_LINEIN_APP_CALLBACK old, P_LINEIN_APP_CALLBACK new)
{
    LINEIN_APP_CALLBACK * pCur, * pLast;

    pCur = gpstLineInControlData->pLineInStateForAPP;
    pLast = NULL;

    if(old != NULL)
    {
        while(pCur != NULL)
        {
            if(pCur->pLineInStateForAPP== old)
            {
                if(new != NULL)
                {
                    pCur->pLineInStateForAPP = new;
                    return RK_SUCCESS;
                }
                else
                {
                    if(pLast == NULL)
                    {
                        gpstLineInControlData->pLineInStateForAPP = pCur->pNext;
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
        pCur = rkos_memory_malloc(sizeof(FM_APP_CALLBACK));
        if((int32)pCur <= 0)
        {
            return RK_ERROR;
        }

        pCur->pNext = gpstLineInControlData->pLineInStateForAPP;
        pCur->pLineInStateForAPP = new;
        gpstLineInControlData->pLineInStateForAPP = pCur;
        return RK_SUCCESS;
    }

    return RK_ERROR;
}


/*******************************************************************************
** Name: LineInControlTask_SendCmd
** Input:uint32 Cmd, void * msg, uint32 Mode
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 11:48:40
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API rk_err_t LineInControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode)
{
    LINEINCONTROL_ASK_QUEUE LineInControlAskQueue;
    LINEINCONTROL_RESP_QUEUE LineInControlRespQueue;
    rk_err_t ret = RK_ERROR;

    rkos_semaphore_take(gpstLineInControlData->LineInControlReqSem, MAX_DELAY);
    if(Cmd == LINEIN_CMD_VOLUMESET)
    {
        LineIn_Process(LINEIN_CMD_VOLUMESET, msg);
        ret = RK_SUCCESS;
    }
    else
    {
        if(Cmd == LINEIN_CMD_DEINIT)
        {
            gpstLineInControlData->LineInPlayerState = LINEIN_PLAYER_STATE_STOP;
        }
        LineInControlAskQueue.cmd = Cmd;
        LineInControlAskQueue.msg = msg;
        LineInControlAskQueue.mode = Mode;
        //printf("LineInControlAskQueue cmd = %d\n",Cmd);
        ret = rkos_queue_send(gpstLineInControlData->LineInControlAskQueue, &LineInControlAskQueue, MAX_DELAY);
        if(ret != RK_SUCCESS)
        {
            printf("LineInControlAskQueue send fail...\n");
            ret = RK_ERROR;
        }
        else
        {
            if(Mode == SYNC_MODE)
            {
                rkos_queue_receive(gpstLineInControlData->LineInControlRespQueue, &LineInControlRespQueue, MAX_DELAY);
                if ((LineInControlRespQueue.cmd == Cmd) && (LineInControlRespQueue.status == RK_SUCCESS))
                {
                    ret = RK_SUCCESS;
                }
                else
                {
                    ret = RK_ERROR;
                }
            }
            else
            {
                ret = RK_SUCCESS;
            }
        }
    }
    rkos_semaphore_give(gpstLineInControlData->LineInControlReqSem);

    return ret;
}
/*******************************************************************************
** Name: LineInControlTask_SetSpectrumEn
** Input:uint32 status
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 11:47:33
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API rk_err_t LineInControlTask_SetSpectrumEn(uint32 status)
{
#ifdef _RK_SPECTRUM_
    gpstLineInControlData->SpectrumEn = status;
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineInControlTask_GetCurSpectrum
** Input:uint8** pSpectrum
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 11:42:05
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API rk_err_t LineInControlTask_GetCurSpectrum(uint8** pSpectrum)
{
#ifdef _RK_SPECTRUM_
    *pSpectrum = gpstLineInControlData->SpectrumOut;
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineInControlTask_GetAudioInfo
** Input:LINEIN_AUDIO_INFO * AudioInfo
** Return: uint32
** Owner:cjh
** Date: 2016.5.11
** Time: 11:40:07
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API uint32 LineInControlTask_GetAudioInfo(LINEIN_AUDIO_INFO * AudioInfo)
{
    if(AudioInfo != NULL)
    {
        AudioInfo->InputType = gpstLineInControlData->LineInInputType;
        AudioInfo->bitpersample = gpstLineInControlData->Bit;
        AudioInfo->Samplerate = gpstLineInControlData->SampleRate;
        AudioInfo->channels = gpstLineInControlData->LRChannel;
        AudioInfo->PlayVolume = gpstLineInControlData->playVolume;
        AudioInfo->EqMode = gpstLineInControlData->EqMode;
    }
    return gpstLineInControlData->LineInPlayerState;
}

/*******************************************************************************
** Name: LineInControlTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:30:01
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API rk_err_t LineInControlTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: LineInControlTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:30:01
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API rk_err_t LineInControlTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: LineInControlTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 9:30:01
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON API void LineInControlTask_Enter(void * arg)
{
    LINEINCONTROL_ASK_QUEUE LineInControlAskQue;
    LINEINCONTROL_RESP_QUEUE LineInControlResQue;

    gpstLineInControlData->hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);

    if((gpstLineInControlData->hAudio == NULL)
    || (gpstLineInControlData->hAudio == (HDC)RK_ERROR)
    || (gpstLineInControlData->hAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("hAudio device open failure");
        RKDev_Close(gpstLineInControlData->hAudio);
        while(1);
    }

     //memset(gpstLineInControlData->POutPcmBuf[1-gpstLineInControlData->OutPcmIndex], 0, sizeof(gpstLineInControlData->POutPcmBuf[1-gpstLineInControlData->OutPcmIndex]));
    LineInControlTask_StartAudioServer();
    AudioDev_FadeInit(gpstLineInControlData->hAudio, gpstLineInControlData->SampleRate / 2,FADE_IN);
    memset(gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex], 0, gpstLineInControlData->OutPcmLen);
    AudioDev_Write(gpstLineInControlData->hAudio, 0, gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex]);
    AudioDev_ReadEnable(gpstLineInControlData->hAudio,1);
    gpstLineInControlData->LineInPlayerState = LINEIN_PLAYER_STATE_PLAY;
    while(1)
    {
        if(rkos_queue_receive(gpstLineInControlData->LineInControlAskQueue, &LineInControlAskQue, MAX_DELAY) == RK_SUCCESS)
        {
            //printf("\n p..p ");
            LineInControlResQue.status = LineIn_Process(LineInControlAskQue.cmd, LineInControlAskQue.msg);
            LineInControlResQue.cmd = LineInControlAskQue.cmd;
            if(LineInControlAskQue.mode == SYNC_MODE)
            {
                //printf(" s..%d ",LineInControlResQue.cmd);
                rkos_queue_send(gpstLineInControlData->LineInControlRespQueue, &LineInControlResQue, MAX_DELAY);
                if(LineInControlAskQue.cmd != LINEIN_CMD_PLAY)
                {
                    continue;
                }
            }

            if(gpstLineInControlData->LineInRecordState == LINEIN_CMD_RECORD)
            {
                if(gpstLineInControlData->pfPcmCallBack != NULL)
                {
                    gpstLineInControlData->pfPcmCallBack((uint8 * )gpstLineInControlData->PInputPcmBuf, gpstLineInControlData->SamplesPerBlock);
                }
            }

            if(gpstLineInControlData->LineInPlayerState == LINEIN_PLAYER_STATE_PLAY)
            {
                #ifdef _RK_SPECTRUM_
                //printf("\n.._RK_SPECTRUM_...AudioPlayState =%d SpectrumCnt =%d ..SpectrumEn==%d\n",gpstAudioControlData->AudioPlayState,gpstAudioControlData->SpectrumCnt,gpstAudioControlData->SpectrumEn);
                if (gpstLineInControlData->SpectrumEn)
                {
                    short Spectrum_data[128];
                    uint32 currentTimeMs;
                    uint32 currentTime200Ms;
                    currentTimeMs = SysTickCounter * 10;
                    currentTime200Ms = currentTimeMs / 100;    //refresh spectrum every 100ms
                    if (currentTime200Ms != gpstLineInControlData->CurrentTimeMsBk)
                    {
                        memset(gpstLineInControlData->SpectrumOut, 0, SPECTRUM_LINE_M*sizeof(char));
                        gpstLineInControlData->CurrentTimeMsBk = currentTime200Ms;

                        {
                            if(gpstLineInControlData->Bit == 32)
                            {
                               int16 i = 0;
                               char *ptr = (char *)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex];
                               for(i=1;i<129;i++)
                               {
                                  Spectrum_data[i-1] = (short)(ptr[4*i-1]<<8 | ptr[4*i-2]);
                               }
                            }
                            else if(gpstLineInControlData->Bit == 24)
                            {
                               int16 i = 0;
                               char *ptr = (char *)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex];
                               for(i=1;i<129;i++)
                               {
                                  Spectrum_data[i-1] = (short)(ptr[3*i-1]<<8 | ptr[3*i-2]);
                               }
                            }
                            else
                            {
                                memcpy(Spectrum_data, (short*)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex], 256);
                            }
                            DoSpectrum((short *)Spectrum_data,&gpstLineInControlData->SpectrumOut[0]);

                            if(2 > gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-3])
                            {
                                gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-2] = gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-3] ;
                                gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-1] = gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-3] ;
                            }
                            else
                            {
                                gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-2] = gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-3] - 1;
                                gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-1] = gpstLineInControlData->SpectrumOut[SPECTRUM_LINE_M-3] - 2;
                            }
                            //printf("SPECTRUM.... callback\n");
                            LINEIN_APP_CALLBACK * pCur;
                            pCur = gpstLineInControlData->pLineInStateForAPP;
                            while(pCur != NULL)
                            {
                                pCur->pLineInStateForAPP(LINEIN_PLAYER_STATE_UPDATA_SPECTRUM);
                                pCur = pCur->pNext;
                            }
                        }
                    }
                }
                #endif
                AudioDev_Write(gpstLineInControlData->hAudio, 0, gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex]);
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
** Name: LineIn_AudioSetVolume
** Input:UINT32 Volume
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 17:52:05
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON FUN rk_err_t LineIn_AudioSetVolume(UINT32 Volume)
{
    return AudioDev_SetVol(gpstLineInControlData->hAudio, Volume);
}
/*******************************************************************************
** Name: LineInControlTask_StartAudioServer
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 17:04:35
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON FUN rk_err_t LineInControlTask_StartAudioServer(void)
{
    AUDIO_DEV_ARG stAudioArg;
    uint32 bufLen;
    uint32 i;
    gpstLineInControlData->playVolume = gSysConfig.OutputVolume;

    stAudioArg.Bit = gpstLineInControlData->Bit;
    stAudioArg.SampleRate = gpstLineInControlData->SampleRate;
    gpstLineInControlData->OutPcmLen = ((gpstLineInControlData->SamplesPerBlock * gpstLineInControlData->LRChannel * gpstLineInControlData->Bit) / 32) * 4;
    if(stAudioArg.Bit == 16)
    {
        bufLen = gpstLineInControlData->OutPcmLen*2;
    }
    else if(stAudioArg.Bit == 24)
    {
        bufLen = (gpstLineInControlData->OutPcmLen + 3)*4/3;
    }

    AudioDev_SetBit(gpstLineInControlData->hAudio, 0, gpstLineInControlData->Bit);
    AudioDev_SetChannel(gpstLineInControlData->hAudio, 0, gpstLineInControlData->LRChannel);
    AudioDev_GetMainTrack(gpstLineInControlData->hAudio);
    AudioDev_SetVol(gpstLineInControlData->hAudio, gpstLineInControlData->playVolume);
    AudioDev_SetTrackLen(gpstLineInControlData->hAudio, gpstLineInControlData->OutPcmLen);
    AudioDev_SetTxSampleRate(gpstLineInControlData->hAudio, 0, gpstLineInControlData->SampleRate);

    gpstLineInControlData->POutPcmBuf[0] = rkos_memory_malloc(bufLen);
    if(gpstLineInControlData->POutPcmBuf[0] == NULL)
    {
        rkos_memory_free(gpstLineInControlData->POutPcmBuf[0]);
        rk_printf("LineIn malloc ERROR\n");
        while(1)
        {
            rkos_sleep(1000);
        }
    }
    memset(gpstLineInControlData->POutPcmBuf[0], 0, sizeof(gpstLineInControlData->POutPcmBuf[0]));

    gpstLineInControlData->POutPcmBuf[1] = rkos_memory_malloc(bufLen);
    if(gpstLineInControlData->POutPcmBuf[1] == NULL)
    {
        rkos_memory_free(gpstLineInControlData->POutPcmBuf[1]);
        rk_printf("LineIn malloc ERROR\n");
        while(1)
        {
            rkos_sleep(1000);
        }
    }
    memset(gpstLineInControlData->POutPcmBuf[1], 0, sizeof(gpstLineInControlData->POutPcmBuf[1]));

    stAudioArg.RecordType = gpstLineInControlData->LineInInputType;//Codec_Line1ADC
    stAudioArg.pfPcmCallBack = LineInControlPcmInput;
    stAudioArg.SamplesPerBlock = gpstLineInControlData->SamplesPerBlock;
    //rk_printf("\n ..SampleRate=%d bufLen = %d Bit=%d SamplesPerBlock=%d LRChannel=%d..\n",gpstLineInControlData->SampleRate ,bufLen, gpstLineInControlData->Bit, gpstLineInControlData->SamplesPerBlock,gpstLineInControlData->LRChannel);

    RKTaskCreate(TASK_ID_AUDIODEVICESERVICE, 0, &stAudioArg, SYNC_MODE);
}
/*******************************************************************************
** Name: LineIn_Process
** Input:UINT32 id, void *msg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 16:51:58
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON FUN rk_err_t LineIn_Process(UINT32 id, void *msg)
{
    LINEIN_APP_CALLBACK * pCur;
    rk_err_t ret;
    uint32 LineIn_state;
    switch (id)
    {
        case LINEIN_CMD_PLAY:
            if(gpstLineInControlData->LineInPlayerState == LINEIN_PLAYER_STATE_PLAY)
            {
                //printf("PLAY...\n");
                gpstLineInControlData->OutPcmIndex = 1 - gpstLineInControlData->OutPcmIndex;
                LineInControlGetPCMAndProcess((uint8 *)gpstLineInControlData->PInputPcmBuf, gpstLineInControlData->SamplesPerBlock);
            }
            //printf("LINEIN_P over.\n");
            break;

        case LINEIN_CMD_PAUSE_RESUME:
            if((UINT32)msg)
            {
                AudioDev_ReadEnable(gpstLineInControlData->hAudio, 0);
                gpstLineInControlData->LineInPlayerState = LINEIN_PLAYER_STATE_PAUSE;
            }
            else
            {
                AudioDev_FadeInit(gpstLineInControlData->hAudio, gpstLineInControlData->SampleRate / 2,FADE_IN);
                gpstLineInControlData->LineInPlayerState = LINEIN_PLAYER_STATE_PLAY;
                memset(gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex], 0, gpstLineInControlData->OutPcmLen);
                AudioDev_Write(gpstLineInControlData->hAudio, 0, gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex]);
                AudioDev_ReadEnable(gpstLineInControlData->hAudio,1);
            }
            break;

        case LINEIN_CMD_VOLUMESET:
                gpstLineInControlData->playVolume = (int16)msg;
                gSysConfig.OutputVolume = gpstLineInControlData->playVolume;
                return LineIn_AudioSetVolume((int16)msg);
                //AUDIO_CALLBACK * pCur;
                //pCur = gpstLineInControlData->pLineInStateForAPP;
                //while(pCur != NULL)
                //{
                //    pCur->pLineInStateForAPP(LINEIN_STATE_UPDATA_LINEIN_CH);
                //    pCur = pCur->pNext;
                //    printf("VOLUMESET....\n");
                //}

        case LINEIN_CMD_DEINIT://LINEIN auti-initialization
            {
                gpstLineInControlData->LineInPlayerState = LINEIN_PLAYER_STATE_STOP;
                //rk_printf("will del.... TASK_ID_AUDIODEVICESERVICE\n");
                //FmDevMuteControl(gpstLineInControlData->hFmDev, 1);
            #ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
                if(RKTaskFind(TASK_ID_AUDIODEVICESERVICE, 0) != NULL)
                {
                    ret = RKTaskDelete(TASK_ID_AUDIODEVICESERVICE, 0, SYNC_MODE);
                    if(ret != RK_SUCCESS)
                    {
                        rk_printf("TASK_ID_AUDIODEVICESERVICE delete error\n");
                    }
                }
            #endif
                rk_printf("=== MSG_LINEIN_DEINIT ===");
            }
            return RK_SUCCESS;

        case LINEIN_CMD_RECORD:
            #ifdef _RECORD_
            gpstLineInControlData->pfPcmCallBack =  RecordPcmInput;
            #endif
            gpstLineInControlData->LineInRecordState = LINEIN_CMD_RECORD;
            break;

        case LINEIN_CMD_RECORD_STOP:
            gpstLineInControlData->pfPcmCallBack = NULL;
            gpstLineInControlData->LineInRecordState = LINEIN_CMD_RECORD_STOP;
            break;

        default:
            return FALSE;
    }
    //printf("\n LineIn cmd process complete..... \n \n");
    //LineIn_Process(LINEIN_CMD_START, (void *)resetvolumet);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineInControlGetPCMAndProcess
** Input:uint8 *buf, uint32 SamplesPerBlock
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 16:40:55
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON FUN void LineInControlGetPCMAndProcess(uint8 *buf, uint32 SamplesPerBlock)
{
    UINT32 i, j;
    uint32 encodeDataLen, encodeDataLenlostlen, writeDataLen, encodeDataddr;
    uint32 index, index_old;

    if(gpstLineInControlData->LRChannel == 1)
    {
        if(RECORD_DATAWIDTH_16BIT == gpstLineInControlData->Bit)
        {
            uint16 * pInputBuffere16 =  (uint16 *)buf;
            uint16 * pOutputBuffere16 = (uint16 *)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex];
            for (index = 0; index < SamplesPerBlock; index++)
            {
                pOutputBuffere16[index] =   pInputBuffere16[2 * index];
            }
        }
        else if(RECORD_DATAWIDTH_24BIT == gpstLineInControlData->Bit)
        {
            uint8 * pInputBuffere8=  (uint8 *)buf;
            uint8 * pOutputBuffere8 = (uint8 *)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex];
            for (index = 0; index < SamplesPerBlock; index++)
            {
                pOutputBuffere8[3 * index]     =  pInputBuffere8[index * 6];
                pOutputBuffere8[3 * index + 1] =  pInputBuffere8[index * 6 + 1];
                pOutputBuffere8[3 * index + 2] =  pInputBuffere8[index * 6 + 2];
            }
        }
        else if(RECORD_DATAWIDTH_32BIT == gpstLineInControlData->Bit)
        {
            uint32 * pInputBuffere32=  (uint32 *)buf;
            uint32 * pOutputBuffere32 = (uint32 *)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex];
            for (index = 0; index < SamplesPerBlock; index++)
            {
                pOutputBuffere32[index]  =  pInputBuffere32[2 * index];
            }
        }

    }
    else if(gpstLineInControlData->LRChannel == 2)
    {
        memcpy((uint8 *)gpstLineInControlData->POutPcmBuf[gpstLineInControlData->OutPcmIndex], (uint8 *)buf ,(SamplesPerBlock * gpstLineInControlData->Bit) / 4);
    }
}
/*******************************************************************************
** Name: LineInControlPcmInput
** Input:uint8 * buf, uint32 SamplesPerBlock
** Return: void
** Owner:cjh
** Date: 2016.5.11
** Time: 16:39:25
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_COMMON_
COMMON FUN void LineInControlPcmInput(uint8 * buf, uint32 SamplesPerBlock)
{
    gpstLineInControlData->PInputPcmBuf = (uint32)buf;
    if(gpstLineInControlData->LineInPlayerState == LINEIN_PLAYER_STATE_PLAY)
    {
        LineInControlTask_SendCmd(LINEIN_CMD_PLAY, NULL, SYNC_MODE);
    }
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LineInControlTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:30:01
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_INIT_
INIT API rk_err_t LineInControlTask_DeInit(void *pvParameters)
{
    LINEIN_APP_CALLBACK * pCur;
    rk_err_t ret;

    AudioDev_RealseMainTrack(gpstLineInControlData->hAudio);
    while(gpstLineInControlData->pLineInStateForAPP != NULL)
    {
       pCur = gpstLineInControlData->pLineInStateForAPP;
       gpstLineInControlData->pLineInStateForAPP = gpstLineInControlData->pLineInStateForAPP->pNext;
       rkos_memory_free(pCur);
    }
    printf("LineInControlTask_DeInit\n");

    if(gpstLineInControlData->hAudio != NULL)
    {
        if(RKDev_Close(gpstLineInControlData->hAudio) != RK_SUCCESS)
        {
            rk_printf("gpstLineInControlData->hAudio close failure\n");
            return RK_ERROR;
        }
    }
    if(gpstLineInControlData->POutPcmBuf[0] != NULL)
    {
        rkos_memory_free(gpstLineInControlData->POutPcmBuf[0]);
        gpstLineInControlData->POutPcmBuf[0] = NULL;
    }
    if(gpstLineInControlData->POutPcmBuf[1] != NULL)
    {
        rkos_memory_free(gpstLineInControlData->POutPcmBuf[1]);
        gpstLineInControlData->POutPcmBuf[1] = NULL;
    }
    rkos_queue_delete(gpstLineInControlData->LineInControlAskQueue);
    rkos_queue_delete(gpstLineInControlData->LineInControlRespQueue);
    rkos_semaphore_delete(gpstLineInControlData->LineInControlReqSem);
    rkos_memory_free(gpstLineInControlData);
    gpstLineInControlData = NULL;
    printf("LineInControlTask_DeInit over\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LineInControlTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.11
** Time: 9:30:01
*******************************************************************************/
_APP_LINE_IN_LINEINCONTROLTASK_INIT_
INIT API rk_err_t LineInControlTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS*   pLineInControlTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_LINEINCONTROL_ARG * pArg = (RK_TASK_LINEINCONTROL_ARG *)arg;
    LINEINCONTROL_TASK_DATA_BLOCK*  pLineInControlTaskData;
    uint16 i;

    printf("LineInControlTask_Init\n");
    if(pLineInControlTask == NULL)
        return RK_PARA_ERR;

    rk_printf("SamplesPerBlock = %d\n",pArg->SamplesPerBlock);
    rk_printf("Samples = %d\n",pArg->Samples);
    rk_printf("LRChannel = %d\n",pArg->LRChannel);
    rk_printf("bits = %d\n",pArg->bits);
    rk_printf("LineInInputType  = %d\n",pArg->LineInInputType);

    pLineInControlTaskData = rkos_memory_malloc(sizeof(LINEINCONTROL_TASK_DATA_BLOCK));
    memset(pLineInControlTaskData, 0, sizeof(LINEINCONTROL_TASK_DATA_BLOCK));
    pLineInControlTaskData->LineInControlAskQueue = rkos_queue_create(1, sizeof(LINEINCONTROL_ASK_QUEUE));
    pLineInControlTaskData->LineInControlRespQueue = rkos_queue_create(1, sizeof(LINEINCONTROL_RESP_QUEUE));
    pLineInControlTaskData->LineInControlReqSem = rkos_semaphore_create(1,1);

    if ((pLineInControlTaskData->LineInControlAskQueue
      && pLineInControlTaskData->LineInControlRespQueue
      && pLineInControlTaskData->LineInControlReqSem) == 0)
    {
        rkos_queue_delete(pLineInControlTaskData->LineInControlAskQueue);
        rkos_queue_delete(pLineInControlTaskData->LineInControlRespQueue);
        rkos_semaphore_delete(pLineInControlTaskData->LineInControlReqSem);
        rkos_memory_free(pLineInControlTaskData);
        return  RK_ERROR;
    }

    pLineInControlTaskData->LRChannel = pArg->LRChannel;
    pLineInControlTaskData->LineInInputType = pArg->LineInInputType;
    pLineInControlTaskData->SamplesPerBlock = pArg->SamplesPerBlock;
    pLineInControlTaskData->SampleRate = pArg->Samples;
    pLineInControlTaskData->Bit = pArg->bits;
    if(pArg->pLineInStateCallBcakForAPP != NULL)
    {
        pLineInControlTaskData->pLineInStateForAPP = rkos_memory_malloc(sizeof(LINEIN_APP_CALLBACK));
        pLineInControlTaskData->pLineInStateForAPP->pNext = NULL;
        pLineInControlTaskData->pLineInStateForAPP->pLineInStateForAPP = pArg->pLineInStateCallBcakForAPP;
    }
    else
    {
        pLineInControlTaskData->pLineInStateForAPP = NULL;
    }
    if(gSysConfig.OutputVolume <= 30)
    {
        pLineInControlTaskData->playVolume = gSysConfig.OutputVolume;
    }
    else
    {
        pLineInControlTaskData->playVolume = 25;
    }
    #ifdef _RK_EQ_
    pLineInControlTaskData->EqMode = gSysConfig.MusicConfig.Eq.Mode;
    #endif
    gpstLineInControlData = pLineInControlTaskData;
    gpstLineInControlData->pfPcmCallBack = NULL;
    gpstLineInControlData->LineInPlayerState = LINEIN_PLAYER_STATE_PAUSE;
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
