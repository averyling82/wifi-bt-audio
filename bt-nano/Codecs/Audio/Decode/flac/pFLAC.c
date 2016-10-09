/* Copyright (C) 2009 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pFLAC.c
Desc    : floe chart of FLAC decode

Author  : Vincent Hsiung (xw@rock-chips.com)
Date    : Apr 10 , 2009
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

#ifdef FLAC_DEC_INCLUDE

#include <stdio.h>
#include <string.h>      //for memcpy(),memmove()
#include "typedef.h"
#include "mailbox.h"

#ifdef A_CORE_DECODE
int ID3_len = 0;
#else

#pragma arm section code = "FlacDecCode", rodata = "FlacDecCode", rwdata = "FlacDecData", zidata = "FlacDecBss"


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

//*************************************************************************************************************//
//the achievement of functions.¡êo
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


#ifdef A_CORE_DECODE
_ATTR_FLACDEC_TEXT_
static unsigned int CheckID3V2Tag(unsigned      char *pucBuffer)
{
    // The first three bytes of      the      tag      should be "ID3".
    if ((pucBuffer[0] !=      'I') ||      (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next      byte should      be the value 3 (i.e. we      support      ID3v2.3.0).
    //if(pucBuffer[3]      != 3)
    if (pucBuffer[3]      <2  && pucBuffer[3]> 4)
    {
        return(0);
    }

    // The next      byte should      be less      than 0xff.
    if (pucBuffer[4]      == 0xff)
    {
        return(0);
    }

    // We don't      care about the next      byte.  The following four bytes      should be
    // less      than 0x80.
    if ((pucBuffer[6] >=      0x80) || (pucBuffer[7] >= 0x80)      ||
            (pucBuffer[8] >=      0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of      the      ID3v2 tag.
    return((pucBuffer[6] <<      21)      | (pucBuffer[7]      << 14) |
           (pucBuffer[8] <<       7)      |  pucBuffer[9]);
}
#endif

_ATTR_FLACDEC_TEXT_
unsigned long FLACDecFunction(unsigned long ulIoctl, unsigned long ulParam1,
                              unsigned long ulParam2, unsigned long ulParam3)
{
#ifdef A_CORE_DECODE
    switch (ulIoctl)
    {
            /* put these to ID3?
                    case SUBFN_CODEC_GETNAME:
                        {
                            return(1);
                        }
                    case SUBFN_CODEC_GETARTIST:
                        {
                            return(1);
                        }

                    case SUBFN_CODEC_GETTITLE:
                        {
                            return(1);
                        }
            */
        case SUBFN_CODEC_OPEN_DEC:
            {
                unsigned char flag[20];

                FILE *rawfile=(FILE*)pRawFileCache;
                RKFIO_FSeek(0,0 ,rawfile);
                RKFIO_FRead(flag,20,rawfile);
                ID3_len = CheckID3V2Tag(flag);
                if (ID3_len == 0)
                {
                    RKFIO_FSeek(0,0 ,rawfile);

                }
                else
                {
                    ID3_len += 10;
                    RKFIO_FSeek(ID3_len,0 ,rawfile);

                }
                return flac_open_dec();
            }

        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                flac_get_buffer(ulParam1,ulParam2);
                return 1;
            }

        case SUBFN_CODEC_DECODE:
            {
                return flac_decode();
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = flac_get_samplerate();
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = flac_get_channels();
                return(1);
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = flac_get_bitrate();
                return(1);
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                *(int *)ulParam1 = flac_get_bps();
                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = flac_get_length();
                return 1;
            }

        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = (long long) flac_get_timepos();
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                flac_seek( ulParam1 / 1000 ); /* seconds */
                return 1;
            }

        case SUBFN_CODEC_CLOSE:
            {
                flac_close_dec();
                return 1;
            }

        default:
            {
                return 0;
            }
    }
#else
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                gpMediaBlock.needDecode = 0;
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            }

        case SUBFN_CODEC_DEC_GETBUFFER:
            {
retry:
                IntDisable(INT_ID_MAILBOX1);
                if (gpMediaBlock.DecodeOver == 1)
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
                                  memcpy(DecDataBuf[0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);
                             }
                             else
                             {
                                 DmaDev_DmaStart(hAudioDma, (uint32)(ret), (UINT32)(gpMediaBlock.Outptr),(uint32)(DecDataBuf[0]),(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 16,&DmaCfg, DMATranferCallBack);

                                 rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);

                             }
                        }
                        #else

                        memcpy(&DecDataBuf[0][0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);

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
                                  memcpy(DecDataBuf[1],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);
                             }
                             else
                             {
                                 DmaDev_DmaStart(hAudioDma, (uint32)(ret), (UINT32)(gpMediaBlock.Outptr),(uint32)(DecDataBuf[1]),(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 16,&DmaCfg, DMATranferCallBack);

                                 rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);

                             }
                        }
                        #else

                        memcpy(&DecDataBuf[1][0],(uint8*)(gpMediaBlock.Outptr) ,(gpMediaBlock.OutLength * gpMediaBlock.BitPerSample) / 4);

                        #endif

                        *(int *)ulParam1 = (uint32)DecDataBuf[1];
                        DecBufID = 0;
                    }

                    IntEnable(INT_ID_MAILBOX1);

                    return(1);
                }
                else
                {
                    //DEBUG("hifi ape get buffer...DecodeOver = %d",gpMediaBlock.DecodeOver);
                    gpMediaBlock.needDecode = 1;
                    IntEnable(INT_ID_MAILBOX1);
                    rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);

                    goto retry;
                }

                return(0);
            }

        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 1;
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
