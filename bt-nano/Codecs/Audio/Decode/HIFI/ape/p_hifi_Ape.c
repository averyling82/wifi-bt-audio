/*
********************************************************************************************
*
*          Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: BBSystem\Codecs\Audio\Decode\ape\pApe.c
* Owner: WJR
* Date: 2014.12.23
* Time: 19:31:51
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.12.23     19:31:51   1.0
********************************************************************************************
*/

#include "rkos.h"
#include "audio_main.h"

#ifdef HIFI_APE_DECODE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/


#include "audio_globals.h"



#pragma arm section code = "ApeHDecCode", rodata = "ApeHDecCode", rwdata = "ApeHDecData", zidata = "ApeHDecBss"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "rkos.h"
#include "audio_file_access.h"
extern unsigned int coed_dma_channel;


static void DMATranferCallBack(uint32 ch)
{
    DmaDev_RealseChannel(hAudioDma, coed_dma_channel);
    rkos_semaphore_give_fromisr(osAudioDecodeOk);
}


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
unsigned long  HIFI_APEDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
                                              unsigned long ulParam2, unsigned long ulParam3)
{

    switch (ulSubFn)
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
        }
        case SUBFN_CODEC_SEEK:
        {
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            return(1);
        }

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

}



#pragma arm section code
#endif
