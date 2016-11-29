/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\APEDec
Desc    : APE decode

Author    : huangxd , Vincent Hisung
Date    : 2007-08-xx
Notes    :

$Log    :
* huangxd . create the file at 08.xx.2007
*
* vincent .    amendment at 08.xx.2007.
*
*/
/****************************************************************/
#include "RKOS.h"
#include "audio_main.h"

#pragma arm section code = "SbcDecCode", rodata = "SbcDecCode", rwdata = "SbcDecData", zidata = "SbcDecBss"

#ifdef SBC_INCLUDE

#include "audio_globals.h"
#include "audio_file_access.h"
#include "typedef.h"
#include "mailbox.h"

#ifdef A_CORE_DECODE
#include "sbc_interface.h"

short *gSbcOutputPtr[2];
extern    unsigned long SRC_Num_Forehead;

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

_ATTR_SBCDEC_TEXT_
unsigned long SbcDecFunction(unsigned long ulIoctl, unsigned long ulParam1,
        unsigned long ulParam2, unsigned long ulParam3)
{
    unsigned int *pTrack,i;

#ifdef  A_CORE_DECODE
    switch (ulIoctl)
    {

            case SUBFN_CODEC_OPEN_DEC:
                {
                    return sbc_open();
                }

            case SUBFN_CODEC_DEC_GETBUFFER:
                {
                    sbc_get_buffer(ulParam1,ulParam2);
                    return 1;
                }

            case SUBFN_CODEC_DECODE:
                {
                    return sbc_dec();
                }

            case SUBFN_CODEC_GETSAMPLERATE:
                {
                    *(int *)ulParam1 = sbc_get_samplerate();
                    return(1);
                }

            case SUBFN_CODEC_GETCHANNELS:
                {
                    *(int *)ulParam1 = sbc_get_channels();
                    return(1);
                }

            case SUBFN_CODEC_GETBITRATE:
                {
                    *(int *)ulParam1 = sbc_get_bitrate();
                    return(1);
                }

            case SUBFN_CODEC_GETLENGTH:
                {
                    *(int *)ulParam1 = sbc_get_length();
                    return 1;
                }

            case SUBFN_CODEC_GETTIME:
                {
                   *(int *)ulParam1 = (long long) sbc_get_timepos() * 1000 / sbc_get_samplerate();
                    return 1;
                }
///*
            case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
                {
                   *(int *)ulParam1 =16;
                   return 1;
                }
//*/

            case SUBFN_CODEC_SEEK:
                {
                    sbc_seek( ulParam1 / 1000 ); /* seconds */
                    return 1;
                }

            case SUBFN_CODEC_CLOSE:
                {
                    sbc_close();
                    return 1;
                }
            case SUBFN_CODEC_SETBUFFER:

                return 1;

            default:
                {
                    return 0;
                }
        }
    #else   //b core decode
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
            gpMediaBlock.BitPerSample=16;
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


