
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: BBSystem\Codecs\Audio\Decode\alac\pAlac.c
* Owner: WJR
* Date: 2014.12.23
* Time: 19:31:51
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.12.23     19:31:51   1.0
********************************************************************************************
*/

//#include "SysInclude.h"
#include "RKOS.h"
#include "audio_main.h"
//#include "audio_file_access.h"
#ifdef HIFI_AlAC_DECODE


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "audio_globals.h"
#include "audio_file_access.h"


#pragma arm section code = "AlacHDecCode", rodata = "AlacHDecCode", rwdata = "AlacHDecData", zidata = "AlacHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_READ_  __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_read")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_WRITE_ __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_write")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_INIT_  __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_init")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_AlAC_PALAC_SHELL_  __attribute__((section("bbsystem_codecs_audio_decode_alac_palac_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern unsigned int coed_dma_channel;
#ifdef A_CORE_DECODE
#include "hifi_alac_MovFile.h"
#include "alac.h"
//uint8 gAlacPingPangBuf[2][4096*8];
unsigned int Alac_out_Length ;
unsigned int  gAlacPingPangIndex = 0;
extern ALACContext alac_con;

extern unsigned long SRC_Num_Forehead;
uint8 *gAlacPingPangBuf[2];
#else
//#include "sysinclude.h"
#include "driverinclude.h"

extern MediaBlock  gpMediaBlock;
extern unsigned char *DecDataBuf[2];
extern unsigned char DecBufID;
static UINT16 DmaTranferCallback;

static void DMATranferCallBack(uint32 ch)
{
    if(ch != coed_dma_channel)
    {
        printf("ch = %d, coed_dma_channel = %d",ch, coed_dma_channel);
        while(1);
    }
    DmaDev_RealseChannel(hAudioDma, coed_dma_channel);
    rkos_semaphore_give_fromisr(osAudioDecodeOk);
}

#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

FILE *alac_file_handle;
extern FILE *pRawFileCache;
int ALAC_filelen ;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: APEDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
READ API unsigned long  HIFI_ALACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{
 //printf("\n enter HIFI_ALACDecFunction ulSubFn =%d\n",ulSubFn);
#ifdef A_CORE_DECODE
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                int ret;
                alac_file_handle=(FILE*)pRawFileCache;
                HIFI_DMA_TO_register();
                ALAC_filelen = RKFIO_FLength(pRawFileCache);
                ret =  Alac_decode_init(alac_file_handle);
                      if(ret < 0)
                        {
                           return ret;
                        }
                Alac_header_parse();
                Alac_out_Length =alac_con.max_samples_per_frame;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
                int out_size;
                Alac_out_Length = Alac_frame_decode(&gAlacPingPangBuf[gAlacPingPangIndex][SRC_Num_Forehead] ,&out_size);
                if(Alac_out_Length <= 0)
                {
                    return 0;
                }
                return 1;
            }
        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long) &gAlacPingPangBuf[gAlacPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Alac_out_Length;

                gAlacPingPangIndex ^= 1;;
                 return(1);
            }


        case SUBFN_CODEC_SEEK:
            {
                 MovIF_SynAudio2Video_h((unsigned int )ulParam1 );
                 return(1);
            }

                    // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = (int ) MovIF_AudioGetCurrentTime_h();
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = alac_con.samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = alac_con.channels;
                return 1;
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = alac_con.sample_size;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                 extern int alac_data_size;
                bitrate = (unsigned long *)ulParam1;
                *bitrate = alac_con.average_bitrate;
                if(*bitrate == 0)
                    *bitrate =((long long )alac_data_size*8*gMovFile_h.audioTimeScale)/(gMovFile_h.uint32AudioDuration);
                *bitrate  += 500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength =  (gMovFile_h.uint32AudioDuration/gMovFile_h.audioTimeScale)*1000;
                return 1;
            }


            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
              HIFI_DMA_TO_Unregister();
              return 1;
            }
        default:
            {
                // Return a failure.
                return(0);
            }
    }
#else
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                gpMediaBlock.needDecode = 0;
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                if(gpMediaBlock.DecodeErr)    //codec decode open error
                    return 0;
                else
                    return(1);
            }
        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 1;
            }
        case SUBFN_CODEC_DEC_GETBUFFER:
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
                    //DEBUG("hifi alac get buffer...DecodeOver = %d",gpMediaBlock.DecodeOver);
                    rk_printf("wait b core...");
                    gpMediaBlock.needDecode = 1;
                    IntEnable(INT_ID_MAILBOX1);
                    rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                    goto retry;
                }

                return(0);
            }

        case SUBFN_CODEC_SEEK:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                return 1;
            }

        // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(unsigned long *)ulParam1 = gpMediaBlock.CurrentPlayTime;
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = gpMediaBlock.SampleRate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = gpMediaBlock.Channel;
                return 1;
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitPerSample;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitRate;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                *(unsigned long *)ulParam1 = gpMediaBlock.TotalPlayTime;
                return 1;
            }

            // Cleanup after the codec.
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
                // Return a failure.
                return(0);
            }
    }
#endif
}


#pragma arm section code
#endif

