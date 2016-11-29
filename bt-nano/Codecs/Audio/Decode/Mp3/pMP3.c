/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pMP3.c
Desc    : MP3 decode flow control

Author  : Vincent Hsiung (xw@rock-chips.com)
Date    : Jan 10 , 2009
Notes   :

$Log    :
*
*
*/
/****************************************************************/
#include "RKOS.h"
#include "../include/audio_main.h"
#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#ifdef MP3_DEC_INCLUDE

#include <stdio.h>
#include <string.h>    //for memcpy(),memmove()
#include "typedef.h"
#include "mailbox.h"

#pragma arm section code = "Mp3DecCode", rodata = "Mp3DecCode", rwdata = "Mp3DecData", zidata = "Mp3DecBss"

#ifdef A_CORE_DECODE

_ATTR_MP3DEC_BSS_ short *outbuf[2];
_ATTR_MP3DEC_BSS_
unsigned int MP3_FORMAT_FLAG ; //后8位->前4位存储MPEG 类型，后四位存储layer类型
//mpeg 1  = 1   mpeg2 = 2   mpeg 2.5 = 3;
//layer1  = 1   layer2 =2   layer3 = 3;




/*
*******************************************************************************
    dynamically tune
*******************************************************************************
*/
_ATTR_MP3DEC_BSS_
unsigned int backup_arm_acc;

_ATTR_MP3DEC_TEXT_
void SWITCH_ARM_ACC(void)
{
#if 0
    if ((*((volatile unsigned long*)0x40180000) & 0x00000040)  == 0)
    {
        backup_arm_acc = *((volatile unsigned long*)0x40180008) & (0x08);
        *((volatile unsigned long*)0x40180008) = (0x08 << 16)  | 0;
    }
#endif
}

_ATTR_MP3DEC_TEXT_
void SWITCH_ARM_NOR(void)
{
#if 0
    if ((*((volatile unsigned long*)0x40180000) & 0x00000040)  == 0)
    {
        *((volatile unsigned long*)0x40180008) = ((0x08) << 16)  | backup_arm_acc;
    }
#endif
}

//*************************************************************************************************************//
//the achievement of functions.：
//SUBFN_CODEC_GETNAME  :   get decoder name
//SUBFN_CODEC_GETARTIST:   get artist name.
//SUBFN_CODEC_GETTITLE :   get song title.
//SUBFN_CODEC_GETBITRATE:  get bit rate.
//SUBFN_CODEC_GETSAMPLERATE: get sample rate.
//SUBFN_CODEC_GETCHANNELS: get channel number.
//SUBFN_CODEC_GETLENGTH :  get total play time [unit:ms]
//SUBFN_CODEC_GETTIME  :   get current play time.[unit:ms].note:this time get by timestamp,there may be error if file is been demage..
//SUBFN_CODEC_OPEN_DEC :   open deooder(initialization.)
//SUBFN_CODEC_DECODE   :   deocode.
//SUBFN_CODEC_ENCODE   :   not support.
//SUBFN_CODEC_SEEK     :   location by time directly.[unit:ms]
//SUBFN_CODEC_CLOSE    :   close decoder.
//SUBFN_CODEC_SETBUFFER:   set cache area,point out the position to put save result.
/******************************************************
Name:
Desc:
Param: ulIoctl child function number.
    ulParam1 child function parameter 1.
    ulParam2 child function parameter 2.
    ulParam3 child function parameter 3.
    ulParam4 child function parameter 4.

Return:
Global:
Note:
Author:
Log:
******************************************************/

#else
//#include "sysinclude.h"

#include "driverinclude.h"
extern MediaBlock  gpMediaBlock;
extern unsigned char *DecDataBuf[2];
extern unsigned char DecBufID;

static UINT16 DmaTranferCallback;
extern unsigned int coed_dma_channel;

static void DMATranferCallBack(uint32 ch)
{
    DmaDev_RealseChannel(hAudioDma, coed_dma_channel);
    rkos_semaphore_give_fromisr(osAudioDecodeOk);
}


#endif


_ATTR_MP3DEC_TEXT_
unsigned long MP3Function(unsigned long ulIoctl, unsigned long ulParam1,
                          unsigned long ulParam2, unsigned long ulParam3)
{
    unsigned int *pTrack,i;

#ifdef  A_CORE_DECODE
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
        {
            return mp3_open(1);
        }

        case SUBFN_CODEC_DEC_GETBUFFER:
        {
            mp3_get_buffer(ulParam1,ulParam2);
            return 1;
        }

        case SUBFN_CODEC_DECODE:
        {
            return mp3_decode();
        }

        case SUBFN_CODEC_GETSAMPLERATE:
        {
            *(int *)ulParam1 = mp3_get_samplerate();
            return(1);
        }

        case SUBFN_CODEC_GETCHANNELS:
        {
            *(int *)ulParam1 = mp3_get_channels();
            return(1);
        }

        case SUBFN_CODEC_GETBITRATE:
        {
            *(int *)ulParam1 = mp3_get_bitrate();
            return(1);
        }

        case SUBFN_CODEC_GETLENGTH:
        {
            *(int *)ulParam1 = mp3_get_length();
            return 1;
        }

        case SUBFN_CODEC_GETTIME:
        {
            *(int *)ulParam1 = (long long) mp3_get_timepos() * 1000 / mp3_get_samplerate();
            return 1;
        }

        case SUBFN_CODEC_SEEK:
        {
            mp3_seek(ulParam1);
            return 1;
        }

        case SUBFN_CODEC_CLOSE:
        {
            mp3_close();
            return 1;
        }

        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
        {
            *(int *)ulParam1 = mp3_get_bitpersample();
            return 1;
        }

        default:
        {
            return 0;
        }
    }
#else   //b core decode
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
        {
            gpMediaBlock.directplay = ulParam1;
            gpMediaBlock.savememory = ulParam2;

            #ifdef CODEC_24BIT
            gpMediaBlock.CodecDataWidth = 24;
            #endif

            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData((uint32)&gpMediaBlock, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            gpMediaBlock.needDecode = 0;
            rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            if (gpMediaBlock.DecodeErr)   //codec decode open error
                return 0;
            else
                return(1);
        }

        case SUBFN_CODEC_DECODE:
        {
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            gpMediaBlock.needDecode = 1;
            rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            if (gpMediaBlock.DecodeErr == 1)
            {
                return 0;
            }
            return 1;
        }

        case SUBFN_CODEC_DEC_GETBUFFER:
        {
            if(gpMediaBlock.savememory)
            {
                *(int *)ulParam1 = gpMediaBlock.Outptr;
                *(int *)ulParam2 = gpMediaBlock.OutLength;
                return 1;
            }
            else
            {
retry:
                IntDisable(INT_ID_MAILBOX1);
                if(gpMediaBlock.DecodeOver == 1)
                {
                    if (gpMediaBlock.DecodeErr == 1)
                    {
                        IntEnable(INT_ID_MAILBOX1);
                        return 0;
                    }

                    // *(int *)ulParam1 = gpMediaBlock.Outptr;
                    *(int *)ulParam2 = gpMediaBlock.OutLength;
                    gpMediaBlock.DecodeOver = 0;
                    MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    //rk_printf("gpMediaBlock.OutLength = %d", gpMediaBlock.OutLength);
                    //memory copy hram 2 lram.
                    if (DecBufID == 0)
                    {
                        #if 1
                        {

                             rk_err_t ret;
                             DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

                             ret = DmaDev_GetChannel(hAudioDma);
                             coed_dma_channel = ret;
                             if (ret < 0)
                             {
                                  //printf("#ID0  NO  DmaDev_GetChannel\n ");
                                  memcpy(DecDataBuf[0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);
                             }
                             else
                             {
                              #if 0
                                 pTrack = (unsigned int *)gpMediaBlock.Outptr;
                                 for (i = 0; i < 50; i++)//pstWriteAudioDev->TrackLen
                                 {
                                     printf("## Track NO.%d = 0x%x \n", i, *pTrack);
                                     pTrack++;
                                 }

                              #endif
                                 //printf("@#ID0 DmaDev_GetChannel ret=%d OutLength =%d BitPerSample=%d\n",ret,gpMediaBlock.OutLength,gpMediaBlock.BitPerSample);
                                 DmaDev_DmaStart(hAudioDma, (uint32)(ret), (UINT32)(gpMediaBlock.Outptr),(uint32)(DecDataBuf[0]),(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 16,&DmaCfg, DMATranferCallBack);

                                 rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);

                             }
                        }
                        #else

                        memcpy(&DecDataBuf[0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);

                        #endif
                        *(int *)ulParam1 = (uint32)DecDataBuf[0];
                        DecBufID = 1;
                    }
                    else
                    {

                        #if 1
                        {

                             rk_err_t ret;
                             DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

                             ret = DmaDev_GetChannel(hAudioDma);
                             coed_dma_channel = ret;
                             if (ret < 0)
                             {
                                  //printf("NO  DmaDev_GetChannel\n ");
                                  memcpy(DecDataBuf[1],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);
                             }
                             else
                             {
                                 //printf("@#DmaDev_GetChannel ret=%d OutLength =%d BitPerSample=%d\n",ret,gpMediaBlock.OutLength,gpMediaBlock.BitPerSample);
                                 DmaDev_DmaStart(hAudioDma, (uint32)(ret), (UINT32)(gpMediaBlock.Outptr),(uint32)(DecDataBuf[1]),(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 16,&DmaCfg, DMATranferCallBack);

                                 rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);

                             }
                        }
                        #else

                        memcpy(&DecDataBuf[1],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);

                        #endif

                        *(int *)ulParam1 = (uint32)DecDataBuf[1];
                        DecBufID = 0;
                    }

                    IntEnable(INT_ID_MAILBOX1);

                    return(1);
                }
                else
                {
                    rk_printf("wait b core...");
                    gpMediaBlock.needDecode = 1;
                    IntEnable(INT_ID_MAILBOX1);
                    rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                    goto retry;
                }

                return(0);
            }
        }


        case SUBFN_CODEC_GETSAMPLERATE:
        {
            *(int *)ulParam1 = gpMediaBlock.SampleRate;
            return(1);
        }

        case SUBFN_CODEC_GETCHANNELS:
        {
            *(int *)ulParam1 = gpMediaBlock.Channel;
            return(1);
        }

        case SUBFN_CODEC_GETBITRATE:
        {
            *(int *)ulParam1 = gpMediaBlock.BitRate;
            return(1);
        }

        case SUBFN_CODEC_GETLENGTH:
        {
            *(int *)ulParam1 = gpMediaBlock.TotalPlayTime;
            return 1;
        }

        case SUBFN_CODEC_GETTIME:
        {
            *(int *)ulParam1 = gpMediaBlock.CurrentPlayTime;
            return 1;
        }

        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
        {
            *(int *)ulParam1 = gpMediaBlock.BitPerSample;
            return(1);
        }

        case SUBFN_CODEC_SEEK:
        {
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            return 1;
        }

        case SUBFN_CODEC_CLOSE:
        {
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            return 1;
        }

        case SUBFN_CODEC_GET_FRAME_LEN:
        {
            *(int *)ulParam1 = gpMediaBlock.OutLength;
        }

        default:
        {
            return 0;
        }
    }
#endif
    return -1;
}

#endif
