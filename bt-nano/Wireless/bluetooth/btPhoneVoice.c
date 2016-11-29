/*
********************************************************************************
*                   Copyright (c) 2009,WangBo
*                      All rights reserved.
*
* File Name：   BlueToothControl.c
*
* Description:
*
* History:      <author>          <time>        <version>
*                 WangBo      2009-4-10       1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"
#include "RKOS.h"

#include "BlueToothControl.h"
//#include "aec_interface.h"
#include "record_globals.h"
#include "btPhoneVoice.h"

#ifdef _HFP_

#define BT_PHONE_VOICE_BUFFER_SIZE          1024
#define BT_PHONE_DATA_SINGLE_SEND           128

extern uint32  BTState;
_ATTR_BLUETOOTHCONTROL_DATA_  uint16 BtAecBuffer[BT_PHONE_VOICE_BUFFER_SIZE];
_ATTR_BLUETOOTHCONTROL_DATA_  uint16 *pBtStreamAecRefBuffer = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_  uint16 *pBtRecordRefBuffer = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_  uint8  pBtStreamInputBuffer[BT_PHONE_VOICE_BUFFER_SIZE*2];
_ATTR_BLUETOOTHCONTROL_DATA_  uint8  pBtRecordInputBuffer[2][BT_PHONE_VOICE_BUFFER_SIZE*2];
_ATTR_BLUETOOTHCONTROL_DATA_  uint32 BtRecordSample = 0;

_ATTR_BLUETOOTHCONTROL_BSS_ pSemaphore RecordFlag;
_ATTR_BLUETOOTHCONTROL_BSS_ pSemaphore StreamFlag;
_ATTR_BLUETOOTHCONTROL_BSS_ pSemaphore RecordSendFlag;

_ATTR_BLUETOOTHCONTROL_BSS_ uint8 bank;

extern int lp_sco_write(uint8 *data, uint16 len, void (*func)(void));
extern int spp_send(uint8 *data, int len, void (*func)(void));

_ATTR_BLUETOOTHCONTROL_CODE_
int32 BTPhoneDataSend(uint8 *data, uint16 len)
{
    uint16 index_send = 0;
    //uint16 left_send = len;
    int ret;
    //rk_printf("len = %d", len);
    if (NULL == data)
    {
        rk_printf("\n data is null \n");
        return RETURN_FAIL;
    }

    while(1)
    {
        ret = lp_sco_write(data + index_send, BT_PHONE_DATA_SINGLE_SEND, NULL);
        //ret = spp_send(data + index_send, BT_PHONE_DATA_SINGLE_SEND, NULL);
        if (ret != ERR_OK)
        {
            rk_printf("\n SCO data write error, ret=%d \n", ret);
            return RETURN_FAIL;
        }
        index_send += BT_PHONE_DATA_SINGLE_SEND;

        if(index_send >= len)
        {
           break;
        }
        //left_send -=  BT_PHONE_DATA_SINGLE_SEND;

    }
#if 0
    //rk_printf("left_send = %d", left_send);
    ret = lp_sco_write(data + index_send-BT_PHONE_DATA_SINGLE_SEND, left_send, NULL);
    //ret = spp_send(data + index_send, BT_PHONE_DATA_SINGLE_SEND, NULL);
    if (ret != ERR_OK)
    {
        rk_printf("\n SCO data left write error, ret=%d \n", ret);
        return RETURN_FAIL;
    }
 #endif
    return RETURN_OK;
}


_ATTR_BLUETOOTHCONTROL_CODE_
void BTPhoneVoiceStreamNofity(uint8 *buf, uint32 len)
{
    if(CHECK_BTSTATE(BTState,BT_CALL_PLAYING))
    {
        memcpy(pBtStreamInputBuffer, buf, len);
        pBtStreamAecRefBuffer = (uint16 *)pBtStreamInputBuffer;
        rkos_semaphore_give(StreamFlag);
    }
}

_ATTR_BLUETOOTHCONTROL_CODE_
void BTPhoneVoiceRecordCallBack(uint8 *buf, uint32 Samples)
{
    if (NULL == buf)
    {
        rk_printf("\n bt_phone:record call back buf is null. \n");
        return;
    }
    if(CHECK_BTSTATE(BTState,BT_CALL_PLAYING))
    {
        int i;
        short *p_form;
        short *p_to;
        rkos_semaphore_take(RecordSendFlag, MAX_DELAY);
        p_form = (short *)buf;
        p_to = (short *)pBtRecordInputBuffer[bank];
        //memcpy(pBtRecordInputBuffer[bank], buf, Samples*2);
        for(i=0;i<Samples;i++)
        {
            p_to[i] = p_form[i*2];
        }
        pBtRecordRefBuffer = (uint16 *)pBtRecordInputBuffer[bank];
        bank = bank ? 0 : 1;
        BtRecordSample = Samples;

        rkos_semaphore_give(RecordFlag);
    }
}

_ATTR_BLUETOOTHCONTROL_CODE_
INIT API rk_err_t BTPhoneVoiceTask_Init(void *pvParameters, void *arg)
{
    int ret;

    rk_printf("\nEnter BTPhoneVoiceTask_Init\n");
#ifdef _AEC_DECODE
    FW_LoadSegment(SEGMENT_ID_AEC, SEGMENT_OVERLAY_ALL);
    ret = Aec_init(BT_PHONE_VOICE_BUFFER_SIZE);
    if (!ret)
        return RK_ERROR;
#endif

    RecordFlag = rkos_semaphore_create(1, 0);
    if (RecordFlag == NULL)
    {
        rk_printf("record semaphore create failed\n");
        return RK_ERROR;
    }

    StreamFlag = rkos_semaphore_create(1, 0);
    if (StreamFlag == NULL)
    {
        rk_printf("stream semaphore create failed\n");
        rkos_semaphore_delete(RecordFlag);
        return RK_ERROR;
    }

    RecordSendFlag = rkos_semaphore_create(1, 1);
    if (RecordSendFlag == NULL)
    {
        rk_printf("record send semaphore create failed\n");
        rkos_semaphore_delete(RecordFlag);
        rkos_semaphore_delete(StreamFlag);
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

_ATTR_BLUETOOTHCONTROL_CODE_
INIT API rk_err_t BTPhoneVoiceTask_DeInit(void *pvParameters)
{
    printf("Enter BTPhoneVoiceTask_DeInit\n");

    if (RecordFlag != NULL)
        rkos_semaphore_delete(RecordFlag);

    if (StreamFlag != NULL)
        rkos_semaphore_delete(StreamFlag);

    if (RecordSendFlag != NULL)
        rkos_semaphore_delete(RecordSendFlag);

#ifdef _AEC_DECODE
    FW_RemoveSegment(SEGMENT_ID_AEC);
#endif
    RKTaskDelete(TASK_ID_AUDIODEVICESERVICE, 0, ASYNC_MODE);
    return RK_SUCCESS;
}

_ATTR_BLUETOOTHCONTROL_CODE_
COMMON API void BTPhoneVoiceTask_Enter(void)
{
    AUDIO_DEV_ARG stAudioArg;
    int ret = 0;
    HDC hAudio;

    rk_printf("Enter BTPhoneVoiceTask_Enter\n");

    stAudioArg.Bit = RECORD_DATAWIDTH_16BIT;
    stAudioArg.RecordType = RECORD_TYPE_MIC_STERO;
    stAudioArg.SampleRate = RECORD_SAMPLE_FREQUENCY_8KHZ;
    stAudioArg.pfPcmCallBack = BTPhoneVoiceRecordCallBack;
    #if (BT_CHIP_CONFIG == BT_CHIP_AP6212)
    stAudioArg.SamplesPerBlock = 128;
    #endif
    #if (BT_CHIP_CONFIG == BT_CHIP_RTL8761ATV)
    stAudioArg.SamplesPerBlock = 1024;
    #endif
    ret = RKTaskCreate(TASK_ID_AUDIODEVICESERVICE, 0, &stAudioArg, SYNC_MODE);

    hAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);
    AudioDev_ReadEnable(hAudio,1);
    RKDev_Close(hAudio);

    if (ret != RK_SUCCESS)
    {
        rk_printf("create TASK_ID_AUDIODEVICESERVICE error!\n");
        goto exit;
    }

    while(1)
    {
#if 1
        if ((rkos_semaphore_take(RecordFlag, MAX_DELAY) == RK_SUCCESS))
        {
            if(CHECK_BTSTATE(BTState,BT_CALL_PLAYING))
            {
#ifdef _AEC_DECODE
                if (rkos_semaphore_take(StreamFlag, MAX_DELAY) == RK_SUCCESS)
                {
                    ret = Aec_do(pBtRecordRefBuffer, pBtStreamAecRefBuffer, BtAecBuffer);
                    if (ret == 0)
                    {
                        continue;
                    }
                    BTPhoneDataSend((uint8 *)BtAecBuffer, BtRecordSample*2);
                }
#else
                BTPhoneDataSend((uint8 *)pBtRecordRefBuffer, BtRecordSample*2);
#endif
                rkos_semaphore_give(RecordSendFlag);
            }
        }
#else
        rkos_sleep(100);
        BTPhoneDataSend(pBtStreamInputBuffer, BT_PHONE_VOICE_BUFFER_SIZE*2);

#endif
    }

exit:
    RKTaskDelete(TASK_ID_AUDIODEVICESERVICE, 0, SYNC_MODE);
    while(1);
}

#endif

#ifdef BT_VOICENOTIFY

_ATTR_BLUETOOTHCONTROL_DATA_ uint16 voiceId = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hFifoVoiceNofity = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static uint32 repeatTime = 0;
_ATTR_BLUETOOTHCONTROL_DATA_ uint8 Buf[1024];
_ATTR_BLUETOOTHCONTROL_DATA_ uint8 stereoBuff[8192];
_ATTR_BLUETOOTHCONTROL_DATA_ static void* btVoiceNofityHandle = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ static HDC hVoiceAudio = NULL;
_ATTR_BLUETOOTHCONTROL_DATA_ BOOL playStop = FALSE;


_ATTR_BLUETOOTHCONTROL_CODE_
void stopVoiceNotify(void)
{
#ifdef BT_VOICENOTIFY

#if 1
    if (playStop == TRUE)
    {
        DEBUG("stopVoiceNotify\n");
        playStop = FALSE;
        /*
        AudioDev_RealseMainTrack(hVoiceAudio);
        RKDev_Close(hVoiceAudio);
        RKTaskDelete2(btVoiceNofityHandle);
        */
        RKTaskDelete(TASK_ID_BT_VOICE_NOTIFY, 0, SYNC_MODE);
    }
#else
    if (TRUE == ThreadCheck(pMainThread, &MusicThread))
    {
        Codec_DACMute();
        while(AudioPlayInfo.VolumeCnt > 1)
        {
            AudioPlayInfo.VolumeCnt--;
            Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
            DelayMs(5);
        }

        if(CurrentCodec == CODEC_BT_VOCIE_NOTIFY)
            ThreadDelete(&pMainThread, &MusicThread);

        ClearMsg(MSG_PLAY_VOICE_NOTIFY_FINISH);
    }
#endif
#endif
}

_ATTR_BLUETOOTHCONTROL_CODE_
int CheckAndStopVoiceNotify(void)
{
#ifdef BT_VOICENOTIFY

#if 1
#else
    if(TRUE == GetMsg(MSG_PLAY_VOICE_NOTIFY_FINISH))
    {
        if (TRUE == ThreadCheck(pMainThread, &MusicThread))
        {
            Codec_DACMute();
            while(AudioPlayInfo.VolumeCnt > 1)
            {
                AudioPlayInfo.VolumeCnt--;
                Codec_SetVolumet(AudioPlayInfo.VolumeCnt);
                DelayMs(5);
            }
            if(CurrentCodec == CODEC_BT_VOCIE_NOTIFY)
                ThreadDelete(&pMainThread, &MusicThread);

            return TRUE;
        }
    }
#endif
#else
    if(TRUE == GetMsg(MSG_PLAY_VOICE_NOTIFY_FINISH))
    {
        return TRUE;
    }

#endif

    return FALSE;
}

_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t VoiceNotifyInit(void *pvParameters)
{

    return RK_SUCCESS;
}

_ATTR_BLUETOOTHCONTROL_CODE_
rk_err_t VoiceNotifyDeInit(void *pvParameters)
{
    AudioDev_RealseMainTrack(hVoiceAudio);
    RKDev_Close(hVoiceAudio);
    //RKTaskDelete2(btVoiceNofityHandle);
    return RK_SUCCESS;
}

_ATTR_BLUETOOTHCONTROL_CODE_
void VoiceNotifyService(void)
{
    SEGMENT_INFO_T stCodeInfo;
    rk_err_t ret;
    uint32 voiceLen, imageBase, offset = 0, bufLen, Bank = 0;
    int16 *monoBuf = NULL;
    int32 i, j;
    uint16 *pStereoBuff = NULL;

    rk_print_string("VoiceNofityService\n");
    hVoiceAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);

    if ((hVoiceAudio == NULL) || (hVoiceAudio == (HDC)RK_ERROR) || (hVoiceAudio == (HDC)RK_PARA_ERR))
    {
        rk_print_string("Audio device open failure");
        while(1);
    }

    ret = AudioDev_GetMainTrack(hVoiceAudio);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("Audio main track Get Failure");
        goto over;
    }

    AudioDev_SetChannel(hVoiceAudio, 0, 2);
    AudioDev_SetTxSampleRate(hVoiceAudio, 0, CodecFS_8000Hz);
    AudioDev_SetTrackLen(hVoiceAudio, 2048);
    AudioDev_SetBit(hVoiceAudio, 0, 16);
    AudioDev_SetVol(hVoiceAudio, 20);

    FW_GetSegmentInfo(voiceId, &stCodeInfo);
    imageBase = stCodeInfo.CodeLoadBase;
    voiceLen = stCodeInfo.CodeImageLength;

    DEBUG("image length=%d\n", voiceLen);

    while(1)
    {
        offset = 0;
        DEBUG("repeat\n");
        while(offset < voiceLen)
        {
            if (offset + 1024 > voiceLen)
                bufLen = voiceLen - offset;
            else
                bufLen = 1024;
            FW_ReadFirmwaveByByte(imageBase+offset, Buf, bufLen);
            offset += bufLen;

            monoBuf = (int16 *)(Buf);
            pStereoBuff = (int16 *)(stereoBuff + Bank*4096);
            /* 16bit mono convert to 16bit stereo */
            for (i = 0, j = 0; i < bufLen/2; ++i)
            {
                pStereoBuff[j++] = monoBuf[i];
                pStereoBuff[j++] = monoBuf[i];
                //printf("0x%x\n", monoBuf[i]);
            }

            ret = AudioDev_Write(hVoiceAudio, 0, (uint8 *)pStereoBuff);
            if(ret != RK_SUCCESS)
            {
                goto over;
            }
            Bank = 1 - Bank;
        }

        if (repeatTime == PALY_TIME_ONCE)
            break;
    }

    rk_print_string("play ok\n");
over:
    DEBUG("error\n");
    AudioDev_RealseMainTrack(hVoiceAudio);
    RKDev_Close(hVoiceAudio);
    //RKTaskDelete2(btVoiceNofityHandle);
    RKTaskDelete(TASK_ID_BT_VOICE_NOTIFY, 0, SYNC_MODE);
}


_ATTR_BLUETOOTHCONTROL_CODE_
void playVoiceNotify(uint16 VoiceId, uint16 mode, uint32 repeat_time)
{
#ifdef BT_VOICENOTIFY
    DEBUG("playVoiceNotify");
    //如果正在提示，先停止
    stopVoiceNotify();
#if 1
    if (playStop == FALSE)
    {
        voiceId = VoiceId;
        repeatTime = repeat_time;
        playStop = TRUE;

        //btVoiceNofityHandle = RKTaskCreate2(VoiceNotifyService, NULL, NULL, "bt_voice_notify",
        //                                    TASK_PRIORITY_BLUETOOTH_CTRL_STACK_SIZE, TASK_PRIORITY_AUDIO_CONTROL, NULL);
        RKTaskCreate(TASK_ID_BT_VOICE_NOTIFY, 0, NULL, SYNC_MODE);

        if(mode == PLAY_MODE_BLOCK)
        {
            while(1)
            {
                if(CheckAndStopVoiceNotify() == TRUE)
                {
                    break;
                }
            }
        }
    }
#else
    if (TRUE != ThreadCheck(pMainThread, &MusicThread))
    {
        AUDIO_THREAD_ARG AudioArg;
        ModuleOverlay(MODULE_ID_AUDIO_CONTROL, MODULE_OVERLAY_ALL);
        ClearMsg(MSG_PLAY_VOICE_NOTIFY_FINISH);
        AudioArg.pAudioOpsInit = BTVoiceNotifyInit;
        AudioArg.FileNum = VoiceId;
        ThreadCreat(&pMainThread, &MusicThread, &AudioArg);
        VoiceRepeatTime = (repeat_time == 0) ? 1 : repeat_time;
        if(mode == PLAY_MODE_BLOCK)
        {
            while(1)
            {
                if(CheckAndStopVoiceNotify() == TRUE)
                {
                    break;
                }
            }
        }
    }
#endif
#else
    SendMsg(MSG_PLAY_VOICE_NOTIFY_FINISH);
#endif
}

#endif

/*
********************************************************************************
*
*                         End of btPhoneVoice.c
*
********************************************************************************
*/




