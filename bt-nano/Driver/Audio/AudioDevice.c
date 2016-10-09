 /*
********************************************************************************************
*
*                Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Driver\Audio\AudioDevice.c
* Owner: Aaron.sun
* Date: 2014.5.7
* Time: 17:45:29
* Desc: Audio Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.5.7     17:45:29   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "BSP.h"
#include "device.h"
#include "DriverInclude.h"
#include "rockcodec.h"
#include "I2sDevice.h"
#include "RockCodecDevice.h"
#include "audio_globals.h"
#include "AudioDevice.h"
#include "effect.h"
#include "fade.h"
#include "global.h"
#include "SysInfoSave.h"
#include "record_globals.h"
#include "FwAnalysis.h"
#include "ShellTask.h"
#include "TaskPlugin.h"
#include "PowerManager.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define RECORDMOD_PLAY_TEST 0xAA

typedef enum _AUDIO_DEV_STATE
{
    AUDIO_DEV_STATE_PLAY,
    AUDIO_DEV_STATE_PAUSE,
    AUDIO_DEV_STATE_STOP,

}AUDIO_DEV_STATE;

typedef enum
{
    AUDIO_DEV_PLAY = 0x11,

} eAUDIO_STATE_TYPE;


typedef  struct _AUDIODEVPLAYSERVICE_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;
    uint32 mode;
}AUDIODEVPLAYSERVICE_RESP_QUEUE;

typedef  struct _AUDIODEVPLAYSERVICE_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;

}AUDIODEVPLAYSERVICE_ASK_QUEUE;

typedef  struct _AUDIODEVPLAYSERVICE_TASK_DATA_BLOCK
{
    pQueue  AudioDevPlayServiceAskQueue;
    pQueue  AudioDevPlayServiceRespQueue;
    pSemaphore osAudioWriteSem;
    //uint32  AudioPtr;
    uint8  DeleteAudioPlayServerFlag;

    P_PCM_CALLBACK pfPcmCallBack;
}AUDIODEVPLAYSERVICE_TASK_DATA_BLOCK;

#define AUDIO_DEV_NUM 1

typedef enum _TRACK_STATUS
{
    TRACK_UNUSED,
    TRACK_IDLE,
    TRACK_READIED,
    TRACK_NUM

}TRACK_STATUS;

typedef struct _AUDIO_TRACK
{
    uint8 * Track;
    uint32 TrackLen;
    uint32 Status;
    uint32 SampleRate;
    uint32 Bit;
    uint32 Channel;
    uint32 MixMain;
    uint32 MixSub;
    uint8 TrackProcess[AUDIO_PROCESS_MIX];

}AUDIO_TRACK;


typedef struct _AUDIO_DEVICE_CLASS
{
    DEVICE_CLASS stAudioDevice;
    pSemaphore osAudioWriteSem;
    pSemaphore osAudioReadReqSem;
    HDC hCodec;

    uint32 TrackLen;
    uint32 SampleRate;
    uint32 Channel;
    uint32 Bit;
    uint8 * Track;
    uint32 Status;
    uint32 TotalCnt;
    uint32 SuspendCnt;
    uint8 TrackProcess[AUDIO_PROCESS_MIX + TRACK_NO_NUM - 1];

    AUDIO_TRACK stSubTrack[TRACK_NO_NUM - 1];

    uint32  playVolume;

    RKEffect    UserEQ;

    uint32 RecordType;
    P_PCM_CALLBACK pfPcmCallBack;
    uint8 * PcmBuf[2];
    uint32 PcmIndex;
    uint32 SamplesPerBlock;
    uint32 PcmLen;
    uint8  DeleteAudioServerFlag;
    uint8  ReadEnable;

    #ifndef _FADE_PROCESS_
    uint32 FadeType;
    #endif

}AUDIO_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static AUDIODEVPLAYSERVICE_TASK_DATA_BLOCK * gpstAudioDevPlayServiceData;

static AUDIO_DEVICE_CLASS * gpstAudioDevInf;
//static AUDIO_DEVICE_DATA_BLOCK * gpstAudioDeviceData;
static uint8 EqMode[8] = {EQ_HEAVY,EQ_POP,EQ_JAZZ,EQ_UNIQUE,EQ_USER,EQ_USER,EQ_NOR,EQ_BASS};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef CODEC_24BIT //24bit

#if 1 //16bit
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint8 Audiooutptr[1][16*(48*4+48*4)] =
{
    //0
    {
        #include "test01_1K0_48_16bit_16.data"
    },
};

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint32 Audiolength = 16*(48*4);//176 * 32/4; 32bit

#else

_DRIVER_AUDIO_AUDIODEVICE_COMMON_
uint8 __align(4) Audiooutptr[1][16*(48*6+48*6/3)] =
{
    //0
    {
        #include "test01_1K0_48_24bit_16.data"
    },
};
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
uint32 Audiolength = 16*(48*6+48*6/3);
#endif

#else //16bit
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint8 Audiooutptr[1][16*(48*4)] =
{
    //0
    {
        #include "test01_1K0_48_16bit_16.data"
    },
};

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint32 Audiolength = 16*(48*4);//176 * 32/4; 32bit

#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t AudioDevCheckHandler(HDC dev);
rk_err_t AudioDevCheckIdle(HDC dev);
rk_err_t AudioDevResume(HDC dev);
rk_err_t AudioDevSuspend(HDC dev, uint32 Level);
void AudioDevShellPlayTask_Enter(void * arg);
rk_err_t AudioShellAudioStop(HDC dev, uint8 * pstr);
rk_err_t AudioShellAudioPlay(HDC dev, uint8 * pstr);
rk_err_t AudioShellAudioServerStop(HDC dev, uint8 * pstr);
void AudioShellRecordPcmSend(uint8 * buf, uint32 len);
rk_err_t AudioShellAudioServerStart(HDC dev, uint8 * pstr);
rk_err_t AudioShellCodecI2sTRX(HDC dev, uint8 * pstr);
void AudioShellGetPCMAndProcess();
void AudioShellPlayPcmInput(uint8 * buf, uint32 len);
rk_err_t AudioShellDelete(HDC dev,  uint8 * pstr);
rk_err_t AudioShellTest(HDC dev, uint8 * pstr);
rk_err_t AudioShellHelp(HDC dev, uint8 * pstr);
rk_err_t AudioShellPlay(HDC dev,  uint8 * pstr);
rk_err_t AudioShellCreate(HDC dev,  uint8 * pstr);
rk_err_t AudioShellOpen(HDC dev,  uint8 * pstr);
rk_err_t AudioShellPcb(HDC dev,  uint8 * pstr);
rk_err_t AudioDevDeInit(AUDIO_DEVICE_CLASS * pstAudioDev);
rk_err_t AudioDevInit(AUDIO_DEVICE_CLASS * pstAudioDev);
void AudioDevExitType(UINT32 Type);
CodecMode_en_t RecordTypeTransform(uint8 recordType);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioDev_ReadEnable
** Input:HDC dev, uint32 Flag
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.22
** Time: 18:25:26
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_ReadEnable(HDC dev, uint32 Flag)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    pstAudioDev->ReadEnable = Flag;
}

/*******************************************************************************
** Name: AudioDevExitInputType
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.19
** Time: 9:13:00
*******************************************************************************/
COMMON FUN void AudioDevExitType(UINT32 Type)
{
    if(Type > Codec_Standby)
    {
        rk_printf("ERROR: type");
        return;
    }
    if(gpstAudioDevInf->hCodec != NULL)
    {
        RockCodecDev_ExitMode(gpstAudioDevInf->hCodec, Type);
    }
}

/*******************************************************************************
** Name: AudioDevPlayServiceTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.25
** Time: 15:26:07
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDevPlayServiceTask_Resume(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: AudioDevPlayServiceTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.25
** Time: 15:26:07
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDevPlayServiceTask_Suspend(uint32 ObjectID)
{

}
/*******************************************************************************
** Name: AudioDevPlayServiceTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2016.3.25
** Time: 15:26:07
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API void AudioDevPlayServiceTask_Enter(void * arg)
{
    AUDIODEVPLAYSERVICE_ASK_QUEUE AudioDevAskQue;
    AUDIODEVPLAYSERVICE_RESP_QUEUE AudioDevResQue;

    rk_printf("PlayService Enter ......\n");

    while(!gpstAudioDevPlayServiceData->DeleteAudioPlayServerFlag)
    {
        rkos_queue_receive(gpstAudioDevPlayServiceData->AudioDevPlayServiceAskQueue, &AudioDevAskQue, MAX_DELAY);
        switch(AudioDevAskQue.event_type)
        {
            default:
                rk_printf("Play default");
                break;
        }
    }

    while(1)
    {
        rkos_sleep(10000);
    }
}
/*******************************************************************************
** Name: AudioDev_FadeOk
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.5
** Time: 15:42:25
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_FadeOk(HDC dev)
{
    #ifdef _FADE_PROCESS_
    if(FadeIsFinished())
    {
        //rk_printf("0");
        return RK_SUCCESS;
    }
    else
    {
        //rk_printf("-1");
        return RK_ERROR;
    }
    #else
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    if(pstAudioDev->FadeType == FADE_IN)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }

    #endif
}

/*******************************************************************************
** Name: AudioDev_FadeInit
** Input:HDC dev,  uint32 samples, uint32 type
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.5
** Time: 15:40:42
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_FadeInit(HDC dev,  uint32 samples,  uint32 type)
{
    #ifdef _FADE_PROCESS_
    FadeInit(0, samples, type);
    #else
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    pstAudioDev->FadeType = type;
    #endif
    return RK_SUCCESS;

}


/*******************************************************************************
** Name: Bit_Convertor_shift
** Input:short *ppsBuffer, long *plLength,int16 bitpersample
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 13:47:42
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API void Bit_Convertor_shift(short *ppsBuffer, uint32 Length, int16 bitpersample)
{
    int16 i = 0;
    int16 offset = 0 ;
#ifdef CODEC_24BIT
    if (bitpersample == 16)
    {
        long *pOut =(long *) ppsBuffer;
        offset = Length*2;
        for (i=offset-1; i >= 0 ; i--)
        {
            pOut[i] = pOut[i] << 8;
        }
    }
    else if ((bitpersample == 24)||(bitpersample == 32))
    {
        long *pOut =(long *) ppsBuffer;
        offset = Length*2;
        for (i=offset-1; i >= 0 ; i--)
        {
            pOut[i] = pOut[i] << 8;
        }
    }
    //*plLength = *plLength*2;
#endif

}

/*******************************************************************************
** Name: Bit_Convertor_DEC
** Input:short *ppsBuffer, long *plLength,int16 bitpersample
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 13:42:16
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API void Bit_Convertor_DEC(short *ppsBuffer, uint32 Length, int16 bitpersample)
{
    int16 i = 0;
    int16 offset = 0 ;
    //printf("in Bit_Convertor_DEC\n");
#ifdef CODEC_24BIT
    if (bitpersample == 16)
    {
        short *pOut = ppsBuffer;
        long *pOut32 =(long *) ppsBuffer;
        offset = Length*2;
        for (i=offset-1; i >= 0 ; i--)
        {
            pOut32[i] = (pOut[i]<<16)>>8;
        }
    }
    else if (bitpersample == 24)
    {
        char *pOut =(char *) ppsBuffer;
        long *pOut32 =(long *) ppsBuffer;
        offset = (Length)*2;
        for (i=offset; i > 0 ; i--)
        {
            pOut32[i-1]  =( pOut[3*i-1]<<24 | pOut[3*i-2]<<16| pOut[3*i-3]<<8)>>8;
        }
        //printf("bitpersample == 24!!!! \n");

    }
    else if (bitpersample == 32)
    {
        long *pOut =(long *) ppsBuffer;
        offset = Length*2;
        for (i=offset-1; i >= 0 ; i--)
        {
            pOut[i] = pOut[i] >> 8;
        }
    }
#else
    if (bitpersample == 16)
    {
    }
    else if (bitpersample == 24)
    {
        char *pOut = (char *)ppsBuffer;
        offset = Length*2;
        for (i=0; i < offset ; i++)
        {
            pOut[2*i]= pOut[3*i+1];
            pOut[2*i+1]= pOut[3*i+2];
        }
    }
    else if (bitpersample == 32)
    {
        char *pOut = (char *)ppsBuffer;
        offset = Length*2;
        for (i=0; i < offset ; i--)
        {
            pOut[2*i]= pOut[4*i+2];
            pOut[2*i+1]= pOut[4*i+3];
        }
    }
#endif
}

/*******************************************************************************
** Name: AudioDev_SetVol
** Input:HDC dev,  uint32 vol
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
rk_err_t AudioDev_SetVol(HDC dev, uint32 vol)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS * )dev;
    if(pstAudioDev == NULL)
    {
        printf("\n AudioDev_SetVol HDC dev == NULL \n");
        return RK_ERROR;
    }
    //printf("\n AudioDev vol=%d \n", vol);
    pstAudioDev->playVolume = vol;
    RockcodecDev_SetVol(pstAudioDev->hCodec, pstAudioDev->UserEQ.Mode, vol);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDev_RealseMainTrack
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.14
** Time: 10:34:20
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_RealseMainTrack(HDC dev)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    pstAudioDev->Status = TRACK_UNUSED;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDev_SetTrackLen
** Input:HDC dev, uint32 len
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.13
** Time: 17:49:25
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetTrackLen(HDC dev, uint32 len)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    //printf("@@@@@@@@@audiodev sour 16bit len=%d\n",pstAudioDev->TrackLen);
    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }
    pstAudioDev->TrackLen = len;
    //printf("audiodev sour 16bit len=%d\n",pstAudioDev->TrackLen);

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: AudioDev_SetSubTrackLen
** Input:HDC dev, uint32 len
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.13
** Time: 17:49:25
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetSubTrackLen(HDC dev,uint32 TrackNo, uint32 len)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    pstAudioDev->stSubTrack[TrackNo].TrackLen = len;

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: AudioDev_SetTrackProcess
** Input:HDC dev, uint8 mainTrackProcess, uint8 subTrackProcess
** Return: rk_err_t
** Owner:Aron.chen
** Date: 2015.5.29
** Time: 09:49:25
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetTrackProcess(HDC dev, uint32 TrackNo, uint8 subTrackProcess)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    pstAudioDev->stSubTrack[TrackNo].TrackProcess[AUDIO_PROCESS_MIX-1] = subTrackProcess;

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: AudioDev_GetSubTrack
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 15:56:10
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_GetSubTrack(HDC dev)
{

    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    uint32 i;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    //pstAudioDev->stSubTrack[0].Status = TRACK_IDLE;
    //pstAudioDev->stSubTrack[0].TrackProcess[AUDIO_PROCESS_MIX] = AUDIO_PROCESS_MIX;
    for (i = 0; i < TRACK_NO_NUM; i++)
    {
        if (pstAudioDev->stSubTrack[i].Status == TRACK_UNUSED)
        {
            pstAudioDev->stSubTrack[i].Status = TRACK_IDLE;

            return i;
        }
    }

    return RK_ERROR;

}

/*******************************************************************************
** Name: AudioDev_GetMainTrack
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 15:55:01
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_GetMainTrack(HDC dev)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if (pstAudioDev->Status == TRACK_UNUSED)
    {
        pstAudioDev->Status = TRACK_IDLE;

        return RK_SUCCESS;
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: AudioDev_SetMixScale
** Input:HDC dev ,uint32 TrackNo, uint32 MixMain, uint32 MixSub
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:52:05
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetMixScale(HDC dev ,uint32 TrackNo, uint32 MixMain, uint32 MixSub)
{
    uint64 temp;
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if ((TrackNo == 0) || (TrackNo >= TRACK_NO_NUM))
    {
        return RK_ERROR;
    }
    temp = (uint64 )(MixMain & 0xffff) + (uint64 )(MixSub & 0xffff);

    if ((temp > 0xffff) || (temp < 1))
    {
        return RK_ERROR;
    }

    MixMain =(uint32 )(((MixMain << 8) + (temp/2)) / temp);
    MixSub = (uint32 )(((MixSub << 8) + (temp/2)) / temp);

    pstAudioDev->stSubTrack[TrackNo].MixMain = MixMain;// - 1?
    pstAudioDev->stSubTrack[TrackNo].MixSub = MixSub;// - 1?

    return RK_SUCCESS;

}


/*******************************************************************************
** Name: AudioDev_SetBit
** Input:HDC dev, uint32 Bit
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:47:28
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetBit(HDC dev, uint32 TrackNo, uint32 Bit)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if (TrackNo >= TRACK_NO_NUM)
    {
        return RK_ERROR;
    }

    if (TrackNo > 0)
    {
        pstAudioDev->stSubTrack[TrackNo - 1].Bit = Bit;
    }
    else
    {
        pstAudioDev->Bit = Bit;
    }

#if 0
    stAudioDevArg.stI2sDevArg.i2smode = I2S_SLAVE_MODE;//;I2S_SLAVE_MODE
    stAudioDevArg.stI2sDevArg.i2sCS = I2S_IN; //I2S_EXT
    stAudioDevArg.stI2sDevArg.I2S_FS = pstAudioDev->SampleRate;
    stAudioDevArg.stI2sDevArg.BUS_FORMAT = I2S_FORMAT;//I2S_FORMAT;

    stAudioDevArg.stI2sDevArg.Data_width = Bit;

    stAudioDevArg.stI2sDevArg.I2S_Bus_mode = I2S_NORMAL_MODE;

    stAudioDevArg.stCodecArg.SampleRate = pstAudioDev->SampleRate;
    stAudioDevArg.stCodecArg.Codecmode = Codec_DACoutHP;
    AudioDev_I2S_Acodec_Mode(pstAudioDev, &stAudioDevArg);
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDev_SetChannel
** Input:HDC dev, uint32 bit
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:46:10
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetChannel(HDC dev, uint32 TrackNo, uint32 Channel)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if (TrackNo >= TRACK_NO_NUM)
    {
        return RK_ERROR;
    }

    if (TrackNo > 0)
    {
        pstAudioDev->stSubTrack[TrackNo - 1].Channel = Channel;
    }
    else
    {
        pstAudioDev->Channel = Channel;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDev_SetSampleRate
** Input:HDC dev, uint32 SamleRate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:43:09
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetRxSampleRate(HDC dev, uint32 TrackNo, uint32 SamleRate)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if (TrackNo >= TRACK_NO_NUM)
    {
        return RK_ERROR;
    }

    if (TrackNo > 0)
    {
        pstAudioDev->stSubTrack[TrackNo - 1].SampleRate = SamleRate;
    }
    else
    {
        RockcodecDev_RxSetRate(pstAudioDev->hCodec, SamleRate);//codec SampleRate
        pstAudioDev->SampleRate = SamleRate;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: AudioDev_SetSampleRate
** Input:HDC dev, uint32 SamleRate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:43:09
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetSampleRate(HDC dev, uint32 TrackNo, uint32 SamleRate)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if (TrackNo >= TRACK_NO_NUM)
    {
        return RK_ERROR;
    }

    if (TrackNo > 0)
    {
        pstAudioDev->stSubTrack[TrackNo - 1].SampleRate = SamleRate;
    }
    else
    {
        RockcodecDev_SetRate(pstAudioDev->hCodec, SamleRate);//codec SampleRate
        pstAudioDev->SampleRate = SamleRate;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: AudioDev_SetModel
** Input:HDC dev,  uint32 TrackNo, uint32 ProcessNo, uint32 ProcessPos
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:38:19
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_SetModel(HDC dev, uint32 TrackNo, uint32 ProcessNo, uint32 ProcessPos)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    if (pstAudioDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if (TrackNo >= TRACK_NO_NUM)
    {
        return RK_ERROR;
    }

    if (TrackNo > 0)
    {
        pstAudioDev->stSubTrack[TrackNo - 1].TrackProcess[ProcessNo] = ProcessPos;
    }
    else
    {
        pstAudioDev->TrackProcess[ProcessNo] = ProcessPos;
    }

    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 16:44:35
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON FUN rk_err_t AudioDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < 1; i++)
    {
        if(gpstAudioDevInf == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}


/*******************************************************************************
** Name: AudioSetVolume
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:46:40
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON FUN void AudioDev_SetVolume(void)
{
    RockcodecDev_SetVol(gpstAudioDevInf->hCodec,
        gpstAudioDevInf->UserEQ.Mode, gpstAudioDevInf->playVolume);
}

/*******************************************************************************
** Name: AudioDev_SetEQ
** Input:HDC dev, uint32 userEQMod, uint32 vol
** Return: void
** Owner:cjh
** Date: 2015.1.1
** Time: 9:53:09
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API void AudioDev_SetEQ(HDC dev, uint32 userEQMod)
{
    AUDIO_EQ_ARG eqARG;
    AUDIO_DEVICE_CLASS *  hAudio = (AUDIO_DEVICE_CLASS * )dev;
    RKEffect *pEffect;
    if(hAudio == NULL)
    {
        printf("ERROR:##hAudio NULL\n");
    }
    pEffect = (RKEffect *)(&(hAudio->UserEQ));

    hAudio->UserEQ.Mode = userEQMod;

    eqARG.audioLen = &hAudio->TrackLen;
    eqARG.playVolume = &hAudio->playVolume;

#ifdef  _RK_EQ_
    if (pEffect->Mode == EQ_BASS)
    {
        if (*(eqARG.playVolume) <= 27 )
        {
            if (VOLTAB_CONFIG == VOL_General)
                pEffect->max_DbGain = 12;
            else
                pEffect->max_DbGain = 12;
        }
        else if (*(eqARG.playVolume) == 28 )
        {
            if (VOLTAB_CONFIG == VOL_General)
                pEffect->max_DbGain = 10;
            else
                pEffect->max_DbGain = 10;
        }
        else if (*(eqARG.playVolume) == 29 )
        {
            if (VOLTAB_CONFIG == VOL_General)
                pEffect->max_DbGain = 8;
            else
                pEffect->max_DbGain = 8;
        }
        else if (*(eqARG.playVolume) == 30 )
        {
            if (VOLTAB_CONFIG == VOL_General)
                pEffect->max_DbGain = 6;
            else
                pEffect->max_DbGain = 6;
        }

    }
    //printf("max_DbGain = %d userEQMod=%d playVolume=%d\n",pEffect->max_DbGain,userEQMod, *(eqARG.playVolume));
    EffectAdjust(pEffect, &eqARG ,hAudio->SampleRate);
    //printf("EQ set EffectAdjust over\n");
    AudioDev_SetVol(dev, hAudio->playVolume);
    //printf("EQ set volume over\n");
#endif
}


/*******************************************************************************
** Name: AudioDev_Write
** Input:HDC dev , uint32 TrackNo, uint8 * buf, uint32 len
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.9
** Time: 13:54:20
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDev_Write(HDC dev , uint32 TrackNo, uint8 * buf)
{
    AUDIO_DEVICE_CLASS * pstWriteAudioDev = (AUDIO_DEVICE_CLASS *)dev;

    uint32 i, j, mixLen;
    uint16 sacleMain, scaleSub;
    uint64 temp;
    uint8 *pMainTrack ,*pSubTrack;
    uint32 *p32MainTrack;

    if (pstWriteAudioDev == NULL)
    {
        printf("pstWriteAudioDev == NULL\n");
        return RK_PARA_ERR;
    }

    if (TrackNo == TRACK_NO_MAIN)
    {
        if (pstWriteAudioDev->Status == TRACK_UNUSED)
        {
            return RK_ERROR;
        }
    }
    else if ((TrackNo >= 0) && (TrackNo < TRACK_NO_NUM))
    {
        if (pstWriteAudioDev->stSubTrack[TrackNo].Status == TRACK_UNUSED)// - 1?
        {
            return RK_ERROR;
        }
    }
    else
    {
        return RK_PARA_ERR;
    }
    if (TrackNo == TRACK_NO_MAIN)
    {
        //call hCode and suspend
        pstWriteAudioDev->Track = buf;
#if 0
        pMainTrack = pstWriteAudioDev->Track;
        for (i = pstWriteAudioDev->TrackLen/3*2; i < pstWriteAudioDev->TrackLen; i++)//pstWriteAudioDev->TrackLen
        {
            rk_printf("##send for printf Track NO.%d = 0x%x \n", i, *pMainTrack);
            pMainTrack++;
        }

#endif

#if 0
        pMainTrack = pstWriteAudioDev->Track;
        for (i = 0; i < 10; i++)//pstWriteAudioDev->TrackLen
        {
            rk_printf("!#!#send for printf Track NO.%d = 0x%x \n", i, *pMainTrack);
            pMainTrack++;
        }

#endif

        //AudioDev_SetEQ(pstWriteAudioDev, EQ_HEAVY); //SET EQ befor set vol
        //AudioDev_SetVol(pstWriteAudioDev, 25);
        //printf("16bit or 24bit Convertor %d\n",pstWriteAudioDev->Bit);
        if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
        {
            //printf("\nWIDTH24 Convertor_DEC len=%d len/6=%d\n",pstWriteAudioDev->TrackLen,pstWriteAudioDev->TrackLen/(3*2));
            Bit_Convertor_DEC((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/6, 24);//pstWriteAudioDev->Channel
        }
        else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
        {
            Bit_Convertor_DEC((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2), 16);//pstWriteAudioDev->Channel
        }
        else if(pstWriteAudioDev->Bit == 32)
        {
            Bit_Convertor_DEC((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(4*2), 32);//pstWriteAudioDev->Channel
        }
        else
        {
            //printf("ERROR:sour music no 16bit or 24bit \n");
        }
        //printf("~~~len= %d \n",gpstAudioDevInf->TrackLen);

#ifdef _RK_EQ_
        if(EQ_NOR != pstWriteAudioDev->UserEQ.Mode)
        {
            /*audio effect process.*/
            #ifdef CODEC_24BIT
            //if(gpstAudioDevInf->SampleRate <= 96000)
            if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
            {
                EffectProcess((long*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(3*2),\
                              &(pstWriteAudioDev->UserEQ));
            }
            else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
            {
                //printf("16BIT TrackLen=%d\n",gpstAudioDevInf->TrackLen);
                EffectProcess((long*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2),\
                              &(pstWriteAudioDev->UserEQ));
            }
            else if(pstWriteAudioDev->Bit == 32)
            {
                //printf("32BIT TrackLen=%d\n",gpstAudioDevInf->TrackLen);
                EffectProcess((long*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(4*2),\
                              &(pstWriteAudioDev->UserEQ));
            }
            #else
                EffectProcess((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2),\
                              &(pstWriteAudioDev->UserEQ));
            #endif
        }
#endif //end #ifdef _RK_EQ_

        if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
        {
            //printf("\nWIDTH24 Convertor_shift\n");
            Bit_Convertor_shift((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(3*2), 24);
        }
        else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
        {
            //printf("@@@@convertor 16bit len=%d\n",pstWriteAudioDev->TrackLen/(2*2));
            Bit_Convertor_shift((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2), 16);
        }
        else if(pstWriteAudioDev->Bit == 32)
        {
            //printf("@@@@convertor 32bit len=%d\n",pstWriteAudioDev->TrackLen/(4*2));
            Bit_Convertor_shift((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(4*2), 32);
        }
        else
        {
            //printf("ERROR3:sour music no 16bit or 24bit or 32bit\n");
        }
        //AudioDev_SetTrackLen(pstWriteAudioDev, pstWriteAudioDev->TrackLen*4);
        //printf("@@@@3sour 16bit len=%d\n",pstWriteAudioDev->TrackLen);

#if 0
        p32MainTrack = (uint32 *)pstWriteAudioDev->Track;
        for (i = 16300/4 ; i < pstWriteAudioDev->TrackLen/4; i++)
        {
            rk_printf("******send for printf Track NO.%d = 0x%x \n", i, *p32MainTrack);
            p32MainTrack++;
        }

#endif

        for (j=1; j<TRACK_NO_NUM; j++)
        {
            if ((pstWriteAudioDev->stSubTrack[j].Status == TRACK_READIED) && (pstWriteAudioDev->Bit == pstWriteAudioDev->stSubTrack[j].Bit))
            {

                // first / or *
                sacleMain = pstWriteAudioDev->stSubTrack[j].MixMain  ;
                scaleSub = pstWriteAudioDev->stSubTrack[j].MixSub  ;

                // To prevent the Sub pointer overflow
                mixLen = (pstWriteAudioDev->TrackLen < pstWriteAudioDev->stSubTrack[j].TrackLen)\
                         ? pstWriteAudioDev->TrackLen : pstWriteAudioDev->stSubTrack[j].TrackLen;

                // ++
                pMainTrack = pstWriteAudioDev->Track;
                pSubTrack = pstWriteAudioDev->stSubTrack[j].Track;
                for (i = 0; i < mixLen; i++)
                {
#if 0
                    rk_printf("**************\n");
                    rk_printf("++++++pMainTrack = 0x%x \n",*pMainTrack);
                    rk_printf("++++++pSubTrack = 0x%x \n",*pSubTrack);
                    rk_printf("==sumMainTcak = 0x%x \n",(((*pMainTrack) * sacleMain)>>8));
                    rk_printf("==sumSubTcak = 0x%x \n",(((*pSubTrack) * scaleSub) >> 8));
                    rk_printf("**************\n");
#endif
                    *pMainTrack =(uint8)((((*pMainTrack) * sacleMain)>>8) + (((*pSubTrack) * scaleSub >> 8)));
                    //rk_printf("~~~Sum Track NO.%d = 0x%x \n", i, *pMainTrack);

                    i++;
                    if (i >= pstWriteAudioDev->TrackLen)
                        break;
                    pMainTrack++;
                    pSubTrack++;
                    *pMainTrack =(uint8)((((*pMainTrack) * sacleMain)>>8) + (((*pSubTrack) * scaleSub >> 8)));
                    if (i >= pstWriteAudioDev->TrackLen)
                        break;
                    pMainTrack++;
                    pSubTrack++;

                }

                pstWriteAudioDev->stSubTrack[j].Status = TRACK_IDLE; //TRACK_UNUSED for next select
            }

        }

#if 0
        pMainTrack = pstWriteAudioDev->Track;
        for (i = 0; i < pstWriteAudioDev->TrackLen; i++)
        {
            rk_printf("~~~send for printf Track NO.%d = 0x%x \n", i, *pMainTrack);
            pMainTrack++;
        }

#endif

#ifdef _FADE_PROCESS_
    #ifdef CODEC_24BIT
        if (AudioDev_FadeOk(pstWriteAudioDev) != RK_SUCCESS)
        {
            if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
            {
                FadeProcess((long*)pstWriteAudioDev->Track,pstWriteAudioDev->TrackLen/(3*2));
            }
            else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
            {
                FadeProcess((long*)pstWriteAudioDev->Track,pstWriteAudioDev->TrackLen/(2*2));
            }
            else if(pstWriteAudioDev->Bit == 32)
            {
                FadeProcess((long*)pstWriteAudioDev->Track,pstWriteAudioDev->TrackLen/(4*2));
            }
        }
    #else
        FadeProcess((long*)pstWriteAudioDev->Track,pstWriteAudioDev->TrackLen/(2*2));
    #endif

#endif


#ifdef CODEC_24BIT
        if (gpstAudioDevInf->Bit == (I2S_DATA_WIDTH24+1))
        {
            //printf("24bit_Write len=%d\n",pstWriteAudioDev->TrackLen*4/3);
            RockcodecDev_Write(pstWriteAudioDev->hCodec, (uint8 *)pstWriteAudioDev->Track, (pstWriteAudioDev->TrackLen*4/3), ASYNC_MODE);
        }
        else if(gpstAudioDevInf->Bit == (I2S_DATA_WIDTH16+1))
        {
            //printf("16bit_Write len=%d\n",pstWriteAudioDev->TrackLen*2);
            RockcodecDev_Write(pstWriteAudioDev->hCodec, (uint8 *)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen*2, ASYNC_MODE);
        }
        else if(pstWriteAudioDev->Bit == 32)
        {
            //printf("sour music  132bit\n");
            RockcodecDev_Write(pstWriteAudioDev->hCodec, (uint8 *)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen, ASYNC_MODE);
        }
        else
        {
            rk_printf("ERROR2:sour music no 16bit or 24bit or 32bit\n");
        }
#else
        RockcodecDev_Write(pstWriteAudioDev->hCodec, (uint8 *)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen, ASYNC_MODE);
#endif
        //printf(" RockcodecDev_Write OK\n");

        //WM8987Dev_Write(pstAudioDev->hCodec, pstAudioDev->Track, pstAudioDev->TrackLen, ASYNC_MODE);
        for (i = 0; i < pstWriteAudioDev->SuspendCnt;)
        {
            rkos_semaphore_give(pstWriteAudioDev->osAudioWriteSem);
            pstWriteAudioDev->SuspendCnt--;
        }
    }
    else
    {

        pstWriteAudioDev->stSubTrack[TrackNo].Status = TRACK_READIED;// - 1?

        for (i = 0; i < AUDIO_PROCESS_MIX; i++)
        {

            switch (pstWriteAudioDev->stSubTrack[TrackNo].TrackProcess[i])// -1?
            {
                case AUDIO_PROCESS_CLOSE:
                    break;

                case AUDIO_PROCESS_EQ:
                    break;

                case AUDIO_PROCESS_FILTER:
                    break;

                case AUDIO_PROCESS_FADE_IN:
                    break;

                case AUDIO_PROCESS_FADE_OUT:
                    break;

                case AUDIO_PROCESS_CHANGE_SPEED:
                    break;

                case AUDIO_PROCESS_CHANGE_TONE:
                    break;

                case AUDIO_PROCESS_ECHO_CANCEL:
                    break;

                case AUDIO_PROCESS_MIX:
                    goto sub_over;

            }

        }

sub_over:

        //rkos_enter_critical();

        pstWriteAudioDev->SuspendCnt++;

        if (pstWriteAudioDev->SuspendCnt >= pstWriteAudioDev->TotalCnt)
        {
            //mix sound
            for (i = 0 ; i < (AUDIO_PROCESS_MIX + TRACK_NO_NUM - 1); i++)
            {
                switch (pstWriteAudioDev->stSubTrack[TrackNo].TrackProcess[i])// -1?
                {
                    case AUDIO_PROCESS_CLOSE:
                        break;

                    case AUDIO_PROCESS_EQ:
                        break;

                    case AUDIO_PROCESS_FILTER:
                        break;

                    case AUDIO_PROCESS_FADE_IN:
                        break;

                    case AUDIO_PROCESS_FADE_OUT:
                        break;

                    case AUDIO_PROCESS_CHANGE_SPEED:
                        break;

                    case AUDIO_PROCESS_CHANGE_TONE:
                        break;

                    case AUDIO_PROCESS_ECHO_CANCEL:
                        break;

                    case AUDIO_PROCESS_MIX:

                        pstWriteAudioDev->stSubTrack[TrackNo].Track = buf;
#if 0//ndef _AudioTest_

                    //printf("16bit or 24bit Convertor\n");
                    if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
                    {
                        //AudioDev_SetTrackLen(pstWriteAudioDev, pstWriteAudioDev->TrackLen/(3*2));//(3*2)
                        printf("sour 24bit len=%d\n",pstWriteAudioDev->TrackLen);
                        Bit_Convertor_DEC((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(3*2), 24);
                    }
                    else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
                    {
                        printf("@@@@1sour 16bit len=%d\n",pstWriteAudioDev->TrackLen);
                        //AudioDev_SetTrackLen(pstWriteAudioDev, pstWriteAudioDev->TrackLen/(2*2));//(2*2)
                        //printf("@@@@2sour 16bit len=%d\n",pstWriteAudioDev->TrackLen);
                        Bit_Convertor_DEC((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2), 16);
                        //printf("Convertor over\n");
                    }
                    else
                    {
                        printf("ERROR:sour music no 16bit or 24bit \n");
                    }
                    //printf("~~~len= %d \n",gpstAudioDevInf->TrackLen);
#ifdef _RK_EQ_
                    if(EQ_NOR != pstWriteAudioDev->UserEQ.Mode)
                    {
                        /*audio effect process.*/
                        #ifdef CODEC_24BIT
                        //if(gpstAudioDevInf->SampleRate <= 96000)
                        printf("16BIT TrackLen=%d\n",gpstAudioDevInf->TrackLen);
                        if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
                        {
                            EffectProcess((long*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(3*2),\
                                          &(pstWriteAudioDev->UserEQ));
                        }
                        else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
                        {
                            EffectProcess((long*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2),\
                                          &(pstWriteAudioDev->UserEQ));
                        }

                        #else
                            EffectProcess((long*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2),\
                                          &(pstWriteAudioDev->UserEQ));
                        #endif
                    }
#endif //end #ifdef _RK_EQ_

                    if (pstWriteAudioDev->Bit == (I2S_DATA_WIDTH24+1))
                    {
                        Bit_Convertor_shift((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(3*2), 24);
                    }
                    else if(pstWriteAudioDev->Bit == (I2S_DATA_WIDTH16+1))
                    {
                        printf("@@@@Convertor_shift len=%d\n",pstWriteAudioDev->TrackLen);
                        Bit_Convertor_shift((short*)pstWriteAudioDev->Track, pstWriteAudioDev->TrackLen/(2*2), 16);
                    }
                    else
                    {
                        printf("ERROR2:sour music no 16bit or 24bit \n");
                    }

                    //AudioDev_SetTrackLen(pstWriteAudioDev, pstWriteAudioDev->TrackLen*4);
                    //printf("@@@@3sour 16bit len=%d\n",pstWriteAudioDev->TrackLen);
#endif //end #ifndef AudioTest

                        //pstWriteAudioDev->stSubTrack[TrackNo].Status = TRACK_IDLE;//TRACK_READIED;

                        break;

                }

            }

        }

        //rkos_exit_critical();

        //suspend
        rkos_semaphore_take(pstWriteAudioDev->osAudioWriteSem, MAX_DELAY);

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
** Name: AudioDevPlayServiceTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.25
** Time: 15:26:07
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT API rk_err_t AudioDevPlayServiceTask_DeInit(void *pvParameters)
{
    gpstAudioDevPlayServiceData->DeleteAudioPlayServerFlag = 1;
    rkos_semaphore_take(gpstAudioDevPlayServiceData->osAudioWriteSem, MAX_DELAY);
    rkos_queue_delete(gpstAudioDevPlayServiceData->AudioDevPlayServiceAskQueue);
    rkos_queue_delete(gpstAudioDevPlayServiceData->AudioDevPlayServiceRespQueue);
    rkos_semaphore_give(gpstAudioDevPlayServiceData->osAudioWriteSem);
    rkos_semaphore_delete(gpstAudioDevPlayServiceData->osAudioWriteSem);
    rkos_memory_free(gpstAudioDevPlayServiceData);

}
/*******************************************************************************
** Name: AudioDevPlayServiceTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.25
** Time: 15:26:07
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT API rk_err_t AudioDevPlayServiceTask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS * pAudioDevPlayServiceTask = (RK_TASK_CLASS*)pvParameters;
    AUDIO_DEV_ARG * pstAudioArg = (AUDIO_DEV_ARG * )arg;
    rk_err_t ret;
    uint32 bufLen;

    AUDIODEVPLAYSERVICE_TASK_DATA_BLOCK*  pAudioDevPlayServiceTaskData;
    if (pAudioDevPlayServiceTask == NULL)
    {
        return RK_PARA_ERR;
    }

    pAudioDevPlayServiceTaskData = rkos_memory_malloc(sizeof(AUDIODEVPLAYSERVICE_TASK_DATA_BLOCK));
    if(pAudioDevPlayServiceTaskData == NULL)
    {
        return RK_ERROR;
    }
    memset(pAudioDevPlayServiceTaskData, NULL, sizeof(AUDIODEVPLAYSERVICE_TASK_DATA_BLOCK));

    pAudioDevPlayServiceTaskData->AudioDevPlayServiceAskQueue = rkos_queue_create(1, sizeof(AUDIODEVPLAYSERVICE_ASK_QUEUE));
    pAudioDevPlayServiceTaskData->AudioDevPlayServiceRespQueue = rkos_queue_create(1, sizeof(AUDIODEVPLAYSERVICE_RESP_QUEUE));
    pAudioDevPlayServiceTaskData->osAudioWriteSem = rkos_semaphore_create(1,1);

    if((pAudioDevPlayServiceTaskData->AudioDevPlayServiceAskQueue == NULL)
        || (pAudioDevPlayServiceTaskData->AudioDevPlayServiceRespQueue == NULL)
        || (pAudioDevPlayServiceTaskData->osAudioWriteSem == NULL))
    {
        rk_printf("ERROR");
        rkos_queue_delete(pAudioDevPlayServiceTaskData->AudioDevPlayServiceAskQueue);
        rkos_queue_delete(pAudioDevPlayServiceTaskData->AudioDevPlayServiceRespQueue);
        rkos_semaphore_delete(pAudioDevPlayServiceTaskData->osAudioWriteSem);
        rkos_memory_free(pAudioDevPlayServiceTaskData);
        return RK_ERROR;
    }
    gpstAudioDevPlayServiceData = pAudioDevPlayServiceTaskData;
    gpstAudioDevPlayServiceData->DeleteAudioPlayServerFlag = 0;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDev_Create
** Input:void * Args
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT API HDC AudioDev_Create(uint32 DevID, void *arg)
{
    AUDIO_DEV_ARG * pstAudioArg = (AUDIO_DEV_ARG * )arg;
    DEVICE_CLASS * pstDev;
    AUDIO_DEVICE_CLASS * pstAudioDev;

    if (pstAudioArg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    pstAudioDev =  rkos_memory_malloc(sizeof(AUDIO_DEVICE_CLASS));
    if (pstAudioDev == NULL)
    {
        return pstAudioDev;
    }

    memset(pstAudioDev, 0, sizeof(AUDIO_DEVICE_CLASS));

    pstAudioDev->osAudioWriteSem = rkos_semaphore_create(1,0);
    pstAudioDev->osAudioReadReqSem = rkos_semaphore_create(1,1);

    if ((pstAudioDev->osAudioWriteSem
           && pstAudioDev->osAudioReadReqSem) == 0)
    {
        rkos_semaphore_delete(pstAudioDev->osAudioWriteSem);
        rkos_semaphore_delete(pstAudioDev->osAudioReadReqSem);

        rkos_memory_free(pstAudioDev);
        return (HDC) RK_ERROR;
    }


    pstDev = (DEVICE_CLASS *)pstAudioDev;

    pstDev->suspend = AudioDevSuspend;
    pstDev->resume  = AudioDevResume;
    pstDev->Idle1EventTime = 10 * PM_TIME;
    pstDev->SuspendMode = ENABLE_MODE;

    pstAudioDev->hCodec = pstAudioArg->hCodec;
    pstAudioDev->SampleRate = pstAudioArg->SampleRate;
    pstAudioDev->Bit = pstAudioArg->Bit;
    pstAudioDev->playVolume = pstAudioArg->Vol;
#ifdef _RK_EQ_
    pstAudioDev->UserEQ.Mode = pstAudioArg->EQMode;
#endif

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_AUDIO_DEV, SEGMENT_OVERLAY_ALL);
#endif
    //device init...
    if (AudioDevInit(pstAudioDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstAudioDev->osAudioWriteSem);
        rkos_semaphore_delete(pstAudioDev->osAudioReadReqSem);
        gpstAudioDevInf = NULL;
        rkos_memory_free(pstAudioDev);
        return (HDC) RK_ERROR;
    }
    gpstAudioDevInf = pstAudioDev;
    //rk_printf(" ##audio inti over\n");
    return (HDC)pstDev;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioDev_Delete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT API rk_err_t AudioDev_Delete(uint32 DevID, void * arg)
{
    AUDIO_DEV_ARG *stParg = (AUDIO_DEV_ARG *)arg;
#if 1
    stParg->hCodec = gpstAudioDevInf->hCodec;
    AudioDevDeInit(gpstAudioDevInf);

    //Free AudioDev memory...
    rkos_semaphore_delete(gpstAudioDevInf->osAudioWriteSem);
    rkos_semaphore_delete(gpstAudioDevInf->osAudioReadReqSem);

    rkos_memory_free(gpstAudioDevInf);
    gpstAudioDevInf = NULL;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_AUDIO_DEV);
    FW_RemoveSegment(SEGMENT_ID_FADE_MODULE);
#endif

#ifdef _RK_EQ_
    Effect_Delete();
#endif

#endif

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: AudioDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT FUN rk_err_t AudioDevInit(AUDIO_DEVICE_CLASS * pstAudioDev)
{
    //create audio driver model
    uint32 i;
    for (i = 0; i < TRACK_NUM - 1; i++)
    {
        pstAudioDev->stSubTrack[i].TrackProcess[0] = AUDIO_PROCESS_MIX_0;
    }
    pstAudioDev->UserEQ.max_DbGain = gSysConfig.MusicConfig.Eq.max_DbGain;

    //set codec samplerate
    RockcodecDev_SetRate(pstAudioDev->hCodec, pstAudioDev->SampleRate);

#ifdef _FADE_PROCESS_
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_FADE_MODULE, SEGMENT_OVERLAY_CODE);
#endif
#endif

    AudioDev_FadeInit((HDC)pstAudioDev, pstAudioDev->SampleRate/2,FADE_IN);

#ifdef _RK_EQ_
    EffectInit();
    EQ_ClearBuff();
    AudioDev_SetEQ(pstAudioDev, EQ_NOR); //SET EQ befor set vol
#endif

    //printf("AudioDevInit set vol\n");
    AudioDev_SetVol(pstAudioDev, pstAudioDev->playVolume);

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 16:38:43
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT FUN rk_err_t AudioDevResume(HDC dev)
{
     AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
     if(AudioDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     pstAudioDev->stAudioDevice.State = DEV_STATE_WORKING;
     RKDev_Resume(pstAudioDev->hCodec);
     return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDevSuspend
** Input:HDC dev, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 16:38:13
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT FUN rk_err_t AudioDevSuspend(HDC dev, uint32 Level)
{
    AUDIO_DEVICE_CLASS * pstAudioDev = (AUDIO_DEVICE_CLASS *)dev;
    if(AudioDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstAudioDev->stAudioDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstAudioDev->stAudioDevice.State = DEV_SATE_IDLE2;
    }

    RKDev_Suspend(pstAudioDev->hCodec);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_INIT_
INIT FUN rk_err_t AudioDevDeInit(AUDIO_DEVICE_CLASS * pstAudioDev)
{
    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: AudioDevService_Task_Enter
** Input:void
** Return: void
** Owner:chad.ma
** Date: 20161.12
** Time: 10:19:35
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API void AudioDevService_Task_Enter(void)
{
    uint32      recordState;
    HDC         hcodec;

    //AudioDev_FadeInit(gpstAudioDevInf, gpstAudioDevInf->SampleRate/2,FADE_IN);

    while(!gpstAudioDevInf->DeleteAudioServerFlag)
    {
        if(gpstAudioDevInf->ReadEnable)
        {
            rkos_semaphore_take(gpstAudioDevInf->osAudioReadReqSem, MAX_DELAY);

            if(gpstAudioDevInf->PcmLen)
            {
                RockcodecDev_Read(gpstAudioDevInf->hCodec,
                    gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex], gpstAudioDevInf->PcmLen, ASYNC_MODE);
            }


            rkos_semaphore_give(gpstAudioDevInf->osAudioReadReqSem);
            gpstAudioDevInf->PcmIndex = 1 - gpstAudioDevInf->PcmIndex;
            if(gpstAudioDevInf->pfPcmCallBack != NULL)
            {
                //if(AudioDev_FadeOk(gpstAudioDevInf) != RK_SUCCESS)
                {
                    //FadeProcess(gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex], gpstAudioDevInf->SamplesPerBlock);
                }
                gpstAudioDevInf->pfPcmCallBack(gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex], gpstAudioDevInf->SamplesPerBlock);
            }
        }
        else
        {
            rkos_sleep(10);
        }


    }

    while(1)
    {
        rkos_sleep(10000);
    }
}


/*******************************************************************************
** Name: AudioDevService_Task_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:chad.ma
** Date: 20161.12
** Time: 10:19:35
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDevService_Task_DeInit(void *pvParameters)
{
    gpstAudioDevInf->DeleteAudioServerFlag = 1;
    printf("...audio device ask_DeInit...\n");
    rkos_semaphore_take(gpstAudioDevInf->osAudioReadReqSem, MAX_DELAY);

    AudioDevExitType(RecordTypeTransform(gpstAudioDevInf->RecordType));
    #ifndef _BROAD_LINE_OUT_
    //AudioDevExitType(Codec_DACoutHP);
    #else
    //AudioDevExitType(Codec_DACoutLINE);
    #endif

    RKDev_Close(gpstAudioDevInf->hCodec);

    rkos_memory_free(gpstAudioDevInf->PcmBuf[0]);
    rkos_memory_free(gpstAudioDevInf->PcmBuf[1]);

    gpstAudioDevInf->PcmLen = 0;

    rkos_semaphore_give(gpstAudioDevInf->osAudioReadReqSem);

    printf("...audio device service delete ok...");

    return RK_SUCCESS;
}

_DRIVER_AUDIO_AUDIODEVICE_COMMON_
static CodecMode_en_t RecordTypeTransform(uint8 recordType)
{
    CodecMode_en_t mode;

    if(recordType < RECORD_TYPE_MIC_STERO
        || recordType > RECORD_TYPE_NULL)
    {
        printf("Error RecordType %d \n",recordType);
    }

    switch(recordType)
    {
        case RECORD_TYPE_MIC_STERO:
            mode = Codec_MicStero;
            break;

        case RECORD_TYPE_LINEIN1:
            mode = Codec_Line1ADC;
            break;

        case RECORD_TYPE_LINEIN2:
            mode = Codec_Line2ADC;
            break;

        case RECORD_TYPE_MIC1_MONO:
            mode = Codec_Mic1Mono;
            break;

        case RECORD_TYPE_MIC2_MONO:
            mode = Codec_Mic2Mono;
            break;

        case RECORD_TYPE_NULL:
            mode = Codec_Standby;
            break;
    }

    return mode;
}

/*******************************************************************************
** Name: AudioDevService_Task_Init
** Input:void *pvParameters
** Return: rk_err_t
** Owner:chad.ma
** Date: 20161.12
** Time: 10:19:35
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_COMMON_
COMMON API rk_err_t AudioDevService_Task_Init(void *pvParameters,void *arg)
{
    AUDIO_DEV_ARG * pstAudioArg = (AUDIO_DEV_ARG * )arg;
    rk_err_t ret;

    //rk_printf("SampleRare = %d old=%d\n",pstAudioArg->SampleRate, gpstAudioDevInf->SampleRate);
    //rk_printf("DataWidth  = %d\n",pstAudioArg->Bit);
    //rk_printf("RecordType  = %d\n",pstAudioArg->RecordType);
    //rk_printf("SamplesPerBlock  = %d\n",pstAudioArg->SamplesPerBlock);
    if(pstAudioArg->RecordType > RECORD_TYPE_NULL)
    {
        return RK_PARA_ERR;
    }
    gpstAudioDevInf->pfPcmCallBack = pstAudioArg->pfPcmCallBack;
    gpstAudioDevInf->RecordType = pstAudioArg->RecordType;
    gpstAudioDevInf->SamplesPerBlock = pstAudioArg->SamplesPerBlock;

    gpstAudioDevInf->hCodec = RKDev_Open(DEV_CLASS_ROCKCODEC, 0, NOT_CARE);

    if(pstAudioArg->Bit == RECORD_DATAWIDTH_16BIT)
    {
        RockcodecDev_SetAdcDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
    }
    else if (pstAudioArg->Bit == RECORD_DATAWIDTH_24BIT)
    {
        RockcodecDev_SetAdcDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
    }

    AudioDev_SetRxSampleRate(gpstAudioDevInf, 0, pstAudioArg->SampleRate);
    RockcodecDev_SetAdcInputMode(gpstAudioDevInf->hCodec, RecordTypeTransform(pstAudioArg->RecordType));

    gpstAudioDevInf->PcmIndex = 0;

    if((gpstAudioDevInf->SamplesPerBlock * 2 * pstAudioArg->Bit) % 32 != 0)
    {
        rk_printf("Samples err");
        return RK_ERROR;
    }

    //gpstAudioDevInf->PcmLen = ((gpstAudioDevInf->SamplesPerBlock * pstAudioArg->Channel * pstAudioArg->Bit) / 32) * 4;
    gpstAudioDevInf->PcmLen = ((gpstAudioDevInf->SamplesPerBlock * 2 * pstAudioArg->Bit) / 32) * 4;
    gpstAudioDevInf->PcmBuf[0] = rkos_memory_malloc(gpstAudioDevInf->PcmLen);
    if(gpstAudioDevInf->PcmBuf[0] == NULL)
    {
        return RK_ERROR;
    }

    gpstAudioDevInf->PcmBuf[1] = rkos_memory_malloc(gpstAudioDevInf->PcmLen);
    if(gpstAudioDevInf->PcmBuf[1] == NULL)
    {
        rkos_memory_free(gpstAudioDevInf->PcmBuf[0]);
        return RK_ERROR;
    }
    gpstAudioDevInf->DeleteAudioServerFlag = 0;
    gpstAudioDevInf->ReadEnable = 0;
    //rk_printf("PcmBuf[1]=0x%x , PcmLen=%d\n", gpstAudioDevInf->PcmBuf[1],gpstAudioDevInf->PcmLen);
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*/



#ifdef _AUDIO_SHELL_
_DRIVER_AUDIO_AUDIODEVICE_DATA_
static SHELL_CMD ShellAudioName[] =
{
    "pcb",AudioShellPcb,"audio.pcb show fm pcb info","NULL",
    "create",AudioShellCreate,"create audio device","NULL",
    "delete",AudioShellDelete,"delete audio device","NULL",
    "test",AudioShellTest,"audio test audio write","NULL",
    "txrx",AudioShellCodecI2sTRX,"audio tx rx","NULL",
    "startrec",AudioShellAudioServerStart,"start audio server when playing","NULL",
    "stoprec",AudioShellAudioServerStop,"delete audio server when playing","NULL",
    "play",AudioShellAudioPlay,"start audio write when recoeding","NULL",
    "stop",AudioShellAudioStop,"stop audio write when recoeding","NULL",
    "help",NULL,"help cmd","NULL",
    "\b",NULL,"NULL","NULL",
};

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint8 *POutPcmBuf[2];

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint8 PlayFlag;

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
pQueue audio_ctrl_queue = NULL;

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
pSemaphore osAudioWriteSem = NULL;

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
void* audio_ctrl_task_handle = 0;

_DRIVER_AUDIO_AUDIODEVICE_SHELL_
uint8 DeleteAudioShellPlayFlag = 0;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioShellPlayPcmInput
** Input:uint8 * buf, uint32 size
** Return: void
** Owner:cjh
** Date: 2016.3.30
** Time: 16:33:30
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN void AudioShellPlayPcmInput(uint8 * buf, uint32 len)
{
    //printf("AudioShellPlayPcmInput\n");
    AudioShellGetPCMAndProcess();
    AudioDev_SetTrackLen(gpstAudioDevInf, gpstAudioDevInf->PcmLen);
    gpstAudioDevInf->Track = POutPcmBuf[gpstAudioDevInf->PcmIndex];
    AudioDev_Write(gpstAudioDevInf, 0, gpstAudioDevInf->Track);
    //printf("AudioDev_Write....\n");
}

/*******************************************************************************
** Name: AudioShellGetPCMAndProcess
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.4.6
** Time: 15:43:00
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN void AudioShellGetPCMAndProcess()
{
    UINT32 i, j;
    uint32 encodeDataLen, encodeDataLenlostlen, writeDataLen, encodeDataddr;
    uint32 index, index_old;

    if(gpstAudioDevInf->Channel == RECORD_CHANNEL_MONO)
    {
        if(RECORD_DATAWIDTH_16BIT == gpstAudioDevInf->Bit)
        {
            uint16 * pInputBuffere16 =  (uint16 *)gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex];
            uint16 * pOutputBuffere16 = (uint16 *)POutPcmBuf[gpstAudioDevInf->PcmIndex];
            for (index = 0; index < gpstAudioDevInf->SamplesPerBlock; index++)
            {
                pOutputBuffere16[index] =   pInputBuffere16[2 * index];
            }
        }
        else if(RECORD_DATAWIDTH_24BIT == gpstAudioDevInf->Bit)
        {
            uint8 * pInputBuffere8=  (uint8 *)gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex];
            uint8 * pOutputBuffere8 = (uint8 *)POutPcmBuf[gpstAudioDevInf->PcmIndex];
            for (index = 0; index < gpstAudioDevInf->SamplesPerBlock; index++)
            {
                pOutputBuffere8[3 * index]     =  pInputBuffere8[index * 6];
                pOutputBuffere8[3 * index + 1] =  pInputBuffere8[index * 6 + 1];
                pOutputBuffere8[3 * index + 2] =  pInputBuffere8[index * 6 + 2];
            }
        }
        else if(RECORD_DATAWIDTH_32BIT == gpstAudioDevInf->Bit)
        {
            uint32 * pInputBuffere32=  (uint32 *)gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex];;
            uint32 * pOutputBuffere32 = (uint32 *)POutPcmBuf[gpstAudioDevInf->PcmIndex];
            for (index = 0; index < gpstAudioDevInf->SamplesPerBlock; index++)
            {
                pOutputBuffere32[index]  =  pInputBuffere32[2 * index];
            }
        }

    }
    else if(gpstAudioDevInf->Channel == RECORD_CHANNEL_STERO)
    {

        //printf("\n PCM OutPut = %x, record size = %d",POutPcmBuf[gpstAudioDevInf->PcmIndex], (gpstAudioDevInf->SamplesPerBlock * gpstAudioDevInf->Bit ) / 4);
        memcpy((uint8 *)POutPcmBuf[gpstAudioDevInf->PcmIndex], (uint8 *)gpstAudioDevInf->PcmBuf[gpstAudioDevInf->PcmIndex] ,(gpstAudioDevInf->SamplesPerBlock * gpstAudioDevInf->Bit) / 4);

    }
}

/*******************************************************************************
** Name: AudioDev_Shell
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL API rk_err_t AudioDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    ShellHelpSampleDesDisplay(dev, ShellAudioName, pstr);

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellAudioName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                   //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellAudioName[i].CmdDes, pItem);

    if(ShellAudioName[i].ShellCmdParaseFun != NULL)
    {
        ShellAudioName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: AudioDevShellPlayTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2016.5.18
** Time: 13:45:59
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN void AudioDevShellPlayTask_Enter(void * arg)
{
    UINT32 cmd;

    rk_printf("ShellPlayTask Enter ......\n");
    while(1)
    {
        rkos_queue_receive(audio_ctrl_queue, &cmd, MAX_DELAY);
        switch(cmd)
        {
            case RECORDMOD_PLAY_TEST:
                while(!DeleteAudioShellPlayFlag)
                {
                    rkos_semaphore_take(osAudioWriteSem, MAX_DELAY);
                    memset(Audiooutptr[0], 0x4a, sizeof(Audiooutptr[0]));
                    AudioDev_Write(gpstAudioDevInf, 0, gpstAudioDevInf->Track);
                    rk_printf("w");
                    rkos_sleep(50);
                    rkos_semaphore_give(osAudioWriteSem);
                }
                break;
            default:
                rk_printf("Play default");
                break;
        }
    }

    while(1)
    {
        rkos_sleep(10000);
    }
}
/*******************************************************************************
** Name: AudioShellAudioStop
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.13
** Time: 15:40:37
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellAudioStop(HDC dev, uint8 * pstr)
{
    DeleteAudioShellPlayFlag = 1;
    if(audio_ctrl_task_handle)
    {
        AudioDev_RealseMainTrack(gpstAudioDevInf);
        if(audio_ctrl_queue)
        {
            rkos_queue_delete(audio_ctrl_queue);
            audio_ctrl_queue = NULL;
        }
        if(osAudioWriteSem)
        {
            rkos_semaphore_delete(osAudioWriteSem);
            osAudioWriteSem = NULL;
        }
        RKTaskDelete2(audio_ctrl_task_handle);
        audio_ctrl_task_handle = NULL;
    }
    else
    {
        rk_printf("audio play no exist...\n");
    }
    rk_printf("AudioShellAudioStop...\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: AudioShellAudioPlay
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.13
** Time: 15:00:44
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellAudioPlay(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    AUDIO_DEV_ARG stParg;
    //Get audioDevice ID...
    uint32 cmd_temp;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    uint32 bufLen;

#if 1
    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) == NULL && (audio_ctrl_task_handle == NULL))
    {
#if 0
        if(pstAudioArg->Bit == RECORD_DATAWIDTH_16BIT)
        {
            RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
        }
        else if (pstAudioArg->Bit == RECORD_DATAWIDTH_24BIT)
        {
            RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
        }
#else
        audio_ctrl_queue = rkos_queue_create(1, sizeof(int));
        if(audio_ctrl_queue == NULL)
        {
            rk_printf("ERROR:audio_ctrl_queue NULL");
        }
        osAudioWriteSem = rkos_semaphore_create(1,1);
        if(osAudioWriteSem == NULL)
        {
            rk_printf("ERROR:osAudioWriteSem NULL");
        }

        #ifdef CODEC_24BIT //24bit
        RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
        #else
        RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
        #endif
#endif
        RockcodecDev_SetMode(gpstAudioDevInf->hCodec, Codec_DACoutHP);
        DeleteAudioShellPlayFlag = 0;
        AudioDev_SetChannel(gpstAudioDevInf, 0, 2);
        AudioDev_SetTrackLen(gpstAudioDevInf, Audiolength);
        AudioDev_SetBit(gpstAudioDevInf, 0, 16);
        rk_printf("$#Bit =%d\n",gpstAudioDevInf->Bit);
        AudioDev_GetMainTrack(gpstAudioDevInf);
        gpstAudioDevInf->Track = (uint8 *)Audiooutptr;
        //AudioDev_SetSampleRate(gpstAudioDevInf, 0, CodecFS_44100Hz);//Smaple rate PLL  CodecFS_44100Hz
        //stParg.Bit = 16;
        //stParg.RecordType = RECORD_TYPE_LINEIN2;
        //stParg.SampleRate = gpstAudioDevInf->SampleRate;
        //stParg.pfPcmCallBack = AudioShellRecordPcmSend;
        //stParg.SamplesPerBlock = 1024;

        audio_ctrl_task_handle = RKTaskCreate2(AudioDevShellPlayTask_Enter, NULL,NULL,"audio_shell_play",
                                        1024, 27,NULL);
        rkos_sleep(100);
        cmd_temp = RECORDMOD_PLAY_TEST;
        rkos_queue_send(audio_ctrl_queue, &cmd_temp, MAX_DELAY);
    }
    else
    {
        rk_printf("other play task exist...\n");
    }
#else
    PlayFlag = 1;
    while(PlayFlag)
    {
        memset(Audiooutptr[0], 0x1a, sizeof(Audiooutptr[0]));
        rk_printf("w");
        AudioDev_Write(gpstAudioDevInf, 0, gpstAudioDevInf->Track);
    }
#endif

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: AudioShellAudioServerStop
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.13
** Time: 14:53:40
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellAudioServerStop(HDC dev, uint8 * pstr)
{
    if(RKTaskFind(TASK_ID_AUDIODEVICESERVICE, 0) != NULL)
    {
        RKTaskDelete(TASK_ID_AUDIODEVICESERVICE, 0, SYNC_MODE);
    }
    else
    {
        rk_printf("audio server no exist...\n");
    }
}
/*******************************************************************************
** Name: AudioShellRecordPcmSend
** Input:uint8 * buf, uint32 len
** Return: void
** Owner:cjh
** Date: 2016.5.13
** Time: 14:46:10
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN void AudioShellRecordPcmSend(uint8 * buf, uint32 len)
{
    rk_print_string("Write... ");
}
/*******************************************************************************
** Name: AudioShellAudioServerStart
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.13
** Time: 14:42:22
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellAudioServerStart(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    AUDIO_DEV_ARG stParg;

    //Get audioDevice ID...

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(RKTaskFind(TASK_ID_AUDIODEVICESERVICE, 0) == NULL)
    {
        stParg.Bit = 16;
        stParg.RecordType = RECORD_TYPE_LINEIN2;
        stParg.SampleRate = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
        stParg.pfPcmCallBack = AudioShellRecordPcmSend;
        stParg.SamplesPerBlock = 1024;
        RKTaskCreate(TASK_ID_AUDIODEVICESERVICE, 0, &stParg, SYNC_MODE);
    }
    else
    {
        rk_printf("audio server exist...\n");
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: AudioShellDelete
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.18
** Time: 11:48:55
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellCodecI2sTRX(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    AUDIO_DEV_ARG stParg;

    //Get audioDevice ID...

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

#if 1
    uint32 bufLen;


#if 0
    if(pstAudioArg->Bit == RECORD_DATAWIDTH_16BIT)
    {
        RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
    }
    else if (pstAudioArg->Bit == RECORD_DATAWIDTH_24BIT)
    {
        RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
    }
#else
    #ifdef CODEC_24BIT //24bit
    RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH24);
    #else
    RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
    #endif
#endif
    RockcodecDev_SetMode(gpstAudioDevInf->hCodec, Codec_DACoutHP);

    AudioDev_SetBit(gpstAudioDevInf, 0, 16);
    AudioDev_SetChannel(gpstAudioDevInf, 0, 2);

    //RockcodecDev_SetDataWidth(gpstAudioDevInf->hCodec,ACodec_I2S_DATA_WIDTH16);
    //AudioDev_SetVol(gpstAudioDevInf->hCodec,25);

    if(gpstAudioDevInf->Bit == 16)
    {
        bufLen = (((1024 * 2 * 16) / 32) * 4)*2;
    }
    else if(gpstAudioDevInf->Bit == 24)
    {
        bufLen = ((((1024 * 2 * 16) / 32) * 4) + 3)*4/3;
    }

    AudioDev_SetTrackLen(gpstAudioDevInf, gpstAudioDevInf->PcmLen);

    POutPcmBuf[0] = rkos_memory_malloc(bufLen);
    if(POutPcmBuf[0] == NULL)
    {
        rkos_memory_free(POutPcmBuf[0]);
        return RK_ERROR;
    }
    rk_printf("POutPcmBuf[0]=0x%x PcmLen=%d\n", POutPcmBuf[0],gpstAudioDevInf->PcmLen);
    memset(POutPcmBuf[0], 0, sizeof(POutPcmBuf[0]));

    POutPcmBuf[1] = rkos_memory_malloc(bufLen);
    if(POutPcmBuf[1] == NULL)
    {
        rkos_memory_free(POutPcmBuf[1]);
        return RK_ERROR;
    }
    memset(POutPcmBuf[1], 0, sizeof(POutPcmBuf[1]));

    rk_printf("PlayService over......Bit=16  len=%d  bufLen =%d\n", gpstAudioDevInf->PcmLen, bufLen);

    AudioDev_GetMainTrack(gpstAudioDevInf);
#endif
    stParg.Bit = 16;
    stParg.RecordType = RECORD_TYPE_LINEIN1;//,RECORD_TYPE_MIC_STERO
    stParg.SampleRate = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
    stParg.pfPcmCallBack = AudioShellPlayPcmInput;
    stParg.SamplesPerBlock = 1024;

    RKTaskCreate(TASK_ID_AUDIODEVICESERVICE, 0, &stParg, SYNC_MODE);
    return RK_SUCCESS;
}



/*******************************************************************************
** Name: AudioShellDelete
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.18
** Time: 11:48:55
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellDelete(HDC dev,  uint8 * pstr)
{
    uint32 DevID;
    AUDIO_DEV_ARG stParg;
    //Get audioDevice ID...
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    if (RKDev_Delete(DEV_CLASS_AUDIO, DevID, &stParg) != RK_SUCCESS)
    {
        printf("RockCodecDev delete failure DevID %d\n",DevID);
        return RK_ERROR;
    }
    if (stParg.hCodec != NULL)
    {
        if (RKDev_Close(stParg.hCodec) != RK_SUCCESS)
        {
            printf("audioDevice hCodec failure\n");
            return RK_ERROR;
        }
    }

#if 1
    AudioDev_RealseMainTrack(gpstAudioDevInf);
    if(POutPcmBuf[0] != NULL)
    {
        rkos_memory_free(POutPcmBuf[0]);
    }
    if(POutPcmBuf[0] != NULL)
    {
        rkos_memory_free(POutPcmBuf[1]);
    }
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.11.3
** Time: 10:11:42
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellTest(HDC dev, uint8 * pstr)
{
    HDC hAudio;
    rk_err_t ret;
    uint32 vol,time = 0,i;
    AUDIO_DEVICE_CLASS * pstAudioDev;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    hAudio = RKDev_Open(DEV_CLASS_AUDIO,0,NOT_CARE);
    if ((hAudio == NULL) || (hAudio == (HDC)RK_ERROR) || (hAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("Audio device open failure\n");

        return RK_ERROR;
    }
    rk_print_string("Audio device open success\n");

    pstAudioDev = (AUDIO_DEVICE_CLASS *)hAudio;
    FREQ_EnterModule(FREQ_AUDIO_INIT);
    //SetI2SFreq(I2S_DEV0,I2S_XIN12M,0);

    //Acodec_pll_Source_sel(CLK_ACODEC_PLL);//
    //Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
    //SetI2SFreq(I2S_DEV0,CLK_ACODEC_PLL,0);
    //OBS_output_Source_sel(obs_clk_i2s0);
    AudioDev_SetTrackLen(gpstAudioDevInf, Audiolength);
    AudioDev_SetBit(pstAudioDev, 0, 16);
    printf("$#Bit =%d\n",gpstAudioDevInf->Bit);
    AudioDev_GetMainTrack(hAudio);
    gpstAudioDevInf->Track = (uint8 *)Audiooutptr;
    AudioDev_SetSampleRate(pstAudioDev, 0, CodecFS_48KHz);//Smaple rate PLL  CodecFS_44100Hz

#if 0
     for(i=0;i < gpstAudioDevInf->TrackLen;i++)
     printf("Old Track[%d]=0x%x\n",i,gpstAudioDevInf->Track[i]);
#endif
    //Bit_Convertor_DEC
    if (gpstAudioDevInf->Bit == (I2S_DATA_WIDTH24+1))
    {
        printf("DEC test sour 24bit len=%d\n",gpstAudioDevInf->TrackLen/(3*gpstAudioDevInf->Channel));
        Bit_Convertor_DEC((short*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(3*gpstAudioDevInf->Channel), 24);
    }
    else if(gpstAudioDevInf->Bit == (I2S_DATA_WIDTH16+1))
    {
        printf("DEC test 2sour 16bit len=%d\n",gpstAudioDevInf->TrackLen/(2*gpstAudioDevInf->Channel));
        Bit_Convertor_DEC((short*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(2*gpstAudioDevInf->Channel), 16);
    }
    else
    {
        printf("DEC test ERROR:sour music no 16bit or 24bit \n");
    }

#ifdef _RK_EQ_
    //set EQ mode
    printf("cjh$#$test EQmode = %d\n", EQ_HEAVY);
    AudioDev_SetEQ(hAudio, EQ_HEAVY); //SET EQ befor set vol
    AudioDev_SetVol(pstAudioDev, 25);
    printf("gpstAudioDevInf->TrackLen = %d \n",gpstAudioDevInf->TrackLen);
    //AudioDev_SetVol(pstAudioDev, 25);
    if(EQ_NOR != gpstAudioDevInf->UserEQ.Mode)
    {
        /*audio effect process.*/
#ifdef CODEC_24BIT
        //if(gpstAudioDevInf->SampleRate <= 96000)
        if (gpstAudioDevInf->Bit == (I2S_DATA_WIDTH24+1))
        {
            EffectProcess((long*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(3*gpstAudioDevInf->Channel),\
                          &(gpstAudioDevInf->UserEQ));
        }
        else if(gpstAudioDevInf->Bit == (I2S_DATA_WIDTH16+1))
        {
            printf("16BIT EffectProcess TrackLen=%d\n",gpstAudioDevInf->TrackLen);
            EffectProcess((long*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(2*gpstAudioDevInf->Channel),\
                          &(gpstAudioDevInf->UserEQ));
        }

#else
        printf("2_16BIT EffectProcess TrackLen=%d\n",gpstAudioDevInf->TrackLen);
        EffectProcess((short*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(2*gpstAudioDevInf->Channel),\
                      &(gpstAudioDevInf->UserEQ));
#endif
    }
#endif //end #ifdef _RK_EQ_
    if (gpstAudioDevInf->Bit == (I2S_DATA_WIDTH24+1))
    {
        Bit_Convertor_shift((short*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(3*gpstAudioDevInf->Channel), 24);
    }
    else if(gpstAudioDevInf->Bit == (I2S_DATA_WIDTH16+1))
    {
        printf("!@@@@convertor shift 16bit len=%d\n",gpstAudioDevInf->TrackLen/(2*gpstAudioDevInf->Channel));
        Bit_Convertor_shift((short*)gpstAudioDevInf->Track, gpstAudioDevInf->TrackLen/(2*gpstAudioDevInf->Channel), 16);
    }
    else
    {
        printf("!ERROR2:sour music no 16bit or 24bit \n");
    }
#if 0
     for(i=0;i < gpstAudioDevInf->TrackLen;i++)
     printf("Track[%d]=0x%x\n",i,gpstAudioDevInf->Track[i]);
#endif
    //printf("^&&^%%& will write\n");

    while (1)
    {
        //AudioDev_Write(pstAudioDev, TRACK_NO_MAIN, (uint8 *)gpstAudioDevInf->Track);
//#ifdef AudioTest
        RockcodecDev_Write(pstAudioDev->hCodec, (uint8 *)Audiooutptr, Audiolength, ASYNC_MODE);
//#endif
        time++;
        if (time == 8000)
        {
            printf("test over\n");
            break;
        }
        //ret = WM8987Dev_Write(pstAudioDev->hCodec, Audiooutptr, Audiolength, ASYNC_MODE);
    }
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: AudioShellPlay
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.13
** Time: 9:08:47
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellPlay(HDC dev,  uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioShellCreate
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellCreate(HDC dev,  uint8 * pstr)
{
    HDC hCodec;
    AUDIO_DEV_ARG stAudioArg;
    rk_err_t ret;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    I2S_DEV_ARG stI2Sarg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__

    //SetI2SFreq(I2S_DEV0, I2S_XIN12M, NULL);//²âÊÔÄ¬ÈÏ12M

    hCodec = RKDev_Open(DEV_CLASS_ROCKCODEC,0,NOT_CARE);

    //hCodec = RKDev_Open(DEV_CLASS_WM8987, 0, NOT_CARE);

    if ((hCodec == NULL) || (hCodec == (HDC)RK_ERROR) || (hCodec == (HDC)RK_PARA_ERR))
    {
        rk_print_string("codec device open failure");

        return RK_SUCCESS;
    }

    stAudioArg.Bit = I2S_DATA_WIDTH24;
    stAudioArg.hCodec = hCodec;
    stAudioArg.SampleRate = CodecFS_48KHz;//CodecFS_44100Hz;//CodecFS_16KHz;
    stAudioArg.Vol = 25;
#ifdef _RK_EQ_
    stAudioArg.EQMode = EQ_NOR;
#endif
    ret = RKDev_Create(DEV_CLASS_AUDIO, 0, &stAudioArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("audio device create failure");
    }
#endif
    return ret;
}


/*******************************************************************************
** Name: AudioShellOpen
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellOpen(HDC dev,  uint8 * pstr)
{
    HDC hCodec;
    AUDIO_DEV_ARG stAudioArg;
    rk_err_t ret;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    I2S_DEV_ARG stI2Sarg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    SetI2SFreq(I2S_DEV0, I2S_XIN12M, NULL);//²âÊÔÄ¬ÈÏ12M
    //uint32 SetI2SFreq(UINT32 I2sId,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)

//    ret=RKDev_Create(DEV_CLASS_DMA, DMA_CHN1, NULL);
//    if (ret != RK_SUCCESS)
//    {
//        UartDev_Write(dev, "DMA RKDev_Create failure\n",25,SYNC_MODE,NULL);
//    }
    /*
        stI2Sarg.hDma = RKDev_Open(DEV_CLASS_DMA, DMA_CHN0, NOT_CARE);
        stI2Sarg.Channel = I2S_SEL_ACODEC;

        ret=RKDev_Create(DEV_CLASS_I2S, I2S_DEV0, &stI2Sarg);
        if (ret != RK_SUCCESS)
        {
            UartDev_Write(dev, "I2S RKDev_Create failure\n",25,SYNC_MODE,NULL);
        }

        stRockCodecDevArg.hI2s = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);
        stRockCodecDevArg.arg.SampleRate = I2S_FS_44100Hz;
#ifndef _BROAD_LINE_OUT_
        stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutHP;
#else
        stRockCodecDevArg.arg.DacOutMode  = Codec_DACoutLINE;
#endif
        stRockCodecDevArg.arg.AdcinMode = Codec_Standby;
#ifdef CODEC_24BIT
        stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_24BIT;
#else
        stRockCodecDevArg.arg.DataWidth = VDW_TX_WIDTH_16BIT;
#endif        //stRockCodecDevArg.pCodecVolumeTable = ACodec_HPoutVol_General;

        ret = RKDev_Create(DEV_CLASS_ROCKCODEC, 0,&stRockCodecDevArg);
        if (ret != RK_SUCCESS)
        {
            UartDev_Write(dev, "open failure",17,SYNC_MODE,NULL);
        }
    */
    hCodec = RKDev_Open(DEV_CLASS_ROCKCODEC,0,NOT_CARE);

    //hCodec = RKDev_Open(DEV_CLASS_WM8987, 0, NOT_CARE);

    if ((hCodec == NULL) || (hCodec == (HDC)RK_ERROR) || (hCodec == (HDC)RK_PARA_ERR))
    {
        rk_print_string("codec device open failure");

        return RK_SUCCESS;
    }

    stAudioArg.Bit = 16;
    stAudioArg.hCodec = hCodec;
    stAudioArg.SampleRate = CodecFS_44100Hz;//CodecFS_16KHz;

    ret = RKDev_Create(DEV_CLASS_AUDIO, 0, &stAudioArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("audio device create failure");
    }

    return ret;
}


/*******************************************************************************
** Name: AudioShellPcb
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.8
** Time: 10:52:45
*******************************************************************************/
_DRIVER_AUDIO_AUDIODEVICE_SHELL_
SHELL FUN rk_err_t AudioShellPcb(HDC dev, uint8 * pstr)
{
    HDC hAudio;
    uint32 DevID;
    AUDIO_DEVICE_CLASS * pstAudioDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    pstAudioDev = gpstAudioDevInf;
    if(pstAudioDev == NULL)
    {
        rk_printf("FmDev%d in not exist", DevID);
        return RK_SUCCESS;
    }
    if(gpstAudioDevInf != NULL)
    {
        rk_printf_no_time(".gpstAudioDevISR[%d]", DevID);
        rk_printf_no_time("    .stAudioDevice");
        rk_printf_no_time("        .next = %08x",pstAudioDev->stAudioDevice.next);
        rk_printf_no_time("        .UseCnt = %d",pstAudioDev->stAudioDevice.UseCnt);
        rk_printf_no_time("        .SuspendCnt = %d",pstAudioDev->stAudioDevice.SuspendCnt);
        rk_printf_no_time("        .DevClassID = %d",pstAudioDev->stAudioDevice.DevClassID);
        rk_printf_no_time("        .DevID = %d",pstAudioDev->stAudioDevice.DevID);
        rk_printf_no_time("        .suspend = %08x",pstAudioDev->stAudioDevice.suspend);
        rk_printf_no_time("        .resume = %08x",pstAudioDev->stAudioDevice.resume);
        rk_printf_no_time("    .osFmOperReqSem = %08x",pstAudioDev->osAudioWriteSem);
        rk_printf_no_time("    .osFmOperSem = %08x",pstAudioDev->osAudioReadReqSem);
        rk_printf_no_time("    .hControlBus = %08x",pstAudioDev->hCodec);
        rk_printf_no_time("    .TrackLen = %d",pstAudioDev->TrackLen);
        rk_printf_no_time("    .SampleRate = %d",pstAudioDev->SampleRate);
        rk_printf_no_time("    .Channel = %d",pstAudioDev->Channel);
        rk_printf_no_time("    .Bit = %d",pstAudioDev->Bit);
        rk_printf_no_time("    .Track = %08x",pstAudioDev->Track);
        rk_printf_no_time("    .Status = %08x",pstAudioDev->Status);
        rk_printf_no_time("    .TotalCnt = %08x",pstAudioDev->TotalCnt);
        rk_printf_no_time("    .SuspendCnt = %08x",pstAudioDev->SuspendCnt);
        rk_printf_no_time("    .TrackProcess = %08x",pstAudioDev->TrackProcess[0]);
        rk_printf_no_time("    .stSubTrack = %08x",pstAudioDev->TrackProcess[0]);
        rk_printf_no_time("    .TrackProcess = %08x",pstAudioDev->TrackProcess[0]);
        rk_printf_no_time("    .playVolume = %08x",pstAudioDev->playVolume);
        rk_printf_no_time("    .UserEQ = %d",pstAudioDev->UserEQ);
        rk_printf_no_time("    .RecordType = %08x",pstAudioDev->RecordType);
        rk_printf_no_time("    .pfPcmCallBack = %08x",pstAudioDev->pfPcmCallBack);
        rk_printf_no_time("    .PcmBuf[0] = %08x",pstAudioDev->PcmBuf[0]);
        rk_printf_no_time("    .PcmBuf[1] = %08x",pstAudioDev->PcmBuf[1]);
        rk_printf_no_time("    .PcmIndex = %d",pstAudioDev->PcmIndex);
        rk_printf_no_time("    .SamplesPerBlock = %d",pstAudioDev->SamplesPerBlock);
        rk_printf_no_time("    .PcmLen = %d",pstAudioDev->PcmLen);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("AudioDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;
}


#else
rk_err_t AudioDev_Shell(HDC dev, uint8 * pstr)
{

}
#endif
#endif

