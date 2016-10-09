/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File         : \Audio\APEDec
Desc      : APE decode

Author   : huangxd , Vincent Hisung
Date      : 2007-08-xx
Notes    :

$Log      :
* huangxd . create the file at 08.xx.2007
*
* vincent .      amendment at 08.xx.2007.
*
*/
/****************************************************************/
#include "RKOS.h"
#include "audio_main.h"

#ifdef APE_DEC_INCLUDE
#include "typedef.h"
#include "mailbox.h"

#include "audio_globals.h"
#include "audio_file_access.h"




#pragma arm section code = "ApeDecCode", rodata = "ApeDecCode", rwdata = "ApeDecData", zidata = "ApeDecBss"


#ifdef A_CORE_DECODE

#include "APEDec.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
** APE:global variable.
*/

tAPE gAPEStruct;           //decoding output struction of ape format.
void *pApeRawFileCache;       //ape file pointer.
#if 1
short *gAPEPingPangBuf[2];
extern  unsigned long SRC_Num_Forehead;

#else

short gAPEPingPangBuf[2][APE_BLOCKS_PER_DECODE*2]; // the ping-pong buffer of ape.
#endif
int gAPEPingPangIndex;

int ape_TitleIndex;
int ape_ArtistIndex;
int ape_AlbumIndex;

long OutLength;

static int ape_frame_cnt;
int ape_dec_error;
extern unsigned int coed_dma_channel;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
** APE:file write and read access.
*/

//warn:only the length of file is litter than 0x7FFFFFFF is suppoted.

long ApeGetFileSize(void *pHandle)
{
    return (RKFIO_FLength(pRawFileCache));
}

long ApeFread(void *pHandle, void *pBuf, unsigned long size)
{
    size_t cbRead;

    cbRead = RKFIO_FRead(pBuf, size, pRawFileCache);
    if (cbRead != size)
    {
        Ape_gDecodeErrorFlag = ERROR_DECOMPRESSING_FRAME;
    }
    return cbRead;
}

long ApeFseek(void *pHandle, long offset, unsigned long mode)
{
    if (RKFIO_FSeek(offset, mode, pRawFileCache) != 0)
    {
        //while(1); // for testing.
    }
    return 0;
}

long ApeFtell(void *pHandle)
{
    return (long)RKFIO_FTell(pRawFileCache);
}

static unsigned int CheckID3V2Tag(unsigned      char *pucBuffer)
{
    // The first three bytes of      the      tag      should be "ID3".
    if ((pucBuffer[0] !=  'I') ||  (pucBuffer[1] != 'D') || (pucBuffer[2] != '3'))
    {
        return(0);
    }

    // The next      byte should      be the value 3 (i.e. we      support      ID3v2.3.0).
    //if(pucBuffer[3]      != 3)
    if (pucBuffer[3]  <2  && pucBuffer[3]> 4)
    {
        return(0);
    }

    // The next      byte should      be less      than 0xff.
    if (pucBuffer[4]  == 0xff)
    {
        return(0);
    }

    // We don't      care about the next      byte.  The following four bytes      should be
    // less      than 0x80.
    if ((pucBuffer[6] >=  0x80) || (pucBuffer[7] >= 0x80)  ||
        (pucBuffer[8] >=  0x80) || (pucBuffer[9] >= 0x80))
    {
        return(0);
    }

    // Return the length of      the      ID3v2 tag.
    return((pucBuffer[6] << 21)   | (pucBuffer[7] << 14) |
           (pucBuffer[8] <<  7)   |  pucBuffer[9]);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
//#include "sysinclude.h"
#include "driverinclude.h"
extern MediaBlock gpMediaBlock;
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


#if 1
//****************************************************************************
//
// The codec plug-in entry point for the APE decoder.
//
//****************************************************************************
//the situation of function finish.
//SUBFN_CODEC_GETNAME  :        not support.
//SUBFN_CODEC_GETARTIST:        get songer name.
//SUBFN_CODEC_GETTITLE :        get track name.
//SUBFN_CODEC_GETBITRATE:       get bit rate.
//SUBFN_CODEC_GETSAMPLERATE:get sampling rate.
//SUBFN_CODEC_GETCHANNELS:      get channel numbers.
//SUBFN_CODEC_GETLENGTH :       get time length,[unit:ms]
//SUBFN_CODEC_GETTIME  :        get current playing time[unit:ms]
//SUBFN_CODEC_OPEN_DEC :        open decoder(initialization)
//SUBFN_CODEC_DECODE   :        decoding.
//SUBFN_CODEC_ENCODE   :        not support.
//SUBFN_CODEC_SEEK     :        location by time[unit:ms]
//SUBFN_CODEC_CLOSE    :        close decoder.

/******************************************************
Name:  APEDecFunction
Desc:  interface functions of ape decoder.
Param: ulIoctl child function number.
    ulParam1 1th parame of child function.
    ulParam2 2th parame of child function.
    ulParam3 3th parame of child function.
    ulParam4 4th parame of child function.

Return: 0-failure 1-success
Global: null
Note: null
Author: HXD
Log:
******************************************************/
unsigned long
APEDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3)

{
    //DEBUG("enter APEDecFunction\n" );
    //unsigned int WaitCounter = 0;
#ifdef A_CORE_DECODE
    DEBUG("#ifdef A_CORE_DECODE APEDecFunction \n" );
    switch (ulSubFn)
    {
            // Decode a frame of data.
        case SUBFN_CODEC_DECODE:
            {

                OutLength = 1152;
                {
                    // variable declares
                    tAPE *pAPE;
                    //short *psLeft, *psRight;
                    long lLength;
                    ape_int32 nBlocksDecoded = -1;
                    ape_int16 nBlocksDecRetVal = ERROR_SUCCESS;

                    pAPE = (tAPE *)&gAPEStruct;

                    if (ape_dec_error != 0)
                    {
                        return 0; //exclude bug:sd card dead machine.stop enter again after decode wrong happened.
                    }

                    if (Ape_gBlocksLeft <= 0)
                    {
                        ape_dec_error = 1;
                        ape_DEBUG("APE NORMAL END");
                        return (0);//mean that the decode is over, no data lefe for decode.
                    }

                    Ape_gDecodeErrorFlag = ERROR_SUCCESS;//no zero mean error happen, zero mean decode finish.
                    {
                        // decode data
                        ape_int32 nRetVal ;

                        Ape_pOutBufferLeft = (ape_uint16*)pAPE->psLeft;
                        Ape_pOutBufferRight = (ape_uint16*)pAPE->psRight;

                        nRetVal = Ape_pApeDecompress->GetData(Ape_pApeDecompress, APE_BLOCKS_PER_DECODE, &nBlocksDecoded);
                        if (nRetVal != ERROR_SUCCESS)
                        {
                            ape_dec_error = 1;
                            return 0;//(nBlocksDecRetVal = 0);//exit(ERROR_INVALID_CHECKSUM);ERROR_INVALID_CHECKSUM
                        }

                        //¸ÃÖ¡½â´íÎóÂë commented by hxd 20070703
                        if (Ape_gDecodeErrorFlag != ERROR_SUCCESS)
                        {
                            ape_dec_error = 1;
                            return 0;//(nBlocksDecRetVal = 0);//ERROR_DECOMPRESSING_FRAME
                        }
                    }

                    //outpur ---------------------------------
                    lLength = APE_BLOCKS_PER_DECODE;
                    OutLength = lLength;
                    pAPE->ulTimePos += nBlocksDecoded;

                    Ape_gBlocksLeft -= nBlocksDecoded;//update the number of no decode blocks.

                    ApeUnBitArrayFillBitArrayOutside();//remove old bitstream and fill new bitstream added by hxd 20070618


                    //assemble the PCM data of finish decode to LRLRLR....,then put to ping-pong buffer.
                    {
                        int i;
                        short *pbuf, *pleft, *pright;

                        pbuf   = (short *)&gAPEPingPangBuf[gAPEPingPangIndex][SRC_Num_Forehead];
                        pleft  = pAPE->psLeft;
                        pright = pAPE->psRight;
                        if (ape_frame_cnt < 5)
                        {
                            for (i = lLength; i > 0; i--)
                            {
                                *pbuf++ = 0;
                                *pbuf++ = 0;
                            }
                        }
                        else
                        {
                            for (i = lLength; i > 0; i--)
                            {
                                if (pAPE->ucChannels ==1)
                                {
                                    *pbuf++ = *pright;
                                    *pbuf++ = *pright++;
                                }
                                else
                                {
                                    *pbuf++ = *pright++;
                                    *pbuf++ = *pleft++;
                                }
                            }
                        }
                    }

                    ape_frame_cnt++;

                    //Success
                    return(1);
                }
            }

            // Prepare the codec to decode a file.
        case SUBFN_CODEC_OPEN_DEC:
            {
                tAPE* pAPE;
                unsigned char flag[20];
                int ID3_Length ;
                CAPEInfo* pAPEInfo = &Ape_gDecompressInfoCreate;
                ape_int32 nStartBlock = -1;
                ape_int32 nFinishBlock = -1;
                ape_int32 nErrorCode = ERROR_SUCCESS;
                WAVEFORMATEX wfeInput;

                ape_dec_error = 0;
                ape_frame_cnt = 0;

                pApeRawFileCache = (void *)1; //the value of opened file handle is 0 in our file system,buf the decoder think 0 is open failure,so set to 1 in here.

                gAPEPingPangIndex = 0;
                memset(&gAPEPingPangBuf[0][0], 0, 2*2*APE_BLOCKS_PER_DECODE*sizeof(short));

                RKFIO_FSeek(0,0 ,(HANDLE)pApeRawFileCache);
                RKFIO_FRead(flag,20,(HANDLE)pApeRawFileCache);
                RKFIO_FSeek(0,0 ,(HANDLE)pApeRawFileCache);
                ID3_Length = CheckID3V2Tag(flag);

                if (CheckAPE() != 0)
                {
                    return 0; //error happened in decode APE header.
                }
                if (ID3_Length == 0)
                {
                    ApeFseek(pApeRawFileCache, 0, 0);
                }
                else
                {
                    ApeFseek(pApeRawFileCache, (ID3_Length+10), 0);
                }

                // The first parameter is a pointer to the MP3 persistent data.
                pAPE = (tAPE *)&gAPEStruct;

                // create and return
                Ape_pApeDecompress = (IAPEDecompress *)ApeDecompressCoreCreate(pAPEInfo, nStartBlock, nFinishBlock, &nErrorCode);

                // get the input format
                Ape_pApeDecompress->GetInfo(APE_INFO_WAVEFORMATEX, (ape_int32) &wfeInput, 0);

                //----FOR DEBUG---can refer the below code when need to decode the id3 orr APETAG information.
                if (((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput, 0)))->GetHasAPETag(((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput, 0)))) == TRUE)
                {
                    //CAPETag* pTmp=(CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0));

                    //CAPETagField* pTmp2;
                    //ape_int32 i;
                    //fprintf(stderr, "APE Tag Bytes: %i\n",pTmp->GetTagBytes(pTmp));
                    //for (i=0;i<7;i++)
                    //for (i=0;i<pTmp->m_nFields;i++)
                    //{
                    //fprintf(stderr, "Has APE Tag: %s\n",pTmp->m_aryFields[i]);
                    //}
                }
                else
                {
                    //fprintf(stderr, "No APE Tag.\n");
                }

                if (((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput, 0)))->GetHasID3Tag(((CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput, 0)))) == TRUE)
                {
                    //CAPETag* pTmp=(CAPETag*)(Ape_pApeDecompress->GetInfo(APE_INFO_TAG, (ape_int32) &wfeInput,0));
                }
                else
                {
                    //fprintf(stderr, "No ID3 Tag.\n");
                }
                //--------------------------------------
                pAPE->ulTimePos = 0;
                pAPE->usSampleRate = pAPEInfo->m_APEFileInfo.nSampleRate;
                pAPE->ucChannels = pAPEInfo->m_APEFileInfo.nChannels;
                pAPE->ulBitRate = pAPEInfo->m_APEFileInfo.nAverageBitrate ;
                pAPE->ulTimeLength = (unsigned long)pAPEInfo->m_APEFileInfo.nLengthMS;//the unit is ms.
                pAPE->usSamplesPerBlock = pAPEInfo->m_APEFileInfo.nBlocksPerFrame;//the smallest unit is one frame to FFW and FFD to ape.
                pAPE->bitPerSamp  =  pAPEInfo->m_APEFileInfo.nBitsPerSample;
                Ape_gTotalBlocks = Ape_gBlocksLeft = Ape_pApeDecompress->GetInfo(APE_DECOMPRESS_TOTAL_BLOCKS, 0, 0);

                if (nErrorCode == 0)  //mod by vincent
                {
                    return 1;
                }
                else
                {
                    return 0;
                }
            }

            // Seek to the specified time position.
        case SUBFN_CODEC_SEEK:
            {
                tAPE *pAPE;
                unsigned long ulPos;

                ape_frame_cnt = 0;
                //clear buffer
                memset( &gAPEPingPangBuf[0][0] , 0 , sizeof(short)*2*APE_BLOCKS_PER_DECODE*2 );

                // The first parameter is a pointer to the APE persistent data.
                pAPE = (tAPE *)&gAPEStruct;
                if (ulParam1 > pAPE->ulTimeLength)//the unit is ms for ulParam1,the unit is also ms.
                {
                    ulParam1 = pAPE->ulTimeLength;
                }

                // Compute the number of frames that occur before the requested time position.
                ulPos = (((ulParam1 / 1000) * pAPE->usSampleRate) / pAPE->usSamplesPerBlock) +
                        (((ulParam1 % 1000) * pAPE->usSampleRate) / (pAPE->usSamplesPerBlock * 1000));//the frame number that is equal with current time.

                // Compute the time for the computed frame number.
                pAPE->ulTimePos = ulPos * pAPE->usSamplesPerBlock;
                Ape_gStartBlock = pAPE->ulTimePos;//FFW or FFD to sample number of current farme.
                Ape_gCurrentBlock = Ape_gStartBlock;
                Ape_gBlocksLeft = Ape_gTotalBlocks;
                Ape_gBlocksLeft -= Ape_gStartBlock;

                //the marks below means decode from current positon.it is similar to decode from first frame.
                Ape_gDecompressorInitial = FALSE;//restart all initialization when set Ape_gDecompressorInitial to false.
                Ape_gBeginDecodeFrameFlag = FALSE;//get the total blocks number afresh.

                // Compute the file position based on the actual seek time.
                return(1);
            }

            // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                // additional code needed
                unsigned long *pulTime;
                tAPE *pAPE;

                // The first parameter is a pointer to the APE persistent data.
                pAPE = (tAPE *)&gAPEStruct;

                // The second parameter is a pointer for the number of seconds.
                pulTime = (unsigned long *)ulParam1;

                if (pAPE->usSampleRate)
                {
                    // Determine the time based on the sample rate.
                    *pulTime = ((pAPE->ulTimePos / pAPE->usSampleRate) * 1000) +
                               (((pAPE->ulTimePos % pAPE->usSampleRate) * 1000) /
                                pAPE->usSampleRate);
                }

                // Success.
                return(1);
            }

            // Return the sample rate at which this file is encoded.
        case SUBFN_CODEC_GETSAMPLERATE:
            {
                unsigned long *pulSampleRate;
                tAPE *pAPE;

                // The first parameter is a pointer to the MP3 persistent data.
                pAPE = (tAPE *)&gAPEStruct;

                pulSampleRate = (unsigned long *)ulParam1;

                // Return the sample rate of the file.
                *pulSampleRate = pAPE->usSampleRate;

                // Success.
                return(1);
            }

            // Return the number of channels in the file.
        case SUBFN_CODEC_GETCHANNELS:
            {
                unsigned long *pulChannels;
                tAPE *pAPE;

                // The first parameter is a pointer to the MP3 persistent data.
                pAPE = (tAPE *)&gAPEStruct;

                pulChannels = (unsigned long *)ulParam1;

                // Return the number of channels in the file.
                *pulChannels = pAPE->ucChannels;

                // Success.
                return(1);
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                unsigned long *pulChannels;
                tAPE *pAPE;
                pAPE = (tAPE *)&gAPEStruct;
                pulChannels = (unsigned long *)ulParam1;
                *pulChannels =pAPE->bitPerSamp;
                return(1);
            }

            // Return the bitrate at which this file is encoded.
        case SUBFN_CODEC_GETBITRATE:
            {
                unsigned long *pulBitRate;
                tAPE *pAPE;

                // The first parameter is a pointer to the MP3 persistent data.
                pAPE = (tAPE *)&gAPEStruct;

                pulBitRate = (unsigned long *)ulParam1;

                // Return the number of channels in the file.
                *pulBitRate = pAPE->ulBitRate+500;

                // Success.
                return(1);
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                unsigned long *pulLength;
                tAPE *pAPE;

                // The first parameter is a pointer to the APE persistent data.
                pAPE = (tAPE *)&gAPEStruct;

                pulLength = (unsigned long *)ulParam1;

                // Return the length of the file.
                *pulLength = pAPE->ulTimeLength;//unit is ms.(millisecond)

                // Success.
                return(1);
            }

        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long) &gAPEPingPangBuf[gAPEPingPangIndex][SRC_Num_Forehead];
                *(unsigned long *)ulParam2 = (unsigned long) APE_BLOCKS_PER_DECODE;

                gAPEPingPangIndex ^= 1;;

                return(1);
            }

            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
                tAPE *pAPE;
                pAPE = (tAPE *) &gAPEStruct;

                pAPE->status = ERROR_SUCCESS;

                if (pAPE->status == ERROR_SUCCESS)
                {
                    return(1);
                }
                else
                {
                    return(0);
                }
            }
        default:
            {
                // Return a failure.
                return(0);
            }
    }
#else
    //DEBUG("#else APEDecFunction \n" );

    switch (ulSubFn)
    {
            // Decode a frame of data.
        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return(1);
            }

            // Prepare the codec to decode a file.
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

            // Seek to the specified time position.
        case SUBFN_CODEC_SEEK:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_SEEK, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                return(1);
            }

            // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(unsigned long *)ulParam1 = gpMediaBlock.CurrentPlayTime;
                // Success.
                return(1);
            }

            // Return the sample rate at which this file is encoded.
        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = gpMediaBlock.SampleRate;
                // Success.
                return(1);
            }

            // Return the number of channels in the file.
        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = gpMediaBlock.Channel;
                // Success.
                return(1);
            }

            // Return the bitrate at which this file is encoded.
        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitRate;
                // Success.
                return(1);
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = gpMediaBlock.TotalPlayTime;
                // Success.
                return(1);
            }

         case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitPerSample;
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

            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                return(1);
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

#endif

#pragma arm section code

#endif

