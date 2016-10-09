/*
********************************************************************************
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* File Name£º   pWAVEnc.c
*
* Description:  WAV coding processing control.
*
* History:      <author>          <time>        <version>
*               WangBo           2009-4-16          1.0
*    desc:    ORG.
********************************************************************************
*/


//#include  "SysInclude.h"
#include  "RKOS.h"
#include  "audio_main.h"

//#include  "FsInclude.h"
//#include  "File.h"
//#include  "FDT.h"

#include  "Audio_globals.h"
#include  "Record_globals.h"
#include  "audio_file_access.h"

#ifdef WAV_ENC_INCLUDE
#ifdef A_CORE_DECODE
#include  "pcm.h"
#include  ".\WAV_LIB\sf_wav.h"
extern  SF_PRIVATE   sf_enc;

_ATTR_WAVENC_DATA_ unsigned short      WavInputBufferIndex;
_ATTR_WAVENC_DATA_ unsigned short      WavInputBufferLength;
_ATTR_WAVENC_DATA_ unsigned short      WavEncDataLength;

_ATTR_WAVENC_DATA_ unsigned short      *pWavEncodeInputBuffer;
_ATTR_WAVENC_DATA_ unsigned short      *pWavPCMInputBuffer;
_ATTR_WAVENC_DATA_ unsigned short      *pWavInputBuffer;
_ATTR_WAVENC_DATA_ unsigned short      *WavEncInput;
_ATTR_WAVENC_DATA_ unsigned short      *WavEncOutPut;
_ATTR_WAVENC_DATA_ tPCM_enc   PCM_s;


_ATTR_WAVENC_TEXT_
void  ADPCMInit(tPCM_enc* pPCM)
{
    InitADPCMEncoder(pPCM);

    sf_enc.sf.channels = pPCM->ucChannels;
    sf_enc.sf.samplerate = pPCM->usSampleRate;

    if(PCM_s.wFormatTag == WAVE_FORMAT_PCM)
    {
        sf_enc.sf.format = (SF_FORMAT_WAV | SF_FORMAT_WAV);
    }
    else
    {
        sf_enc.sf.format = (SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM);
    }

    if(pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
    {
        sf_enc.mode = SFM_WRITE;
        sf_enc.datalength = pPCM->ulLength;
        msadpcm_enc_init (&sf_enc, pPCM->usBytesPerBlock, pPCM->usSamplesPerBlock);
    }
    else if(pPCM->wFormatTag == WAVE_FORMAT_PCM)
    {
        sf_enc.mode = SFM_WRITE;
        sf_enc.datalength = pPCM->ulLength;
    }

}


/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeVariableInit()
  Author        : WangBo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                 WangBo         2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_WAVENC_TEXT_
void WavEncodeVariableInit(EncodeArg * stEncodeArg)
{
    UINT16 i;

    WavInputBufferIndex   = 0;   //used for buffer switch.
    pWavInputBuffer       = WavEncInput;
    pWavPCMInputBuffer    = pWavInputBuffer;
    pWavEncodeInputBuffer = pWavInputBuffer + AD_PIPO_BUFFER_SIZE;

    for (i = 0; i < AD_PIPO_BUFFER_SIZE * 2; i++)
    {
        *(pWavInputBuffer + i) = 0;
    }


    PCM_s.usSampleRate = stEncodeArg->sampleRate;    //sampling rate.
    PCM_s.ucChannels = stEncodeArg->channel;
    PCM_s.uBitsPerSample = stEncodeArg->dataWidth;

    if(RECORD_PCM == stEncodeArg->encodeSubType)
    {
        PCM_s.wFormatTag = WAVE_FORMAT_PCM;
    }
    else
    {
        PCM_s.wFormatTag = WAVE_FORMAT_ADPCM;
    }

    ADPCMInit(&PCM_s);


    WavInputBufferLength = PCM_s.usSamplesPerBlock;     //the number of AD sampling.

}

/*
--------------------------------------------------------------------------------
  Function name : void WavEncodeHeaderInit()
  Author        : WangBo
  Description   : write the head of wav file.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               WangBo           2009-4-16          1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_ATTR_WAVENC_TEXT_
void WavEncodeHeaderInit()
{
    PCMWAVEFORMAT sWaveFormat;
    int ulIdx,i;
    extern int AdaptCoeff11[];
    extern int AdaptCoeff22[];
    uint8 * WavEncodeHeadBuffer;

    WavEncodeHeadBuffer = (uint8 *)pWavEncodeInputBuffer;


    for (i=4095; i>=0; i--)                   //init  WavEncodeHeadBuffer[512]
    {
        WavEncodeHeadBuffer[i]=0x00;
    }

    WavEncodeHeadBuffer[0] = 'R';
    WavEncodeHeadBuffer[1] = 'I';
    WavEncodeHeadBuffer[2] = 'F';
    WavEncodeHeadBuffer[3] = 'F';

    WavEncodeHeadBuffer[8]  = 'W';
    WavEncodeHeadBuffer[9]  = 'A';
    WavEncodeHeadBuffer[10] = 'V';
    WavEncodeHeadBuffer[11] = 'E';

    WavEncodeHeadBuffer[12] = 'f';
    WavEncodeHeadBuffer[13] = 'm';
    WavEncodeHeadBuffer[14] = 't';
    WavEncodeHeadBuffer[15] = ' ';
    WavEncodeHeadBuffer[16] = 50;
    WavEncodeHeadBuffer[17] = 0;
    WavEncodeHeadBuffer[18] = 0;
    WavEncodeHeadBuffer[19] = 0;

    if(PCM_s.wFormatTag == WAVE_FORMAT_PCM)
    {
        sWaveFormat.wFormatTag = 1;
        sWaveFormat.nChannels = PCM_s.ucChannels;
        sWaveFormat.nSamplesPerSec = PCM_s.usSampleRate;
        sWaveFormat.nAvgBytesPerSec = PCM_s.usByteRate;
        sWaveFormat.nBlockAlign = PCM_s.usBytesPerBlock;
        sWaveFormat.wBitsPerSample = 16;
        sWaveFormat.cbSize = 32;
        sWaveFormat.wSamplesPerBlock = PCM_s.usSamplesPerBlock;
        sWaveFormat.wNumCoef = 0;
        for (ulIdx = 0; ulIdx < 7; ulIdx++)
        {
            sWaveFormat.aCoef[ulIdx].iCoef1 = 0;
            sWaveFormat.aCoef[ulIdx].iCoef2 = 0;
        }
    }
    else
    {
        sWaveFormat.wFormatTag = 2;
        sWaveFormat.nChannels = PCM_s.ucChannels;
        sWaveFormat.nSamplesPerSec = PCM_s.usSampleRate;
        sWaveFormat.nAvgBytesPerSec = PCM_s.usByteRate;
        sWaveFormat.nBlockAlign = PCM_s.usBytesPerBlock;
        sWaveFormat.wBitsPerSample = 4;
        sWaveFormat.cbSize = 32;
        sWaveFormat.wSamplesPerBlock = PCM_s.usSamplesPerBlock;
        sWaveFormat.wNumCoef = 7;

        for (ulIdx = 0; ulIdx < 7; ulIdx++)
        {
            sWaveFormat.aCoef[ulIdx].iCoef1 = AdaptCoeff11[ulIdx];
            sWaveFormat.aCoef[ulIdx].iCoef2 = AdaptCoeff22[ulIdx];
        }
    }

    memcpy(WavEncodeHeadBuffer + 20, (void *)&sWaveFormat, 50);

    WavEncodeHeadBuffer[70] = 'f';
    WavEncodeHeadBuffer[71] = 'a';
    WavEncodeHeadBuffer[72] = 'c';
    WavEncodeHeadBuffer[73] = 't';
    WavEncodeHeadBuffer[74] = 4;
    WavEncodeHeadBuffer[75] = 0;
    WavEncodeHeadBuffer[76] = 0;
    WavEncodeHeadBuffer[77] = 0;

    WavEncodeHeadBuffer[78] = PCM_s.ulTimePos;
    WavEncodeHeadBuffer[79] = PCM_s.ulTimePos >> 8;
    WavEncodeHeadBuffer[80] = PCM_s.ulTimePos >> 16;
    WavEncodeHeadBuffer[81] = PCM_s.ulTimePos >> 24;

    WavEncodeHeadBuffer[82] = 0x4c;//"list"
    WavEncodeHeadBuffer[83] = 0x49;
    WavEncodeHeadBuffer[84] = 0x53;
    WavEncodeHeadBuffer[85] = 0x54;

    WavEncodeHeadBuffer[86] = 0x9e;
    WavEncodeHeadBuffer[87] = 0x0f;      //by Vincent

    WavEncodeHeadBuffer[90] = 0x49;//INFO
    WavEncodeHeadBuffer[91] = 0x4e;
    WavEncodeHeadBuffer[92] = 0x46;
    WavEncodeHeadBuffer[93] = 0x4f;

    WavEncodeHeadBuffer[94] = 'R';
    WavEncodeHeadBuffer[95] = 'o';
    WavEncodeHeadBuffer[96] = 'c';
    WavEncodeHeadBuffer[97] = 'k';

    WavEncodeHeadBuffer[98]  = 0x92; //by Vincent
    WavEncodeHeadBuffer[99]  = 0x0f; //by Vincent

    WavEncodeHeadBuffer[4088] = 'd';
    WavEncodeHeadBuffer[4089] = 'a';
    WavEncodeHeadBuffer[4090] = 't';
    WavEncodeHeadBuffer[4091] = 'a';

    WavEncodeHeadBuffer[4092] = 0Xff;
    WavEncodeHeadBuffer[4093] = 0Xff;
    WavEncodeHeadBuffer[4094] = 0Xff;
    WavEncodeHeadBuffer[4095] = 0Xff;

    WavEncodeHeadBuffer[4] = 0Xff;
    WavEncodeHeadBuffer[5] = 0Xff;
    WavEncodeHeadBuffer[6] = 0Xff;
    WavEncodeHeadBuffer[7] = 0Xff;

    RKFIO_BWrite(WavEncodeHeadBuffer,4096);

}

#else
extern RecordBlock  gRecordBlock;
extern pSemaphore osRecordEncodeOk;
#endif

_ATTR_WAVENC_TEXT_
unsigned long
PCMEncFunction(unsigned long ulIoctl, unsigned long ulParam1,
             unsigned long ulParam2, unsigned long ulParam3
            )
{
    #ifdef A_CORE_DECODE
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_ENC:
        {
            WavEncodeVariableInit((EncodeArg *)ulParam1);
            WavEncodeHeaderInit();
            *(short**)ulParam2 = pWavPCMInputBuffer;
            *(long*)ulParam3 = WavInputBufferLength;
            return (1);
        }

        case SUBFN_CODEC_ENC_GETBUFFER:
        {
            pWavEncodeInputBuffer  = pWavPCMInputBuffer;
            WavInputBufferIndex    = 1 - WavInputBufferIndex;
            pWavPCMInputBuffer     = pWavInputBuffer + WavInputBufferIndex * AD_PIPO_BUFFER_SIZE;  //switch buffer

            *(short**)ulParam1 = pWavPCMInputBuffer;
            *(long*)ulParam2 = WavInputBufferLength;
            return(1);
        }

        case SUBFN_CODEC_ENCODE:
        {
            if(PCM_s.wFormatTag == WAVE_FORMAT_ADPCM)
            {
                char * OutBuf;
                WavEncDataLength = msadpcm_write_s(&sf_enc, pWavEncodeInputBuffer , WavInputBufferLength * 2, &OutBuf);
                RKFIO_BWrite(OutBuf, WavEncDataLength * 4);
            }
            else
            {
                RKFIO_BWrite((char *)pWavEncodeInputBuffer, WavInputBufferLength * (PCM_s.uBitsPerSample / 8) * PCM_s.ucChannels);
            }
            return (1);
        }

        default:
        {
            return 0;
        }
    }
    #else

    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_ENC:
        {
            rk_printf("open encode");
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
            if (gRecordBlock.encodeErr == 1)
            {
                return 0;
            }

            *(short**)ulParam2 = (short *)gRecordBlock.unenc_bufptr;
            *(long*)ulParam3 = gRecordBlock.unenc_length;
            return (1);
        }

        case SUBFN_CODEC_ENC_GETBUFFER:
        {
        retry:

            IntDisable(INT_ID_MAILBOX1);
            if(gRecordBlock.encodeOver == 1)
            {
                 //rk_printf("get enc buf");

                if (gRecordBlock.encodeErr == 1)
                {
                    IntEnable(INT_ID_MAILBOX1);
                    return 0;
                }

                gRecordBlock.encodeOver = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(ulParam2, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                IntEnable(INT_ID_MAILBOX1);
                rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);

                *(short**)ulParam1 = (short *)gRecordBlock.unenc_bufptr;
                *(long*)ulParam2 = gRecordBlock.unenc_length;
            }
            else
            {
                rk_printf("wait b core...");
                gRecordBlock.needencode = 1;
                IntEnable(INT_ID_MAILBOX1);
                rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
                goto retry;
            }
            return(1);
        }

        case SUBFN_CODEC_ENCODE:
        {
            return (1);
        }

        case SUBFN_CODEC_GETBITRATE:
        {
             *(int *)ulParam1 = gRecordBlock.Bitrate;
             return(1);
        }

        default:
        {
            return 0;
        }
    }
    #endif
}

#endif

