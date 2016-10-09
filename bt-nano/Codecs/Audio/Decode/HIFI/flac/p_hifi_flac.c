
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
* FileName: BBSystem\Codecs\Audio\Decode\flac\pFlac.c
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

#ifdef HIFI_FlAC_DECODE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "audio_file_access.h"
#include "audio_globals.h"


#pragma arm section code = "FlacHDecCode", rodata = "FlacHDecCode", rwdata = "FlacHDecData", zidata = "FlacHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_READ_  __attribute__((section("bbsystem_codecs_audio_decode_flac_pflac_read")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_WRITE_ __attribute__((section("bbsystem_codecs_audio_decode_flac_pflac_write")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_INIT_  __attribute__((section("bbsystem_codecs_audio_decode_flac_pflac_init")))
#define _BBSYSTEM_CODECS_AUDIO_DECODE_FlAC_PFLAC_SHELL_  __attribute__((section("bbsystem_codecs_audio_decode_flac_pflac_shell")))


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern unsigned int coed_dma_channel;
#ifdef A_CORE_DECODE
#include "flacdec.h"
//uint8 gFlacPingPangBuf[2][4096*8];
extern unsigned long SRC_Num_Forehead;
uint8 *gFlacPingPangBuf[2];
unsigned int Flac_out_Length ;  //sample points
unsigned int gFlacPingPangIndex = 0;
extern FLACStreaminfo s_FLAC_INFO;
int ID3_len = 0;

#else
//#include "sysinclude.h"
#include "driverinclude.h"
extern MediaBlock  gpMediaBlock;
extern unsigned char *DecDataBuf[2];
extern unsigned char DecBufID;

static UINT16 DmaTranferCallback;

static void DMATranferCallBack(uint32 ch)
{
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


FILE *flac_file_handle;
extern FILE *pRawFileCache;
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
static unsigned int CheckID3V2Tag(unsigned      char *pucBuffer)
{
    // The first three bytes of      the      tag      should be "ID3".
    if ((pucBuffer[0] !='I') ||      (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next      byte should      be the value 3 (i.e. we      support      ID3v2.3.0).
    //if(pucBuffer[3]      != 3)
    if (pucBuffer[3]<2  && pucBuffer[3]> 4)
    {
        return(0);
    }

    // The next      byte should      be less      than 0xff.
    if (pucBuffer[4]== 0xff)
    {
        return(0);
    }

    // We don't      care about the next      byte.  The following four bytes      should be
    // less      than 0x80.
    if ((pucBuffer[6] >= 0x80) || (pucBuffer[7] >= 0x80)      ||
            (pucBuffer[8] >= 0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of      the      ID3v2 tag.
    return((pucBuffer[6] <<  21)      | (pucBuffer[7]  << 14) |
           (pucBuffer[8] <<  7)      |  pucBuffer[9]);
}


/*******************************************************************************
** Name: FLACDecFunction
** Input:unsigned long ulSubFn, unsigned long ulParam1,
** Return: unsigned long
** Owner:WJR
** Date: 2014.12.23
** Time: 19:31:54
*******************************************************************************/
READ API unsigned long  HIFI_FLACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{
#ifdef A_CORE_DECODE
    switch (ulSubFn)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                unsigned char flag[20];
                int ret ;
                flac_file_handle =pRawFileCache;

                RKFIO_FSeek(0,0 ,flac_file_handle);
                RKFIO_FRead(flag,20,flac_file_handle);
                ID3_len = CheckID3V2Tag(flag);
                if (ID3_len == 0)
                {
                    RKFIO_FSeek(0,0 ,flac_file_handle);
                }
                else
                {
                    ID3_len += 10;
                    RKFIO_FSeek(ID3_len,0 ,flac_file_handle);
                }
                        HIFI_DMA_TO_register();
                ret = FLAC_decode_init();
                if(ret < 0)
                {
                  return 0;
                }

                Flac_out_Length = s_FLAC_INFO.max_blocksize;
                return 1;
            }
        case SUBFN_CODEC_DECODE:
            {
                int ret;
                ret = FLAC_frame_decode(&gFlacPingPangBuf[gFlacPingPangIndex][SRC_Num_Forehead] ,&Flac_out_Length);
                if(ret <= 0)
                {
                    return 0;
                }
                if(s_FLAC_INFO.samples_decoded >= s_FLAC_INFO.samples )
                {
                  Hifi_Flac_Printf("END\n");
                  return 0;
                }
                return 1;
            }
        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long) &gFlacPingPangBuf[gFlacPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) Flac_out_Length;

                 gFlacPingPangIndex ^= 1;
                 return 1;
            }


        case SUBFN_CODEC_SEEK:
            {
                hifi_flac_seek( ulParam1/1000); /* seconds */
                return 1;
            }

                    // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                unsigned long *curtime;
                curtime = (unsigned long *)ulParam1;
                *curtime = (unsigned long)((long long)s_FLAC_INFO.samples_decoded *1000/ s_FLAC_INFO.samplerate);
                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *SampleRate;
                SampleRate = (unsigned long *)ulParam1;
                *SampleRate = s_FLAC_INFO.samplerate;
                return  1;
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *channels;
                channels = (unsigned long *)ulParam1;
                *channels = s_FLAC_INFO.channels;
                return 1;
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                unsigned long *bps;
                bps = (unsigned long *)ulParam1;
                *bps = s_FLAC_INFO.bps;
                return 1;
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *bitrate;
                bitrate = (unsigned long *)ulParam1;
                *bitrate= s_FLAC_INFO.bitrate +500;
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *TimeLength;
                TimeLength = (unsigned long *)ulParam1;
                *TimeLength = (long long)s_FLAC_INFO.samples*1000 / s_FLAC_INFO.samplerate;
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
                *(int *)ulParam1 = gpMediaBlock.CurrentPlayTime;
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
                *(int *)ulParam1 = gpMediaBlock.TotalPlayTime;
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
