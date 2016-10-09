/* Copyright (C) 2013 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File    : \Audio\pOGG.c
Desc    : floe chart of FLAC decode

Author    : wu jiangrui (wjr@rock-chips.com)
Date    : Aug 24 , 2013
Notes    :

$Log    :
*
*
*/
/****************************************************************/
#include "RKOS.h"
#include "../include/audio_main.h"
#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#ifdef OGG_DEC_INCLUDE
#include "audio_globals.h"
#include "audio_file_access.h"
//#include "OsInclude.h"


#include "typedef.h"
#include "mailbox.h"
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
//SUBFN_CODEC_SEEK       :   location by time directly.[unit:ms]
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
#pragma arm section code = "OggDecCode", rodata = "OggDecCode", rwdata = "OggDecData", zidata = "OggDecBss"

#ifdef A_CORE_DECODE

#include "lib/ivorbiscodec.h"
#include "lib/ivorbisfile.h"

static FILE *rawfile;
static OggVorbis_File vf;
#define OUT_SIZE 2048
//char pcmout[2][OUT_SIZE];//4096 /* take 4k out of the data segment, not the stack */
short *OggPcmOut[2];
extern  unsigned long SRC_Num_Forehead;

int select = 0;
int out_size = OUT_SIZE;
int i = 0;
signed char  f_open_dec_table;
signed char f_pcm_out;
signed char f_dec_table;
char dec_buf[OUT_SIZE*2];
int pos = 0;
//int ogg_cnt = 0;
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

unsigned long OGGDecFunction(unsigned long ulIoctl, unsigned long ulParam1,
                             unsigned long ulParam2, unsigned long ulParam3)
{
    //printf("\n enter OGGDecFunction ulIoctl =%d\n",ulIoctl);

#ifdef A_CORE_DECODE

    int current_section = 0;
    char **ptr;
    vorbis_info *vi;
    int ret;
    int wr_ret;
    int i;

    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                rawfile=(FILE*)pRawFileCache;

                RKFIO_FSeek(0, SEEK_SET, rawfile);

                if (ov_open(rawfile, &vf, NULL, 0) < 0)
                {
                    printf("\ov_open return 0 \n");
                    return 0;
                }
                //ogg_cnt =1;
                return 1;
            }

        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                *(unsigned long*)ulParam1 = (unsigned long)(&OggPcmOut[select][SRC_Num_Forehead]);

                *(unsigned long *)ulParam2 = (unsigned long) (OUT_SIZE/4); // char 转成long

                select^=1;

                return 1;
            }

        case SUBFN_CODEC_DECODE:
            {
                int tick_end;

                if (vf.vi.channels == 2)
                {
                    char *output = (char*)&OggPcmOut[select][SRC_Num_Forehead];
                    while (pos < OUT_SIZE)
                    {
                        //ret=ov_read(&vf,&pcmout[select][pos],OUT_SIZE-pos,&current_section);
                        ret=ov_read(&vf,&output[pos],OUT_SIZE-pos,&current_section);

                        if (ret <= 0)
                        {
                            //FileClose(f_pcm_out);
                            return 0;
                        }
                        else
                        {
                            pos += ret;
                        }
                    }
                    pos -= OUT_SIZE;
                }
                else
                {
                    while (pos < OUT_SIZE/2)
                    {
                        ret=ov_read(&vf,&dec_buf[pos],OUT_SIZE/2-pos,&current_section);
                        if (ret <= 0)
                        {
                            //FileClose(f_pcm_out);
                            return 0;
                        }
                        else
                        {
                            pos += ret;
                        }
                    }
                    {
                        short * input;
                        short *output;

                        output = (short *)&OggPcmOut[select][SRC_Num_Forehead];
                        input = (short *)dec_buf;
                        for (i=0;i<OUT_SIZE/4;i++)
                        {
                            output[2*i] = input[i];
                            output[2*i+1] = input[i];
                        }
                    }
                    pos -= OUT_SIZE/2;
                }
                // if(ogg_cnt ==1)
                {
                    //ogg_cnt =0;
                    //tick_end= GetSysTick();
                    //printf("开始 %d ",tick_end);
                }
                return 1;
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = vf.vi.rate;
                return(1);
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                *(int *)ulParam1 = 16;
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = vf.vi.channels;
                return(1);
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                *(long *)ulParam1 = vf.vi.bitrate_nominal;

                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(long *)ulParam1 = ov_time_total(&vf,-1);
                return 1;
            }

        case SUBFN_CODEC_GETTIME:
            {
                *(long *)ulParam1 = ov_time_tell(&vf);
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                ov_time_seek_page(&vf, ulParam1);
                return 1;
            }

        case SUBFN_CODEC_CLOSE:
            {
                //ov_clear(&vf);
                //int tick_start = GetSysTick();
                //printf("结束 %d ",tick_start);
                //ogg_cnt = 0;
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
                //printf("\SUBFN_CODEC_OPEN_DEC =%d\n",SUBFN_CODEC_OPEN_DEC);
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                gpMediaBlock.needDecode = 0;
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                if (gpMediaBlock.DecodeErr)   //codec decode open error
                    return 0;
                else
                    return(1);
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
                    rk_printf("wait b core...");
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
                *(long *)ulParam1 = gpMediaBlock.BitRate;

                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                *(long *)ulParam1 = gpMediaBlock.TotalPlayTime;
                return 1;
            }

        case SUBFN_CODEC_GETTIME:
            {
                *(long *)ulParam1 = gpMediaBlock.CurrentPlayTime;

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

}

#endif

