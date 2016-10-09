/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Codecs\Audio\Decode\Amr\pAmr.c
* Owner: cjh
* Date: 2016.5.26
* Time: 14:33:43
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.26     14:33:43   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#include "RKOS.h"
#include "../include/audio_main.h"
#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"

#ifdef AMR_DEC_INCLUDE
#ifdef A_CORE_DECODE

#include <stdio.h>
#include <string.h>    //for memcpy(),memmove()
#include "typedef.h"
#include <stdlib.h>
#include <errno.h>
#include "cnst.h"
#include "mode.h"
#include "frame.h"
#include "strfunc.h"
#include "sp_enc.h"
#include "pre_proc.h"
#include "sid_sync.h"
#include "vadname.h"
#include "e_homing.h"
#include "sp_dec.h"
#include "d_homing.h"
#include "rk_types.h"
#include "mailbox.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#pragma arm section code = "AmrDecCode", rodata = "AmrDecCode", rwdata = "AmrDecData", zidata = "AmrDecBss"

#include "amr.h"

_ATTR_AMRDEC_BSS_
short *gAmrOutbuf[2];


_ATTR_AMRDEC_BSS_
amr_Decoder_t amr_decoder;
_ATTR_AMRDEC_BSS_
uint16 speech[L_FRAME];
_ATTR_AMRDEC_BSS_
enum Mode mode = (enum Mode)0;

_ATTR_AMRDEC_BSS_
uint32 bits[8];    //pack the 244 info bits into 8 u32-Words for the channel encoder

_ATTR_AMRDEC_BSS_
enum RXFrameType rx_type = (enum RXFrameType)0;
_ATTR_AMRDEC_BSS_
enum TXFrameType tx_type = (enum TXFrameType)0;
_ATTR_AMRDEC_BSS_
uint16 serial[SERIAL_FRAMESIZE];   /* coded bits                    */
_ATTR_AMRDEC_DATA_
uint8 FileHeader[7];   /* AMR header                    */

_ATTR_AMRDEC_DATA_
uint16 dtx_mode = 0;

#ifdef EFR_ON_AMR
_ATTR_AMRDEC_DATA_
uint16 efr_flag = 1;
#else
_ATTR_AMRDEC_DATA_
uint16 efr_flag = 0;
#endif

extern unsigned long SRC_Num_Forehead; //for src
#else
//#include "sysinclude.h"

#include "driverinclude.h"

extern unsigned int coed_dma_channel;

extern MediaBlock  gpMediaBlock;
extern unsigned char *DecDataBuf[2];
extern unsigned char DecBufID;
static UINT16 DmaTranferCallback;

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
static void DMATranferCallBack(void)
{
    //printf("DMACallBack give sema osAudioDecodeOk\n");
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
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AmrFunction
** Input:unsigned long ulIoctl, unsigned long ulParam1,unsigned long ulParam2,unsigned long ulParam3,unsigned long ulParam4
** Return: unsigned long
** Owner:cjh
** Date: 2016.5.26
** Time: 14:50:02
*******************************************************************************/
_ATTR_AMRDEC_TEXT_
API unsigned long AmrFunction(unsigned long ulIoctl, unsigned long ulParam1,unsigned long ulParam2,unsigned long ulParam3)
{
    unsigned int *pTrack,i;
    uint32 frame;
    uint8 tempType;
    uint8 tempMode;
#ifdef  A_CORE_DECODE
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                rawfile=(FILE*)pRawFileCache;

                amr_decoder = amr_DecoderCreate();
                RKFIO_FSeek(0, 0 ,rawfile);
                frame = 0;
            #ifndef EFR_ON_AMR
                RKFIO_FRead((uint8 *)FileHeader,7,rawfile);
            #else
                RKFIO_FRead((uint8 *)FileHeader,7,rawfile);
            #endif
                RKFIO_FSeek(0,0 ,rawfile);
            #ifndef EFR_ON_AMR
                tempMode = (enum Mode) FileHeader[6] & 0x1E;
                switch (tempMode)
                {
                    case 0x04:
                        mode = MR475;
                        break;
                    case 0x0C:
                        mode = MR515;
                        break;
                    case 0x14:
                        mode = MR59;
                        break;
                    case 0x1C:
                        mode = MR67;
                        break;
                    case 0x24:
                        mode = MR74;
                        break;
                    case 0x2C:
                        mode = MR795;
                        break;
                    case 0x34:
                        mode = MR102;
                        break;
                    case 0x3C:
                        mode = MR122;
                        break;

                    default:
                        break;
                }

                tx_type = (enum TXFrameType)FileHeader[6] & 0x20;
                 ++frame;
                 if ( (frame%50) == 0) {
                    rk_printf("\r err frame=%d  ", frame);
                 }
                 if(frame == 26)
                     frame = frame;
            #endif
            #ifdef EFR_ON_AMR
                mode = MR122;
                if((serial[0]==0)&&(serial[245]==0))
                {
                    tx_type = TX_SPEECH_GOOD;
                }
                else
                {
                    tx_type = TX_NO_DATA;
                    for(i=0; i<8; i++)
                        bits[i] = cn_bits[i];
                }
            #endif

                return 1;
            }

        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long)(&gAmrOutbuf[select][SRC_Num_Forehead]);

                *(unsigned long *)ulParam2 = (unsigned long) (OUTWRITE/2);

                select += 1;
                select = select%2;
                return(1);
            }

        case SUBFN_CODEC_DECODE:
            {
                Amr_decode(amr_decoder, &bits[0], mode, tx_type, &speech, 14);
                return 1;
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                tempType = (serial[4] & 0xFF0)>>8;
                switch(tempType)
                {
                    case 0x04:
                    case 0x0C:
                    case 0x14:
                    case 0x1C:
                    case 0x24:
                    case 0x2C:
                    case 0x34:
                    case 0x3C:
                        *(int *)ulParam1 = 8000; //8KHZ
                        break;

                    default:
                        *(int *)ulParam1 = 16000;
                        break;
                }
                AmrSamplerate = *(int *)ulParam1;
                //*(int *)ulParam1 = 160000;
                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = 1;
                return(1);
            }

        case SUBFN_CODEC_GETBITRATE:
            {
                tempType = (serial[4] & 0xFF0)>>8;
                switch(tempType)
                {
                    case 0x04:
                        *(int *)ulParam1 = 4750;
                        break;

                    case 0x0C:
                        *(int *)ulParam1 = 5150;
                        break;

                    case 0x14:
                        *(int *)ulParam1 = 5900;
                        break;

                    case 0x1C:
                        *(int *)ulParam1 = 6700;
                        break;

                    case 0x24:
                        *(int *)ulParam1 = 7400;
                        break;

                    case 0x2C:
                        *(int *)ulParam1 = 7950;
                        break;

                    case 0x34:
                        *(int *)ulParam1 = 1020;
                        break;

                    case 0x3C:
                        *(int *)ulParam1 = 1220;
                        break;

                    default:
                        break;
                }
                return 1;
            }

        case SUBFN_CODEC_GETLENGTH://nLengthMS
            {
                unsigned long *pulLength;

                pulLength = (unsigned long *)ulParam1;
                *pulLength = //pPCM->ulTimeLength;
                return(1);
            }

        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = (long long) mp3_get_timepos() * 1000 / AmrSamplerate;
                return 1;
            }

        case SUBFN_CODEC_SEEK:
            {
                return 1;
            }

        case SUBFN_CODEC_CLOSE:
            {
                if(amr_decoder != NULL)
                {
                    amr_DecoderDestroy(amr_decoder);
                }
                return 1;
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            *(int *)ulParam1 = 16;
            return 1;
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
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
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
                    //DEBUG("hifi ape get buffer...DecodeOver = %d",gpMediaBlock.DecodeOver);
                    gpMediaBlock.needDecode = 1;
                    IntEnable(INT_ID_MAILBOX1);
                    rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                    goto retry;
                }

                return(0);
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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
