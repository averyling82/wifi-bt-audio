/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Audio\AudioControlTask.c
* Owner: aaron.sun
* Date: 2015.6.18
* Time: 10:19:27
* Version: 1.0
* Desc: Audio Control Task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.18     10:19:27   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_AUDIO_AUDIOCONTROLTASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"

#ifdef _RK_EQ_
#include "effect.h"
#endif

#ifdef _RK_SPECTRUM_
#include "Spectrum.h"
#endif
#include "audio_globals.h"
#include "SysInfoSave.h"
#ifdef _MEDIA_MODULE_
#include "AddrSaveMacro.h"
#endif
#include "AudioControlTask.h"
#include "audio_file_access.h"

#ifdef _FADE_PROCESS_
#include "Fade.h"
#endif

#ifdef _USE_GUI_
#include "FileInfo.h"
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _IN_AUDIO_CONTROL_

#ifdef _RK_SPECTRUM_
#define SPECTRUM_LINE_M      12     // 10跟频谱线
//#define SpectrumLoopTime     3
#endif

typedef struct _TRACK_INFO
{
    unsigned long  TotalTime;
    unsigned long  LastTime;
    unsigned long  CurrentTime;
    int            samplerate;
    int            bitrate;
    int            channels;
    int            total_length;
    int            cur_pos;
    int            bitpersample;
}TRACK_INFO;

typedef  struct _AUDIOCONTROL_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;

}AUDIOCONTROL_RESP_QUEUE;

typedef  struct _AUDIOCONTROL_ASK_QUEUE
{
    uint32 cmd;
    void * msg;
    uint32 mode;

}AUDIOCONTROL_ASK_QUEUE;

typedef rk_err_t (* P_AUDIO_PLAY_LIST_INIT)(void * pAudioFileInfo);      //audio callback funciton

typedef rk_err_t (* P_AUDIO_PLAY_NEXT_FILE)(void * pAudioFileInfo, int direct);      //audio callback funciton


typedef struct _SYS_FILE_INFO
{
    uint32      TotalFiles;     //the total number of current direction/disk
    uint32      CurrentFileNum; // 当前文件编号
    uint32      BaseID;
    uint32      Range; //cycle direction or once direction
    uint32      RepeatMode;
    uint8      *pExtStr;       //file type
    uint16      path[MAX_DIRPATH_LEN];
    void *      pPlayList;
    uint32      PlayOrder; // random or order
    P_AUDIO_PLAY_LIST_INIT pPlayListInit;
    P_AUDIO_PLAY_NEXT_FILE pPlayNextFile;
    HDC         hDirDev;

} SYS_FILE_INFO;

typedef  struct _AUDIOCONTROL_TASK_DATA_BLOCK
{
    pQueue      AudioControlAskQueue;
    pQueue      AudioControlRespQueue;

    HDC         hAudio;
    HDC         hFile[3];
    HDC         hBcore;
    HDC         hMsg;
    HDC         hFifo;
    AUDIO_CALLBACK * pfAudioState;
    uint32      EqMode;
    uint32      TrackNo;
    int32       CurrentDecCodec;
    uint32      AudioPlayState;
    uint32      AudioPlayerState;
    uint32      AudioCodecOpenErr;
    uint32      AudioPtr;
    uint32      AudioNeedDecode;
    uint32      AudioLen;
    uint32      AudioErrorFrameNum;
    uint32      AudioEndFade;
    uint32      AudioFadeInStart;
    uint32      AudioFadeOutStart;
    int32       AudioNextFile;
    uint32      ABRequire;
    uint32      AudioABStart;
    uint32      AudioABEnd;
    uint32      PlayDirect;
    uint32      playVolume;
    uint32      StreamEnd;
    uint32      AudioStopMode;   //normal stop or Force//Audio_Stop_Normal or Audio_Stop_Force
    uint32      AudioPlayFileNum;
    uint32      AudioErrorFileCount;//sen #20090803#1 all audio file is not support
    uint32      AudioFileSeekOffset;
    uint32      TaskObjectID;
    uint32      ucSelPlayType;  // add by phc
    uint32      defaultCodecType;  // add by phc
    uint32      MusicLongFileName[MAX_FILENAME_LEN];

    TRACK_INFO     pAudioRegKey;
    SYS_FILE_INFO  AudioFileInfo;

#ifdef _USE_GUI_
    SUB_DIR_INFO    SubDirInfo; //Floder -> All files, Dir info
#endif

#ifdef _MEDIA_MODULE_
    MEDIA_FLODER_INFO_STRUCT    MediaFloderInfo;
#endif

#ifdef _RK_SPECTRUM_
    uint8 SpectrumOut[SPECTRUM_LINE_M];//存放转换过后的M根谱线的能量
    uint32 SpectrumEn;
    uint32 CurrentTimeMsBk;
#endif

    uint32 MaxTrackSegment;
    uint32 TrackSegment;
    uint32 TrackSize;
    uint32 TrackTime;
    uint32 SaveMemory;
    uint32 DirctPlay;

}AUDIOCONTROL_TASK_DATA_BLOCK;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static AUDIOCONTROL_TASK_DATA_BLOCK * gpstAudioControlData;

#ifdef _RK_EQ_
static uint8 EqMode[8] = {EQ_HEAVY,EQ_POP,EQ_JAZZ,EQ_UNIQUE,EQ_USER,EQ_USER,EQ_NOR,EQ_BASS};
static short UseEqTable[CUSTOMEQ_LEVELNUM] = {-10, -6, -3, 0, 3, 6, 10};
#endif



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint32 gDecCmd;
uint32 gDecData;
pSemaphore osAudioDecodeOk;
HDC hAudioDma;
uint32 gACKDone;

volatile unsigned int coed_dma_channel;
MediaBlock    gpMediaBlock;
FILE_HANDLE_t gFileHandle;

unsigned char  * DecDataBuf[2];

unsigned char DecBufID;

char        AudioFileExtString[]   = "MP1MP2MP3XXXWAVAPELACAACM4AOGGMP43GPSBCFLAWMAAMR"; //XXX is other decode by third company developed
char        RecordFileExtString[]  = "WAV";

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t AudioSeekTo(uint32 ms);
rk_err_t AudioControlTaskCheckIdle(HTC hTask);
void AudioStart(void);
void AudioStop(int ReqType);
rk_err_t AudioUp();
rk_err_t AudioNext();
void AudioFFDStop(void);
rk_err_t AudioFFD(uint32 StepLen);
void AudioFFWStop(void);
rk_err_t AudioFFW(uint32 StepLen);
rk_err_t AudioFFResume(void);
rk_err_t AudioResume(void);
rk_err_t AudioFFPause(void);
rk_err_t AudioPause(void);
rk_err_t AudioGetNextMusic(int msg);
void AudioFFStop(void);
void MediaSetFloderInfo(void);
uint16 MediaGetTotalFiles(UINT32 DirClus);
uint16 MediaGetTotalSubDir(UINT32 DirClus);
uint32 MediaGetDirClus(UINT32 DirClus, uint32 index);
void MediaGotoNextDir(void);
uint32 MediaGetCurFileNum(uint32 GlobeFileNum, UINT16 CurDirDeep, UINT32 CurDirClus);
rk_err_t MediaFloderGetFilePathByCurNum(uint32 DirClus, uint16 * path, uint32 CurNum);
rk_err_t MediaFloderPlayListNextFile(SYS_FILE_INFO * pstAudioFileInfo, int direct);
rk_err_t MediaGetFilePathByCurNum(uint16 * path, uint32 CurNum);
rk_err_t MediaPlayListNextFile(SYS_FILE_INFO * pAudioFileInfo, int direct);
rk_err_t DirPlayListNextFile(SYS_FILE_INFO * pAudioFileInfo, int direct);
rk_err_t DirPlayListInit(SYS_FILE_INFO * pAudioFileInfo);
void AudioFREQDeInit(void);
void AudioFileClose(void);
void AudioWaitBBStop(void);
void AudioDeHWInit(void);
void CloseTrack(void);
void AudioVariableDeInit(void);
void AudioWaitBBStart(void);
int32 CheckID3V2Tag(uint8 *pucBuffer);
void AudioCheckStreamType(uint16 * path,  HDC hFile);
uint8 GetFileType(UINT16 *ExtendName, UINT8 *pStr);
void AudioCodec(UINT16 *pBuffer, UINT8 *pStr);
void AudioDecodeProc(AUDIO_CMD id, void * msg);
void AudioHoldonInit(void);
void AudioSetVolume(uint32 Volume);
void AudioFREQInit(void);
rk_err_t AudioCodecOpen(void);
void AudioHWInit(void);
rk_err_t AudioFileOpen(void);
void AudioVariableInit(void);
void SysFindFileInit(SYS_FILE_INFO *pSysFileInfo,UINT16 GlobalFileNum,UINT16 FindFileRange,UINT16 PlayMode, uint8 *pExtStr);
void RegMBoxDecodeSvc(void);
__irq void AudioDecodingGetOutBuffer(void);
rk_err_t AudioControlTask_Resume(void);
rk_err_t AudioControlTask_Suspend(void);




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _MEDIA_MODULE_
/*******************************************************************************
** Name: AudioPlayer_SetFloderInfo
** Input:void
** Return: void
** Owner:ctf
** Date: 2015.11.6
** Time: 10:12:48
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API void AudioPlayer_SetFloderInfo(void)
{
    if(gpstAudioControlData->AudioFileInfo.Range == FIND_FILE_RANGE_ALL)//All files
    {
        MediaSetFloderInfo();
    }
}
#endif

/*******************************************************************************
** Name: AudioPlayer_GetAudioInfo
** Input:void * AudioInfo
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.6
** Time: 10:12:48
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API uint32 AudioPlayer_GetAudioInfo(AUDIO_INFO * AudioInfo)
{
    if(AudioInfo != NULL)
    {
        AudioInfo->AudioSource = gpstAudioControlData->ucSelPlayType;
        AudioInfo->Bitrate = gpstAudioControlData->pAudioRegKey.bitrate;
        AudioInfo->bitpersample = gpstAudioControlData->pAudioRegKey.bitpersample;
        AudioInfo->Samplerate = gpstAudioControlData->pAudioRegKey.samplerate;
        AudioInfo->channels = gpstAudioControlData->pAudioRegKey.channels;
        AudioInfo->CurrentFileNum = gpstAudioControlData->AudioFileInfo.CurrentFileNum;
        AudioInfo->TotalFiles = gpstAudioControlData->AudioFileInfo.TotalFiles;
        memcpy((uint8 *)AudioInfo->path, (uint8 *)gpstAudioControlData->AudioFileInfo.path, MAX_DIRPATH_LEN * 2);
        AudioInfo->PlayOrder = gpstAudioControlData->AudioFileInfo.PlayOrder;
        AudioInfo->RepeatMode = gpstAudioControlData->AudioFileInfo.RepeatMode;
        AudioInfo->PlayVolume = gpstAudioControlData->playVolume;
        AudioInfo->playerr = gpstAudioControlData->AudioCodecOpenErr;
        AudioInfo->EqMode = gpstAudioControlData->EqMode;
        AudioInfo->BaseID = gpstAudioControlData->AudioFileInfo.BaseID;
        #ifdef _MEDIA_MODULE_
        memcpy((UINT8*) &(AudioInfo->MediaFloderInfo), (UINT8*) &(gpstAudioControlData->MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));
        #endif
    }

    return gpstAudioControlData->AudioPlayerState;
}

/*******************************************************************************
** Name: AudioPlayer_GetTotalTime
** Input:uint32 * time
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.31
** Time: 16:20:42
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioPlayer_GetTotalTime(uint32 * time)
{
    *time = gpstAudioControlData->pAudioRegKey.TotalTime;
}

/*******************************************************************************
** Name: AudioPlayer_SetSpectrumEn
** Input:uint32 * time
** Return: rk_err_t
** Owner: cjh
** Date: 2016.1.16
** Time: 16:38:51
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioPlayer_SetSpectrumEn(uint32 status)
{
#ifdef _RK_SPECTRUM_
    gpstAudioControlData->SpectrumEn = status;
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioPlayer_GetCurSpectrum
** Input:uint32 * time
** Return: rk_err_t
** Owner: cjh
** Date: 2016.1.16
** Time: 16:38:51
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioPlayer_GetCurSpectrum(uint8** pSpectrum)
{
    uint8 i;
#ifdef _RK_SPECTRUM_
    *pSpectrum = gpstAudioControlData->SpectrumOut;
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: AudioPlayer_GetCurTime
** Input:uint32 * time
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.31
** Time: 16:19:51
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioPlayer_GetCurTime(uint32 * time)
{
    *time = gpstAudioControlData->pAudioRegKey.CurrentTime;
}

/*******************************************************************************
** Name: AudioPlayer_GetState
** Input:uint32 * state
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.31
** Time: 13:34:41
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioPlayer_GetState(uint32 * state)
{
    *state = gpstAudioControlData->AudioPlayState;
}

/*******************************************************************************
** Name: AudioStart
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:17:59
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API void AudioStart(void)
{
    int ret = 0;
    uint32 timeout = 200;

    //DEBUG("Audio Start");

    FREQ_EnterModule(FREQ_AUDIO_INIT);

    AudioVariableInit();

    if (RK_ERROR == AudioFileOpen())
    {
        FREQ_ExitModule(FREQ_AUDIO_INIT);
        DEBUG("Audio File Open Error");
        {
            AUDIO_CALLBACK * pCur;

            pCur = gpstAudioControlData->pfAudioState;

            while(pCur != NULL)
            {
                pCur->pfAudioState(AUDIO_STATE_ERROR);
                pCur = pCur->pNext;
            }
        }
        gpstAudioControlData->AudioCodecOpenErr = 1;
        return;
    }

    AudioHWInit();

    if (RK_ERROR == AudioCodecOpen())
    {
        FREQ_ExitModule(FREQ_AUDIO_INIT);
        gpstAudioControlData->AudioPlayState = AUDIO_STATE_PLAY;
        AudioStop(Audio_Stop_Force);
        DEBUG("Codec Open Error1");
        if((gpstAudioControlData->ucSelPlayType == SOURCE_FROM_HTTP)
        ||(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_BT)
        ||(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_XXX))
        {
            AUDIO_CALLBACK * pCur;

            gpstAudioControlData->AudioPlayerState = AUDIO_STATE_STOP;

            pCur = gpstAudioControlData->pfAudioState;

            while(pCur != NULL)
            {
                pCur->pfAudioState(AUDIO_STATE_STOP);
                pCur = pCur->pNext;
            }
        }
        else
        {
            AUDIO_CALLBACK * pCur;

            pCur = gpstAudioControlData->pfAudioState;

            while(pCur != NULL)
            {
                pCur->pfAudioState(AUDIO_STATE_ERROR);
                pCur = pCur->pNext;
            }
            gpstAudioControlData->AudioCodecOpenErr = 1;
        }
        return;
    }

   // DEBUG("\n audio codec open success.\n ");

    FREQ_ExitModule(FREQ_AUDIO_INIT);

    AudioFREQInit();

    if((((gpstAudioControlData->AudioLen * 1000) / gpstAudioControlData->pAudioRegKey.samplerate) > 100)
        && (gpstAudioControlData->SaveMemory == 0))
    {
        uint32 i;

        for(i = 1; i < 32; i++)
        {
            rk_printf("i = %d", i);
            if((gpstAudioControlData->AudioLen % i) == 0)
            {

                if((((gpstAudioControlData->AudioLen / i) * 1000) / gpstAudioControlData->pAudioRegKey.samplerate) < 100)
                {
                    gpstAudioControlData->TrackSize = gpstAudioControlData->AudioLen / i;
                    gpstAudioControlData->TrackTime = ((gpstAudioControlData->TrackSize * 1000) / gpstAudioControlData->pAudioRegKey.samplerate);
                    gpstAudioControlData->MaxTrackSegment = i;
                    gpstAudioControlData->AudioLen = gpstAudioControlData->TrackSize;
                    rk_printf("gpstAudioControlData->MaxTrackSegment = %d", gpstAudioControlData->MaxTrackSegment);
                    rk_printf("gpstAudioControlData->TrackSize = %d", gpstAudioControlData->TrackSize);
                    rk_printf("gpstAudioControlData->TrackTime = %d", gpstAudioControlData->TrackTime);
                    break;
                }
            }
        }

        if(i == 32)
        {
            gpstAudioControlData->MaxTrackSegment = 1;
        }
        //gpstAudioControlData->MaxTrackSegment = 1;
        gpstAudioControlData->TrackSegment = gpstAudioControlData->MaxTrackSegment - 1;


    }
    else
    {
        gpstAudioControlData->MaxTrackSegment = 1;
        gpstAudioControlData->TrackSegment = gpstAudioControlData->MaxTrackSegment - 1;
    }

    //AudioHoldonInit();

    AudioDev_SetTrackLen(gpstAudioControlData->hAudio, gpstAudioControlData->AudioLen * 2 * gpstAudioControlData->pAudioRegKey.bitpersample / 8);//gpstAudioControlData->pAudioRegKey.channels
    AudioDev_SetChannel(gpstAudioControlData->hAudio, gpstAudioControlData->TrackNo, gpstAudioControlData->pAudioRegKey.channels);
    //rk_printf(">>>bitpersample =%d",gpstAudioControlData->pAudioRegKey.bitpersample);
    AudioDev_SetBit(gpstAudioControlData->hAudio, gpstAudioControlData->TrackNo, gpstAudioControlData->pAudioRegKey.bitpersample);

    AudioDev_SetTxSampleRate(gpstAudioControlData->hAudio, gpstAudioControlData->TrackNo, gpstAudioControlData->pAudioRegKey.samplerate);

    AudioDev_SetByPass(gpstAudioControlData->hAudio, gpstAudioControlData->SaveMemory);

    #ifdef _RK_EQ_
    rk_printf("eq = %d", gSysConfig.MusicConfig.Eq.Mode);
    AudioDev_SetEQ(gpstAudioControlData->hAudio, gSysConfig.MusicConfig.Eq.Mode); //SET EQ befor set vol
    #endif


    if(gpstAudioControlData->AudioPlayerState == AUDIO_STATE_FFW)
    {
        gpstAudioControlData->pAudioRegKey.CurrentTime = gpstAudioControlData->pAudioRegKey.TotalTime;
        //rk_printf("gpstAudioControlData->pAudioRegKey.CurrentTime = %d", gpstAudioControlData->pAudioRegKey.CurrentTime);
    }

    if(gpstAudioControlData->AudioPlayerState == AUDIO_STATE_PLAY)
    {
        gpstAudioControlData->AudioPlayState = AUDIO_STATE_PLAY;
    }
    else
    {
        gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;
    }

    {
        AUDIO_CALLBACK * pCur;

        pCur = gpstAudioControlData->pfAudioState;

        while(pCur != NULL)
        {
            pCur->pfAudioState(AUDIO_STATE_MUSIC_CHANGE);
            pCur = pCur->pNext;
        }
    }
}

/*******************************************************************************
** Name: AudioStop
** Input:UINT16 ReqType
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:01:28
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API void AudioStop(int ReqType)
{

    if(gpstAudioControlData->AudioPlayState == AUDIO_STATE_STOP)//already stoped
    {
        return;
    }

    #ifdef _RK_EQ_
    AudioDev_SetEQ(gpstAudioControlData->hAudio, EQ_NOR);
    #endif

    if(gpstAudioControlData->SaveMemory)
    {
        rkos_sleep(500);
    }

    AudioSetVolume(0);

    AudioVariableDeInit();

    CloseTrack();

    AudioDeHWInit();

    AudioFileClose();

    AudioFREQDeInit();

    gpstAudioControlData->AudioPlayState = AUDIO_STATE_STOP;

    if((gpstAudioControlData->ucSelPlayType == SOURCE_FROM_HTTP)
        ||(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_BT)
        ||(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_XXX))
    {
        if(Audio_Stop_Normal == ReqType)
        {
            gpstAudioControlData->AudioPlayerState = AUDIO_STATE_STOP;

            {
                AUDIO_CALLBACK * pCur;

                pCur = gpstAudioControlData->pfAudioState;

                while(pCur != NULL)
                {
                    pCur->pfAudioState(AUDIO_STATE_STOP);
                    pCur = pCur->pNext;
                }
            }
        }
    }
    else if (Audio_Stop_Normal == ReqType)
    {
        gpstAudioControlData->AudioStopMode = Audio_Stop_Normal;
        AudioGetNextMusic(1);
    }
    else if (Audio_Stop_PrevFile == ReqType)
    {
        gpstAudioControlData->AudioStopMode = Audio_Stop_Force;
        AudioGetNextMusic(-1);
    }
    else if(Audio_Stop_NextFile == ReqType)
    {
        gpstAudioControlData->AudioStopMode = Audio_Stop_Force;
        AudioGetNextMusic(1);
    }

    rk_printf("AudioStop over ReqType = %d, curSate = %d", ReqType, gpstAudioControlData->AudioPlayerState);

}

/*******************************************************************************
** Name: AudioNextMusic
** Input:uint32 msg
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:17:38
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioNextMusic(int direct)
{
    if(direct == 1)
    {
        AudioStop(Audio_Stop_Force);
        gpstAudioControlData->AudioStopMode = Audio_Stop_Force;
        AudioGetNextMusic(1);
    }
    else
    {
        AudioStop(Audio_Stop_Force);
        gpstAudioControlData->AudioStopMode = Audio_Stop_Force;
        AudioGetNextMusic(-1);
    }
}
/*******************************************************************************
** Name: AudioFFStop
** Input:VOID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:16:29
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API void AudioFFStop(void)
{
    if(gpstAudioControlData->AudioPlayState == AUDIO_STATE_PAUSE)
    {
        rk_printf("FF Stop");
        CodecSeek(gpstAudioControlData->pAudioRegKey.CurrentTime, 0);
        gpstAudioControlData->TrackSegment = gpstAudioControlData->MaxTrackSegment - 1;
        AudioResume();
    }
    else
    {
        gpstAudioControlData->AudioPlayerState = AUDIO_STATE_PLAY;
    }
}

/*******************************************************************************
** Name: AudioFFD
** Input:unsigned long StepLen
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:15:42
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioFFD(uint32 StepLen)
{
    //rk_printf("AudioFFD StepLen=%d",StepLen);

    if (AUDIO_STATE_STOP == gpstAudioControlData->AudioPlayerState)
    {
        return TRUE;
    }

    if (AUDIO_STATE_FFD != gpstAudioControlData->AudioPlayerState)  //wait for mute
    {
        #ifdef _FADE_PROCESS_
        if (AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayState)
        {
            gpstAudioControlData->AudioFadeOutStart = 1;
        }
        #else
            gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;
        #endif

        gpstAudioControlData->AudioPlayerState = AUDIO_STATE_FFD; // audio player state
    }
    else //if (AUDIO_STATE_FFD == AudioPlayState)   //FFD
    {
        gpstAudioControlData->pAudioRegKey.CurrentTime = gpstAudioControlData->pAudioRegKey.CurrentTime + StepLen;

        if(gpstAudioControlData->pAudioRegKey.CurrentTime > gpstAudioControlData->pAudioRegKey.TotalTime)
            gpstAudioControlData->pAudioRegKey.CurrentTime  = gpstAudioControlData->pAudioRegKey.TotalTime;

        //rk_printf("gpstAudioControlData->pAudioRegKey.CurrentTime = %d, step = %d", gpstAudioControlData->pAudioRegKey.CurrentTime, StepLen);

    }

    //if(gpstAudioControlData->pfAudioState != NULL)
    //    gpstAudioControlData->pfAudioState(AUDIO_STATE_TIME_CHANGE);

    return TRUE;
}

/*******************************************************************************
** Name: AudioFFW
** Input:unsigned long StepLen
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:14:17
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioFFW(uint32 StepLen)
{
    //rk_printf("AudioFFW StepLen=%d",StepLen);

    if (AUDIO_STATE_STOP == gpstAudioControlData->AudioPlayerState)
    {
        return TRUE;
    }

    if (AUDIO_STATE_FFW != gpstAudioControlData->AudioPlayerState)  //wait for mute
    {
        #ifdef _FADE_PROCESS_
        if (AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayState)
        {
            gpstAudioControlData->AudioFadeOutStart = 1;
        }
        #else
            gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;
        #endif

        gpstAudioControlData->AudioPlayerState = AUDIO_STATE_FFW; // audio player state
    }
    else //if (AUDIO_STATE_FFW == AudioPlayState)   //FFW
    {
        gpstAudioControlData->pAudioRegKey.CurrentTime = (gpstAudioControlData->pAudioRegKey.CurrentTime > StepLen) ? (gpstAudioControlData->pAudioRegKey.CurrentTime - StepLen) : 0;
    }

    //if(gpstAudioControlData->pfAudioState != NULL)
    //    gpstAudioControlData->pfAudioState(AUDIO_STATE_TIME_CHANGE);

    return TRUE;
}
/*******************************************************************************
** Name: AudioFFResume
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:13:45
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioFFResume(void)
{

    if (AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayerState)
    {
        return RK_SUCCESS;
    }

    rk_printf("=== AudioFFResume in ===  ");

    CodecSeek(gpstAudioControlData->pAudioRegKey.CurrentTime, 0);

    gpstAudioControlData->AudioPlayState   = AUDIO_STATE_PLAY;

    DEBUG("=== AudioResume out ===");

    return TRUE;
}
/*******************************************************************************
** Name: AudioResume
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:13:21
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioResume(void)
{

    if (AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayerState)
    {
        return RK_SUCCESS;
    }

    rk_printf("=== AudioResume in ===  ");


    gpstAudioControlData->AudioErrorFrameNum = 0;

    gpstAudioControlData->AudioPlayerState = AUDIO_STATE_PLAY;
    gpstAudioControlData->AudioPlayState   = AUDIO_STATE_PLAY;

    DEBUG("=== AudioResume out ===");

    return TRUE;
}
/*******************************************************************************
** Name: AudioFFPause
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:12:26
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioFFPause(void)
{

    rk_printf("=== Audio FF Pause in ===");

    if ((AUDIO_STATE_FFW == gpstAudioControlData->AudioPlayerState)
        || (AUDIO_STATE_FFD == gpstAudioControlData->AudioPlayerState))
    {
        gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;
    }

    rk_printf("=== Audio FF Pause out ===");

    return TRUE;
}

/*******************************************************************************
** Name: AudioPause
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 16:09:19
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioPause(void)
{

    rk_printf("=== AudioPause in ===");

    if (AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayerState)
    {
        gpstAudioControlData->AudioPlayerState = AUDIO_STATE_PAUSE;

        #ifdef _FADE_PROCESS_
        gpstAudioControlData->AudioFadeOutStart = 1;
        #else
        gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;
        {
            AUDIO_CALLBACK * pCur;

            pCur = gpstAudioControlData->pfAudioState;

            while(pCur != NULL)
            {
                pCur->pfAudioState(AUDIO_STATE_PAUSE);
                pCur = pCur->pNext;
            }
        }
        #endif
    }

    rk_printf("=== AudioPause out ===");
    return TRUE;
}
/*******************************************************************************
** Name: AudioGetNextMusic
** Input:UINT32 msg
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.22
** Time: 14:55:51
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioGetNextMusic(int msg)
{
    int ret = 0;
    uint32  Playednumber;

/*****JJJHHH always repeat**/
    printf("JJJHHH RepeatMode=%d\n",gpstAudioControlData->AudioFileInfo.RepeatMode);
	gpstAudioControlData->AudioFileInfo.RepeatMode = AUDIO_ALLREPEAT;
/*******/
	
    switch (gpstAudioControlData->AudioFileInfo.RepeatMode)
    {
        case AUDIO_REPEAT://single repeat
        case AUDIO_REPEAT1:
            if (Audio_Stop_Force == gpstAudioControlData->AudioStopMode)
            {
                gpstAudioControlData->AudioFileInfo.pPlayNextFile(&gpstAudioControlData->AudioFileInfo, 1);
            }
            break;

        case AUDIO_ALLONCE://only once all song play cycle.
            Playednumber = gpstAudioControlData->AudioFileInfo.CurrentFileNum;
            if (gpstAudioControlData->AudioFileInfo.PlayOrder == AUDIO_RAND)
            {
                Playednumber = gpstAudioControlData->AudioPlayFileNum;
            }

            if((Playednumber >= gpstAudioControlData->AudioFileInfo.TotalFiles ) && (gpstAudioControlData->AudioStopMode != Audio_Stop_Force))
            {
                //SysFindFile(&gpstAudioControlData->AudioFileInfo,(INT16)msg);
                if (gpstAudioControlData->AudioFileInfo.PlayOrder == AUDIO_RAND)
                {
                    gpstAudioControlData->AudioPlayFileNum = 1;
                }
                AudioStart();

                //SCH
                CodecSeek(0, 0);
                if (gpstAudioControlData->pAudioRegKey.CurrentTime >= gpstAudioControlData->pAudioRegKey.TotalTime)
                {
                    gpstAudioControlData->pAudioRegKey.CurrentTime = gpstAudioControlData->pAudioRegKey.TotalTime;
                }

                AudioPause();

                return TRUE;
            }
            else
            {
                gpstAudioControlData->AudioFileInfo.pPlayNextFile(&gpstAudioControlData->AudioFileInfo, msg);

                if (gpstAudioControlData->AudioFileInfo.PlayOrder == AUDIO_RAND)
                {
                    gpstAudioControlData->AudioPlayFileNum += msg;
                    if (gpstAudioControlData->AudioPlayFileNum < 1)
                    {
                        gpstAudioControlData->AudioPlayFileNum = gpstAudioControlData->AudioFileInfo.TotalFiles;
                    }
                }
            }
            break;

        case AUDIO_ALLREPEAT://all cycle play
            gpstAudioControlData->AudioFileInfo.pPlayNextFile(&gpstAudioControlData->AudioFileInfo, msg);
            break;


        case AUDIO_FOLDER_ONCE://directory once.
            Playednumber = gpstAudioControlData->AudioFileInfo.CurrentFileNum + 1;
            if (gpstAudioControlData->AudioFileInfo.PlayOrder == AUDIO_RAND)
            {
                Playednumber = gpstAudioControlData->AudioPlayFileNum + 1;
            }

            if ((Playednumber >= gpstAudioControlData->AudioFileInfo.TotalFiles) && (gpstAudioControlData->AudioStopMode != Audio_Stop_Force))
            {
                gpstAudioControlData->AudioFileInfo.pPlayNextFile(&gpstAudioControlData->AudioFileInfo, msg);

                if (gpstAudioControlData->AudioFileInfo.PlayOrder == AUDIO_RAND)
                {
                    gpstAudioControlData->AudioPlayFileNum = 0;
                }

                gpstAudioControlData->AudioPlayerState = AUDIO_STATE_PAUSE;
                AudioStart();

                #if 0
                //SCH
                CodecSeek(0, 0);
                if (gpstAudioControlData->pAudioRegKey.CurrentTime >= gpstAudioControlData->pAudioRegKey.TotalTime)
                {
                    gpstAudioControlData->pAudioRegKey.CurrentTime = gpstAudioControlData->pAudioRegKey.TotalTime;
                    //FileInfo[(uint32)pRawFileCache].Offset = FileInfo[(uint32)pRawFileCache].FileSize;
                }
                AudioPause();
                #endif

                return TRUE;
            }
            else
            {
                rk_printf("next music  cur music = %d", Playednumber);

                gpstAudioControlData->AudioFileInfo.pPlayNextFile(&gpstAudioControlData->AudioFileInfo, msg);

                if (gpstAudioControlData->AudioFileInfo.PlayOrder == AUDIO_RAND)
                {
                    gpstAudioControlData->AudioPlayFileNum += msg;
                    if (gpstAudioControlData->AudioPlayFileNum < 1)
                    {
                        gpstAudioControlData->AudioPlayFileNum = gpstAudioControlData->AudioFileInfo.TotalFiles;
                    }
                }
            }
            break;

        case AUIDO_FOLDER_REPEAT://directory cycle.
            gpstAudioControlData->AudioFileInfo.pPlayNextFile(&gpstAudioControlData->AudioFileInfo, msg);
            break;

        default:
            ret = FALSE;
            break;
    }

    AudioStart();

    return ret;
}

/*******************************************************************************
** Name: AudioControlTask_SetStateChangeFunc
** Input:uint32 Cmd
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 16:19:05
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioControlTask_SetStateChangeFunc(P_AUDIO_CALLBACK old, P_AUDIO_CALLBACK new)
{
    AUDIO_CALLBACK * pCur, * pLast;

    pCur = gpstAudioControlData->pfAudioState;
    pLast = NULL;

    if(old != NULL)
    {
        while(pCur != NULL)
        {
            if(pCur->pfAudioState == old)
            {
                if(new != NULL)
                {
                    pCur->pfAudioState = new;
                    return RK_SUCCESS;
                }
                else
                {
                    if(pLast == NULL)
                    {
                        gpstAudioControlData->pfAudioState = pCur->pNext;
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

        pCur = rkos_memory_malloc(sizeof(AUDIO_CALLBACK));
        if((int32)pCur <= 0)
        {
            return RK_ERROR;
        }

        pCur->pNext = gpstAudioControlData->pfAudioState;
        pCur->pfAudioState = new;
        gpstAudioControlData->pfAudioState = pCur;
        return RK_SUCCESS;
    }
    else
    {
        pCur = rkos_memory_malloc(sizeof(AUDIO_CALLBACK));
        if((int32)pCur <= 0)
        {
            return RK_ERROR;
        }

        pCur->pNext = gpstAudioControlData->pfAudioState;
        pCur->pfAudioState = new;
        gpstAudioControlData->pfAudioState = pCur;
        return RK_SUCCESS;
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: AudioControlTask_SendCmd
** Input:uint32 Cmd
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.26
** Time: 16:19:05
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API rk_err_t AudioControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode)
{
    AUDIOCONTROL_ASK_QUEUE AudioControlAskQueue;
    AUDIOCONTROL_RESP_QUEUE AudioControlRespQueue;

    if(gpstAudioControlData->AudioPlayerState == AUDIO_STATE_STOP)
    {
        if(Cmd != AUDIO_CMD_DECSTART)
            return RK_SUCCESS;
    }

    if(Cmd == AUDIO_CMD_VOLUMESET)
    {
        AudioDecodeProc(AUDIO_CMD_VOLUMESET, msg);
        return RK_SUCCESS;
    }


    AudioControlAskQueue.cmd = Cmd;
    AudioControlAskQueue.msg = msg;
    AudioControlAskQueue.mode = Mode;

    if(gpstAudioControlData->hFifo != NULL)
    {
        if((Cmd == AUDIO_CMD_STOP) || (Cmd == AUDIO_CMD_NEXTFILE) || (Cmd == AUDIO_CMD_PREVFILE))
        {
            rk_printf("fifo force stop...");
            fifoDev_ForceStop(gpstAudioControlData->hFifo);
        }
    }

    rkos_queue_send(gpstAudioControlData->AudioControlAskQueue, &AudioControlAskQueue, MAX_DELAY);

    if(Mode == SYNC_MODE)
    {
        rkos_queue_receive(gpstAudioControlData->AudioControlRespQueue, &AudioControlRespQueue, MAX_DELAY);
        if ((AudioControlRespQueue.cmd == Cmd) && AudioControlRespQueue.status == RK_SUCCESS)
        {
            if((Cmd == AUDIO_CMD_STOP)
                || (Cmd == AUDIO_CMD_NEXTFILE)
                || (Cmd == AUDIO_CMD_PREVFILE)
                || (Cmd == AUDIO_CMD_FFD)
                || (Cmd == AUDIO_CMD_FFW)
                || (Cmd == AUDIO_CMD_RESUME)
                || (Cmd == AUDIO_CMD_PAUSE)
                || (Cmd == AUDIO_CMD_SEEKTO))
            {
                while(gpstAudioControlData->AudioFadeOutStart != 0)
                {
                    rkos_sleep(10);
                }
            }
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
** Name: AudioControlTask_Enter
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 10:21:46
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON API void AudioControlTask_Enter(void)
{
    short *pBuffer1, * pBuffer2, * pBuffer3;
    int  i,j,k = 0;
    int32 DecodeErr;
    uint32 readlen, totalRead;
    int32 DataTemp;

    AUDIOCONTROL_ASK_QUEUE AudioControlAskQue;
    AUDIOCONTROL_RESP_QUEUE AudioControlResQue;
    HTC hSelf;

    rk_printf("11audio control task enter...");

    while (RKTaskCreate(TASK_ID_STREAMCONTROL, 0, NULL, SYNC_MODE) != RK_SUCCESS)
    {
        rkos_sleep(10);
    }

    hSelf = RKTaskGetRunHandle();

    gpstAudioControlData->AudioPlayState = AUDIO_STATE_STOP;

    if((gpstAudioControlData->ucSelPlayType != SOURCE_FROM_HTTP)
        && (gpstAudioControlData->ucSelPlayType != SOURCE_FROM_BT)
        && (gpstAudioControlData->ucSelPlayType != SOURCE_FROM_XXX))
    {
        gpstAudioControlData->AudioPlayerState = AUDIO_STATE_PLAY;

        if(gpstAudioControlData->AudioFileInfo.TotalFiles)
        {
            AudioStart();
        }
    }

replay:
    while(1)
    {
        if ((AUDIO_STATE_STOP == gpstAudioControlData->AudioPlayState)||(AUDIO_STATE_PAUSE == gpstAudioControlData->AudioPlayState))
        {
            //rk_printf("task_enter state=%d\n",gpstAudioControlData->AudioPlayState);
            if(rkos_queue_receive(gpstAudioControlData->AudioControlAskQueue, &AudioControlAskQue, MAX_DELAY) == RK_SUCCESS)
            {
                if(AudioControlTaskCheckIdle(hSelf) != RK_SUCCESS)
                {
                    continue;
                }

                AudioDecodeProc(AudioControlAskQue.cmd, AudioControlAskQue.msg);
                AudioControlResQue.cmd = AudioControlAskQue.cmd;
                AudioControlResQue.status = RK_SUCCESS;
                if(AudioControlAskQue.mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstAudioControlData->AudioControlRespQueue, &AudioControlResQue, MAX_DELAY);
                }
            }
            else
            {
                if(AudioControlTaskCheckIdle(hSelf) != RK_SUCCESS)
                {
                    continue;
                }
            }
        }
        else
        {
            if(rkos_queue_receive(gpstAudioControlData->AudioControlAskQueue, &AudioControlAskQue, 0) == RK_SUCCESS)
            {
                if(AudioControlTaskCheckIdle(hSelf) != RK_SUCCESS)
                {
                    continue;
                }

                //rk_printf("222rkos_queue_receive cmd %d",AudioControlAskQue.cmd);
                AudioDecodeProc(AudioControlAskQue.cmd, AudioControlAskQue.msg);
                AudioControlResQue.cmd = AudioControlAskQue.cmd;
                AudioControlResQue.status = RK_SUCCESS;

                if(AudioControlAskQue.mode == SYNC_MODE)
                {
                    rkos_queue_send(gpstAudioControlData->AudioControlRespQueue, &AudioControlResQue, MAX_DELAY);
                }

            }
            else
            {
                if(AudioControlTaskCheckIdle(hSelf) != RK_SUCCESS)
                {
                    continue;
                }
            }
        }

        if (AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayState)
        {
            if(gpstAudioControlData->TrackSegment >= (gpstAudioControlData->MaxTrackSegment - 1))
            {
                CodecGetTime(&gpstAudioControlData->pAudioRegKey.CurrentTime);

                if(gpstAudioControlData->SaveMemory)
                {
                    DecodeErr = CodecDecode();
                    CodecGetDecBuffer((short*)&gpstAudioControlData->AudioPtr, &gpstAudioControlData->AudioLen);
                }
                else
                {
                    DecodeErr = CodecGetDecBuffer((short*)&gpstAudioControlData->AudioPtr, &gpstAudioControlData->AudioLen);
                }

                if (0 == DecodeErr)   // Decoding end or error
                {
                    if (gpstAudioControlData->StreamEnd == 0)
                    {
                        //rk_printf("------1 file error------ ");
                        if(gpstAudioControlData->SaveMemory)
                        {
                            #ifdef CODEC_24BIT
                            memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 8));
                            #else
                            memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 4));
                            #endif
                        }
                        else
                        {
                            memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 2 * gpstAudioControlData->pAudioRegKey.bitpersample) / 8);
                        }
                        #if 0
                        {
                            AUDIO_CALLBACK * pCur;

                            pCur = gpstAudioControlData->pfAudioState;

                            while(pCur != NULL)
                            {
                                pCur->pfAudioState(AUDIO_STATE_ERROR);
                                pCur = pCur->pNext;
                            }
                        }
                        #endif
                    }
                    else
                    {
                        //rk_printf("------2 decode end------ ");
                        if(gpstAudioControlData->SaveMemory)
                        {
                            #ifdef CODEC_24BIT
                            memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 8));
                            #else
                            memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 4));
                            #endif
                        }
                        else
                        {
                            memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 2 * gpstAudioControlData->pAudioRegKey.bitpersample) / 8);
                        }
                    }


                    //if((gpstAudioControlData->AudioEndFade == 1) && (gpstAudioControlData->AudioFadeOutStart == 1))
                    if(gpstAudioControlData->AudioFadeOutStart == 1)
                    {
                        goto ERR_IN_FADE;
                    }
                    else
                    {
                        AudioStop(Audio_Stop_Normal);
                        goto replay;
                    }

                }

                {
                    uint32 i, j, k;
                    uint8 temp;
                    uint32 MovLen1, MovLen2, MovLen3;


                    if(gpstAudioControlData->MaxTrackSegment > 1)
                    {
                        for(i = 0;  i < gpstAudioControlData->MaxTrackSegment; i++)
                        {
                            MovLen1 = (gpstAudioControlData->MaxTrackSegment - i - 1) * gpstAudioControlData->TrackSize * 8;
                            MovLen3 = (gpstAudioControlData->MaxTrackSegment - i - 1) * gpstAudioControlData->TrackSize * gpstAudioControlData->pAudioRegKey.bitpersample * 2 / 8;
                            //rk_printf("MovLen1 = %d, i = %d", MovLen1, i);
                            for(j = 0; j < gpstAudioControlData->TrackSize; j++)
                            {
                                MovLen2 = (gpstAudioControlData->TrackSize - j - 1) *  gpstAudioControlData->pAudioRegKey.bitpersample * 2 / 8;
                                //rk_printf("MovLen2 = %d, j = %d", MovLen2, j);

                                for(k = 0; k < (gpstAudioControlData->pAudioRegKey.bitpersample * 2 / 8); k++)
                                {
                                    ((uint8*)gpstAudioControlData->AudioPtr)[MovLen1 + MovLen2 + k] = ((uint8*)gpstAudioControlData->AudioPtr)[MovLen3 + MovLen2 + k];
                                }
                            }
                        }
                        gpstAudioControlData->AudioLen = gpstAudioControlData->TrackSize;
                    }
                }

                gpstAudioControlData->TrackSegment = 0;

            }
            else
            {
                gpstAudioControlData->AudioPtr += (gpstAudioControlData->TrackSize * 8);
                gpstAudioControlData->TrackSegment++;
                gpstAudioControlData->pAudioRegKey.CurrentTime += gpstAudioControlData->TrackTime;
            }

            if(gpstAudioControlData->pAudioRegKey.CurrentTime - gpstAudioControlData->pAudioRegKey.LastTime >= 1000)
            {
                gpstAudioControlData->pAudioRegKey.LastTime = (gpstAudioControlData->pAudioRegKey.CurrentTime / 1000) * 1000;
                //rk_printf("gpstAudioControlData->pAudioRegKey.CurrentTime = %d, %d", gpstAudioControlData->pAudioRegKey.CurrentTime, gpstAudioControlData->pAudioRegKey.CurrentTime%1000);
                {
                    AUDIO_CALLBACK * pCur;

                    pCur = gpstAudioControlData->pfAudioState;

                    while(pCur != NULL)
                    {
                        pCur->pfAudioState(AUDIO_STATE_TIME_CHANGE);
                        pCur = pCur->pNext;
                    }
                }
            }


            // fade in init
            if (gpstAudioControlData->AudioErrorFrameNum < 2)
            {
                if (++gpstAudioControlData->AudioErrorFrameNum >= 2)
                {
                    //rk_printf("here player=%d playstate=%d",gpstAudioControlData->AudioPlayerState,gpstAudioControlData->AudioPlayState);

                    AudioDev_FadeInit(gpstAudioControlData->hAudio, gpstAudioControlData->pAudioRegKey.samplerate/2,FADE_IN);
                    AudioDev_SetVol(gpstAudioControlData->hAudio, gpstAudioControlData->playVolume);
                    gpstAudioControlData->AudioFadeInStart = 1;

                    {
                        AUDIO_CALLBACK * pCur;

                        pCur = gpstAudioControlData->pfAudioState;

                        while(pCur != NULL)
                        {
                            pCur->pfAudioState(AUDIO_STATE_PLAY);
                            pCur = pCur->pNext;
                        }
                    }
                }
                else
                {
                    #ifdef _FADE_PROCESS_
                    gpstAudioControlData->AudioFadeInStart = 0;
                    #endif
                }
            }

            if(gpstAudioControlData->AudioFadeInStart == 0)
            {
                memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 2 * gpstAudioControlData->pAudioRegKey.bitpersample) / 8);
            }


            if(gpstAudioControlData->AudioFadeOutStart)
            {
                if (AudioDev_FadeOk(gpstAudioControlData->hAudio) == RK_SUCCESS)
                {
                    if (gpstAudioControlData->AudioEndFade == 0)
                    {
                        rk_printf("fade out start");
                        AudioDev_FadeInit(gpstAudioControlData->hAudio, gpstAudioControlData->pAudioRegKey.samplerate / 32,FADE_OUT);
                        gpstAudioControlData->AudioEndFade = 1;
                    }
                }
            }
            else if ((gpstAudioControlData->pAudioRegKey.TotalTime - gpstAudioControlData->pAudioRegKey.CurrentTime) <= 500)
            {
                if (AudioDev_FadeOk(gpstAudioControlData->hAudio) == RK_SUCCESS)
                {
                    if (gpstAudioControlData->AudioEndFade == 0)
                    {
                        AudioDev_FadeInit(gpstAudioControlData->hAudio, gpstAudioControlData->pAudioRegKey.samplerate / 32,FADE_OUT);
                        gpstAudioControlData->AudioEndFade = 1;
                        gpstAudioControlData->AudioNextFile = 0;
                    }
                }
            }

            if (gpstAudioControlData->AudioEndFade == 1)
            {
                if (AudioDev_FadeOk(gpstAudioControlData->hAudio) == RK_SUCCESS)
                {

                ERR_IN_FADE:

                    if(gpstAudioControlData->AudioFadeOutStart)
                    {
                        rk_printf("fade out over");
                        if(gpstAudioControlData->AudioPlayerState != AUDIO_STATE_PLAY)
                        {
                            gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;

                            if(gpstAudioControlData->AudioPlayerState == AUDIO_STATE_PAUSE)
                            {
                                AUDIO_CALLBACK * pCur;

                                pCur = gpstAudioControlData->pfAudioState;

                                while(pCur != NULL)
                                {
                                    pCur->pfAudioState(AUDIO_STATE_PAUSE);
                                    pCur = pCur->pNext;
                                }
                            }
                        }
                        else
                        {
                            if(gpstAudioControlData->AudioNextFile == 0)
                            {
                                // out play over
                                AudioStop(Audio_Stop_Normal);
                            }
                            else if(gpstAudioControlData->AudioNextFile == Audio_Stop_Force)
                            {
                                //app send stop cmd
                                AudioStop(Audio_Stop_Force);
                                gpstAudioControlData->AudioPlayerState = AUDIO_STATE_STOP;
                                {
                                    AUDIO_CALLBACK * pCur;

                                    pCur = gpstAudioControlData->pfAudioState;

                                    while(pCur != NULL)
                                    {
                                        pCur->pfAudioState(AUDIO_STATE_STOP);
                                        pCur = pCur->pNext;
                                    }
                                }
                            }
                            else
                            {
                                //app send next or prev file;
                                AudioStop(Audio_Stop_Force);
                                gpstAudioControlData->AudioStopMode = Audio_Stop_Force;
                                AudioGetNextMusic(gpstAudioControlData->AudioNextFile);
                            }
                        }
                        gpstAudioControlData->AudioFadeOutStart = 0;
                        gpstAudioControlData->AudioEndFade = 0;
                        goto replay;
                    }

                    memset((uint8*)gpstAudioControlData->AudioPtr, 0x00, (gpstAudioControlData->AudioLen * 2 * gpstAudioControlData->pAudioRegKey.bitpersample) / 8);

                }
            }

#ifdef _RK_SPECTRUM_
            //printf("\n.._RK_SPECTRUM_...AudioPlayState =%d SpectrumCnt =%d ..SpectrumEn==%d\n",gpstAudioControlData->AudioPlayState,gpstAudioControlData->SpectrumCnt,gpstAudioControlData->SpectrumEn);
            if (gpstAudioControlData->SpectrumEn)
            {
                short Spectrum_data[128];
                uint32 currentTimeMs;
                uint32 currentTime200Ms;
                currentTimeMs = SysTickCounter * 10;
                currentTime200Ms = currentTimeMs / 100;    //refresh spectrum every 100ms
                if (currentTime200Ms != gpstAudioControlData->CurrentTimeMsBk)
                {
                    memset(gpstAudioControlData->SpectrumOut, 0, SPECTRUM_LINE_M*sizeof(char));
                    gpstAudioControlData->CurrentTimeMsBk = currentTime200Ms;

                    if(AUDIO_STATE_PLAY == gpstAudioControlData->AudioPlayState)
                    {
                        if(gpstAudioControlData->pAudioRegKey.bitpersample == 32)
                        {
                           int i = 0;
                           char *ptr = (char *)gpstAudioControlData->AudioPtr;
                           for(i=1;i<129;i++)
                           {
                              Spectrum_data[i-1] = (short)(ptr[4*i-1]<<8 | ptr[4*i-2]);
                           }
                        }
                        else if(gpstAudioControlData->pAudioRegKey.bitpersample == 24)
                        {
                           int i = 0;
                           char *ptr = (char *)gpstAudioControlData->AudioPtr;
                           for(i=1;i<129;i++)
                           {
                              Spectrum_data[i-1] = (short)(ptr[3*i-1]<<8 | ptr[3*i-2]);
                           }
                        }
                        else
                        {
                            memcpy(Spectrum_data, (short*)gpstAudioControlData->AudioPtr, 256);
                        }

                        DoSpectrum((short *)Spectrum_data,&gpstAudioControlData->SpectrumOut[0]);

                        if(2 > gpstAudioControlData->SpectrumOut[9])
                        {
                            gpstAudioControlData->SpectrumOut[10] = gpstAudioControlData->SpectrumOut[9] ;
                            gpstAudioControlData->SpectrumOut[11] = gpstAudioControlData->SpectrumOut[9] ;
                        }
                        else
                        {
                            gpstAudioControlData->SpectrumOut[10] = gpstAudioControlData->SpectrumOut[9] - 1;
                            gpstAudioControlData->SpectrumOut[11] = gpstAudioControlData->SpectrumOut[9] - 2;
                        }

                        AUDIO_CALLBACK * pCur;
                        pCur = gpstAudioControlData->pfAudioState;
                        while(pCur != NULL)
                        {
                            pCur->pfAudioState(AUDIO_STATE_UPDATA_SPECTRUM);
                            pCur = pCur->pNext;
                        }
                    }
                }
            }
#endif
            AudioDev_Write(gpstAudioControlData->hAudio, 0, (uint8 *)gpstAudioControlData->AudioPtr);
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
** Name: AudioSeekTo
** Input:uint32 ms
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2016.9.26
** Time: 15:18:51
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t AudioSeekTo(uint32 ms)
{

    if (AUDIO_STATE_STOP == gpstAudioControlData->AudioPlayerState)
    {
        return TRUE;
    }

    gpstAudioControlData->AudioPlayState = AUDIO_STATE_PAUSE;

    gpstAudioControlData->AudioPlayerState = AUDIO_STATE_FFD; // audio player state
    gpstAudioControlData->pAudioRegKey.CurrentTime = ms;

    CodecSeek(gpstAudioControlData->pAudioRegKey.CurrentTime, 0);
    gpstAudioControlData->TrackSegment = gpstAudioControlData->MaxTrackSegment - 1;
    AudioResume();

    {
        AUDIO_CALLBACK * pCur;

        pCur = gpstAudioControlData->pfAudioState;

        while(pCur != NULL)
        {
            pCur->pfAudioState(AUDIO_STATE_TIME_CHANGE);
            pCur = pCur->pNext;
        }
    }

    return TRUE;
}


/*******************************************************************************
** Name: AudioControlTaskCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 19:50:53
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t AudioControlTaskCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        pTask->State = TASK_STATE_WORKING;
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

#ifdef _MEDIA_MODULE_
/*******************************************************************************
** Name: MediaSetFloderInfo
** Input:
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void MediaSetFloderInfo(void)
{
    int i;
    uint32 CurFileNum;
    uint16 TotalItems;
    uint16 MaxDispItem = 0;

    CurFileNum = MediaGetCurFileNum(gpstAudioControlData->AudioFileInfo.CurrentFileNum, gpstAudioControlData->MediaFloderInfo.MusicDirDeep,
                    gpstAudioControlData->MediaFloderInfo.DirClus[gpstAudioControlData->MediaFloderInfo.MusicDirDeep]);

    for(i = gpstAudioControlData->MediaFloderInfo.MusicDirDeep; i <= gpstAudioControlData->SubDirInfo.DirDeep; i++)
    {
        TotalItems = gpstAudioControlData->SubDirInfo.TotalSubDir[i] + gpstAudioControlData->SubDirInfo.TotalFile[i];
        MaxDispItem = TotalItems < MAX_DISP_ITEM_NUM ? TotalItems : MAX_DISP_ITEM_NUM;

        gpstAudioControlData->MediaFloderInfo.DirClus[i] = gpstAudioControlData->SubDirInfo.DirClus[i];

        if(i == gpstAudioControlData->SubDirInfo.DirDeep)
        {
            gpstAudioControlData->MediaFloderInfo.CurItemId[i] = gpstAudioControlData->SubDirInfo.TotalSubDir[i] + CurFileNum;
        }
        else
        {
            gpstAudioControlData->MediaFloderInfo.CurItemId[i] = gpstAudioControlData->SubDirInfo.CurDirNum[i];
        }

        if(gpstAudioControlData->SubDirInfo.TotalSubDir[i] > 0) //add All files
            gpstAudioControlData->MediaFloderInfo.CurItemId[i]++;

        if(gpstAudioControlData->MediaFloderInfo.CurItemId[i] > (MAX_DISP_ITEM_NUM - 1))
        {
            gpstAudioControlData->MediaFloderInfo.Cursor[i] = MAX_DISP_ITEM_NUM - 1;
        }
        else
        {
            gpstAudioControlData->MediaFloderInfo.Cursor[i] = gpstAudioControlData->MediaFloderInfo.CurItemId[i];
        }
    }

    gpstAudioControlData->MediaFloderInfo.MusicDirDeep = gpstAudioControlData->SubDirInfo.DirDeep;
}

/*******************************************************************************
** Name: MediaGetTotalFiles
** Input:uint32 DirClus
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN uint16 MediaGetTotalFiles(UINT32 DirClus)
{
    uint32 offset;
    uint16 TotalFiles = 0;
    FILE_TREE_BASIC FileTreeBasic;

    if (DirClus != 0xffffffff)
    {
        for (offset = DirClus; ; offset++)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(offset), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));
#ifdef _RK_CUE_
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE || FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
#else
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
#endif
            {
                TotalFiles++;
            }

            if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
            {
                break;
            }
        }
    }

    return TotalFiles;
}

/*******************************************************************************
** Name: MediaGetTotalSubDir
** Input:uint32 DirClus
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN uint16 MediaGetTotalSubDir(UINT32 DirClus)
{
    uint32 offset;
    uint16 TotSubDir = 0;
    FILE_TREE_BASIC FileTreeBasic;

    if (DirClus != 0xffffffff)
    {
        for (offset = DirClus; ; offset++)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(offset), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));

            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
            {
                TotSubDir++;
            }

            if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
            {
                break;
            }
        }
    }

    return TotSubDir;
}

/*******************************************************************************
** Name: MediaGetTotalSubDir
** Input:
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN uint32 MediaGetDirClus(UINT32 DirClus, uint32 Index)
{
    UINT32 cluster = 0xffffffff;
    UINT16 temp;
    FILE_TREE_BASIC FileTreeBasic;

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), (uint8 *)&temp, 2);
    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));
    if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
    {
        FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 2)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), (uint8 *)&cluster, 4);
    }

    return cluster;
}

/*******************************************************************************
** Name: MediaGotoNextDir
** Input:
** Return: void
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void MediaGotoNextDir(void)
{
    if ((gpstAudioControlData->SubDirInfo.TotalSubDir[gpstAudioControlData->SubDirInfo.DirDeep] == 0) //该目录下没有子目录, 要找父目录或同级目录
            || (gpstAudioControlData->SubDirInfo.DirDeep == (MAX_DIR_DEPTH - 1))) //该目录下所有子目录查找完, 要找父目录或同级目录
    {
        while(1)
        {
            if(gpstAudioControlData->SubDirInfo.DirDeep == gpstAudioControlData->MediaFloderInfo.MusicDirDeep) //找到All files所在层目录, 不能再往上找
            {
                return;
            }

            gpstAudioControlData->SubDirInfo.DirDeep--; //即指向上一级目录

            if(gpstAudioControlData->SubDirInfo.CurDirNum[gpstAudioControlData->SubDirInfo.DirDeep]
                < (gpstAudioControlData->SubDirInfo.TotalSubDir[gpstAudioControlData->SubDirInfo.DirDeep] - 1))
            {
                gpstAudioControlData->SubDirInfo.CurDirNum[gpstAudioControlData->SubDirInfo.DirDeep]++;
                gpstAudioControlData->SubDirInfo.DirDeep++;

                gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep]
                    = MediaGetDirClus(gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep - 1], gpstAudioControlData->SubDirInfo.CurDirNum[gpstAudioControlData->SubDirInfo.DirDeep - 1]);
                break;
            }
        }
    }
    else //该目录下还有子目录,要找它的第一个子目录
    {
        gpstAudioControlData->SubDirInfo.CurDirNum[gpstAudioControlData->SubDirInfo.DirDeep] = 0;
        gpstAudioControlData->SubDirInfo.DirDeep++;

        gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep]
            = MediaGetDirClus(gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep - 1], 0);
    }

    gpstAudioControlData->SubDirInfo.TotalFile[gpstAudioControlData->SubDirInfo.DirDeep]
        = MediaGetTotalFiles(gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep]);

    gpstAudioControlData->SubDirInfo.TotalSubDir[gpstAudioControlData->SubDirInfo.DirDeep]
        = MediaGetTotalSubDir(gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep]);
}

/*******************************************************************************
** Name: MediaFloderGetCurFileNum
** Input:
** Return:
** Owner:ctf
** Date: 2015.11.3
** Time: 13:56:02
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN uint32 MediaGetCurFileNum(uint32 GlobeFileNum, UINT16 CurDirDeep, UINT32 CurDirClus)
{
    uint32 FileNum = GlobeFileNum;

    gpstAudioControlData->SubDirInfo.DirDeep = CurDirDeep;
    gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep] = CurDirClus;
    gpstAudioControlData->SubDirInfo.TotalFile[gpstAudioControlData->SubDirInfo.DirDeep] = MediaGetTotalFiles(CurDirClus);
    gpstAudioControlData->SubDirInfo.TotalSubDir[gpstAudioControlData->SubDirInfo.DirDeep] = MediaGetTotalSubDir(CurDirClus);

    while (FileNum >= gpstAudioControlData->SubDirInfo.TotalFile[gpstAudioControlData->SubDirInfo.DirDeep])
    {
        FileNum -= gpstAudioControlData->SubDirInfo.TotalFile[gpstAudioControlData->SubDirInfo.DirDeep];

        do
        {
            MediaGotoNextDir();           //遍历下一个目录，找完子目录再找同级目录

        }
        while ((gpstAudioControlData->SubDirInfo.TotalFile[gpstAudioControlData->SubDirInfo.DirDeep] == 0) && (CurDirDeep != gpstAudioControlData->SubDirInfo.DirDeep));

        if (gpstAudioControlData->SubDirInfo.DirDeep == CurDirDeep)
        {
            FileNum = 0;
            break;
        }
    }

    return FileNum;
}

/*******************************************************************************
** Name: MediaFloderGetFilePathByCurNum
** Input:uint16 * path, uint32 curnum
** Return: rk_err_t
** Owner:ctf
** Date: 2015.11.3
** Time: 13:56:02
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t MediaFloderGetFilePathByCurNum(uint32 DirClus, uint16 * path, uint32 CurNum)
{
    UINT16 temp;
    UINT16 pathlen = 0;
    UINT16 filelen = 0;
    FILE_TREE_BASIC FileTreeBasic;
    UINT16 LongFileName[MAX_FILENAME_LEN];

    memset(LongFileName, 0, MAX_FILENAME_LEN *2);
    memset(path, 0, MAX_FILENAME_LEN *2);

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + CurNum), (uint8 *)&temp, 2);
    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ FILE_NAME_SAVE_ADDR_OFFSET, (uint8 *)LongFileName, MAX_FILENAME_LEN*2);
    filelen = StrLenW(LongFileName);

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_PATH_SAVE_ADDR_OFFSET, (uint8 *)path, MAX_FILENAME_LEN*2);
    pathlen = StrLenW(path);

    if(pathlen + filelen < MAX_DIRPATH_LEN)
    {
        memcpy(path + pathlen, LongFileName, filelen*2);
    }
    else
    {
        printf("file filepath >= 259\n");
        return RK_ERROR;
    }

    *(path + pathlen + filelen) = 0x0000;

    //debug_hex((char *)path, StrLenW(path) * 2  + 2, 16);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaFloderPlayListNextFile
** Input:SYS_FILE_INFO * pAudioFileInfo, int next
** Return: rk_err_t
** Owner:ctf
** Date: 2015.11.3
** Time: 14:54:10
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t MediaFloderPlayListNextFile(SYS_FILE_INFO * pstAudioFileInfo, int direct)
{
    int i;
    uint32 offset;
    uint32 CurFileNum;
    uint16 TotalItems;
    uint16 MaxDispItem = 0;

    if(pstAudioFileInfo->Range == FIND_FILE_RANGE_ALL)   //All files
    {
        if(direct < 0)
        {
            if(pstAudioFileInfo->CurrentFileNum == 0)
                pstAudioFileInfo->CurrentFileNum = pstAudioFileInfo->TotalFiles - 1;
            else
                pstAudioFileInfo->CurrentFileNum += direct;
        }
        else
        {
            pstAudioFileInfo->CurrentFileNum += direct;

            if(pstAudioFileInfo->CurrentFileNum >= pstAudioFileInfo->TotalFiles)
                pstAudioFileInfo->CurrentFileNum = 0;
        }

        CurFileNum = MediaGetCurFileNum(pstAudioFileInfo->CurrentFileNum, gpstAudioControlData->MediaFloderInfo.MusicDirDeep,
            gpstAudioControlData->MediaFloderInfo.DirClus[gpstAudioControlData->MediaFloderInfo.MusicDirDeep]);

        offset = gpstAudioControlData->SubDirInfo.TotalSubDir[gpstAudioControlData->SubDirInfo.DirDeep] + CurFileNum;

        MediaFloderGetFilePathByCurNum(gpstAudioControlData->SubDirInfo.DirClus[gpstAudioControlData->SubDirInfo.DirDeep], pstAudioFileInfo->path, offset);
    }
    else
    {
        TotalItems = pstAudioFileInfo->TotalFiles + gpstAudioControlData->MediaFloderInfo.TotalSubDir;
        MaxDispItem = TotalItems < MAX_DISP_ITEM_NUM ? TotalItems : MAX_DISP_ITEM_NUM;

        if(direct < 0)
        {
            if(pstAudioFileInfo->CurrentFileNum == 0)
            {
                pstAudioFileInfo->CurrentFileNum = pstAudioFileInfo->TotalFiles - 1;
                gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] = MaxDispItem - 1;
            }
            else
            {
                pstAudioFileInfo->CurrentFileNum += direct;

                if(gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] != gpstAudioControlData->MediaFloderInfo.TotalSubDir)
                    gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] += direct;
            }
        }
        else
        {
            gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] += direct;
            if(gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] >= MaxDispItem)
            {
                gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] = MaxDispItem - 1;
            }

            pstAudioFileInfo->CurrentFileNum += direct;
            if(pstAudioFileInfo->CurrentFileNum >= pstAudioFileInfo->TotalFiles)
            {
                pstAudioFileInfo->CurrentFileNum = 0;
                gpstAudioControlData->MediaFloderInfo.Cursor[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] = gpstAudioControlData->MediaFloderInfo.TotalSubDir;
            }
        }

        gpstAudioControlData->MediaFloderInfo.CurItemId[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] = pstAudioFileInfo->CurrentFileNum + gpstAudioControlData->MediaFloderInfo.TotalSubDir;

        if(gpstAudioControlData->MediaFloderInfo.TotalSubDir > 0)
            offset = gpstAudioControlData->MediaFloderInfo.CurItemId[gpstAudioControlData->MediaFloderInfo.MusicDirDeep] - 1;
        else
            offset = gpstAudioControlData->MediaFloderInfo.CurItemId[gpstAudioControlData->MediaFloderInfo.MusicDirDeep];

        MediaFloderGetFilePathByCurNum(gpstAudioControlData->MediaFloderInfo.DirClus[gpstAudioControlData->MediaFloderInfo.MusicDirDeep], pstAudioFileInfo->path, offset);
    }
}

/*******************************************************************************
** Name: MediaGetFilePathByCurNum
** Input:uint16 * path, uint32 curnum
** Return: rk_err_t
** Owner:ctf
** Date: 2015.11.3
** Time: 13:56:02
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t MediaGetFilePathByCurNum(uint16 * path, uint32 CurNum)
{
#ifdef _MEDIA_MODULE_
    UINT16 i;
    UINT16 temp;
    UINT8  ucBufTemp[8];
    UINT32 AddrOffset;

    uint16 fileLen = 0;
    uint16 pathLen = 0;

    UINT16 LongFileName[MAX_FILENAME_LEN];
    UINT8  FileInfoBuf[MAX_FILENAME_LEN *2];

    memset(path, 0, MAX_DIRPATH_LEN*2);
    memset(LongFileName, 0, MAX_FILENAME_LEN*2);

    FW_ReadDataBaseByByte((gSysConfig.MediaDirTreeInfo.ulFileSortInfoSectorAddr << 9) + (UINT32)((CurNum + gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[gSysConfig.MediaDirTreeInfo.MusicDirDeep]) * 2), ucBufTemp, 2);
    temp = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); //获得对应的文件保存号

    memset(FileInfoBuf, 0, MEDIA_ID3_SAVE_CHAR_NUM *2);
    AddrOffset = (UINT32)(temp)*BYTE_NUM_SAVE_PER_FILE + DIR_PATH_SAVE_ADDR_OFFSET;
    FW_ReadDataBaseByByte((gSysConfig.MediaDirTreeInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, FileInfoBuf, MAX_FILENAME_LEN*2);
    for(i=0; i<MAX_FILENAME_LEN; i++)
    {
        path[i] = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }
    pathLen = StrLenW(path);

    memset(FileInfoBuf, 0, MEDIA_ID3_SAVE_CHAR_NUM *2);
    AddrOffset = (UINT32)(temp)*BYTE_NUM_SAVE_PER_FILE + FILE_NAME_SAVE_ADDR_OFFSET;
    FW_ReadDataBaseByByte((gSysConfig.MediaDirTreeInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, FileInfoBuf, MAX_FILENAME_LEN*2);
    for(i=0; i<MAX_FILENAME_LEN; i++)
    {
        LongFileName[i] = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }
    fileLen = StrLenW(LongFileName);

    if(pathLen + fileLen < MAX_DIRPATH_LEN)
    {
        memcpy(path + pathLen, LongFileName, fileLen*2);
    }
    else
    {
        printf("file filepath >= 259\n");
        return RK_ERROR;
    }

    *(path + pathLen + fileLen) = 0x0000;
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaPlayListNextFile
** Input:SYS_FILE_INFO * pAudioFileInfo, int next
** Return: rk_err_t
** Owner:ctf
** Date: 2015.11.3
** Time: 14:54:10
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t MediaPlayListNextFile(SYS_FILE_INFO * pstAudioFileInfo, int direct)
{

    uint16 MaxDispItem = 0;
    MaxDispItem = pstAudioFileInfo->TotalFiles < MAX_DISP_ITEM_NUM ? pstAudioFileInfo->TotalFiles : MAX_DISP_ITEM_NUM;

    if(direct < 0)
    {
        if(pstAudioFileInfo->CurrentFileNum == 0)
        {
            pstAudioFileInfo->CurrentFileNum = pstAudioFileInfo->TotalFiles - 1;
            gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] = MaxDispItem - 1;
        }
        else
        {
            pstAudioFileInfo->CurrentFileNum += direct;

            if(gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] != 0)
                gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] += direct;
        }
    }
    else
    {
        gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] += direct;
        if(gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] >= MaxDispItem)
        {
            gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] = MaxDispItem - 1;
        }

        pstAudioFileInfo->CurrentFileNum += direct;
        if(pstAudioFileInfo->CurrentFileNum >= pstAudioFileInfo->TotalFiles)
        {
            pstAudioFileInfo->CurrentFileNum = 0;
            gSysConfig.MediaDirTreeInfo.Cursor[gSysConfig.MediaDirTreeInfo.MusicDirDeep] = 0;
        }
    }

    gSysConfig.MediaDirTreeInfo.CurItemId[gSysConfig.MediaDirTreeInfo.MusicDirDeep] = pstAudioFileInfo->CurrentFileNum;

    MediaGetFilePathByCurNum(pstAudioFileInfo->path, pstAudioFileInfo->CurrentFileNum);

    return RK_SUCCESS;
}
#endif

#ifdef _FS_
/*******************************************************************************
** Name: DirPlayListNextFile
** Input:SYS_FILE_INFO * pAudioFileInfo, int next
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 14:54:10
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t DirPlayListNextFile(SYS_FILE_INFO * pstAudioFileInfo, int direct)
{
    if(pstAudioFileInfo->Range == FIND_FILE_RANGE_ALL)
    {
        pstAudioFileInfo->CurrentFileNum += direct;

        DirDev_GetFilePathByGlobalNum(pstAudioFileInfo->hDirDev, pstAudioFileInfo->path, pstAudioFileInfo->CurrentFileNum);
    }
    else
    {
        if(direct < 0)
        {
            if(pstAudioFileInfo->CurrentFileNum == 0)
            {
                pstAudioFileInfo->CurrentFileNum = pstAudioFileInfo->TotalFiles - 1;
            }
            else
            {
                pstAudioFileInfo->CurrentFileNum += direct;
            }
        }
        else
        {
            pstAudioFileInfo->CurrentFileNum += direct;

            if(pstAudioFileInfo->CurrentFileNum >= pstAudioFileInfo->TotalFiles)
            {
                pstAudioFileInfo->CurrentFileNum = 0;
            }
        }

        rk_printf("curfile = %d, total file = %d RepeatMode=%d", pstAudioFileInfo->CurrentFileNum, pstAudioFileInfo->TotalFiles,pstAudioFileInfo->RepeatMode);

        DirDev_GetFilePathByCurNum(pstAudioFileInfo->hDirDev, pstAudioFileInfo->path, pstAudioFileInfo->CurrentFileNum);
    }
}
/*******************************************************************************
** Name: DirPlayListInit
** Input:SYS_FILE_INFO * pAudioFileInfo
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 14:52:35
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t DirPlayListInit(SYS_FILE_INFO * pstAudioFileInfo)
{
    uint16 FileName[256];
    uint32 PathLen, FileNameLen;
    uint16 * pPath;
    uint32 IsFile;


    pPath = (uint16 *)pstAudioFileInfo->path;
    PathLen = StrLenW(pPath);

    IsFile = 1;

    if(pPath[PathLen - 1] == '\\')
    {
        IsFile = 0;
    }

    if(IsFile)
    {
        pPath += PathLen;



        while(*pPath != '\\')
        {
            PathLen--;
            pPath--;
        }


        FileNameLen = StrLenW(pPath + 1);

        memcpy((uint8 *)FileName, (uint8 *)(pPath + 1), FileNameLen * 2 + 2);

        *(pPath + 1) = 0;

    }

    if(DirDev_BuildDirInfo(pstAudioFileInfo->hDirDev,
        pstAudioFileInfo->pExtStr, pstAudioFileInfo->path) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    if(pstAudioFileInfo->Range == FIND_FILE_RANGE_ALL)
    {
        if(DirDev_GetTotalFile(pstAudioFileInfo->hDirDev, &pstAudioFileInfo->TotalFiles) != RK_SUCCESS)
        {
            return RK_ERROR;
        }

        if(pstAudioFileInfo->TotalFiles == 0)
        {
            return RK_EMPTY;
        }

        DirDev_GetFilePathByGlobalNum(pstAudioFileInfo->hDirDev, pstAudioFileInfo->path, pstAudioFileInfo->CurrentFileNum);
    }
    else
    {
        if(DirDev_GetCurDirTotalFile(pstAudioFileInfo->hDirDev, &pstAudioFileInfo->TotalFiles) != RK_SUCCESS)
        {
            return RK_ERROR;
        }

        if(pstAudioFileInfo->TotalFiles == 0)
        {
            return RK_EMPTY;
        }

        if(pstAudioFileInfo->CurrentFileNum >= pstAudioFileInfo->TotalFiles)
        {
            pstAudioFileInfo->CurrentFileNum = 0;
        }

        if(IsFile)
        {
            pstAudioFileInfo->CurrentFileNum = 0;

            do
            {
                DirDev_GetFilePathByCurNum(pstAudioFileInfo->hDirDev, pstAudioFileInfo->path, pstAudioFileInfo->CurrentFileNum);
                if(StrCmpW(pPath + 1, FileName, FileNameLen) == 0)
                {
                    break;
                }
                else
                {
                    pstAudioFileInfo->CurrentFileNum++;
                }

            }while(pstAudioFileInfo->CurrentFileNum < pstAudioFileInfo->TotalFiles);

            if(pstAudioFileInfo->CurrentFileNum >= pstAudioFileInfo->TotalFiles)
            {
                return RK_ERROR;
            }

        }
        else
        {
            DirDev_GetFilePathByCurNum(pstAudioFileInfo->hDirDev, pstAudioFileInfo->path, pstAudioFileInfo->CurrentFileNum);
        }
    }

}
#endif

/*******************************************************************************
** Name: AudioFREQDeInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:45:55
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioFREQDeInit(void)
{
    switch (CurrentDecCodec)
    {
#ifdef SBC_INCLUDE
                case (CODEC_SBC_DEC):
                    {
                        FREQ_ExitModule(FREQ_SBC);
                        break;
                    }
#endif


#ifdef MP3_DEC_INCLUDE
        case (CODEC_MP3_DEC):
            {
                if ((gpstAudioControlData->pAudioRegKey.bitrate <= 128000)&&(gpstAudioControlData->pAudioRegKey.samplerate <= 44100))
                {
                    FREQ_ExitModule(FREQ_MP3);
                }
                else
                {
                    FREQ_ExitModule(FREQ_MP3H);
                }
                break;
            }
#endif

#ifdef XXX_DEC_INCLUDE
        case (CODEC_XXX_DEC):
            {
                if (gpstAudioControlData->pAudioRegKey.bitrate <= 129000)
                {
                    if ((gpstAudioControlData->pAudioRegKey.samplerate == CodecFS_32KHz) &&(gpstAudioControlData->pAudioRegKey.bitrate/1000 == 22))
                    {
                        FREQ_ExitModule(FREQ_XXXH);
                    }
                    else
                        FREQ_ExitModule(FREQ_XXX);
                }
                else
                {
                    FREQ_ExitModule(FREQ_XXXH);
                }
                break;
            }
#endif

#ifdef AAC_DEC_INCLUDE
        case (CODEC_AAC_DEC):
            {
                FREQ_ExitModule(FREQ_AAC);
                break;
            }
#endif

#ifdef WAV_DEC_INCLUDE
        case (CODEC_WAV_DEC):
            {
                if(gpstAudioControlData->pAudioRegKey.samplerate > 48000)
                {
                    FREQ_ExitModule(FREQ_HWAV);
                }
                else
                {
                    FREQ_ExitModule(FREQ_WAV);
                }
                break;
            }
#endif

#ifdef AMR_DEC_INCLUDE
        case (CODEC_AMR_DEC):
            {
                FREQ_ExitModule(FREQ_AMR);
                break;
            }
#endif

#ifdef APE_DEC_INCLUDE
        case (CODEC_APE_DEC):
            {
                if (gpstAudioControlData->pAudioRegKey.samplerate <= CodecFS_48KHz)
                    FREQ_ExitModule(FREQ_APE);
                else
                    FREQ_ExitModule(FREQ_HAPE);
                break;
            }
#endif

#ifdef FLAC_DEC_INCLUDE
        case (CODEC_FLAC_DEC):
            {
                FREQ_ExitModule(FREQ_FLAC);
                break;
            }
#endif
#ifdef OGG_DEC_INCLUDE
        case (CODEC_OGG_DEC):
            if (((gpstAudioControlData->pAudioRegKey.bitrate <= 240000)&&(gpstAudioControlData->pAudioRegKey.channels ==1)) ||
                    ((gpstAudioControlData->pAudioRegKey.bitrate > 54000)&&(gpstAudioControlData->pAudioRegKey.bitrate <= 172000)&&(gpstAudioControlData->pAudioRegKey.channels ==2)))
            {
                FREQ_ExitModule(FREQ_OGG);
                break;
            }
            else if ((gpstAudioControlData->pAudioRegKey.bitrate > 172000)&&(gpstAudioControlData->pAudioRegKey.bitrate < 256000)&&(gpstAudioControlData->pAudioRegKey.channels ==2))
            {
                FREQ_ExitModule(FREQ_NOGG);
                break;
            }
            else if (( gpstAudioControlData->pAudioRegKey.bitrate >= 256000)&&( gpstAudioControlData->pAudioRegKey.bitrate <= 380000)&&(gpstAudioControlData->pAudioRegKey.channels ==2))
            {
                FREQ_ExitModule(FREQ_HOGG);
                break;
            }
            else if (((380000 <gpstAudioControlData->pAudioRegKey.bitrate <= 500000)&&(gpstAudioControlData->pAudioRegKey.channels ==2))||(gpstAudioControlData->pAudioRegKey.bitrate <= 54000))
            {
                FREQ_ExitModule(FREQ_EHOGG);
                break;
            }
#endif
#ifdef HIFI_APE_DECODE
        case (CODEC_HIFI_APE_DEC):
            {
                FREQ_ExitModule(FREQ_HAPE);
                break;
            }
#endif
#ifdef HIFI_FlAC_DECODE
        case (CODEC_HIFI_FLAC_DEC):
            {
                FREQ_ExitModule(FREQ_HFLAC);
                break;
            }
#endif
#ifdef HIFI_AlAC_DECODE
        case (CODEC_HIFI_ALAC_DEC):
            {
                FREQ_ExitModule(FREQ_HALAC);
                break;
            }
#endif
    }

#ifdef DC_FILTER
    if ((gpstAudioControlData->pAudioRegKey.samplerate != CodecFS_44100Hz) && (gpstAudioControlData->pAudioRegKey.samplerate != CodecFS_22050Hz)&&(gpstAudioControlData->pAudioRegKey.samplerate != CodecFS_11025Hz))
    {
        if ((CurrentDecCodec != CODEC_WAV_DEC)&&((gpstAudioControlData->pAudioRegKey.samplerate == CodecFS_32KHz) || (gpstAudioControlData->pAudioRegKey.bitrate >= 256000)))
        {
            FREQ_ExitModule(FREQ_DC_FILTER_HIGH);
        }
        else if ((CurrentDecCodec == CODEC_MP3_DEC)&&(((MP3_FORMAT_FLAG == 0x11)&&(gpstAudioControlData->pAudioRegKey.bitrate == 128000))||
                 (((MP3_FORMAT_FLAG & 0xF0) == 0x20)&&(gpstAudioControlData->pAudioRegKey.bitrate <= 128000)&&(gpstAudioControlData->pAudioRegKey.bitrate >= 96000))))
        {
            FREQ_ExitModule(FREQ_DC_FILTER_HIGH);
        }
        else if (((CurrentDecCodec != CODEC_MP3_DEC)&&(gpstAudioControlData->pAudioRegKey.samplerate == CodecFS_48KHz))
                 ||((CurrentDecCodec == CODEC_MP3_DEC)&&(gpstAudioControlData->pAudioRegKey.samplerate == CodecFS_48KHz)&&(MP3_FORMAT_FLAG ==0x11) ))
        {
            FREQ_ExitModule(FREQ_DC_FILTER_HIGH);
        }
        else
        {
            FREQ_ExitModule(FREQ_DC_FILTER_LOW);
        }
    }
#endif

}
/*******************************************************************************
** Name: AudioFileClose
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:38:35
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioFileClose(void)
{
    #ifdef _FS_
    if(gpstAudioControlData->hFifo == NULL)
    {
        FileDev_CloseFile(gpstAudioControlData->hFile[0]);
        gpstAudioControlData->hFile[0] = NULL;

#ifdef FLAC_DEC_INCLUDE
        if (CurrentDecCodec == CODEC_FLAC_DEC)
        {
            FileDev_CloseFile(gpstAudioControlData->hFile[1]);
            gpstAudioControlData->hFile[1] = NULL;
        }
#endif

#ifdef AAC_DEC_INCLUDE
        if (CurrentDecCodec == CODEC_AAC_DEC)
        {
            FileDev_CloseFile(gpstAudioControlData->hFile[1]);
            gpstAudioControlData->hFile[1] = NULL;

            FileDev_CloseFile(gpstAudioControlData->hFile[2]);
            gpstAudioControlData->hFile[2] = NULL;
        }
#endif

#ifdef HIFI_AlAC_DECODE
        if (CurrentDecCodec == CODEC_HIFI_ALAC_DEC)
        {
            FileDev_CloseFile(gpstAudioControlData->hFile[1]);
            gpstAudioControlData->hFile[1] = NULL;

            FileDev_CloseFile(gpstAudioControlData->hFile[2]);
            gpstAudioControlData->hFile[2] = NULL;
        }
#endif
    }
#endif

}
/*******************************************************************************
** Name: AudioWaitBBStop
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:29:19
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioWaitBBStop(void)
{
    rk_printf("shut off b core");
    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_CLOSE,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData(1,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
}
/*******************************************************************************
** Name: AudioDeHWInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:25:55
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioDeHWInit(void)
{
    switch (CurrentDecCodec)
    {

#ifdef  SBC_INCLUDE
                case CODEC_SBC_DEC:
#ifdef A_CORE_DECODE
                    {
                        //...

                        //Int Disable
                        //...

                        //clock gate&Reset Init
                    }
#else
                    {
                        //...
                        AudioWaitBBStop();
                        BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                        FW_RemoveSegment(SEGMENT_ID_SBC_DECODE_BIN);
                        FW_RemoveSegment(SEGMENT_ID_SBC_DECODE);
                #endif
                    }
#endif
                    break;
#endif


#ifdef MP3_DEC_INCLUDE
        case CODEC_MP3_DEC:
#ifdef A_CORE_DECODE
            {
#if(MP3_EQ_WAIT_SYNTH)
                {
                    mp3_wait_synth();
                }
#endif

                //Int Disable
                IntDisable(INT_ID_IMDCT);
                IntDisable(INT_ID_SYNTH);
                IntPendingClear(INT_ID_IMDCT);
                IntPendingClear(INT_ID_SYNTH);
                IntUnregister(INT_ID_IMDCT);
                IntUnregister(INT_ID_SYNTH);
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_MP3_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_MP3_DECODE);
                #endif
            }
#endif

            ScuSoftResetCtr(IMDCT_SRST, 1);
            ScuSoftResetCtr(SYNTH_SRST, 1);
            ScuClockGateCtr(HCLK_SYNTH_GATE,0);
            ScuClockGateCtr(HCLK_IMDCT_GATE,0);

            break;
#endif

#ifdef  XXX_DEC_INCLUDE
        case CODEC_XXX_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_XXX_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_XXX_DECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef  AAC_DEC_INCLUDE
        case CODEC_AAC_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_AAC_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_AAC_DECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef WAV_DEC_INCLUDE
        case CODEC_WAV_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_WAV_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_WAV_DECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef AMR_DEC_INCLUDE
        case CODEC_AMR_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_AMR_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_AMR_DECODE);
                #endif
            }
#endif
            break;
#endif


#ifdef APE_DEC_INCLUDE
        case CODEC_APE_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_APE_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_APE_DECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef FLAC_DEC_INCLUDE
        case CODEC_FLAC_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_FLAC_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_FLAC_DECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef OGG_DEC_INCLUDE
        case CODEC_OGG_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_OGG_DECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_OGG_DECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef HIFI_APE_DECODE
        case CODEC_HIFI_APE_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_APE_HDECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_APE_HDECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef HIFI_FlAC_DECODE
        case CODEC_HIFI_FLAC_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_FLAC_HDECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_FLAC_HDECODE);
                #endif
            }
#endif
            break;
#endif

#ifdef HIFI_AlAC_DECODE
        case CODEC_HIFI_ALAC_DEC:
#ifdef A_CORE_DECODE
            {
                //...

                //Int Disable
                //...

                //clock gate&Reset Init
            }
#else
            {
                //...
                AudioWaitBBStop();
                BcoreDev_ShutOff(gpstAudioControlData->hBcore);
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(SEGMENT_ID_ALAC_HDECODE_BIN);
                FW_RemoveSegment(SEGMENT_ID_ALAC_HDECODE);
                #endif
            }
#endif
            break;
#endif

        default:
            break;
    }

}
/*******************************************************************************
** Name: CloseTrack
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:23:17
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void CloseTrack(void)
{
    CodecClose();
}

/*******************************************************************************
** Name: AudioVariableDeInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 11:21:09
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioVariableDeInit(void)
{
    return;
}
/*******************************************************************************
** Name: AudioWaitBBStart
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.25
** Time: 17:57:12
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioWaitBBStart(void)
{
    gFileHandle.codecType = CurrentDecCodec;
    /*
    #if 1
    StreamControlTask_SendFileHandle(gpstAudioControlData->hFile[0], 0);
    StreamControlTask_SendFileHandle(gpstAudioControlData->hFile[1], 1);
    StreamControlTask_SendFileHandle(gpstAudioControlData->hFile[2], 2);
    #else
    StreamControlTask_SendFileHandle(gpstAudioControlData->hFifo, 4);
    #endif
    */
    if(gpstAudioControlData->hFifo == NULL)
    {
        StreamControlTask_SendFileHandle(gpstAudioControlData->hFile[0], 0);
        StreamControlTask_SendFileHandle(gpstAudioControlData->hFile[1], 1);
        StreamControlTask_SendFileHandle(gpstAudioControlData->hFile[2], 2);
    }
    else
    {
        StreamControlTask_SendFileHandle(gpstAudioControlData->hFifo, 9);
    }

    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FILE_OPEN,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteA2BData((int)&gFileHandle,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    rk_printf("AudioWait Bcore Init...");
    rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
    rk_printf("AudioWait Bcore  Init OK");
}

/*******************************************************************************
** Name: AudioControlTask_AudioDecodeProc
** Input:MSG_ID id, void * msg
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:12:20
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioDecodeProc(AUDIO_CMD id, void * msg)
{
    BOOLEAN ret = TRUE;
    unsigned long  HoldOnTimeTemp;
    AUDIO_DEV_ARG stParg;

    switch (id)
    {
        case AUDIO_CMD_DECSTART:
            if(gpstAudioControlData->AudioPlayerState == AUDIO_STATE_STOP)
            {
                gpstAudioControlData->AudioPlayerState = AUDIO_STATE_PLAY;
                AudioStart();
            }
            break;

        case AUDIO_CMD_STOP:
            if(gpstAudioControlData->AudioPlayerState != AUDIO_STATE_STOP)
            {
                #ifdef _FADE_PROCESS_
                if(gpstAudioControlData->AudioPlayState != AUDIO_STATE_PLAY)
                {
                    AudioStop((int)msg);
                    if(((int)msg) == Audio_Stop_Force)
                    {
                        gpstAudioControlData->AudioPlayerState = AUDIO_STATE_STOP;
                        {
                            AUDIO_CALLBACK * pCur;

                            pCur = gpstAudioControlData->pfAudioState;

                            while(pCur != NULL)
                            {
                                pCur->pfAudioState(AUDIO_STATE_STOP);
                                pCur = pCur->pNext;
                            }
                        }
                    }
                }
                else
                {
                    gpstAudioControlData->AudioFadeOutStart = 1;
                    gpstAudioControlData->AudioNextFile = (int)msg;
                }
                #else
                AudioStop((int)msg);
                if((int)msg == Audio_Stop_Force)
                {
                    gpstAudioControlData->AudioPlayerState = AUDIO_STATE_STOP;
                    {
                        AUDIO_CALLBACK * pCur;

                        pCur = gpstAudioControlData->pfAudioState;

                        while(pCur != NULL)
                        {
                            pCur->pfAudioState(AUDIO_STATE_STOP);
                            pCur = pCur->pNext;
                        }
                    }
                }
                #endif
            }
            break;

        case AUDIO_CMD_PAUSE:
            AudioPause();
            break;

        case AUDIO_CMD_RESUME:
            AudioResume();
            break;

        case AUDIO_CMD_FFD:     //FFW
            AudioFFD((int)msg);
            break;

        case AUDIO_CMD_FFW:     //FFD
            AudioFFW((int)msg);
            break;

        case AUDIO_CMD_SEEKTO:
            if(gpstAudioControlData->DirctPlay == 0)
            {
                AudioSeekTo((int)msg);
            }
            break;
#if 0
        case AUDIO_CMD_ABPLAY:
            AudioSetAB();
            break;

        case AUDIO_CMD_ABSETA:
            AudioSetAB_A();
            break;

        case MSG_AUDIO_ABSTOP:
            AudioABStop();
            break;
#endif

        case AUDIO_CMD_VOLUMESET:
            gpstAudioControlData->playVolume = (int)msg;
            gSysConfig.OutputVolume = gpstAudioControlData->playVolume;
            AudioSetVolume((int)msg);
            {
                AUDIO_CALLBACK * pCur;

                pCur = gpstAudioControlData->pfAudioState;

                while(pCur != NULL)
                {
                    pCur->pfAudioState(AUDIO_STATE_VOLUME_CHANGE);
                    pCur = pCur->pNext;
                }
            }
            break;

        case AUDIO_CMD_EQSET:
            gpstAudioControlData->EqMode = (uint32)msg;
            AudioDev_SetEQ(gpstAudioControlData->hAudio, (uint32)msg);
            break;

        case AUDIO_CMD_REPEATSET:
            if(gpstAudioControlData->AudioFileInfo.RepeatMode > AUDIO_REPEAT)
            {
                gpstAudioControlData->AudioFileInfo.RepeatMode = (uint8)msg + AUDIO_ALLONCE;
                gpstAudioControlData->AudioFileInfo.Range = FIND_FILE_RANGE_ALL;
            }
            else
            {
                gpstAudioControlData->AudioFileInfo.RepeatMode = (uint8)msg;
                gpstAudioControlData->AudioFileInfo.Range = FIND_FILE_RANGE_DIR;
            }

            gSysConfig.MusicConfig.RepeatModeBak = gpstAudioControlData->AudioFileInfo.RepeatMode;
            break;


        case AUDIO_CMD_SHUFFLESET:
            gpstAudioControlData->AudioFileInfo.PlayOrder = (uint8)msg;
            gSysConfig.MusicConfig.PlayOrder = (uint8)msg;
            break;


        case AUDIO_CMD_NEXTFILE:
            if(gpstAudioControlData->AudioPlayState != AUDIO_STATE_PLAY)
            {
                AudioNextMusic(1);
            }
            else
            {
                #ifdef _FADE_PROCESS_
                gpstAudioControlData->AudioFadeOutStart = 1;
                gpstAudioControlData->AudioNextFile = Audio_Stop_NextFile;
                #else
                AudioNextMusic(1);
                #endif
            }
            break;

        case AUDIO_CMD_PREVFILE:
            if(gpstAudioControlData->AudioPlayState != AUDIO_STATE_PLAY)
            {
                AudioNextMusic(-1);
            }
            else
            {
                #ifdef _FADE_PROCESS_
                gpstAudioControlData->AudioFadeOutStart = 1;
                gpstAudioControlData->AudioNextFile = Audio_Stop_PrevFile;
                #else
                AudioNextMusic(-1);
                #endif
            }
            break;

        case AUDIO_CMD_FF_PAUSE:
            AudioFFPause();
            break;

        case AUDIO_CMD_FF_RESUME:
            AudioFFResume();
            break;

        case AUDIO_CMD_FF_STOP:
            AudioFFStop();
            break;

        default:
            ret = FALSE;
            break;


    }
    return;

}


/*******************************************************************************
** Name: AudioSetVolume
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:46:40
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioSetVolume(uint32 Volume)
{
    AudioDev_SetVol(gpstAudioControlData->hAudio, Volume);
}

/*******************************************************************************
** Name: AudioDecodingGetOutBuffer
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 14:53:22
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
__irq COMMON FUN void AudioDecodingGetOutBuffer(void)
{
    uint32 cmd;
    uint32 data;

    cmd  = MailBoxReadB2ACmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadB2AData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    //printf("\n MailBoxReadB2ACmd\n");
    gDecCmd  = cmd;
    gDecData = data;

    MailBoxClearB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
    //printf("B2A Decode Service cmd = %d\n",cmd);

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL:
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL:
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;

        case MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock) - 4);
            gpMediaBlock.DecodeOver = 1;
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CMPL:

            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock) - 4);

            gpMediaBlock.DecodeOver = 1;

            //printf("gpMediaBlock.needDecode = %d", gpMediaBlock.needDecode);
            if(gpMediaBlock.needDecode)
            {
                gpMediaBlock.needDecode = 0;
                //printf("decode ok");
                rkos_semaphore_give_fromisr(osAudioDecodeOk);
            }
            break;

        case MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL:
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;


        case MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock) - 4);
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL:
            memcpy(&gpMediaBlock,(MediaBlock *)data,sizeof(MediaBlock) - 4);
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL:
            //printf("MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL\n");
            rkos_semaphore_give_fromisr(osAudioDecodeOk);
            break;

        case MEDIA_MSGBOX_CMD_FLAC_SEEKFAST:
            #if 0
            {
                FILE* fp;
                gpFlacSeekFastParm = (FLAC_SEEKFAST_OP_t*)data;
                fp = (FILE*)gpFlacSeekFastParm->fp;
                FileInfo[(int)fp].Offset = gpFlacSeekFastParm->offset;
                FileInfo[(int)fp].Clus   = gpFlacSeekFastParm->clus;

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData(0,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            #endif
            break;


        case MEDIA_MSGBOX_CMD_FLAC_GETSEEK_INFO:
            #if 0
            {
                gpFlacSeekFastParm->clus = FileInfo[(int)data].Clus;
                gpFlacSeekFastParm->offset = FileInfo[(int)data].Offset;

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_INFO_CMPL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
                MailBoxWriteA2BData((uint32)gpFlacSeekFastParm,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
            }
            #endif
            break;


        default:
            return;
    }
}

/*******************************************************************************
** Name: AudioControlTask_Resume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 10:21:46
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t AudioControlTask_Resume(void)
{

}
/*******************************************************************************
** Name: AudioControlTask_Suspend
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 10:21:46
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t AudioControlTask_Suspend(void)
{

}

/*******************************************************************************
** Name: CheckID3V2Tag
** Input:uint8 *buf
** Return: int32
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 16:27:47
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN int32 CheckID3V2Tag(uint8 *pucBuffer)
{
    // The first three bytes of the tag should be "ID3".
    if ((pucBuffer[0] !=    'I') || (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next byte should be the value 3 (i.e. we support ID3v2.3.0).
    //if(pucBuffer[3]   != 3)
    if (pucBuffer[3] < 2 && pucBuffer[3] > 4)
    {
        return(0);
    }

    // The next byte should be less than 0xff.
    if (pucBuffer[4] == 0xff)
    {
        return(0);
    }

    // We don't care about the next byte.  The following four bytes should be
    // less than 0x80.
    if ((pucBuffer[6] >= 0x80) || (pucBuffer[7] >= 0x80)    ||
            (pucBuffer[8] >= 0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of the ID3v2 tag.
    return((pucBuffer[6] << 21) | (pucBuffer[7] << 14) |
           (pucBuffer[8] <<  7) |  pucBuffer[9]);
}

/*******************************************************************************
** Name: AudioCheckStreamType
** Input:uint16 * path,  HDC hFile
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 16:21:48
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioCheckStreamType(uint16 * path,  HDC hFile)
{
    uint8 char_buf[512];
    uint8 *buf = char_buf;
    int ID3_Length ;
    int  Redundancy_len = 0;
    uint32 pathlen;

    if(gpstAudioControlData->hFifo != NULL)
    {
        fifoDev_Read(hFile, char_buf, 512, 0, SYNC_MODE, NULL);
    }
    else
    {
        #ifdef _FS_
        FileDev_ReadFile(hFile,char_buf, 512);
        #endif
    }
#if 0
    {
        uint32 i;

        for (i = 0; i < 512; i++)
        {
            if (i%16 == 0)
            {
                printf("\n");
            }
            printf("0x%2x ", buf[i]);
        }
    }
#endif

    ID3_Length = CheckID3V2Tag(buf);
    if (ID3_Length)
    {
        rk_printf("audio auto anlayse == id3");
        ID3_Length += 10;
        if (ID3_Length < (512 -17))
        {
            buf += ID3_Length;
            Redundancy_len =  ID3_Length;
        }
        else
        {
            if(gpstAudioControlData->hFifo != NULL)
            {
                fifoDev_ReadSeek(hFile, 0, ID3_Length, 0);
            }
            else
            {
                #ifdef _FS_
                FileDev_FileSeek(hFile, 0, ID3_Length);
                #endif
            }

            if(gpstAudioControlData->hFifo != NULL)
            {
                fifoDev_Read(hFile, char_buf, 512, 0,  SYNC_MODE, NULL);
            }
            else
            {
                #ifdef _FS_
                FileDev_ReadFile(hFile,char_buf, 512);
                #endif
            }
        }
        rk_printf("ID3 len = 0x%x",ID3_Length);
    }

    while (1)
    {
        if ((buf[0] == 0x30) && (buf[1] == 0x26) && (buf[2] == 0xB2))
        {
            rk_printf("audio auto anlayse == XXX");
            *path++ = 'X';
            *path++ = 'X';
            *path++ = 'X';
            break;
        }

        if ((buf[2] == 0x41) && (buf[3] == 0x4D) && (buf[4] == 0x52))
        {
            rk_printf("audio auto anlayse == AMR");
            *path++ = 'A';
            *path++ = 'M';
            *path++ = 'R';
            break;
        }

        if ((((*(uint16 *)buf) & 0xe0ff) == 0xe0ff) &&((buf[15] !='a') &&(buf[16] !='a') &&(buf[17] !='c')))
        {
            uint32 framelen, framesec, frameoffset;

            uint32 i;

            framelen = ((uint16)((buf[3] & 0x03) << 11)) | ((uint16)((buf[4])  << 3)) | ((uint16)(buf[5] >> 5));

            rk_printf("audio auto anlayse == maybe no.id3.mp3, framesize = %d", framelen);

            if (framelen == 0)
            {
                rk_printf("audio auto anlayse == no.id3.mp3");
                *path++ = 'M';
                *path++ = 'P';
                *path++ = '3';
                break;
            }
            else
            {
                framesec = framelen / 512;
                frameoffset = framelen % 512;

                if (framesec > 0)
                {
                    if(gpstAudioControlData->hFifo != NULL)
                    {
                        fifoDev_ReadSeek(hFile, SEEK_SET, framesec * 512+ID3_Length, 0);
                    }
                    else
                    {
                        #ifdef _FS_
                        FileDev_FileSeek(hFile, SEEK_SET, framesec * 512+ID3_Length);
                        #endif
                    }

                    if(gpstAudioControlData->hFifo != NULL)
                    {
                        fifoDev_Read(hFile, char_buf, 512, 0, SYNC_MODE, NULL);
                    }
                    else
                    {
                        #ifdef _FS_
                        FileDev_ReadFile(hFile, char_buf, 512);
                        #endif
                    }

                    buf = char_buf;
#if 0
                    {
                        uint32 i;

                        for (i = 0; i < 512; i++)
                        {
                            if (i%16 == 0)
                            {
                                printf("\n");
                            }
                            printf("0x%2x ", buf[i]);
                        }
                    }
#endif
                }

                if (((buf[frameoffset] & 0xff) == 0xff)  && ((buf[frameoffset+1] & 0xf0) == 0xf0)&&((buf[frameoffset+3] & 0x03) !=0x03))
                {
                    rk_printf("audio auto anlayse == AAC");
                    *path++ = 'A';
                    *path++ = 'A';
                    *path++ = 'C';
                    break;
                }
                else
                {
                    rk_printf("audio auto anlayse == no.id3.mp3");

                    *path++ = 'M';
                    *path++ = 'P';
                    *path++ = '3';
                    break;
                }
            }


        }

        if ((buf[4] == 'f') && (buf[5] == 't') && (buf[6] == 'y') && (buf[7] == 'p'))
        {
            int16 tempcnt = 0;
            int8 isgetaacm4a = 0;
            int  aac_moovstart = 0;

            for (tempcnt = 0; tempcnt < 512; tempcnt++)
            {
                if ((buf[tempcnt] == 's') && (buf[tempcnt + 1] == 't') && (buf[tempcnt + 2] == 's') && (buf[tempcnt + 3] == 'd'))
                {
                    if ((buf[tempcnt + 16] == 'm') && (buf[tempcnt + 17] == 'p') && (buf[tempcnt + 18] == '4') && (buf[tempcnt + 19] == 'a'))
                    {
                        rk_printf("\naudio auto anlayse cx1== AAC\n");
                        isgetaacm4a = 7;
                        *path++ = 'A';
                        *path++ = 'A';
                        *path++ = 'C';
                        break;
                    }

                    if ((buf[tempcnt + 16] == 'a') && (buf[tempcnt + 17] == 'l') && (buf[tempcnt + 18] == 'a') && (buf[tempcnt + 19] == 'c'))
                    {
                        rk_printf("\naudio auto anlayse cx1== M4A\n");
                        isgetaacm4a = 7;
                        *path++ = 'M';
                        *path++ = '4';
                        *path++ = 'A';
                        break;
                    }
                }

                if ((buf[tempcnt] == 'm') && (buf[tempcnt + 1] == 'd') && (buf[tempcnt + 2] == 'a') && (buf[tempcnt + 3] == 't'))
                {
                    aac_moovstart = (buf[tempcnt - 4] << 24) + (buf[tempcnt - 3] << 16) + (buf[tempcnt - 2] << 8) + buf[tempcnt - 1];
                    if(gpstAudioControlData->hFifo != NULL)
                    {
                        fifoDev_ReadSeek(hFile, SEEK_SET, aac_moovstart + 0x80, 0);
                        fifoDev_Read(hFile, char_buf, 512, 0, SYNC_MODE, NULL);
                    }
                    else
                    {
                        #ifdef _FS_
                        FileDev_FileSeek(hFile, SEEK_SET, aac_moovstart + 0x80);
                        FileDev_ReadFile(hFile, char_buf, 512);
                        #endif
                    }

                    for (tempcnt = 0; tempcnt < 512; tempcnt++)
                    {
                        if ((char_buf[tempcnt] == 's') && (char_buf[tempcnt + 1] == 't') && (char_buf[tempcnt + 2] == 's') && (char_buf[tempcnt + 3] == 'd'))
                        {
                            if ((char_buf[tempcnt + 16] == 'm') && (char_buf[tempcnt + 17] == 'p') && (char_buf[tempcnt + 18] == '4') && (char_buf[tempcnt + 19] == 'a'))
                            {
                                rk_printf("\naudio auto anlayse cx2 == AAC\n");
                                isgetaacm4a = 7;
                                *path++ = 'A';
                                *path++ = 'A';
                                *path++ = 'C';
                                //break;
                            }

                            if ((char_buf[tempcnt + 16] == 'a') && (char_buf[tempcnt + 17] == 'l') && (char_buf[tempcnt + 18] == 'a') && (char_buf[tempcnt + 19] == 'c'))
                            {
                                rk_printf("\naudio auto anlayse cx2== ALAC\n");
                                isgetaacm4a = 7;
                                *path++ = 'M';
                                *path++ = '4';
                                *path++ = 'A';
                                //break;
                            }
                        }

                        if (isgetaacm4a == 7)
                            break;
                    }
                }

                if (isgetaacm4a == 7)
                    break;
            }

            if (isgetaacm4a == 7)
                    break;


            rk_printf("audio auto anlayse cx2== XXX");
            *path++ = 'M';
            *path++ = '4';
            *path++ = 'A';
            break;

        }

        if ((buf[9] == 0) && (buf[10] == 0) && (buf[11] == 'l') && (buf[12] == 'i') &&
                 (buf[13] == 'b') && (buf[14] == 'f') && (buf[15] == 'a') && (buf[16] == 'a') && (buf[17] == 'c'))
        {
            rk_printf("audio auto anlayse == AAC");
            *path++ = 'A';
            *path++ = 'A';
            *path++ = 'C';
            break;
        }

        if ((buf[0] == 'R') && (buf[1] == 'I') && (buf[2] == 'F') && (buf[3] == 'F'))
        {
            rk_printf("audio auto anlayse == WAV");
            *path++ = 'W';
            *path++ = 'A';
            *path++ = 'V';
            break;
        }

        if ((buf[0] == 'f') && (buf[1] == 'L') && (buf[2] == 'a') && (buf[3] == 'C'))
        {
            rk_printf("audio auto anlayse == flac");
            *path++ = 'L';
            *path++ = 'A';
            *path++ = 'C';
            break;
        }

        if ((buf[0] == 'M') && (buf[1] == 'A') && (buf[2] == 'C'))
        {
            rk_printf("audio auto anlayse == ape");
            *path++ = 'A';
            *path++ = 'P';
            *path++ = 'E';
            break;
        }

        if ((buf[0] == 'O') && (buf[1] == 'g') && (buf[2] == 'g') && (buf[3] == 'S'))
        {
            rk_printf("audio auto anlayse == ogg");
            *path++ = 'O';
            *path++ = 'G';
            *path++ = 'G';
            break;
        }

        if ((buf[0] == 0x9C) && (buf[1] == 0xFD) && (buf[2] == 0x30) && (buf[3] == 0x22))
        {
            rk_printf("audio auto anlayse == SBC");
            *path++ = 'S';
            *path++ = 'B';
            *path++ = 'C';
            break;
        }

        buf++;
        Redundancy_len++;
        if (Redundancy_len == 512)
        {
            int ret;

            if(gpstAudioControlData->hFifo != NULL)
            {
                ret = fifoDev_Read(hFile, char_buf, 512, 0, SYNC_MODE, NULL);
            }
            else
            {
                #ifdef _FS_
                ret = FileDev_ReadFile(hFile,char_buf, 512);
                #endif
            }

            if (ret < 512)
            {
                DEBUG("Don't know file type");
                break;
            }
            buf = char_buf;
            Redundancy_len = 0;
        }
        else if (Redundancy_len == 1024)
        {
            DEBUG("Don't know file type");
            break;
        }


    }

    if(gpstAudioControlData->hFifo != NULL)
    {
        fifoDev_ReadSeek(hFile, SEEK_SET, 0, 0);
    }
    else
    {
        #ifdef _FS_
        FileDev_FileSeek(hFile, SEEK_SET, 0);
        #endif
    }

}

/*******************************************************************************
** Name: GetFileType
** Input:UINT16 *ExtendName, UINT8 *pStr
** Return: uint8
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 16:13:08
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN uint8 GetFileType(UINT16 *ExtendName, UINT8 *pStr)
{
    UINT8 Len;
    UINT8 Retval = 0xff;
    UINT8 i;

    uint8 * pBuffer;

    i = 0;

    Len = strlen((char*)pStr);

    pBuffer = (uint8 *)ExtendName;

    while (i <= Len)
    {
        i += 3;
        if (((*(pBuffer + 0) == *(pStr + 0)) || ((*(pBuffer + 0) + ('a' - 'A')) == *(pStr + 0)) || ((*(pBuffer + 0) - ('a' - 'A')) == *(pStr + 0)))
            && ((*(pBuffer + 2) == *(pStr + 1)) || ((*(pBuffer + 2) + ('a' - 'A')) == *(pStr + 1)) || ((*(pBuffer + 2) - ('a' - 'A')) == *(pStr + 1)))
            && ((*(pBuffer + 2) == *(pStr + 1)) || ((*(pBuffer + 2) + ('a' - 'A')) == *(pStr + 1)) || ((*(pBuffer + 2) - ('a' - 'A')) == *(pStr + 1)))
            && ((*(pBuffer + 2) == *(pStr + 1)) || ((*(pBuffer + 2) + ('a' - 'A')) == *(pStr + 1)) || ((*(pBuffer + 2) - ('a' - 'A')) == *(pStr + 1))))
        {
            break;
        }
        pStr += 3;
    }

    if (i <= Len)
    {
        Retval = i / 3;
    }
    return (Retval);
}

/*******************************************************************************
** Name: AudioCodec
** Input:UINT16 *pBuffer, UINT8 *pStr
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 16:11:33
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioCodec(UINT16 *pBuffer, UINT8 *pStr)
{
    UINT8 temp;
    temp = GetFileType(pBuffer, pStr);
    switch (temp)
    {
        case 1:     //mp3
        case 2:     //mp2
        case 3:     //mp1
#ifdef MP3_DEC_INCLUDE
            CurrentDecCodec = CODEC_MP3_DEC;
#endif
            break;

        case 4:     //XXX
#ifdef  XXX_DEC_INCLUDE
            CurrentDecCodec = CODEC_XXX_DEC;
#endif
            break;

        case 5:     //wav
#ifdef WAV_DEC_INCLUDE
            CurrentDecCodec = CODEC_WAV_DEC;
#endif
            break;

        case 6:     //ape
#ifdef APE_DEC_INCLUDE
            CurrentDecCodec = CODEC_APE_DEC;
#endif

#ifdef HIFI_APE_DECODE
            CurrentDecCodec = CODEC_HIFI_APE_DEC;
#endif
            break;

        case 7:     //flac
#ifdef FLAC_DEC_INCLUDE
            CurrentDecCodec = CODEC_FLAC_DEC;
#endif
#ifdef HIFI_FlAC_DECODE
            CurrentDecCodec = CODEC_HIFI_FLAC_DEC;
#endif
            break;
        case 8:     //AAC
        case 11:    //MP4
        case 12:    // 3GP
#ifdef AAC_DEC_INCLUDE
            CurrentDecCodec = CODEC_AAC_DEC;
#endif
            break;

        case 9:     //M4A
#ifdef HIFI_AlAC_DECODE
            CurrentDecCodec = CODEC_HIFI_ALAC_DEC;
#endif
            break;

        case 10:    //OGG
#ifdef OGG_DEC_INCLUDE
            CurrentDecCodec = CODEC_OGG_DEC;
#endif
            break;

        case 13:    //SBC
#ifdef SBC_INCLUDE
            CurrentDecCodec = CODEC_SBC_DEC;
#endif
            break;

        case 16:    //AMR
#ifdef _AMR_DECODE_
            rk_printf("type : AMR\n\n");
            CurrentDecCodec = CODEC_AMR_DEC;
#endif
            break;
        default:
            CurrentDecCodec = 0xff;
            break;
    }
}
/*******************************************************************************
** Name: AudioHoldonInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:49:15
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioHoldonInit(void)
{

}
/*******************************************************************************
** Name: AudioFREQInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:46:06
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioFREQInit(void)
{
    switch (CurrentDecCodec)
    {
#ifdef SBC_INCLUDE
                        case (CODEC_SBC_DEC):
                            {
                                FREQ_EnterModule(FREQ_SBC);
                                break;
                            }
#endif


#ifdef MP3_DEC_INCLUDE
        case (CODEC_MP3_DEC):
            {
                if ((gpstAudioControlData->pAudioRegKey.bitrate <= 128000)&&(gpstAudioControlData->pAudioRegKey.samplerate <= 44100))
                {
                    FREQ_EnterModule(FREQ_MP3);
                }
                else
                {
                    FREQ_EnterModule(FREQ_MP3H);

                }
                break;
            }
#endif

#ifdef XXX_DEC_INCLUDE
        case (CODEC_XXX_DEC):
            {
                if (gpstAudioControlData->pAudioRegKey.bitrate < 128000)
                {
                    if ((gpstAudioControlData->pAudioRegKey.samplerate == CodecFS_32KHz) &&(gpstAudioControlData->pAudioRegKey.bitrate/1000 == 22))
                    {
                        FREQ_EnterModule(FREQ_XXXH);
                    }
                    else if ((gpstAudioControlData->pAudioRegKey.samplerate == CodecFS_44100Hz) &&(gpstAudioControlData->pAudioRegKey.bitrate/1000 == 48))
                    {
                        FREQ_EnterModule(FREQ_XXXH);
                    }
                    else
                        FREQ_EnterModule(FREQ_XXX);
                }
                else
                {
                    FREQ_EnterModule(FREQ_XXXH);
                }
                break;
            }
#endif

#ifdef AAC_DEC_INCLUDE
        case (CODEC_AAC_DEC):
            {
                FREQ_EnterModule(FREQ_AAC);
                break;
            }
#endif

#ifdef WAV_DEC_INCLUDE
        case (CODEC_WAV_DEC):
            {
                if(gpstAudioControlData->pAudioRegKey.samplerate > 48000)
                {
                    FREQ_EnterModule(FREQ_HWAV);
                }
                else
                {
                    FREQ_EnterModule(FREQ_WAV);
                }
                break;
            }
#endif

#ifdef AMR_DEC_INCLUDE
        case (CODEC_AMR_DEC):
            {
                FREQ_EnterModule(FREQ_AMR);
                break;
            }
#endif

#ifdef APE_DEC_INCLUDE
        case (CODEC_APE_DEC):
            {
                if (gpstAudioControlData->pAudioRegKey.samplerate <= CodecFS_48KHz)
                    FREQ_EnterModule(FREQ_APE);
                else
                    FREQ_EnterModule(FREQ_HAPE);
                break;
            }
#endif

#ifdef FLAC_DEC_INCLUDE
        case (CODEC_FLAC_DEC):
            {
                FREQ_EnterModule(FREQ_FLAC);
                break;
            }
#endif
#ifdef OGG_DEC_INCLUDE
        case (CODEC_OGG_DEC):
            {
                if (((gpstAudioControlData->pAudioRegKey.bitrate <= 240000)&&(gpstAudioControlData->pAudioRegKey.channels ==1)) ||
                        ((gpstAudioControlData->pAudioRegKey.bitrate > 54000)&&(gpstAudioControlData->pAudioRegKey.bitrate <= 172000)&&(gpstAudioControlData->pAudioRegKey.channels ==2)))
                {
                    DEBUG("ENTER FREQ_OGG");
                    FREQ_EnterModule(FREQ_OGG);
                    break;
                }
                else if ((gpstAudioControlData->pAudioRegKey.bitrate > 172000)&&(gpstAudioControlData->pAudioRegKey.bitrate < 256000)&&(gpstAudioControlData->pAudioRegKey.channels ==2))
                {
                    DEBUG("ENTER FREQ_NOGG");
                    FREQ_EnterModule(FREQ_NOGG);
                    break;
                }
                else if ((gpstAudioControlData->pAudioRegKey.bitrate >= 256000)&&(gpstAudioControlData->pAudioRegKey.bitrate <= 380000)&&(gpstAudioControlData->pAudioRegKey.channels ==2))
                {
                    DEBUG("ENTER FREQ_HOGG");
                    FREQ_EnterModule(FREQ_HOGG);
                    break;
                }
                else if (((gpstAudioControlData->pAudioRegKey.bitrate <= 500000)&&(gpstAudioControlData->pAudioRegKey.channels ==2))||(gpstAudioControlData->pAudioRegKey.bitrate <= 54000))
                {
                    FREQ_EnterModule(FREQ_EHOGG);
                    DEBUG("ENTER FREQ_EHOGG");
                    break;
                }
            }
#endif
#ifdef HIFI_APE_DECODE
        case (CODEC_HIFI_APE_DEC):
            {
                FREQ_EnterModule(FREQ_HAPE);
                break;
            }
#endif
#ifdef HIFI_FlAC_DECODE
        case (CODEC_HIFI_FLAC_DEC):
            {
                FREQ_EnterModule(FREQ_HFLAC);
                break;
            }
#endif
#ifdef HIFI_AlAC_DECODE
        case (CODEC_HIFI_ALAC_DEC):
            {
                FREQ_EnterModule(FREQ_HALAC);
                break;
            }
#endif
    }

#ifdef DC_FILTER
    if ((gpstAudioControlData->pAudioRegKey.samplerate != FS_44100Hz) && (gpstAudioControlData->pAudioRegKey.samplerate != FS_22050Hz)&&(gpstAudioControlData->pAudioRegKey.samplerate != FS_11025Hz))
    {
        if ((CurrentDecCodec != CODEC_WAV_DEC)&&((gpstAudioControlData->pAudioRegKey.samplerate == FS_32KHz) || (gpstAudioControlData->pAudioRegKey.bitrate >= 256000)))
        {
            FREQ_EnterModule(FREQ_DC_FILTER_HIGH);
        }
        else if ((CurrentDecCodec == CODEC_MP3_DEC)&&(((MP3_FORMAT_FLAG == 0x11)&&(gpstAudioControlData->pAudioRegKey.bitrate == 128000))||
                 (((MP3_FORMAT_FLAG & 0xF0) == 0x20)&&(gpstAudioControlData->pAudioRegKey.bitrate <= 128000)&&(gpstAudioControlData->pAudioRegKey.bitrate >= 96000))))
        {
            FREQ_EnterModule(FREQ_DC_FILTER_HIGH);
        }
        else if (((CurrentDecCodec != CODEC_MP3_DEC)&&(gpstAudioControlData->pAudioRegKey.samplerate == FS_48KHz))
                 ||((CurrentDecCodec == CODEC_MP3_DEC)&&(gpstAudioControlData->pAudioRegKey.samplerate == FS_48KHz)&&(MP3_FORMAT_FLAG ==0x11) ))
        {
            FREQ_EnterModule(FREQ_DC_FILTER_HIGH);
        }
        else
        {
            FREQ_EnterModule(FREQ_DC_FILTER_LOW);
        }
        ret = DC1_FilterInit(gpstAudioControlData->pAudioRegKey.samplerate);

        gpstAudioControlData->pAudioRegKey.samplerate = ret;
    }
#endif
}
/*******************************************************************************
** Name: AudioCodecOpen
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:44:41
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t AudioCodecOpen(void)
{
    if (1 != CodeOpenDec(gpstAudioControlData->DirctPlay, gpstAudioControlData->SaveMemory))
    {
        rk_printfA("CodeOpenDec != 1\n");
        return ERROR;
    }
    CodecGetLength(&gpstAudioControlData->pAudioRegKey.TotalTime);
    CodecGetSampleRate(&gpstAudioControlData->pAudioRegKey.samplerate);
    CodecGetDecBitrate(&gpstAudioControlData->pAudioRegKey.bitrate);
    CodecGetChannels(&gpstAudioControlData->pAudioRegKey.channels);
    CodecGetBitPerSample(&gpstAudioControlData->pAudioRegKey.bitpersample);
    CodecGetFrameLen(&gpstAudioControlData->AudioLen);

#ifdef _MEMORY_LEAK_CHECH_
    if(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_HTTP)
    {
        if(gpstAudioControlData->AudioLen * 8 > 12 * 1024)
        {
            rk_printfA("sampels per frame too large");
            return ERROR;
        }
    }
#endif


    CodecGetTime(&gpstAudioControlData->pAudioRegKey.CurrentTime);

    gpstAudioControlData->pAudioRegKey.LastTime = gpstAudioControlData->pAudioRegKey.CurrentTime;

    rk_printfA("channel=%d bitpersample = %d fs = %d bitrate = %d len = %d, time=%dm:%ds %d ms",gpstAudioControlData->pAudioRegKey.channels,gpstAudioControlData->pAudioRegKey.bitpersample,gpstAudioControlData->pAudioRegKey.samplerate,gpstAudioControlData->pAudioRegKey.bitrate,gpstAudioControlData->AudioLen,gpstAudioControlData->pAudioRegKey.TotalTime/60000,gpstAudioControlData->pAudioRegKey.TotalTime/1000%60,gpstAudioControlData->pAudioRegKey.TotalTime%1000);

    if ((gpstAudioControlData->pAudioRegKey.bitrate <= 0)||(gpstAudioControlData->pAudioRegKey.samplerate <= 0))
    {
        return ERROR;
    }

#ifdef A_CORE_DECODE
    AudioCodecGetBufferSize(CurrentDecCodec, gpstAudioControlData->pAudioRegKey.samplerate);

#ifdef MP3_DEC_INCLUDE
    if (CurrentDecCodec == CODEC_MP3_DEC)
    {
        mp3_wait_synth();
    }
#endif

#ifdef FLAC_DEC_INCLUDE
    if (CurrentDecCodec != CODEC_FLAC_DEC)
#endif
    {
#ifdef AAC_DEC_INCLUDE
        if (CurrentDecCodec != CODEC_AAC_DEC)
#endif
        {
            //AudioFileChangeBuf(pRawFileCache,CodecBufSize);
        }
    }

#ifdef  XXX_DEC_INCLUDE
    if (CODEC_XXX_DEC == CurrentDecCodec)
    {

    }
#endif

#endif

    return OK;
}
/*******************************************************************************
** Name: AudioHWInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:43:55
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioHWInit(void)
{
    HDC hBcoreDev;
    switch (CurrentDecCodec)
    {

#ifdef  SBC_INCLUDE
        case CODEC_SBC_DEC:
            FW_LoadSegment(SEGMENT_ID_SBC_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_SBC_DECODE_BIN);
#endif

                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif


#ifdef MP3_DEC_INCLUDE
        case CODEC_MP3_DEC:

            FW_LoadSegment(SEGMENT_ID_MP3_DECODE, SEGMENT_OVERLAY_ALL);

            //clock gate&Reset Init
            ScuClockGateCtr(HCLK_SYNTH_GATE,1);
            ScuClockGateCtr(HCLK_IMDCT_GATE,1);
            ScuSoftResetCtr(IMDCT_SRST, 1);
            ScuSoftResetCtr(SYNTH_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(IMDCT_SRST, 0);
            ScuSoftResetCtr(SYNTH_SRST, 0);

#ifdef A_CORE_DECODE
            {
                //Int Init
                IntPendingClear(INT_ID_IMDCT);
                IntPendingClear(INT_ID_SYNTH);
                IntRegister(INT_ID_IMDCT, imdct36_handler);
                IntRegister(INT_ID_SYNTH, synth_handler);
                IntEnable(INT_ID_IMDCT);
                IntEnable(INT_ID_SYNTH);

                //Others
                AcceleratorHWInit();
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
               //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_MP3_DECODE_BIN);
#endif
                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef  XXX_DEC_INCLUDE
        case CODEC_XXX_DEC:
            FW_LoadSegment(SEGMENT_ID_XXX_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_XXX_DECODE_BIN);
#endif

                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef  AAC_DEC_INCLUDE
        case CODEC_AAC_DEC:
            FW_LoadSegment(SEGMENT_ID_AAC_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //

#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_AAC_DECODE_BIN);
#endif

                //Others
                if(gpstAudioControlData->hFifo == NULL)
                {
                    #ifdef _FS_
                    memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                    gFileHandle.handle1 = (unsigned char)0;
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);

                    gFileHandle.handle2 = (unsigned char)1;
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[1], &gFileHandle.curfileoffset[1]);

                    gFileHandle.handle3 = (unsigned char)2;
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[2], &gFileHandle.curfileoffset[2]);
                    #endif
                }
                else
                {
                    while(!gFileHandle.filesize)
                    {
                        fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gpstAudioControlData->AudioLen);
                        rk_printf("audiocontrol TotalSize = %d", gpstAudioControlData->AudioLen);
                    }
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gpstAudioControlData->AudioFileSeekOffset, 0);
                }

                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef WAV_DEC_INCLUDE
        case CODEC_WAV_DEC:
            FW_LoadSegment(SEGMENT_ID_WAV_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_WAV_DECODE_BIN);
#endif

                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef AMR_DEC_INCLUDE
        case CODEC_AMR_DEC:
            printf("AMR_BIN SEGMENT...\n");
            FW_LoadSegment(SEGMENT_ID_AMR_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                printf("audioHWinit amr\n");
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_AMR_DECODE_BIN);
#endif

                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    printf("audioHWinit amr gFileHandle.filesize=%d\n",gFileHandle.filesize);
                    gpMediaBlock.Total_length = gFileHandle.filesize;
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
                printf("audioHWinit AudioWaitBBStart....\n");
            }
#endif
            break;
#endif

#ifdef APE_DEC_INCLUDE
        case CODEC_APE_DEC:
            FW_LoadSegment(SEGMENT_ID_APE_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {

#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_APE_DECODE_BIN);
#endif

                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef FLAC_DEC_INCLUDE
        case CODEC_FLAC_DEC:
            FW_LoadSegment(SEGMENT_ID_FLAC_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //

#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_FLAC_DECODE_BIN);
#endif


                //Others
                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;
                if(gpstAudioControlData->hFifo == NULL)
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);


                    gFileHandle.handle2 = (unsigned char)1;
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[1], &gFileHandle.curfileoffset[1]);
                    #endif
                }
                else
                {
                    while(!gFileHandle.filesize)
                    {
                        fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gpstAudioControlData->AudioLen);
                        rk_printf("audiocontrol TotalSize = %d", gpstAudioControlData->AudioLen);
                    }
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gpstAudioControlData->AudioFileSeekOffset, 0);
                }

                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef OGG_DEC_INCLUDE
        case CODEC_OGG_DEC:
            FW_LoadSegment(SEGMENT_ID_OGG_DECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //

#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_OGG_DECODE_BIN);
#endif
                if(gpstAudioControlData->hFifo)
                {

                }
                else
                {
                   //Others
                    #ifdef _FS_
                    memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                    gFileHandle.handle1 = (unsigned char)0;
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef HIFI_APE_DECODE
        case CODEC_HIFI_APE_DEC:
            FW_LoadSegment(SEGMENT_ID_APE_HDECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_APE_HDECODE_BIN);
#endif

                //Others
                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;
                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef HIFI_FlAC_DECODE
        case CODEC_HIFI_FLAC_DEC:
            FW_LoadSegment(SEGMENT_ID_FLAC_HDECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_FLAC_HDECODE_BIN);
#endif
                memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                gFileHandle.handle1 = (unsigned char)0;

                if(gpstAudioControlData->hFifo)
                {
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);
                }
                else
                {
                    #ifdef _FS_
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);
                    #endif
                }
                AudioWaitBBStart();
            }
#endif
            break;
#endif

#ifdef HIFI_AlAC_DECODE
        case CODEC_HIFI_ALAC_DEC:
            FW_LoadSegment(SEGMENT_ID_ALAC_HDECODE, SEGMENT_OVERLAY_ALL);
#ifdef A_CORE_DECODE
            {
                //clock gate&Reset Init
                //...

                //Int Init
                //...

                //Others
                //...
                //AudioCodecGetBufferSize(CurrentDecCodec, FS_44100Hz);
                AudioCodec(gpstAudioControlData->AudioFileInfo.path, (UINT8 *)AudioFileExtString);
            }
#else
            {
                //
#ifdef BB_SYS_JTAG
                BcoreDev_Start(gpstAudioControlData->hBcore,SEGMENT_ID_BB_CODE);
#else
                BcoreDev_Start(gpstAudioControlData->hBcore, SEGMENT_ID_ALAC_HDECODE_BIN);
#endif
                if(gpstAudioControlData->hFifo)
                {
                    memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                    gFileHandle.handle1 = (unsigned char)0;
                    fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &gFileHandle.filesize);
                    fifoDev_GetOffset(gpstAudioControlData->hFifo, &gFileHandle.curfileoffset[0], 0);

                    gFileHandle.handle2 = (unsigned char)1;
                    gFileHandle.handle3 = (unsigned char)2;
                    fifoDev_GetOffset(gpstAudioControlData->hFile[1], &gFileHandle.curfileoffset[1], 1);
                    fifoDev_GetOffset(gpstAudioControlData->hFile[2], &gFileHandle.curfileoffset[2], 2);
                }
                else
                {
                    #ifdef _FS_
                    //Others
                    memset(&gFileHandle,0,sizeof(FILE_HANDLE_t));
                    gFileHandle.handle1 = (unsigned char)0;
                    FileDev_GetFileSize(gpstAudioControlData->hFile[0], &gFileHandle.filesize);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[0], &gFileHandle.curfileoffset[0]);

                    gFileHandle.handle2 = (unsigned char)1;
                    gFileHandle.handle3 = (unsigned char)2;
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[1], &gFileHandle.curfileoffset[1]);
                    FileDev_GetFileOffset(gpstAudioControlData->hFile[2], &gFileHandle.curfileoffset[2]);

                    #endif
                }

                AudioWaitBBStart();
            }
#endif
            break;
#endif

        default:
            break;
    }
}
/*******************************************************************************
** Name: AudioFileOpen
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:35:12
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN rk_err_t AudioFileOpen(void)
{
    #ifdef _FS_
    FILE_ATTR stFileAttr;
    #endif

    rk_err_t ret;
    uint16 PathLen;
    uint32 TotalSize;
    uint16 ExtendName[3];

    int CurrentCodecSave;

    #ifdef _FS_
    stFileAttr.ShortName[11] = 0;
    #endif


    PathLen = StrLenW(gpstAudioControlData->AudioFileInfo.path);

    memcpy((uint8 *)ExtendName, (uint8 *)gpstAudioControlData->AudioFileInfo.path[PathLen - 3], 6);

    //debug_hex((char *)gpstAudioControlData->AudioFileInfo.path, PathLen * 2  + 2, 16);

    if(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_FILE_BROWSER ||
        gpstAudioControlData->ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY ||
        gpstAudioControlData->ucSelPlayType == SOURCE_FROM_DB_FLODER ||
        (gpstAudioControlData->ucSelPlayType == SOURCE_FROM_RECORD))
    {
        #ifdef _FS_
        stFileAttr.Path = gpstAudioControlData->AudioFileInfo.path;
        stFileAttr.FileName = NULL;

        AudioCodec(ExtendName, (UINT8 *)AudioFileExtString);
        CurrentCodecSave = CurrentDecCodec;

        gpstAudioControlData->hFile[0] = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
        if ((rk_err_t)gpstAudioControlData->hFile[0] <= 0)
        {
            rk_printf("FileDev_OpenFile faile return -1");
            return RK_ERROR;
        }

        DEBUG("CurrentDecCodec = %d",CurrentDecCodec);

        AudioCheckStreamType(ExtendName, gpstAudioControlData->hFile[0]);
        AudioCodec(ExtendName, (UINT8 *)AudioFileExtString);
        #endif


    }
    else if(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_HTTP)
    {
        memcpy(gpstAudioControlData->AudioFileInfo.path, L"C:\\dlna.demo", 26);
        CurrentDecCodec = gpstAudioControlData->defaultCodecType;
        CurrentCodecSave = CurrentDecCodec;

        TotalSize = 0;

        ret = fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &TotalSize);
        rk_printf("TotalSize = %d", TotalSize);

        if(ret == RK_ERROR)
        {
            return RK_ERROR;
        }

        if(gpstAudioControlData->DirctPlay)
        {
            fifoDev_SetFIFOLevel(gpstAudioControlData->hFifo, 30 * 1024, 6 * 1024);
            CurrentDecCodec = gpstAudioControlData->defaultCodecType;
            CurrentCodecSave = CurrentDecCodec;
        }
        else
        {
            fifoDev_SetFIFOLevel(gpstAudioControlData->hFifo, 100 * 1024, 6 * 1024);
            AudioCheckStreamType(ExtendName, gpstAudioControlData->hFifo);
            DEBUG("CurrentDecCodec = %d",CurrentDecCodec);
            AudioCodec(ExtendName, (UINT8 *)AudioFileExtString);
        }
    }
    else if(gpstAudioControlData->ucSelPlayType == SOURCE_FROM_XXX)
    {
        memcpy(gpstAudioControlData->AudioFileInfo.path, L"C:\\XXX.demo", 32);
        CurrentDecCodec = gpstAudioControlData->defaultCodecType;
        CurrentCodecSave = CurrentDecCodec;

        TotalSize = 0;

        ret = fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &TotalSize);
        if(ret == RK_ERROR)
        {
            return RK_ERROR;
        }

        //fifoDev_SetFIFOLevel(gpstAudioControlData->hFifo, 10 * 1024, 2 * 1024);
        fifoDev_SetFIFOLevel(gpstAudioControlData->hFifo, 256 * 1024, 32 * 1024);
        rk_printf("TotalSize = %d", TotalSize);
    }
    else //only one audio format
    {
        memcpy(gpstAudioControlData->AudioFileInfo.path, L"C:\\bt.demo", 22);
        CurrentDecCodec = gpstAudioControlData->defaultCodecType;
        CurrentCodecSave = CurrentDecCodec;

        TotalSize = 0;

        ret = fifoDev_GetTotalSize(gpstAudioControlData->hFifo, &TotalSize);
        if(ret == RK_ERROR)
        {
            return RK_ERROR;
        }

        //fifoDev_SetFIFOLevel(gpstAudioControlData->hFifo, 10 * 1024, 2 * 1024);
        fifoDev_SetFIFOLevel(gpstAudioControlData->hFifo, 15 * 1024, 6 * 1024);
        rk_printf("TotalSize = %d", TotalSize);
    }

    if ((CurrentDecCodec == 0xff) && (CurrentCodecSave == 0xff))
    {
        rk_printf("ERROR!!! CurrentDecCodec == 0x%02x",CurrentDecCodec);
        return ERROR;
    }
    else if((CurrentDecCodec == 0xff) && (CurrentCodecSave != 0xff))
    {
        CurrentDecCodec = CurrentCodecSave;
    }



#ifdef FLAC_DEC_INCLUDE
   if (CODEC_FLAC_DEC == CurrentDecCodec)
   {
        if(gpstAudioControlData->hFifo == NULL)
        {
            gpstAudioControlData->hFile[1] = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
            if ((rk_err_t)gpstAudioControlData->hFile[1] <= 0)
            {
              rk_printf("1FileDev_OpenFile faile return -1");
              return RK_ERROR;
            }
        }
   }
#endif

#ifdef AAC_DEC_INCLUDE
   if (CODEC_AAC_DEC == CurrentDecCodec)
   {
        if(gpstAudioControlData->hFifo == NULL)
        {
            gpstAudioControlData->hFile[1] = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
            if ((rk_err_t)gpstAudioControlData->hFile[1] <= 0)
            {
              rk_printf("1FileDev_OpenFile faile return -1");
              return RK_ERROR;
            }
            gpstAudioControlData->hFile[2] = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
            if ((rk_err_t)gpstAudioControlData->hFile[2] <= 0)
            {
              rk_printf("2FileDev_OpenFile faile return -1");
              return RK_ERROR;
            }
        }

   }
#endif

#ifdef HIFI_AlAC_DECODE
   if (CODEC_HIFI_ALAC_DEC == CurrentDecCodec)
   {
        if(gpstAudioControlData->hFifo == NULL)
        {
            gpstAudioControlData->hFile[1] = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
            if ((rk_err_t)gpstAudioControlData->hFile[1] <= 0)
            {
                rk_printf("1FileDev_OpenFile faile return -1");
                return RK_ERROR;
            }
            gpstAudioControlData->hFile[2] = FileDev_OpenFile(FileSysHDC, NULL, READ_ONLY, &stFileAttr);
            if ((rk_err_t)gpstAudioControlData->hFile[2] <= 0)
            {
                rk_printf("2FileDev_OpenFile faile return -1");
                return RK_ERROR;
            }
        }
   }
#endif

    rk_printf("short name:");
    #if 0//def _FS_//jjjhhh 20161126
    UartDev_Write(UartHDC, stFileAttr.ShortName, 11, SYNC_MODE, NULL);
    #endif

    return OK;
}

/*******************************************************************************
** Name: AudioVariableInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.24
** Time: 11:19:53
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_COMMON_
COMMON FUN void AudioVariableInit(void)
{
    gpstAudioControlData->pAudioRegKey.CurrentTime = 0;
    gpstAudioControlData->pAudioRegKey.TotalTime = 1;

    gpstAudioControlData->AudioFadeOutStart = 0;
    gpstAudioControlData->AudioFadeInStart = 0;

    gpstAudioControlData->AudioErrorFrameNum = 0;

    gpstAudioControlData->AudioEndFade = 0;

    gpstAudioControlData->AudioCodecOpenErr = 0;

    CurrentDecCodec = 0xff;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AudioControlTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 10:21:46
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_INIT_
INIT API rk_err_t AudioControlTask_DeInit(void *pvParameters)
{
    uint8 i;

    rk_printf("AudioControlTask_DeInit\n");

    if(gpstAudioControlData->AudioPlayerState != AUDIO_STATE_STOP)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
    }

    {
        AUDIO_CALLBACK * pCur;

        pCur = gpstAudioControlData->pfAudioState;

        while(pCur != NULL)
        {
            pCur->pfAudioState(AUDIO_STATE_DELETE);
            pCur = pCur->pNext;
        }
    }

    AudioDev_RealseMainTrack(gpstAudioControlData->hAudio);

    if (gpstAudioControlData->hAudio != NULL)
    {
        if (RKDev_Close(gpstAudioControlData->hAudio) != RK_SUCCESS)
        {
            rk_printf("hAudio close failure\n");
            return RK_ERROR;
        }
    }

    if (gpstAudioControlData->AudioFileInfo.hDirDev != NULL)
    {
        if (RKDev_Close(gpstAudioControlData->AudioFileInfo.hDirDev) != RK_SUCCESS)
        {
            rk_printf("hAudio close failure\n");
            return RK_ERROR;
        }
    }


    if (gpstAudioControlData->hBcore != NULL)
    {
        if (RKDev_Close(gpstAudioControlData->hBcore) != RK_SUCCESS)
        {
            rk_printf("hBcore close failure\n");
            return RK_ERROR;
        }
    }

    if (gpstAudioControlData->hFifo != NULL)
    {
        if (RKDev_Close(gpstAudioControlData->hFifo) != RK_SUCCESS)
        {
            rk_printf("hFifo close failure\n");
            return RK_ERROR;
        }
    }

    if(osAudioDecodeOk)
        rkos_semaphore_delete(osAudioDecodeOk);
    if(gpstAudioControlData->AudioControlAskQueue)
        rkos_queue_delete(gpstAudioControlData->AudioControlAskQueue);

    if(gpstAudioControlData->AudioControlRespQueue)
        rkos_queue_delete(gpstAudioControlData->AudioControlRespQueue);

    if(hAudioDma)
    {
        if(RKDev_Close(hAudioDma) != RK_SUCCESS)
        {
            rk_printf("hAudioDma close failure\n");
            return RK_ERROR;
        }
    }

    {
        AUDIO_CALLBACK * pCur;
        while(gpstAudioControlData->pfAudioState != NULL)
        {
           pCur = gpstAudioControlData->pfAudioState;
           gpstAudioControlData->pfAudioState = gpstAudioControlData->pfAudioState->pNext;
           rkos_memory_free(pCur);
        }
    }

    if(gpstAudioControlData)
        rkos_memory_free(gpstAudioControlData);
    gpstAudioControlData = NULL;

    if(DecDataBuf[0])
        rkos_memory_free(DecDataBuf[0]);
    if(DecDataBuf[1])
        rkos_memory_free(DecDataBuf[1]);

    DecDataBuf[0] = NULL;
    DecDataBuf[1] = NULL;

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_AUDIOCONTROL_TASK);
    #endif

    RKTaskDelete(TASK_ID_STREAMCONTROL, 0, ASYNC_MODE);

    printf("audio delete ok");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: AudioControlTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 10:21:46
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_INIT_
INIT API rk_err_t AudioControlTask_Init(void *pvParameters, void *arg)
{
    uint32 i;
    int32 ret=0;
    RK_TASK_CLASS*   pAudioControlTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_AUDIOCONTROL_ARG * pArg = (RK_TASK_AUDIOCONTROL_ARG *)arg;
    AUDIOCONTROL_TASK_DATA_BLOCK*  pAudioControlTaskData;

    if (pAudioControlTask == NULL)
    {
        rk_printf("audio task para error");
        return RK_PARA_ERR;
    }
    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_AUDIOCONTROL_TASK, SEGMENT_OVERLAY_CODE);
    #endif

    pAudioControlTaskData = NULL;
    pAudioControlTaskData = rkos_memory_malloc(sizeof(AUDIOCONTROL_TASK_DATA_BLOCK));
    if(pAudioControlTaskData == NULL)
    {
        rk_printf("pAudioControlTaskData == null");
    }
    memset(pAudioControlTaskData, NULL, sizeof(AUDIOCONTROL_TASK_DATA_BLOCK));

    pAudioControlTaskData->AudioControlAskQueue= rkos_queue_create(1, sizeof(AUDIOCONTROL_ASK_QUEUE));
    pAudioControlTaskData->AudioControlRespQueue = rkos_queue_create(1, sizeof(AUDIOCONTROL_RESP_QUEUE));
    pAudioControlTaskData->TaskObjectID = pAudioControlTask->TaskObjectID;

    pAudioControlTaskData->ucSelPlayType = pArg->ucSelPlayType;

    if(pArg->pfAudioState != NULL)
    {
        pAudioControlTaskData->pfAudioState = rkos_memory_malloc(sizeof(AUDIO_CALLBACK));
        pAudioControlTaskData->pfAudioState->pNext = NULL;
        pAudioControlTaskData->pfAudioState->pfAudioState = pArg->pfAudioState;
    }
    else
    {
        pAudioControlTaskData->pfAudioState = NULL;
    }


    if (pArg->ucSelPlayType == SOURCE_FROM_HTTP)
    {
        pAudioControlTaskData->hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);

        if ((pAudioControlTaskData->hFifo == NULL)
        || (pAudioControlTaskData->hFifo == (HDC)RK_ERROR)
        || (pAudioControlTaskData->hFifo == (HDC)RK_PARA_ERR))
        {
            rk_print_string("fifo device open failure");
            rk_printf("0 hfifo -1\n");
            RKDev_Close(pAudioControlTaskData->hFifo);
            while(1);
        }
        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = -1;

        pAudioControlTaskData->SaveMemory = pArg->SaveMemory;
        pAudioControlTaskData->DirctPlay = pArg->DirectPlay;

        if(pAudioControlTaskData->DirctPlay)
        {
            pAudioControlTaskData->defaultCodecType = pArg->CodecType;
        }

    }
    else if(pArg->ucSelPlayType == SOURCE_FROM_BT)
    {
        pAudioControlTaskData->hFifo = RKDev_Open(DEV_CLASS_FIFO, 1, NOT_CARE);
        pAudioControlTaskData->defaultCodecType = CODEC_SBC_DEC;
        if ((pAudioControlTaskData->hFifo == NULL)
        || (pAudioControlTaskData->hFifo == (HDC)RK_ERROR)
        || (pAudioControlTaskData->hFifo == (HDC)RK_PARA_ERR))
        {
            rk_print_string("fifo device open failure");
            rk_printf("1 hfifo -1\n");
            RKDev_Close(pAudioControlTaskData->hFifo);
            return RK_ERROR;
        }

        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = -1;
    }

    #ifdef _FS_
    else if((pArg->ucSelPlayType == SOURCE_FROM_FILE_BROWSER) || (pArg->ucSelPlayType == SOURCE_FROM_RECORD))
    {
        pAudioControlTaskData->AudioFileInfo.hDirDev = RKDev_Open(DEV_CLASS_DIR, 0 ,NOT_CARE);
        if ((pAudioControlTaskData->AudioFileInfo.hDirDev == NULL)
        || (pAudioControlTaskData->AudioFileInfo.hDirDev == (HDC)RK_ERROR)
        || (pAudioControlTaskData->AudioFileInfo.hDirDev == (HDC)RK_PARA_ERR))
        {
            rk_print_string("au dir device open failure");
            return RK_ERROR;
        }

        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = pArg->FileNum;

        memcpy(pAudioControlTaskData->AudioFileInfo.path, pArg->filepath, MAX_DIRPATH_LEN * 2);
        pAudioControlTaskData->AudioFileInfo.pExtStr = AudioFileExtString;

        pAudioControlTaskData->AudioFileInfo.pPlayListInit = DirPlayListInit;
        pAudioControlTaskData->AudioFileInfo.pPlayNextFile = DirPlayListNextFile;

        pAudioControlTaskData->AudioFileInfo.pPlayListInit(&pAudioControlTaskData->AudioFileInfo);

    }
    #endif

#ifdef _MEDIA_MODULE_
    else if(pArg->ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
    {

        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = pArg->FileNum;
        pAudioControlTaskData->AudioFileInfo.TotalFiles = pArg->TotalFiles;

        memcpy(pAudioControlTaskData->AudioFileInfo.path, pArg->filepath, MAX_DIRPATH_LEN * 2);
        pAudioControlTaskData->AudioFileInfo.pExtStr = AudioFileExtString;

        pAudioControlTaskData->AudioFileInfo.BaseID = gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[gSysConfig.MediaDirTreeInfo.MusicDirDeep];
        pAudioControlTaskData->AudioFileInfo.pPlayNextFile = MediaPlayListNextFile;

    }
    else if(pArg->ucSelPlayType == SOURCE_FROM_DB_FLODER)
    {
        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = pArg->FileNum;
        pAudioControlTaskData->AudioFileInfo.TotalFiles = pArg->TotalFiles;
        pAudioControlTaskData->AudioFileInfo.pExtStr = AudioFileExtString;

        memcpy((UINT8*) &(pAudioControlTaskData->MediaFloderInfo), (UINT8*) &(pArg->MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));
        memcpy(pAudioControlTaskData->AudioFileInfo.path, pArg->filepath, MAX_DIRPATH_LEN * 2);

        pAudioControlTaskData->AudioFileInfo.pPlayNextFile = MediaFloderPlayListNextFile;
    }
#endif
    else if(pArg->ucSelPlayType == SOURCE_FROM_XXX)
    {
        pAudioControlTaskData->hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
        pAudioControlTaskData->defaultCodecType = CODEC_HIFI_ALAC_DEC;
        if ((pAudioControlTaskData->hFifo == NULL)
        || (pAudioControlTaskData->hFifo == (HDC)RK_ERROR)
        || (pAudioControlTaskData->hFifo == (HDC)RK_PARA_ERR))
        {
            rk_print_string("fifo device open failure");
            rk_printf("1 hfifo -1\n");
            RKDev_Close(pAudioControlTaskData->hFifo);
            return RK_ERROR;
        }

        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = -1;

    }
    else
    {
        pAudioControlTaskData->hFifo = NULL;
    }

    pAudioControlTaskData->AudioPlayerState = AUDIO_STATE_STOP;

    if(pAudioControlTaskData->ucSelPlayType != SOURCE_FROM_DB_FLODER)
    {
        pAudioControlTaskData->AudioFileInfo.RepeatMode = gSysConfig.MusicConfig.RepeatMode;
        pAudioControlTaskData->AudioFileInfo.Range =  FIND_FILE_RANGE_DIR;
    }
    else
    {
        pAudioControlTaskData->AudioFileInfo.RepeatMode = gSysConfig.MusicConfig.RepeatModeBak;
        if (pAudioControlTaskData->AudioFileInfo.RepeatMode > AUDIO_REPEAT)
        {
            pAudioControlTaskData->AudioFileInfo.CurrentFileNum = 0;
            pAudioControlTaskData->AudioFileInfo.Range = FIND_FILE_RANGE_ALL;
        }
        else
        {
            pAudioControlTaskData->AudioFileInfo.Range = FIND_FILE_RANGE_DIR;
        }
    }

    pAudioControlTaskData->AudioFileInfo.PlayOrder = gSysConfig.MusicConfig.PlayOrder;
    pAudioControlTaskData->playVolume = gSysConfig.OutputVolume;

    #ifdef _RK_EQ_
    pAudioControlTaskData->EqMode = gSysConfig.MusicConfig.Eq.Mode;
    #endif

#if 0

    if (pAudioControlTaskData->AudioFileInfo.TotalFiles > SORT_FILENUM_DEFINE)
    {
        pAudioControlTaskData->AudioFileInfo.TotalFiles = SORT_FILENUM_DEFINE;
        if (pAudioControlTaskData->AudioFileInfo.CurrentFileNum > pAudioControlTaskData->AudioFileInfo.TotalFiles)
            pAudioControlTaskData->AudioFileInfo.CurrentFileNum = pAudioControlTaskData->AudioFileInfo.TotalFiles - 1;
    }


    if (pAudioControlTaskData->pAudioRegKey.PlayOrder == AUDIO_RAND)
    {
        pAudioControlTaskData->AudioFileInfo.CurrentFileNum = 0;

        if (pAudioControlTaskData->AudioFileInfo.TotalFiles == 1)
        {
            server_ori_seed = 0;
        }
        else if (pAudioControlTaskData->AudioFileInfo.TotalFiles == 2)
        {
            if (pAudioControlTaskData->AudioFileInfo.CurrentFileNum == 1)
            {
                server_ori_seed = 1;
            }
            else
            {
                server_ori_seed = 0;
            }
        }
        else
        {
            server_ori_seed =(SysTickCounter % pAudioControlTaskData->AudioFileInfo.TotalFiles);

            i = 0;
            do
            {
                if ((server_ori_seed == gSysConfig.MusicConfig.ShuffleSeed)
                        || (server_ori_seed == (pAudioControlTaskData->AudioFileInfo.CurrentFileNum - 1)))
                {
                    //new seed
                    server_ori_seed =(SysTickCounter % pAudioControlTaskData->AudioFileInfo.TotalFiles);
                    DelayMs(10);
                }
                else
                {
                    break;
                }
            }
            while (i < 200);

        }
        CreateRandomList(pAudioControlTaskData->AudioFileInfo.TotalFiles, server_ori_seed, MaxShuffleAllCount);
        gSysConfig.MusicConfig.ShuffleSeed = server_ori_seed;
        rand_first_flag = 1;
    }

#endif

    pAudioControlTaskData->hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);

    if ((pAudioControlTaskData->hAudio == NULL)
        || (pAudioControlTaskData->hAudio == (HDC)RK_ERROR)
        || (pAudioControlTaskData->hAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("Audio device open failure");
        return RK_ERROR;
    }

    pAudioControlTaskData->TrackNo = AudioDev_GetMainTrack(pAudioControlTaskData->hAudio);

    //printf("SetEQ EQ_POP\n");
    pAudioControlTaskData->hBcore = RKDev_Open(DEV_CLASS_BCORE, 0, NOT_CARE);

    if ((pAudioControlTaskData->hBcore == NULL)
        || (pAudioControlTaskData->hBcore == (HDC)RK_ERROR)
        || (pAudioControlTaskData->hBcore == (HDC)RK_PARA_ERR))
    {
        rk_print_string("Bcore device open failure");
        return RK_ERROR;
    }

    gpstAudioControlData = pAudioControlTaskData;

#ifndef A_CORE_DECODE
    hAudioDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    osAudioDecodeOk = rkos_semaphore_create(1, 0);
    RegMBoxDecodeSvc();
#endif

    rk_printf("RegMBoxDecodeSvc  \n");

    if(pAudioControlTaskData->SaveMemory == 0)
    {
        if((pArg->ucSelPlayType == SOURCE_FROM_HTTP) || (pArg->ucSelPlayType == SOURCE_FROM_BT))
        {
            #ifdef _MEMORY_LEAK_CHECH_
            DecDataBuf[0] = rkos_memory_malloc(1024 * 12);
            #else
            DecDataBuf[0] = rkos_memory_malloc(1024 * 32);
            #endif

            if(NULL == DecDataBuf[0])
            {
                rk_printf("ERROR: DecDataBuf[0] malloc faile\n");
                return RK_ERROR;
            }

            #ifdef _MEMORY_LEAK_CHECH_
            DecDataBuf[1] = rkos_memory_malloc(1024 * 12);
            #else
            DecDataBuf[1] = rkos_memory_malloc(1024 * 32);
            #endif

            if(NULL == DecDataBuf[1])
            {
                rk_printf("ERROR: DecDataBuf[1] malloc faile\n");
                rkos_memory_free(DecDataBuf[0]);
                return RK_ERROR;
            }
        }
        else if(pArg->ucSelPlayType == SOURCE_FROM_XXX)
        {
            DecDataBuf[0] = rkos_memory_malloc(1024 * 3 * 10);
            if(NULL == DecDataBuf[0])
            {
                rk_printf("ERROR: DecDataBuf[0] malloc faile\n");
                return RK_ERROR;
            }

            DecDataBuf[1] = rkos_memory_malloc(1024 * 3 * 10);
            if(NULL == DecDataBuf[1])
            {
                rk_printf("ERROR: DecDataBuf[1] malloc faile\n");
                rkos_memory_free(DecDataBuf[0]);
                return RK_ERROR;
            }
        }
        else
        {
            DecDataBuf[0] = rkos_memory_malloc(1024 * 36);
            if(NULL == DecDataBuf[0])
            {
                rk_printf("ERROR: DecDataBuf[0] malloc faile\n");
                return RK_ERROR;
            }

            DecDataBuf[1] = rkos_memory_malloc(1024 * 36);
            if(NULL == DecDataBuf[1])
            {
                rk_printf("ERROR: DecDataBuf[1] malloc faile\n");
                rkos_memory_free(DecDataBuf[0]);
                return RK_ERROR;
            }
        }
    }

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
** Name: RegMBoxDecodeSvc
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 14:55:01
*******************************************************************************/
_APP_AUDIO_AUDIOCONTROLTASK_INIT_
INIT FUN void RegMBoxDecodeSvc(void)
{
    IntRegister(INT_ID_MAILBOX1, (void*)AudioDecodingGetOutBuffer);
    IntPendingClear(INT_ID_MAILBOX1);
    IntEnable(INT_ID_MAILBOX1);
    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_1);
}
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

