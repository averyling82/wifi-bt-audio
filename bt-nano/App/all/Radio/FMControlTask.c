/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\FM\FMControlTask.c
* Owner: cjh
* Date: 2016.3.15
* Time: 11:43:46
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.15     11:43:46   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_FM_FMCONTROLTASK_C__

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
#include "Fade.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define RadioOutputVol  (gSysConfig.OutputVolume)
#define TUNERFREQINDEX           20
#define TUNERFREQSTEP            10              /* UP/DOWN TUNER frequency STEP    unit 100K        */
#define NOEXISTFMSTATION         0x11

#ifdef _RK_SPECTRUM_
#define SPECTRUM_LINE_M      12     // 10跟频谱线
//#define SpectrumLoopTime     3
#endif

typedef  struct _FMCONTROL_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;
}FMCONTROL_RESP_QUEUE;

typedef  struct _FMCONTROL_ASK_QUEUE
{
    uint32 cmd;
    void * msg;
    uint32 mode;
}FMCONTROL_ASK_QUEUE;

typedef  struct _FMCONTROL_TASK_DATA_BLOCK
{
    pQueue  FMControlAskQueue;
    pQueue  FMControlRespQueue;
    pSemaphore FMControlReqSem;
    HDC     hFmDev;
    HDC     hAudio;
    FM_APP_CALLBACK * pfmStateForAPP;

    UINT16    FmSearchMode;                     // FM_SearchModeState_ByHand , FM_SearchModeState_Auto
    int16     FmSearchDirect;                   // search direction.
    UINT32    FmArea;                           // SID_RadioListAreaChina,  SID_RadioListAreaJapan, SID_RadioListAreaUSA, SID_RadioListAreaEurope
    BOOLEAN   FmStereo;                         // TRUE -- STEREO; FALSE -- MONO
    UINT16    FmState;                          // FM_State_HandStepFreq , FM_State_AutoSearch, FM_State_StepStation, FM_State_Searching, FM_State_Idle
    UINT32    FmFreq;                           // current fm station frequency.
    UINT32    FmFreqBackUp;                     // Back Up current fm station frequency.
    UINT16    FmSaveNum;
    UINT16    CurFmPresetNo;
    UINT16    FmFreqArray[FREQMAXNUMBLE];
   // UINT16    FmFreqChArray[FREQMAXNUMBLE];
   // UINT16    FmFreqJpArray[FREQMAXNUMBLE];
   // UINT16    FmFreqCaArray[FREQMAXNUMBLE];
    //UINT16    PreFmSaveNum; //prevent auto-search station refresh screen too much.

    UINT32    PInputPcmBuf;
    uint8 *   POutPcmBuf[2];
    UINT32    OutPcmLen;
    uint32    OutPcmIndex;
    uint32    LRChannel;
    uint32    SamplesPerBlock;
    uint32    FMInputType;
    uint32    Bit;
    uint32    SampleRate;
    uint32    EqMode;
    uint32    playVolume;
    uint32    FMPlayerState; //Play/pause/record
    uint32    FMRecordState; //Play/pause
#ifdef _RK_SPECTRUM_
    uint8  SpectrumOut[SPECTRUM_LINE_M];//存放转换过后的M根谱线的能量
    uint32 SpectrumEn;
    uint32 CurrentTimeMsBk;
#endif

}FMCONTROL_TASK_DATA_BLOCK;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static FMCONTROL_TASK_DATA_BLOCK * gpstFMControlData;



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
void FmControlPcmInput(uint8 * buf, uint32 SamplesPerBlock);
void FmControlGetPCMAndProcess(uint8 *buf, uint32 SamplesPerBlock);
rk_err_t FM_SearchByHand(int16 direct);
rk_err_t FM_AudioSetVolume(UINT32 Volume);
rk_err_t FM_SetCurStation(int16 dir);
rk_err_t FM_SaveAndDel(int16 cmd);
rk_err_t FM_ModuleInitial(HDC dev, UINT32 area);
rk_err_t FM_Process(UINT32 id, void *msg);
rk_err_t FM_StepChangeFreq(UINT16 updownflag, UINT16 step);
void FM_Start(void);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FMControlTask_CurFreqIsInPreset
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.6.8
** Time: 14:00:37
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMControlTask_CurFreqIsInPreset(void)
{
    uint8 i;
    for(i=0; i<gpstFMControlData->FmSaveNum; i++)
    {
        if(gpstFMControlData->FmFreqArray[i] == gpstFMControlData->FmFreq)
        {
            gpstFMControlData->CurFmPresetNo = i+1;
            printf("return RK_SUCCESS i=%d FmFreq=%d FmFreqArray[%d]\n",i,gpstFMControlData->FmFreq, gpstFMControlData->FmFreqArray[i]);
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}
/*******************************************************************************
** Name: FMPlayer_GetCurSpectrum
** Input:uint8** pSpectrum
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.3
** Time: 16:30:08
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMPlayer_GetCurSpectrum(uint8** pSpectrum)
{
#ifdef _RK_SPECTRUM_
    *pSpectrum = gpstFMControlData->SpectrumOut;
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMUI_GetAudioInfo
** Input:FM_AUDIO_INFO * AudioInfo
** Return: uint32
** Owner:cjh
** Date: 2016.4.21
** Time: 11:47:26
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API uint32 FMUI_GetAudioInfo(FM_AUDIO_INFO * AudioInfo)
{
    if(AudioInfo != NULL)
    {
        AudioInfo->FMInputType = gpstFMControlData->FMInputType;
        AudioInfo->bitpersample = gpstFMControlData->Bit;
        AudioInfo->Samplerate = gpstFMControlData->SampleRate;
        AudioInfo->channels = gpstFMControlData->LRChannel;
        AudioInfo->PlayVolume = gpstFMControlData->playVolume;
        AudioInfo->EqMode = gpstFMControlData->EqMode;
        AudioInfo->CurFreq = gpstFMControlData->FmFreq;
        AudioInfo->CurFmPresetNo = gpstFMControlData->CurFmPresetNo;
        AudioInfo->FmPresetNum = gpstFMControlData->FmSaveNum;
        AudioInfo->FmStereo = gpstFMControlData->FmStereo;
        AudioInfo->FmSearchMode = gpstFMControlData->FmSearchMode;
    }
    return gpstFMControlData->FMPlayerState;
}

/*******************************************************************************
** Name: FMControlTask_SetSpectrumEn
** Input:uint32 status
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 10:47:08
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMControlTask_SetSpectrumEn(uint32 status)
{
#ifdef _RK_SPECTRUM_
    gpstFMControlData->SpectrumEn = status;
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMControlTask_SetStateChangeFunc
** Input:P_AUDIO_CALLBACK old, P_AUDIO_CALLBACK new
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.21
** Time: 10:13:04
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMControlTask_SetStateChangeFunc(P_FM_APP_CALLBACK old, P_FM_APP_CALLBACK new)
{
    FM_APP_CALLBACK * pCur, * pLast;

    pCur = gpstFMControlData->pfmStateForAPP;
    pLast = NULL;

    if(old != NULL)
    {
        while(pCur != NULL)
        {
            if(pCur->pfmStateForAPP == old)
            {
                if(new != NULL)
                {
                    pCur->pfmStateForAPP = new;
                    return RK_SUCCESS;
                }
                else
                {
                    if(pLast == NULL)
                    {
                        gpstFMControlData->pfmStateForAPP = pCur->pNext;
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

        pCur->pNext = gpstFMControlData->pfmStateForAPP;
        pCur->pfmStateForAPP = new;
        gpstFMControlData->pfmStateForAPP = pCur;
        return RK_SUCCESS;
    }

    return RK_ERROR;
}
/*******************************************************************************
** Name: FMControlTask_SendCmd
** Input:uint32 Cmd, void * msg, uint32 Mode
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.8
** Time: 16:35:33
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode)
{
    FMCONTROL_ASK_QUEUE FMControlAskQueue;
    FMCONTROL_RESP_QUEUE FMControlRespQueue;
    rk_err_t ret = RK_ERROR;

    rkos_semaphore_take(gpstFMControlData->FMControlReqSem, MAX_DELAY);

    if((gpstFMControlData->FmState == FM_CMD_HAND_AUTOSEARCH) && (Cmd == FM_CMD_HAND_AUTOSEARCH))
    {
        ret = RK_SUCCESS;
    }
    else if((gpstFMControlData->FmState == FM_CMD_AUTOSEARCH) && (Cmd == FM_CMD_AUTOSEARCH))
    {
        ret = RK_SUCCESS;
    }
    else
    {
        //printf("SendCmd FmState hand3= %d Cmd=%d\n",gpstFMControlData->FmState, Cmd);
        if(((gpstFMControlData->FmState == FM_CMD_HAND_AUTOSEARCH) && (Cmd != FM_CMD_HAND_AUTOSEARCH))
            || ((gpstFMControlData->FmState == FM_CMD_AUTOSEARCH) && (Cmd != FM_CMD_AUTOSEARCH)))
        {
            if(Cmd != FM_CMD_PLAY)
            {
                FMControlAskQueue.cmd = FM_CMD_STOP_SEARCH;
                FMControlAskQueue.msg = msg;
                FMControlAskQueue.mode = Mode;
                goto exit;
                //gpstFMControlData->FmState = FM_CMD_Idle;
                //gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                //ret = RK_SUCCESS;
            }
        }
        else if(Cmd == FM_CMD_VOLUMESET)
        {
            FM_Process(FM_CMD_VOLUMESET, msg);
            ret = RK_SUCCESS;
        }
        else
        {
            if(Cmd == FM_CMD_DEINIT)
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_STOP;
            }

            FMControlAskQueue.cmd = Cmd;
            FMControlAskQueue.msg = msg;
            FMControlAskQueue.mode = Mode;
        exit:
            ret = rkos_queue_send(gpstFMControlData->FMControlAskQueue, &FMControlAskQueue, MAX_DELAY);
            if(ret != RK_SUCCESS)
            {
                printf("FMControlAskQueue send fail fmcontrol...\n");
                ret = RK_ERROR;
            }
            else
            {
                if(Mode == SYNC_MODE)
                {
                    rkos_queue_receive(gpstFMControlData->FMControlRespQueue, &FMControlRespQueue, MAX_DELAY);
                    if ((FMControlRespQueue.cmd == Cmd) && FMControlRespQueue.status == RK_SUCCESS)
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
    }
    rkos_semaphore_give(gpstFMControlData->FMControlReqSem);
    return ret;
}
/*******************************************************************************
** Name: FMControlTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:44:15
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMControlTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: FMControlTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:44:15
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t FMControlTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: FMControlTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2016.3.15
** Time: 11:44:15
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API void FMControlTask_Enter(void * arg)
{
    FMCONTROL_ASK_QUEUE FmControlAskQue;
    FMCONTROL_RESP_QUEUE FmControlResQue;
    rk_err_t ret;
    uint32 fm_state;

    //rk_printf("FMControlTask_Enter... FmArea = %d\n",gpstFMControlData->FmArea);
    if(DeviceTask_CreateDeviceList(DEVICE_LIST_FM, NULL, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("FM device list create fail\n");
         while(1);
    }
    gpstFMControlData->hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);

    if((gpstFMControlData->hFmDev == NULL)
    || (gpstFMControlData->hFmDev == (HDC)RK_ERROR)
    || (gpstFMControlData->hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("hFmDev device open failure");
        RKDev_Close(gpstFMControlData->hFmDev);
        while(1);
    }

    gpstFMControlData->hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);

    if((gpstFMControlData->hAudio == NULL)
    || (gpstFMControlData->hAudio == (HDC)RK_ERROR)
    || (gpstFMControlData->hAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("hAudio device open failure");
        rk_printf("0 hAudio -1\n");
        RKDev_Close(gpstFMControlData->hAudio);
        while(1);
    }
    printf("FmFreq = %d\n", gpstFMControlData->FmFreq);
    //rk_printf("\n FM_Start ....\n");
    FM_Start();
    //FM_Process(FM_CMD_INIT, (void *)1);
    FM_Process(FM_CMD_START, (void *)1);
    FmDevVolSet(gpstFMControlData->hFmDev, 15);
    gpstFMControlData->FmState = FM_CMD_Idle;

    //memset(gpstFMControlData->POutPcmBuf[1-gpstFMControlData->OutPcmIndex], 0, sizeof(gpstFMControlData->POutPcmBuf[1-gpstFMControlData->OutPcmIndex]));
    AudioDev_FadeInit(gpstFMControlData->hAudio, gpstFMControlData->SampleRate / 2,FADE_IN);
    memset(gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex], 0, gpstFMControlData->OutPcmLen);
    AudioDev_Write(gpstFMControlData->hAudio, 0, gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex]);
    AudioDev_ReadEnable(gpstFMControlData->hAudio, 1);

    while(1)
    {
        if(FM_CMD_Idle == gpstFMControlData->FmState)
        {
            if(rkos_queue_receive(gpstFMControlData->FMControlAskQueue, &FmControlAskQue, MAX_DELAY) == RK_SUCCESS)
            {
                //printf("\n p..p %d",FmControlAskQue.cmd);
                FmControlResQue.status = FM_Process(FmControlAskQue.cmd, FmControlAskQue.msg);
                FmControlResQue.cmd = FmControlAskQue.cmd;
                if(FmControlAskQue.mode == SYNC_MODE)
                {
                    //printf(" s..%d ",FmControlResQue.cmd);
                    rkos_queue_send(gpstFMControlData->FMControlRespQueue, &FmControlResQue, MAX_DELAY);
                }
            }
        }
        else
        {
            if(rkos_queue_receive(gpstFMControlData->FMControlAskQueue, &FmControlAskQue, 0) == RK_SUCCESS)
            {
                //printf("k..k ");
                FmControlResQue.status = FM_Process(FmControlAskQue.cmd, FmControlAskQue.msg);
                FmControlResQue.cmd = FmControlAskQue.cmd;
                if(FmControlAskQue.mode == SYNC_MODE)
                {
                    //printf(" v..%d ",FmControlResQue.cmd );
                    rkos_queue_send(gpstFMControlData->FMControlRespQueue, &FmControlResQue, MAX_DELAY);
                }
            }
        }

        #ifdef __APP_RECORD_RECORDCONTROL_C__
        if(gpstFMControlData->FMRecordState == FM_CMD_RECORD)
        {
            RecordPcmInput((uint8 * )gpstFMControlData->PInputPcmBuf, gpstFMControlData->SamplesPerBlock);
        }
        #endif


        #if 1
        //printf("fp:0=%d cmd15=%d ",gpstFMControlData->FMPlayerState, FmControlAskQue.cmd);
        if((gpstFMControlData->FMPlayerState == FM_PLAYER_STATE_PLAY) && (FmControlAskQue.cmd == FM_CMD_PLAY))
        {
            #ifdef _RK_SPECTRUM_
            if (gpstFMControlData->SpectrumEn)
            {
                short Spectrum_data[128];
                uint32 currentTimeMs;
                uint32 currentTime200Ms;
                currentTimeMs = SysTickCounter * 10;
                currentTime200Ms = currentTimeMs / 100;    //refresh spectrum every 100ms
                if (currentTime200Ms != gpstFMControlData->CurrentTimeMsBk)
                {
                    memset(gpstFMControlData->SpectrumOut, 0, SPECTRUM_LINE_M*sizeof(char));
                    gpstFMControlData->CurrentTimeMsBk = currentTime200Ms;

                    {
                        if(gpstFMControlData->Bit == 32)
                        {
                           int16 i = 0;
                           char *ptr = (char *)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex];
                           for(i=1;i<129;i++)
                           {
                              Spectrum_data[i-1] = (short)(ptr[4*i-1]<<8 | ptr[4*i-2]);
                           }
                        }
                        else if(gpstFMControlData->Bit == 24)
                        {
                           int16 i = 0;
                           char *ptr = (char *)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex];
                           for(i=1;i<129;i++)
                           {
                              Spectrum_data[i-1] = (short)(ptr[3*i-1]<<8 | ptr[3*i-2]);
                           }
                        }
                        else
                        {
                            memcpy(Spectrum_data, (short*)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex], 256);
                        }
                        DoSpectrum((short *)Spectrum_data,&gpstFMControlData->SpectrumOut[0]);

                        if(2 > gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-3])
                        {
                            gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-2] = gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-3] ;
                            gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-1] = gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-3] ;
                        }
                        else
                        {
                            gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-2] = gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-3] - 1;
                            gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-1] = gpstFMControlData->SpectrumOut[SPECTRUM_LINE_M-3] - 2;
                        }
                        //printf("SPECTRUM.... callback\n");
                        FM_APP_CALLBACK * pCur;
                        pCur = gpstFMControlData->pfmStateForAPP;
                        while(pCur != NULL)
                        {
                            pCur->pfmStateForAPP(FM_PLAYER_STATE_UPDATA_SPECTRUM);
                            pCur = pCur->pNext;
                        }
                    }
                }
            }
            #endif
            AudioDev_Write(gpstFMControlData->hAudio, 0, gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex]);
        }
        #endif

        //rk_printf("FmControlTask FmState =%d FmFreq= %d\n",gpstFMControlData->FmState, gpstFMControlData->FmFreq);
        if(FM_CMD_AUTOSEARCH == gpstFMControlData->FmState)
        {
            ret = FM_SearchByHand(FM_DIRECT_INC);
            if(FM_FoundStation == ret)
            {
                //call back app
                rk_printf("auto search =%d HZ\n",gpstFMControlData->FmFreq);
                if(FM_SaveAndDel(1) == RK_ERROR)
                {
                    rk_printf("Save NUM >= FREQMAXNUMBLE\n");
                    gpstFMControlData->FmFreq = gpstFMControlData->FmFreqArray[0];
                    gpstFMControlData->CurFmPresetNo = 1;
                    gpstFMControlData->FmState = FM_CMD_Idle;
                    gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                    FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
                    fm_state = FM_PLAYER_STATE_AUTOSEACH_END;
                }
                else
                {
                    gpstFMControlData->CurFmPresetNo = gpstFMControlData->FmSaveNum;
                    fm_state = FM_STATE_UPDATA_FM_PRESET;
                }
            }
            else if(FM_InvalidStation == ret)
            {
                //gpstFMControlData->FmState = FM_CMD_Idle;
                rk_printf("FM_FoundStation  FM_InvalidStation\n");
                if(gpstFMControlData->FmSaveNum > 0)
                {
                    gpstFMControlData->FmFreq = gpstFMControlData->FmFreqArray[0];
                    gpstFMControlData->CurFmPresetNo = 1;
                }
                else
                {
                    gpstFMControlData->FmFreq = FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0);
                    fm_state = FM_STATE_UPDATA_FM_CH;
                }
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
                fm_state = FM_PLAYER_STATE_AUTOSEACH_END;
            }
            else
            {
                fm_state = FM_STATE_UPDATA_FM_CH;
            }

            if(gpstFMControlData->FmFreq >= FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 1))
            {

                if(gpstFMControlData->FmSaveNum > 0)
                {
                    gpstFMControlData->FmFreq = gpstFMControlData->FmFreqArray[0];
                    gpstFMControlData->CurFmPresetNo = 1;
                }
                else
                {
                    gpstFMControlData->FmFreq = FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0);
                }
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
                fm_state = FM_PLAYER_STATE_AUTOSEACH_END;
            }

            FM_APP_CALLBACK * pCur;
            pCur = gpstFMControlData->pfmStateForAPP;
            while(pCur != NULL)
            {
                pCur->pfmStateForAPP(fm_state);
                pCur = pCur->pNext;
            }
            //FM_Process(FM_CMD_AUTOSEARCH, gpstFMControlData->FmSearchDirect);
        }
        else if(FM_CMD_HAND_AUTOSEARCH == gpstFMControlData->FmState)
        {
            rk_printf("HAND_AUTOSEARCH FmFreq= %d\n", gpstFMControlData->FmFreq);
            ret = FM_SearchByHand(gpstFMControlData->FmSearchDirect);
            if(FM_FoundStation == ret)
            {
                //gpstFMControlData->FmState = FM_CMD_Idle;
                rk_printf("HAND_AUTOSEARCH  FM_FoundStation\n");
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
            }
            else if(FM_InvalidStation == ret)
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
            }
            //call back app
            FM_APP_CALLBACK * pCur;
            pCur = gpstFMControlData->pfmStateForAPP;
            while(pCur != NULL)
            {
                rk_printf("HAND_AUTOSEARCH  call CH\n");
                pCur->pfmStateForAPP(FM_STATE_UPDATA_FM_CH);
                pCur = pCur->pNext;
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
** Name: FmControlPcmInput
** Input:uint8 * buf, uint32 len
** Return: void
** Owner:cjh
** Date: 2016.4.20
** Time: 11:08:16
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN void FmControlPcmInput(uint8 * buf, uint32 SamplesPerBlock)
{
    FMCONTROL_ASK_QUEUE FMControlAskQueue;
    FMCONTROL_RESP_QUEUE FMControlRespQueue;
    rk_err_t ret = RK_ERROR;
    gpstFMControlData->PInputPcmBuf = (uint32)buf;
    if(gpstFMControlData->FMPlayerState == FM_PLAYER_STATE_PLAY)
    {
        FMControlTask_SendCmd(FM_CMD_PLAY, NULL, SYNC_MODE);
    }
}

/*******************************************************************************
** Name: FmControlGetPCMAndProcess
** Input:uint8 *buf, uint32 len
** Return: void
** Owner:cjh
** Date: 2016.4.20
** Time: 11:10:00
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN void FmControlGetPCMAndProcess(uint8 *buf, uint32 SamplesPerBlock)
{
    UINT32 i, j;
    uint32 encodeDataLen, encodeDataLenlostlen, writeDataLen, encodeDataddr;
    uint32 index, index_old;

    if(gpstFMControlData->LRChannel == RECORD_CHANNEL_MONO)
    {
        if(RECORD_DATAWIDTH_16BIT == gpstFMControlData->Bit)
        {
            uint16 * pInputBuffere16 =  (uint16 *)buf;
            uint16 * pOutputBuffere16 = (uint16 *)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex];
            for (index = 0; index < SamplesPerBlock; index++)
            {
                pOutputBuffere16[index] =   pInputBuffere16[2 * index];
            }
        }
        else if(RECORD_DATAWIDTH_24BIT == gpstFMControlData->Bit)
        {
            uint8 * pInputBuffere8=  (uint8 *)buf;
            uint8 * pOutputBuffere8 = (uint8 *)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex];
            for (index = 0; index < SamplesPerBlock; index++)
            {
                pOutputBuffere8[3 * index]     =  pInputBuffere8[index * 6];
                pOutputBuffere8[3 * index + 1] =  pInputBuffere8[index * 6 + 1];
                pOutputBuffere8[3 * index + 2] =  pInputBuffere8[index * 6 + 2];
            }
        }
        else if(RECORD_DATAWIDTH_32BIT == gpstFMControlData->Bit)
        {
            uint32 * pInputBuffere32=  (uint32 *)buf;
            uint32 * pOutputBuffere32 = (uint32 *)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex];
            for (index = 0; index < SamplesPerBlock; index++)
            {
                pOutputBuffere32[index]  =  pInputBuffere32[2 * index];
            }
        }

    }
    else if(gpstFMControlData->LRChannel == RECORD_CHANNEL_STERO)
    {
        //printf("\n PCM OutPut = %x, record size = %d",POutPcmBuf[gpstAudioDevInf->PcmIndex], (gpstAudioDevInf->SamplesPerBlock * gpstAudioDevInf->Bit ) / 4);
        memcpy((uint8 *)gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex], (uint8 *)buf ,(SamplesPerBlock * gpstFMControlData->Bit) / 4);
    }
}

/*******************************************************************************
** Name: FM_SearchByHand
** Input:UINT16 direct, void *msg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.29
** Time: 11:39:21
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN rk_err_t FM_SearchByHand(int16 direct)
{
    if(direct != FM_DIRECT_CUR)
    {
        FMDevStepChangeFreq(gpstFMControlData->hFmDev, direct,&gpstFMControlData->FmFreq, TUNERFREQSTEP);
    }
    return FmDevSearchByHand(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq);
}
/*******************************************************************************
** Name: FM_AudioSetVolume
** Input:UINT32 Volume
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.29
** Time: 10:35:04
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN rk_err_t FM_AudioSetVolume(UINT32 Volume)
{
    return AudioDev_SetVol(gpstFMControlData->hAudio, Volume);
}
/*******************************************************************************
** Name: FM_GetPrevStation
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.29
** Time: 10:26:23
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN rk_err_t FM_SetCurStation(int16 dir)
{	//page
    //UINT16 PreFmSaveNum = gpstFMControlData->CurFmPresetNo;
    if(gpstFMControlData->FmSaveNum < 1)
    {
        return NOEXISTFMSTATION;
    }

    if(dir == -1)
    {
        if(gpstFMControlData->CurFmPresetNo <= 1)
        {
            gpstFMControlData->CurFmPresetNo = 1;//gpstFMControlData->FmSaveNum;
            return RK_ERROR;
        }
        else
        {
            gpstFMControlData->CurFmPresetNo--;
        }
        gpstFMControlData->FmFreq = gpstFMControlData->FmFreqArray[gpstFMControlData->CurFmPresetNo-1];

        if(((gpstFMControlData->FmFreq < FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0)) || (gpstFMControlData->FmFreq > FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 1))))
        {
            gpstFMControlData->FmFreq = FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0);
        }
    }
    else if(dir == 1)
    {
        gpstFMControlData->CurFmPresetNo++;
        if(gpstFMControlData->CurFmPresetNo > gpstFMControlData->FmSaveNum)
        {
            gpstFMControlData->CurFmPresetNo = gpstFMControlData->FmSaveNum;//0;
            return RK_ERROR;
        }
        gpstFMControlData->FmFreq = gpstFMControlData->FmFreqArray[gpstFMControlData->CurFmPresetNo-1];

        if(((gpstFMControlData->FmFreq < FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0)) || (gpstFMControlData->FmFreq > FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 1))))
        {
            gpstFMControlData->FmFreq = FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0);
        }
    }
    else if(dir == 0)
    {
        gpstFMControlData->CurFmPresetNo = 1;
        gpstFMControlData->FmFreq = gpstFMControlData->FmFreqArray[0];

        if(((gpstFMControlData->FmFreq < FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0)) || (gpstFMControlData->FmFreq > FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 1))))
        {
            gpstFMControlData->FmFreq = FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0);
        }
    }
    else
    {
        return RK_ERROR;
    }
    return FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
}

/*******************************************************************************
** Name: FM_StepFreqProcess
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.3.29
** Time: 10:09:46
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN void FM_StepFreqProcess(void)
{
    FMDevStepChangeFreq(gpstFMControlData->hFmDev, gpstFMControlData->FmSearchDirect, &gpstFMControlData->FmFreq, TUNERFREQSTEP);

    gpstFMControlData->FmFreq = (gpstFMControlData->FmFreq / 10) * 10;
}
/*******************************************************************************
** Name: FM_SaveAndDel
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.28
** Time: 16:54:11
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN rk_err_t FM_SaveAndDel(int16 cmd)
{
    UINT32 freqpre, freqcur, j;

    if(cmd == 1)
    {
        if(gpstFMControlData->FmSaveNum >= FREQMAXNUMBLE)
        {
            return RK_ERROR;
        }

        if(gpstFMControlData->FmSaveNum > 0)
        {
            freqpre = gpstFMControlData->FmFreqArray[gpstFMControlData->FmSaveNum-1] / 10;
        }
        else
        {
            freqpre = 0;
        }
        freqcur = (gpstFMControlData->FmFreq) / 10;
        //rk_printf("+++++++freqpre%d freqcur=%d\n",freqpre, freqcur);
        if (freqpre != freqcur)
        {
            gpstFMControlData->FmFreqArray[gpstFMControlData->FmSaveNum] = gpstFMControlData->FmFreq;
            gpstFMControlData->FmSaveNum++;
            gpstFMControlData->CurFmPresetNo = gpstFMControlData->FmSaveNum;
        }
        //rk_printf("+++++++gpstFMControlData->FmSaveNum = %d FmFreq=%d\n",gpstFMControlData->FmSaveNum, gpstFMControlData->FmFreq);
    }
    else if(cmd == -1)
    {
        if((gpstFMControlData->FmSaveNum < 1) || (gpstFMControlData->CurFmPresetNo < 1))
        {
            rk_printf("preset NUM < 1 FmSaveNum=%d CurFmPresetNo=%d\n",gpstFMControlData->FmSaveNum, gpstFMControlData->CurFmPresetNo);
            return RK_ERROR;
        }

        //if(gpstFMControlData->CurFmPresetNo <= gpstFMControlData->FmSaveNum)
        {
            for(j = gpstFMControlData->CurFmPresetNo;j < gpstFMControlData->FmSaveNum;j++)
            {
                //rk_printf("%d CurFmPresetNo=%d FmSaveNum=%d\n",j,gpstFMControlData->CurFmPresetNo,gpstFMControlData->FmSaveNum);
                gpstFMControlData->FmFreqArray[j-1] = gpstFMControlData->FmFreqArray[j];
            }
            //rk_printf("[%d] = 0\n",j-1);
            gpstFMControlData->FmFreqArray[j-1] = 0;
            gpstFMControlData->FmSaveNum--;
            if(gpstFMControlData->CurFmPresetNo > gpstFMControlData->FmSaveNum)
            {
                gpstFMControlData->CurFmPresetNo = gpstFMControlData->FmSaveNum;
            }
        }
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FM_ModuleInitial
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.28
** Time: 11:03:34
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN rk_err_t FM_ModuleInitial(HDC dev, UINT32 area)
{
    if(area < RADIO_USA)
    {
        gpstFMControlData->FmArea = area;
    }
    else
    {
        rk_printf("area par err!\n");
        gpstFMControlData->FmArea = RADIO_CHINA;
    }
    return FmDevSetInitArea(dev, gpstFMControlData->FmArea);
}

/*******************************************************************************
** Name: FM_Process
** Input:MSG_ID id, void *msg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.28
** Time: 10:56:40
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN rk_err_t FM_Process(UINT32 id, void *msg)
{
    FM_APP_CALLBACK * pCur;
    rk_err_t ret;
    uint32 fm_state;
    switch (id)
    {
        case FM_CMD_PLAY:
            if(gpstFMControlData->FMPlayerState == FM_PLAYER_STATE_PLAY)
            {
                gpstFMControlData->OutPcmIndex = 1 - gpstFMControlData->OutPcmIndex;
                FmControlGetPCMAndProcess((uint8 *)gpstFMControlData->PInputPcmBuf, gpstFMControlData->SamplesPerBlock);
            }
            break;

        case FM_CMD_PAUSE_RESUME:
            //printf("PAUSE RESUME....\n");
            gpstFMControlData->FmState = FM_CMD_Idle;
            if((UINT32)msg)
            {
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PAUSE;
                //FmDevMuteControl(gpstFMControlData->hFmDev, 1);
            }
            else
            {
                AudioDev_FadeInit(gpstFMControlData->hAudio, gpstFMControlData->SampleRate / 2,FADE_IN);
                memset(gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex], 0, gpstFMControlData->OutPcmLen);
                AudioDev_Write(gpstFMControlData->hAudio, 0, gpstFMControlData->POutPcmBuf[gpstFMControlData->OutPcmIndex]);
                AudioDev_ReadEnable(gpstFMControlData->hAudio, 1);
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                //FmDevMuteControl(gpstFMControlData->hFmDev, 0);
            }
            break;

        case FM_CMD_INIT://fm initialization
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;

                //FMVol_Resume();
                if(RK_ERROR == FM_ModuleInitial(gpstFMControlData->hFmDev, (UINT32)msg))//gpstFMControlData->FmArea
                {
                    rk_printf("FM_ModuleInitial ERROR \n");
                    return RK_ERROR;
                }
                DelayMs(100);
                ret = FmDevMuteControl(gpstFMControlData->hFmDev, 0);
            }
            return ret;

        case FM_CMD_START://start fm play
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
                pCur = gpstFMControlData->pfmStateForAPP;
                while(pCur != NULL)
                {
                    pCur->pfmStateForAPP(FM_STATE_UPDATA_FM_CH);
                    pCur = pCur->pNext;
                    printf("VOLUMESET....\n");
                }
                printf("..hFmDev=0x%x FmFreq=%d FmStereo=%d FmArea=%d \n",gpstFMControlData->hFmDev,gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
            }
            return RK_SUCCESS;

        case FM_CMD_VOLUMESET:
                gpstFMControlData->playVolume = (int16)msg;
                gSysConfig.OutputVolume = gpstFMControlData->playVolume;
                return FM_AudioSetVolume((int16)msg);
                //AUDIO_CALLBACK * pCur;
                //pCur = gpstFMControlData->pfmStateForAPP;
                //while(pCur != NULL)
                //{
                //    pCur->pfmStateForAPP(FM_STATE_UPDATA_FM_CH);
                //    pCur = pCur->pNext;
                //    printf("VOLUMESET....\n");
                //}
                //return RK_SUCCESS;
        case FM_CMD_MANUAL:
            {
                printf("...FM_CMD_MANUAL.... %d\n",(int16)msg);
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FmSearchMode = FM_STATE_HANDSEARCH;
                gpstFMControlData->FmSearchDirect = (int16)msg;
                if(gpstFMControlData->FmSearchDirect != FM_DIRECT_CUR)
                {
                    ret = FM_SearchByHand(gpstFMControlData->FmSearchDirect);
                    FmDevStart(gpstFMControlData->hFmDev, gpstFMControlData->FmFreq, gpstFMControlData->FmStereo ,gpstFMControlData->FmArea);
                    FmDevMuteControl(gpstFMControlData->hFmDev, 0);

                    pCur = gpstFMControlData->pfmStateForAPP;
                    while(pCur != NULL)
                    {
                        pCur->pfmStateForAPP(FM_STATE_UPDATA_FM_CH);
                        pCur = pCur->pNext;
                        //printf("FM_CMD_MANUAL....CH\n");
                    }
                }
                //if((UINT32)msg)
                //{
                //    resetvolumet = 1;
                //    FmDevMuteControl(gpstFMControlData->hFmDev, 0);
                //}
                //printf("pro ret\n");
                return ret;
            }

        case FM_CMD_AUTOSEARCH:
            if(FM_CMD_AUTOSEARCH == gpstFMControlData->FmState)
            {
                break;
            }
            gpstFMControlData->FmSearchDirect = FM_DIRECT_INC;
            gpstFMControlData->FmState = FM_CMD_AUTOSEARCH;
            gpstFMControlData->FmSearchMode = FM_STATE_AUTOSEARCH;
            gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PAUSE;
            if((UINT32)msg)
            {
                FmDevMuteControl(gpstFMControlData->hFmDev, 1);
            }
            if(gpstFMControlData->FmArea ==  RADIO_JAPAN)
            {
                gpstFMControlData->FmFreq = 7600;
            }
            else
            {
                gpstFMControlData->FmFreq = 8750;//page
            }
            gpstFMControlData->FmSaveNum = 0;
            gpstFMControlData->FmFreqBackUp = gpstFMControlData->FmFreq;

            //call back waring app delete all;
            break;

        case FM_CMD_AUTOSEARCH_RESUME:
            gpstFMControlData->FmState = FM_CMD_AUTOSEARCH;
            break;

        case FM_CMD_HAND_AUTOSEARCH:
            gpstFMControlData->FmSearchDirect = (UINT32)msg;
            if(FM_CMD_HAND_AUTOSEARCH == gpstFMControlData->FmState)
            {
                break;
            }
            gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PAUSE;
            gpstFMControlData->FmSearchMode = FM_STATE_HANDSTEPFREQ;
            gpstFMControlData->FmFreqBackUp = gpstFMControlData->FmFreq;
            gpstFMControlData->FmState = FM_CMD_HAND_AUTOSEARCH;
            //FmDevMuteControl(gpstFMControlData->hFmDev, 1);
            break;

        case FM_CMD_STEROSWITCH://stereo or not set.
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FmStereo = gSysConfig.RadioConfig.FmStereo;
                FmDevSetStereo(gpstFMControlData->hFmDev, gpstFMControlData->FmStereo);
            }
            break;

        case FM_CMD_STEPFREQ://inc or dec one frequency unit.
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FmSearchMode = FM_STATE_HANDSTEPFREQ;
                gpstFMControlData->FmSearchDirect = (UINT32)msg;
                FM_StepFreqProcess();
            }
            break;

        case FM_CMD_STEPSTATION:
            {
                //if((int16)msg != FM_DIRECT_CUR)
                {
                    ret = FM_SetCurStation((int16)msg);
                    if(ret == NOEXISTFMSTATION)
                    {
                        fm_state = FM_STATE_NO_EXIST_FM_STATION;
                    }
                    else if(ret == RK_ERROR)
                    {
                        return RK_SUCCESS;
                    }
                    else
                    {
                        gpstFMControlData->FmState = FM_CMD_Idle;
                        gpstFMControlData->FmSearchMode = FM_STATE_STEPSTATION;
                        gpstFMControlData->FmSearchDirect = (int16)msg;
                        FmDevMuteControl(gpstFMControlData->hFmDev, 0);
                        gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                        fm_state = FM_STATE_UPDATA_FM_PRESET;
                    }
                    //gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                    pCur = gpstFMControlData->pfmStateForAPP;
                    while(pCur != NULL)
                    {
                        pCur->pfmStateForAPP(fm_state);
                        pCur = pCur->pNext;
                    }
                }
                //------TODO-------send message to ui,display freqency point.
            }
            break;

        case FM_CMD_GETSTEROSTATUS://get status of stereo
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                DelayMs(200);
                gpstFMControlData->FmStereo = FmDevGetStereoStatus(gpstFMControlData->hFmDev);
                //APP Display Stereo
            }
            return RK_SUCCESS;

        case FM_CMD_STOP_SEARCH://get status of stereo
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                //gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_PLAY;
                //FmDevMuteControl(gpstFMControlData->hFmDev, 0);
            }
            return RK_SUCCESS;

        case FM_CMD_DEINIT://FM auti-initialization
            {
                gpstFMControlData->FmState = FM_CMD_Idle;
                gpstFMControlData->FMPlayerState = FM_PLAYER_STATE_STOP;
                rk_printf("will del.... TASK_ID_AUDIODEVICESERVICE\n");
                //FmDevMuteControl(gpstFMControlData->hFmDev, 1);
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
                FMDevPowerOffDeinit(gpstFMControlData->hFmDev);
                rk_printf("=== MSG_FM_DEINIT ===");
            }
            return RK_SUCCESS;

        case FM_CMD_RECORD:
            gpstFMControlData->FMRecordState = FM_CMD_RECORD;
            break;

        case FM_CMD_RECORD_STOP:
            gpstFMControlData->FMRecordState = FM_CMD_RECORD_STOP;
            break;

        case FM_CMD_SAVE_CH:
            ret = FM_SaveAndDel(1);
            if(ret == RK_SUCCESS)
            {
                fm_state = FM_STATE_SAVE_CUR_FM_CH;
            }
            else
            {
                fm_state = FM_STATE_SAVE_CUR_FM_CH_FULL;
            }
            pCur = gpstFMControlData->pfmStateForAPP;
            while(pCur != NULL)
            {
                pCur->pfmStateForAPP(fm_state);
                pCur = pCur->pNext;
            }
            break;

        case FM_CMD_DELETE_CH:
            if(gpstFMControlData->FmFreq == gpstFMControlData->FmFreqArray[gpstFMControlData->CurFmPresetNo -1])
            {
                FM_SaveAndDel(-1);
                pCur = gpstFMControlData->pfmStateForAPP;
                while(pCur != NULL)
                {
                    pCur->pfmStateForAPP(FM_STATE_DELETE_PRESET_FM_CH);
                    pCur = pCur->pNext;
                }
            }
            else
            {
                rk_printf("cur freq no exit\n");
            }

            break;
        case FM_CMD_DELETE_ALL_CH:
            memset(gpstFMControlData->FmFreqArray, 0, sizeof(gpstFMControlData->FmFreqArray));
            gpstFMControlData->CurFmPresetNo = 0;
            gpstFMControlData->FmSaveNum = 0;

            pCur = gpstFMControlData->pfmStateForAPP;
            while(pCur != NULL)
            {
                pCur->pfmStateForAPP(FM_STATE_DELETE_PRESET_FM_CH);
                pCur = pCur->pNext;
            }
            break;

        case FM_CMD_Idle:
            gpstFMControlData->FmState = FM_CMD_Idle;
            printf("FMPlayerState = %d",gpstFMControlData->FMPlayerState);
            break;

        default:
            gpstFMControlData->FmState = FM_CMD_Idle;
            return FALSE;
    }
    //printf("\n fm cmd process complete..... \n \n");
    //FM_Process(FM_CMD_START, (void *)resetvolumet);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FM_Start
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.3.25
** Time: 14:36:52
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON FUN void FM_Start(void)
{
    AUDIO_DEV_ARG stAudioArg;
    uint32 bufLen;
    uint32 i;
    gpstFMControlData->playVolume = gSysConfig.OutputVolume;
    //gpstFMControlData->FmArea = gSysConfig.RadioConfig.FmArea;
    //gpstFMControlData->FmStereo = gSysConfig.RadioConfig.FmStereo;
    gpstFMControlData->FmSaveNum = gSysConfig.RadioConfig.FmSaveNum;
    gpstFMControlData->FmSearchMode = gSysConfig.RadioConfig.FmState;

    //gpstFMControlData->PreFmSaveNum = gSysConfig.RadioConfig.FmSaveNum;
    gpstFMControlData->FmSaveNum = gSysConfig.RadioConfig.FmSaveNum;
    gpstFMControlData->CurFmPresetNo = 0;
    for(i = 0; i < gpstFMControlData->FmSaveNum; i++)
    {
        if((gSysConfig.RadioConfig.FmFreqArray[i] < FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0)) ||  (gSysConfig.RadioConfig.FmFreqArray[i] > FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 1)))
        {
            gpstFMControlData->FmFreqArray[i] = 0;
        }
        else
        {
            gpstFMControlData->FmFreqArray[i] = gSysConfig.RadioConfig.FmFreqArray[i];
        }
    }

    if(gSysConfig.RadioConfig.FmFreq < FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 0) || gSysConfig.RadioConfig.FmFreq > FmDevGetMinOrMaxFreq(gpstFMControlData->hFmDev, 1))
    {
        if(gpstFMControlData->FmArea ==  RADIO_JAPAN)
        {
            gpstFMControlData->FmFreq = 7600;
        }
        else
        {
            gpstFMControlData->FmFreq = 8750;//page
        }
    }
    else
    {
        gpstFMControlData->FmFreq = gSysConfig.RadioConfig.FmFreq;
    }

/*
    #ifdef CODEC_24BIT //24bit
    RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
    #else
    RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
    #endif
    RockcodecDev_SetMode(gpstAudioDevInf->hCodec, Codec_DACoutHP);
*/

    stAudioArg.Bit = gpstFMControlData->Bit;
    stAudioArg.SampleRate = gpstFMControlData->SampleRate;
    gpstFMControlData->OutPcmLen = ((gpstFMControlData->SamplesPerBlock * gpstFMControlData->LRChannel * gpstFMControlData->Bit) / 32) * 4;
    if(stAudioArg.Bit == 16)
    {
        bufLen = gpstFMControlData->OutPcmLen*2;
    }
    else if(stAudioArg.Bit == 24)
    {
        bufLen = (gpstFMControlData->OutPcmLen + 3)*4/3;
    }
/*
    #ifdef CODEC_24BIT //24bit
    RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
    #else
    RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
    #endif
    RockcodecDev_SetMode(gpstAudioDevInf->hCodec, Codec_DACoutHP);
*/
    AudioDev_SetBit(gpstFMControlData->hAudio, 0, gpstFMControlData->Bit);
    AudioDev_SetChannel(gpstFMControlData->hAudio, 0, gpstFMControlData->LRChannel);
    AudioDev_GetMainTrack(gpstFMControlData->hAudio);
    AudioDev_SetVol(gpstFMControlData->hAudio, gpstFMControlData->playVolume);
    AudioDev_SetTrackLen(gpstFMControlData->hAudio, gpstFMControlData->OutPcmLen);
    AudioDev_SetSampleRate(gpstFMControlData->hAudio, 0, gpstFMControlData->SampleRate);

    gpstFMControlData->POutPcmBuf[0] = rkos_memory_malloc(bufLen);
    if(gpstFMControlData->POutPcmBuf[0] == NULL)
    {
        rkos_memory_free(gpstFMControlData->POutPcmBuf[0]);
        rk_printf("fm malloc ERROR\n");
        while(1)
        {
            rkos_sleep(1000);
        }
    }
    //rk_printf("POutPcmBuf[0]=0x%x PcmLen=%d\n", gpstFMControlData->POutPcmBuf[0],gpstFMControlData->OutPcmLen);
    memset(gpstFMControlData->POutPcmBuf[0], 0, sizeof(gpstFMControlData->POutPcmBuf[0]));

    gpstFMControlData->POutPcmBuf[1] = rkos_memory_malloc(bufLen);
    if(gpstFMControlData->POutPcmBuf[1] == NULL)
    {
        rkos_memory_free(gpstFMControlData->POutPcmBuf[1]);
        rk_printf("fm malloc ERROR\n");
        while(1)
        {
            rkos_sleep(1000);
        }
    }
    memset(gpstFMControlData->POutPcmBuf[1], 0, sizeof(gpstFMControlData->POutPcmBuf[1]));

    stAudioArg.RecordType = gpstFMControlData->FMInputType;//Codec_Line1ADC
    stAudioArg.pfPcmCallBack = FmControlPcmInput;
    stAudioArg.SamplesPerBlock = gpstFMControlData->SamplesPerBlock;
    //rk_printf("\n ..SampleRate=%d bufLen = %d Bit=%d SamplesPerBlock=%d LRChannel=%d..\n",gpstFMControlData->SampleRate ,bufLen, gpstFMControlData->Bit, gpstFMControlData->SamplesPerBlock,gpstFMControlData->LRChannel);

    //RKTaskCreate(TASK_ID_AUDIODEVPLAYSERVICE, 0, &stAudioArg, SYNC_MODE);
    RKTaskCreate(TASK_ID_AUDIODEVICESERVICE, 0, &stAudioArg, SYNC_MODE);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FMControlTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:44:15
*******************************************************************************/
_APP_FM_FMCONTROLTASK_INIT_
INIT API rk_err_t FMControlTask_DeInit(void *pvParameters)
{
    FM_APP_CALLBACK * pCur;
    rk_err_t ret;

    gSysConfig.RadioConfig.FmFreq = gpstFMControlData->FmFreq;
    gSysConfig.RadioConfig.FmSaveNum = gpstFMControlData->FmSaveNum;
    memcpy(gSysConfig.RadioConfig.FmFreqArray, gpstFMControlData->FmFreqArray, sizeof(gpstFMControlData->FmFreqArray));
    AudioDev_RealseMainTrack(gpstFMControlData->hAudio);
    FmDevMuteControl(gpstFMControlData->hFmDev, 1);
    FmDevStop(gpstFMControlData->hFmDev);

    while(gpstFMControlData->pfmStateForAPP != NULL)
    {
       pCur = gpstFMControlData->pfmStateForAPP;
       gpstFMControlData->pfmStateForAPP = gpstFMControlData->pfmStateForAPP->pNext;
       rkos_memory_free(pCur);
    }
    printf("FMControlTask_DeInit\n");

    if (gpstFMControlData->hAudio != NULL)
    {
        if (RKDev_Close(gpstFMControlData->hAudio) != RK_SUCCESS)
        {
            rk_printf("gpstFMControlData->hAudio close failure\n");
            return RK_ERROR;
        }
    }

    if (gpstFMControlData->hFmDev != NULL)
    {
        if (RKDev_Close(gpstFMControlData->hFmDev) != RK_SUCCESS)
        {
            rk_printf("gpstFMControlData->hFmDev close failure\n");
            return RK_ERROR;
        }
    }

    if(DeviceTask_DeleteDeviceList(DEVICE_LIST_FM, NULL, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("FM device list delete fail\n");
         while(1);
    }
    if(gpstFMControlData->POutPcmBuf[0] != NULL)
    {
        rkos_memory_free(gpstFMControlData->POutPcmBuf[0]);
        gpstFMControlData->POutPcmBuf[0] = NULL;
    }
    if(gpstFMControlData->POutPcmBuf[1] != NULL)
    {
        rkos_memory_free(gpstFMControlData->POutPcmBuf[1]);
        gpstFMControlData->POutPcmBuf[1] = NULL;
    }

    rkos_semaphore_delete(gpstFMControlData->FMControlReqSem);
    rkos_queue_delete(gpstFMControlData->FMControlAskQueue);
    rkos_queue_delete(gpstFMControlData->FMControlRespQueue);
    rkos_memory_free(gpstFMControlData);
    gpstFMControlData = NULL;
    printf("FMControlTask_DeInit over\n");

    FREQ_ExitModule(FREQ_FM);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMControlTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:44:15
*******************************************************************************/
_APP_FM_FMCONTROLTASK_INIT_
INIT API rk_err_t FMControlTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS*   pFMControlTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_FMCONTROL_ARG * pArg = (RK_TASK_FMCONTROL_ARG *)arg;
    FMCONTROL_TASK_DATA_BLOCK*  pFMControlTaskData;
    uint16 i;

    FREQ_EnterModule(FREQ_FM);

    printf("FMControlTask_Init\n");
    if(pFMControlTask == NULL)
        return RK_PARA_ERR;

    rk_printf("SamplesPerBlock = %d\n",pArg->SamplesPerBlock);
    rk_printf("LRChannel = %d\n",pArg->LRChannel);
    rk_printf("FMInputType  = %d\n",pArg->FMInputType);

    pFMControlTaskData = rkos_memory_malloc(sizeof(FMCONTROL_TASK_DATA_BLOCK));
    memset(pFMControlTaskData, NULL, sizeof(FMCONTROL_TASK_DATA_BLOCK));
    pFMControlTaskData->FMControlAskQueue = rkos_queue_create(1, sizeof(FMCONTROL_ASK_QUEUE));
    pFMControlTaskData->FMControlRespQueue = rkos_queue_create(1, sizeof(FMCONTROL_RESP_QUEUE));
    pFMControlTaskData->FMControlReqSem = rkos_semaphore_create(1,1);

    if ((pFMControlTaskData->FMControlAskQueue
      && pFMControlTaskData->FMControlRespQueue
      && pFMControlTaskData->FMControlReqSem) == 0)
    {
        rkos_queue_delete(pFMControlTaskData->FMControlAskQueue);
        rkos_queue_delete(pFMControlTaskData->FMControlAskQueue);
        rkos_semaphore_delete(pFMControlTaskData->FMControlReqSem);
        rkos_memory_free(pFMControlTaskData);
        return  RK_ERROR;
    }

    pFMControlTaskData->FmStereo = pArg->FmStereo;
    pFMControlTaskData->FmArea = pArg->FmArea;
    pFMControlTaskData->LRChannel = pArg->LRChannel;
    pFMControlTaskData->FMInputType = pArg->FMInputType;
    pFMControlTaskData->SamplesPerBlock = pArg->SamplesPerBlock;

    if(pArg->pfmStateCallBcakForAPP != NULL)
    {
        pFMControlTaskData->pfmStateForAPP = rkos_memory_malloc(sizeof(FM_APP_CALLBACK));
        pFMControlTaskData->pfmStateForAPP->pNext = NULL;
        pFMControlTaskData->pfmStateForAPP->pfmStateForAPP = pArg->pfmStateCallBcakForAPP;
    }
    else
    {
        pFMControlTaskData->pfmStateForAPP = NULL;
    }
    if(gSysConfig.OutputVolume <= 30)
    {
        pFMControlTaskData->playVolume = gSysConfig.OutputVolume;
    }
    else
    {
        pFMControlTaskData->playVolume = 25;
    }
    pFMControlTaskData->EqMode = gSysConfig.MusicConfig.Eq.Mode;
    gpstFMControlData = pFMControlTaskData;

    if (RECORD_QUALITY_HIGH == gSysConfig.RecordConfig.RecordQuality)  //quality record.
    {
        DEBUG("FM_FS = FS_192KHz;");
        gpstFMControlData->SampleRate = I2S_FS_192KHz;
        gpstFMControlData->Bit = 24;
    }
    else
    {
        DEBUG("FM_FS = FS_41KHz;");
        gpstFMControlData->SampleRate = I2S_FS_44100Hz;
        gpstFMControlData->Bit = 16;
    }
    gpstFMControlData->FMRecordState = 0;
    rk_printf("FMControlTask_Init over\n");
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
