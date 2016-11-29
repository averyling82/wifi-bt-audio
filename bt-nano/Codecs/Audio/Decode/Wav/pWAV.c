/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File : \Audio\ADPCM
Desc : WAV解码。包括PCM WAV , IMA-ADPCM WAV , MS-ADPCM WAV 。

Author : FSH , Vincent Hisung
Date : 2007-08-xx
Notes :

$Log :
* vincent     2007/08/xx 建立此文件
*
*/
/****************************************************************/
#include "RKOS.h"
#include "../include/audio_main.h"

#ifdef WAV_DEC_INCLUDE

#include "../include/audio_globals.h"

#include "../include/audio_file_access.h"


#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "mailbox.h"

#pragma arm section code = "WavDecCode", rodata = "WavDecCode", rwdata = "WavDecData", zidata = "WavDecBss"

#ifdef A_CORE_DECODE
#include "WAV_LIB/sf_wav.h"
#include "PCM.H"

#define INFO_BYTE_SIZE 32

typedef struct
{
    char Artist[INFO_BYTE_SIZE];
    char Name[INFO_BYTE_SIZE];
    char Product[INFO_BYTE_SIZE];
    char Genre[INFO_BYTE_SIZE];
}tINFO;


__align(4)
_ATTR_WAVDEC_BSS_  char PcmOutputBuff[2730*2*4];
_ATTR_WAVDEC_BSS_  tPCM pcm_s;
_ATTR_WAVDEC_BSS_  short  *WavoutBuf[2];

extern SF_PRIVATE sf;
extern  unsigned long SRC_Num_Forehead; //for src
extern _ATTR_AUDIO_DATA_  int CodecBufSize;
#else
//#include "sysinclude.h"
#include "driverinclude.h"
extern MediaBlock  gpMediaBlock;
extern unsigned char *DecDataBuf[2];
extern unsigned char DecBufID;
extern unsigned int coed_dma_channel;

static UINT16 DmaTranferCallback;

static void DMATranferCallBack(uint32 ch)
{
    DmaDev_RealseChannel(hAudioDma, coed_dma_channel);
    rkos_semaphore_give_fromisr(osAudioDecodeOk);
}

#endif

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
_ATTR_WAVDEC_TEXT_
unsigned long
PCMFunction(unsigned long ulIoctl, unsigned long ulParam1,
            unsigned long ulParam2, unsigned long ulParam3
           )
{

   // printf("PCMFunction \n");
#ifdef A_CORE_DECODE
    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_DEC:
            {
                tPCM *pPCM;
                tINFO pINFO;

                int i;
                int isErr;

                unsigned long ulPos;

                pPCM = &pcm_s;

                memset(&pINFO , 0 , sizeof(pINFO));
                memset(&pcm_s , 0 , sizeof(tPCM));

                pPCM->N_remain = 0;
                pPCM->N_GeneratedSample = 0;

                isErr = InitPCMDecoder(pPCM);

                if (pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
                {
                    sf.sf.channels = pcm_s.ucChannels;
                    sf.sf.samplerate = pcm_s.usSampleRate;
                    sf.sf.format = (SF_FORMAT_WAV | SF_FORMAT_MS_ADPCM);
                    sf.mode = SFM_READ;

                    if (pcm_s.ulLength == 0xffffffff)
                        pcm_s.ulLength = 0x7fffffff;

                    sf.datalength = pcm_s.ulLength;

                    /* wav_w64_msadpcm_init */
                    msadpcm_dec_init (&sf, pcm_s.usBytesPerBlock, pcm_s.usSamplesPerBlock);
                }

                //if (pPCM->usByteRate != 0)
                //    pPCM->ulTimeLength = 1000 * ((long long)(RKFIO_FLength(pRawFileCache)) / (long long) pPCM->usByteRate);

                //----------------------------------------

                //set cache buffer area.
                {
                    unsigned long size;

//                    pPCM->pOutput = (BufferState *) & pPCM->sPlayBuffer;

                    if (pPCM->wFormatTag == WAVE_FORMAT_DVI_ADPCM)
                        size = WAV_IMAMAX_PCM_LENGTH;
                    else
                        if (pPCM->wFormatTag == WAVE_FORMAT_PCM)
                            size = pPCM->usSamplesPerBlock;
                        else
                            size = pPCM->usSamplesPerBlock;

                    pPCM->msadpcm_outsize = size * 2;

                }


                pPCM->OutLength = 1152 ; // pPCM->usSamplesPerBlock;

                ulPos = 0 ;

                pPCM->ulTimePos = ulPos * pPCM->usSamplesPerBlock;

                if (pPCM->wFormatTag == 1)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock) * pPCM->usSamplesPerBlock; //(PCM Seek )
                }

                else //else if(pPCM->sWaveFormat.wFormatTag == 2)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock); //(ADPCM Seek )
                }

                if (ulPos > pPCM->ulLength)
                    return 0;

                if (pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
                {
                    RKFIO_FSeek( (int)ulPos , 0 , pRawFileCache );
                    pPCM->OutLength =  pPCM->usSamplesPerBlock* pPCM->channels ;// maybe error
                }
                else if (pPCM->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
                {
                    RKFIO_FSeek( (int)(ulPos&~511), 0 , pRawFileCache );
                    pPCM->usValid = RKFIO_FRead( pPCM->pucEncodedData, 1024 , pRawFileCache );

                    #if 1

                      if( pPCM->ucChannels == 1 )
                            pPCM->OutLength = (pPCM->usBytesPerBlock - 4) * 8/pPCM->sPCMHeader.uBitsPerSample ;
                      else
                            pPCM->OutLength = (pPCM->usBytesPerBlock - 8) * 4/pPCM->sPCMHeader.uBitsPerSample ;

                    #endif
                    if (pPCM->usValid < 1024)
                        return 0;
                }
                else
                {
                    RKFIO_FSeek( (int)(pPCM->ulDataStart), 0 , pRawFileCache );
                    pPCM->OutLength = pPCM->usSamplesPerBlock;
                    //printf("wav open pcm len = %d \n",pPCM->OutLength);
                }

                pPCM->usOffset = (unsigned short)(ulPos & 511);

                pPCM->ulDataValid = pPCM->ulLength +  pPCM->ulDataStart - (ulPos & ~511);

                WavoutBuf[1] = (short *)PcmOutputBuff;


                return(isErr);
            }
            /*
                    case SUBFN_CODEC_GETARTIST:
                        {
                            char **ppcName;
                            ppcName = (char **)ulParam2;
                            *ppcName = 0;
                            return(1);
                        }

                    case SUBFN_CODEC_GETTITLE:
                        {
                            char **ppcName;
                            ppcName = (char **)ulParam2;
                            *ppcName = 0;

                            return(1);
                        }
            */

        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *pulBitRate;
                tPCM *pPCM;

                pPCM = &pcm_s;

                pulBitRate = (unsigned long *)ulParam1;
                *pulBitRate = pPCM->usByteRate * 8;

                return(1);
            }

        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *pulSampleRate;
                tPCM *pPCM;

                pPCM = &pcm_s;

                pulSampleRate = (unsigned long *)ulParam1;
                *pulSampleRate = pPCM->usSampleRate;

                return(1);
            }

        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *pulChannels;
                tPCM *pPCM;

                pPCM = &pcm_s;

                pulChannels = (unsigned long *)ulParam1;
                *pulChannels = pPCM->ucChannels;

                return(1);
            }

        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *pulLength;
                tPCM *pPCM;

                pPCM = &pcm_s;
                pulLength = (unsigned long *)ulParam1;
                *pulLength = pPCM->ulTimeLength;
                return(1);
            }

        case SUBFN_CODEC_GETTIME:
            {
                unsigned long *pulTime;
                tPCM *pPCM;

                pPCM = &pcm_s;
                pulTime = (unsigned long *)ulParam1;
                *pulTime = (((__int64)pPCM->ulTimePos * 1000 / pPCM->usSampleRate)) +
                           (((((__int64)pPCM->ulTimePos * 1000 % pPCM->usSampleRate)) / pPCM->usSampleRate));
                return(1);
            }

        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                //ulParam1 = buf ..
                //ulParam2 = length ..

                tPCM *pPCM = &pcm_s;
                *(short**)ulParam1 = &WavoutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead];
                if (pPCM->CurDecodeIdx==0)
                    pPCM->CurDecodeIdx = 1;
                else
                    pPCM->CurDecodeIdx = 0;

                *(long*)ulParam2 = pPCM->OutLength;

                return(1);
            }

        case SUBFN_CODEC_DECODE:
            {
                tPCM *pPCM;
                short *psLeft, *psRight;
                short *pOut;
                char  *pOut_24;
                long lLength, lBlockSize;
                long ret;
                short *ptr;
                char *char_ptr;
                unsigned int Counter = 0;

                pPCM = &pcm_s;

                switch (pPCM->wFormatTag)
                {
                    case WAVE_FORMAT_ADPCM:
                        {
                            pPCM->OutLength = msadpcm_read_s (&sf, &WavoutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead] , /* pPCM->usSamplesPerBlock */ 1152 )
                                              / pcm_s.ucChannels ;

                            pPCM->ulTimePos += pPCM->OutLength;

                            if (pPCM->OutLength == 0)
                                return 0;


                            return 1;
                        }

                    case WAVE_FORMAT_IMA_ADPCM:
                        {
                            int i;

                            //--------------------------------
                            if ((pPCM->ulLength) == 0)
                                return 0;

                            if ((pPCM->usOffset) >= (pPCM->ulLength))
                                return 0;

                            if (pPCM->usValid < (pPCM->usOffset & ~3))
                                return 0;
                            //--------------------------------

                            ret = IMAADPCM_FORMAT(pPCM);

                            pPCM->ulTimePos += ret;

                            if (ret > 0)
                                pPCM->OutLength = ret;



                            if (ret != 0)
                                ret = 1;
                            else
                                ret = 0;

                            return(ret);
                        }

                    case WAVE_FORMAT_PCM:
                        {
                            int i;

                            if ((pPCM->ulLength) == 0)
                                return 0;

                            if ((pPCM->usOffset) >= (pPCM->ulLength))
                                return 0;

                            if (pPCM->usValid == 0)
                                return 0;

                            pOut = &WavoutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead];
                            pOut_24 = (char *)&pPCM->OutputBuff[pPCM->CurDecodeIdx][SRC_Num_Forehead];

                            if (pPCM->uBitsPerSample == 16)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock << pPCM->ucChannels);
                            }
                            else if (pPCM->uBitsPerSample == 8)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock * pPCM->ucChannels);
                            }
                            else if (pPCM->uBitsPerSample == 24)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock * pPCM->ucChannels*3);

                            }
                            else if (pPCM->uBitsPerSample == 32)
                            {
                                lBlockSize = (pPCM->usSamplesPerBlock << (pPCM->ucChannels+1));
                            }
                            else
                                return 0;

                            pPCM->usValid = RKFIO_FRead( pPCM->pucEncodedData, lBlockSize , pRawFileCache );

                            if ( pPCM->usValid < lBlockSize )
                                return 0;

                            pPCM->usOffset += pPCM->usValid;

                            //16 bit quantity
                            if (pPCM->uBitsPerSample == 16)
                            {
                                ptr = (short *)pPCM->pucEncodedData;
                                if (pPCM->ucChannels == 1)
                                {
                                    for (i = 0; i < lBlockSize / 2 ; i++)
                                    {
                                        *pOut++  = *ptr   ;      // L
                                        *pOut++  = *ptr++ ;      // R
                                    }
                                }
                                else      //channels == 2
                                {
                                    for (i = 0; i < lBlockSize / 2 ; i++)
                                    {
                                        if (i % 2 == 0) // L,R,L,R
                                        {
                                            *pOut++   = *ptr++ ; //LLL
                                        }
                                        else
                                        {
                                            *pOut++   = *ptr++ ; //RRR
                                        }
                                    }
                                }
                            }
                            else if (pPCM->uBitsPerSample == 8) //8 bit quantity
                            {
                                char_ptr = (char *)pPCM->pucEncodedData;

                                if (pPCM->ucChannels == 1)  //mono
                                {

                                    for (i = 0; i < lBlockSize ; i++)
                                    {
                                        //L
                                        *pOut++   = (short)(((*char_ptr) - 0x80) << 8);//Convert 8BIT(per sample) to 16BIT(per sample)
                                        //R
                                        *pOut++   = (short)(((*char_ptr) - 0x80) << 8);//Convert 8BIT(per sample) to 16BIT(per sample)

                                        char_ptr++ ;
                                    }
                                }
                                else  //stereo
                                {
                                    for (i = 0 ; i < lBlockSize ; i++)
                                    {
                                        // L,R,L,R
                                        if (i % 2 == 0)
                                        {
                                            *pOut++  = (short)(((*char_ptr) - 0x80) << 8);
                                            char_ptr++ ;      //LLL
                                        }
                                        else
                                        {
                                            *pOut++  = (short)(((*char_ptr) - 0x80) << 8);
                                            char_ptr++ ;      //RRR
                                        }
                                    }
                                }
                            }
                            else if (pPCM->uBitsPerSample == 24)
                            {
                                int temp ;
                                if (pPCM->ucChannels == 1)
                                {
                                    for (i = 0; i < lBlockSize / 3 ; i++)
                                    {
                                        temp = pPCM->pucEncodedData[3*i+2]<<16|pPCM->pucEncodedData[3*i+1]<<8
                                               |pPCM->pucEncodedData[3*i];
                                        *(int *)pOut_24  = temp;      // L
                                        pOut_24 += 3;
                                        *(int *)pOut_24  = temp;      // R
                                        pOut_24 += 3;

                                    }
                                }
                                else      //channels == 2
                                {
                                    for (i = 0; i < lBlockSize / 3 ; i++)
                                    {
                                        temp = pPCM->pucEncodedData[3*i+2]<<16|pPCM->pucEncodedData[3*i+1]<<8
                                               |pPCM->pucEncodedData[3*i];
                                        *(int *)pOut_24  = temp;      // L
                                        pOut_24 += 3;


                                    }
                                }
                            }
                            else if (pPCM->uBitsPerSample == 32)
                            {
                                int temp ;
                                if (pPCM->ucChannels == 1)
                                {
                                    for (i = 0; i < (lBlockSize >>2) ; i++)
                                    {
                                        temp = pPCM->pucEncodedData[4*i+3]<<24|pPCM->pucEncodedData[4*i+2]<<16
                                               |pPCM->pucEncodedData[4*i+1]<<8|pPCM->pucEncodedData[4*i];
                                        *(int *)pOut_24  = temp;      // L
                                        pOut_24 += 4;
                                        *(int *)pOut_24  = temp;      // R
                                        pOut_24 += 4;

                                    }
                                }
                                else      //channels == 2
                                {
                                    for (i = 0; i < (lBlockSize >>2) ; i++)
                                    {
                                        temp = pPCM->pucEncodedData[4*i+3]<<24|pPCM->pucEncodedData[4*i+2]<<16
                                               |pPCM->pucEncodedData[4*i+1]<<8|pPCM->pucEncodedData[4*i];
                                        *(int *)pOut_24  = temp;
                                        pOut_24 += 4;
                                    }
                                }
                            }

                            else  //here mean the quantity bits do not support.
                                return 0;
                            /* {
                                  char *pcm = (char*)&WavOutBuf[pPCM->CurDecodeIdx][SRC_Num_Forehead];
                                  int i;
                                   printf("out\n");
                                  for(i=0;i<16;i++)
                                 {
                                    printf("%02x ",*pcm++);
                                    printf("%02x ",*pcm++);
                                    pcm++;
                                    pcm++;
                                 }
                                  printf("\n\n");
                             }*/

                            pPCM->OutLength = pPCM->usSamplesPerBlock;
                            //--------------------------------------------

                            if (pPCM->uBitsPerSample == 16)
                            {
                                pPCM->ulTimePos += (lBlockSize >> pPCM->ucChannels);
                            }
                            else if (pPCM->uBitsPerSample == 8)
                            {
                                pPCM->ulTimePos += (lBlockSize >> (pPCM->ucChannels - 1));
                            }
                            else if (pPCM->uBitsPerSample == 24)
                            {
                                pPCM->ulTimePos += pPCM->usSamplesPerBlock;
                            }
                            else if (pPCM->uBitsPerSample == 32)
                            {
                                pPCM->ulTimePos += pPCM->usSamplesPerBlock;
                            }

                            //decoding is finished,if it had reached the end of file.
                            if (pPCM->usValid < lBlockSize)
                            {
                                return 0;
                            }



                            return 1;
                        }
                    default:
                        return 0; //error.
                }
            }

        case SUBFN_CODEC_SEEK:
            {
                tPCM *pPCM;
                unsigned long ulPos;

                pPCM = &pcm_s;

                if (ulParam1 > pPCM->ulTimeLength)
                {
                    ulParam1 = pPCM->ulTimeLength;
                }

                ulPos = (((ulParam1 / 1000) * pPCM->usSampleRate) / pPCM->usSamplesPerBlock) +
                        (((ulParam1 % 1000) * pPCM->usSampleRate) / (pPCM->usSamplesPerBlock * 1000));

                pPCM->ulTimePos = ulPos * pPCM->usSamplesPerBlock;

                msadpcm_seek_set(&sf,pPCM->usSamplesPerBlock * ulPos);

                if (pPCM->wFormatTag == 1)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock) * pPCM->usSamplesPerBlock; //(PCM Seek )
                }

                else //else if(pPCM->sWaveFormat.wFormatTag == 2)
                {
                    ulPos = pPCM->ulDataStart + (ulPos * pPCM->usBytesPerBlock); //(ADPCM Seek )
                }

                if (ulPos > pPCM->ulLength)
                    return 0;

                if (pPCM->wFormatTag == WAVE_FORMAT_ADPCM)
                {
                    RKFIO_FSeek( (int)ulPos , 0 , pRawFileCache );
                }
                else if (pPCM->wFormatTag == WAVE_FORMAT_IMA_ADPCM)
                {
                    RKFIO_FSeek( (int)(ulPos&~511), 0 , pRawFileCache );
                    pPCM->usValid = RKFIO_FRead( pPCM->pucEncodedData, 1024 , pRawFileCache );

                    if (pPCM->usValid < 1024)
                        return 0;
                }
                else if (pPCM->wFormatTag == WAVE_FORMAT_PCM)
                {
                    RKFIO_FSeek( (int)(ulPos), 0 , pRawFileCache );
                }


                pPCM->usOffset = (unsigned short)(ulPos & 511);

                pPCM->ulDataValid = pPCM->ulLength +  pPCM->ulDataStart - (ulPos & ~511);

                return(1);
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                tPCM *pPCM;
                unsigned long *bps;
                pPCM = &pcm_s;
                bps = (unsigned long *)ulParam1;
                *bps = pPCM->uBitsPerSample;
                if (*bps <= 16)
                {
                    *bps  = 16;
                }
                return 1;
            }
        case SUBFN_CODEC_CLOSE:
            {
                tPCM *pPCM;

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

        case SUBFN_CODEC_GETBITRATE:
        {
            *(int *)ulParam1 = gpMediaBlock.BitRate;
            return(1);
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

        case SUBFN_CODEC_GETLENGTH:
        {
            *(unsigned long *)ulParam1 = gpMediaBlock.TotalPlayTime;
            return(1);
        }

        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
        {
            *(int *)ulParam1 = gpMediaBlock.BitPerSample;
            return(1);
        }

        case SUBFN_CODEC_GETTIME:
        {
            *(unsigned long *)ulParam1 = gpMediaBlock.CurrentPlayTime;
            return(1);
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
                //DEBUG("wav get buffer...DecodeOver = %d",gpMediaBlock.DecodeOver);
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
                   // printf("myoutlength = %d\n",(gpMediaBlock.OutLength));
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
                                //  printf("myoutlength2 = %d\n",(gpMediaBlock.OutLength));
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
                                 // printf("myoutlength3 = %d\n",(gpMediaBlock.OutLength));
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

        case SUBFN_CODEC_SEEK:
        {
            //DEBUG("codec seek times = %d",ulParam1);
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
            return(1);
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
