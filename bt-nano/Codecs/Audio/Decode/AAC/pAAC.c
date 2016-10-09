
/* Copyright (C) 2007 ROCK-CHIPS FUZHOU . All Rights Reserved. */
/*
File      : \Audio\APEDec
Desc      : decode for AAC format.

Author      : huangxd , Vincent Hisung
Date      : 2007-08-xx
Notes      :

$Log      :
* huangxd . create the file at 08.xx.2007
*
* vincent .      amendment at 08.xx.2007.
*
*/
/****************************************************************/

#include "RKOS.h"
#include "audio_main.h"


#ifdef AAC_DEC_INCLUDE

#include "audio_globals.h"
#include "audio_file_access.h"

#include "typedef.h"
#include "mailbox.h"
//#include "APEDec.h"

#pragma arm section code = "AacDecCode", rodata = "AacDecCode", rwdata = "AacDecData", zidata = "AacDecBss"


#define INREAD 2048
#define OUTWRITE 2048



#ifdef A_CORE_DECODE
#include "pAAC.h"

static char rdBuf[INREAD];
#if 0

static short outBuf[2][OUTWRITE];
#else
short *AACoutBuf[2];
extern  unsigned long SRC_Num_Forehead;
#endif
// static short outBuf[1024*3];
static HAACDecoder *hAACDecoder;

static FILE  *rawfile=NULL;
static  char *path=NULL;



static int left=0;

static char *p=rdBuf;
static char *pb=rdBuf;
static int size=sizeof(rdBuf);
static int select=0;
//for m4a_aac
static int m4a =0;
static int m4a_rate = 44100;
static int m4a_channels = 2;
static int m4a_bit= 16;
static int m4a_bitrate = 256000;
 int m4a_filelen = 0;
//#define TEST



#include "MovFile.h"

static M4a_Name  m4apath;
extern  MovFileinf gMovFile;

extern void MovGlobalVarInit(void);
extern int MovFileInit(FILE* aac_raw_file );
extern void MovFileClose(void);
extern int MovFileParsing(MOV_ST *videoFile);
extern int movBufSeek(MovBuf *movbuf, int seeksize, int mode);
extern int MovIF_AudioGetStream(char *buff, int size, int mode);
extern int MovIF_SynAudio2Video(unsigned int timems);
extern unsigned int MovIF_AudioGetCurrentTime(void);

#ifdef TEST
extern int xp,yp,num;

#else
#define testhl
#define showhl

#endif

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

#define ICT ((((*((volatile unsigned int *)(0x60000000+0x10))))-((*((volatile unsigned int *)(0x60000000+0x4)))))>>2)

unsigned long
AACDecFunction(unsigned long ulSubFn, unsigned long ulParam1,
               unsigned long ulParam2, unsigned long ulParam3)

{

#ifdef A_CORE_DECODE
    AAC_TIMEINFO *info;
    int curread;
    static int test = 0;
    int ret,err;
    int m4a_len;
    HDC hFileOper;

    info=GetInfo_AAC();

    //#define M4A_TEST

    switch (ulSubFn)
    {
            // Decode a frame of data.
        case SUBFN_CODEC_DECODE:
            {
                if (m4a == 0)
                {
                    ret= RKFIO_FRead(pb,size,rawfile);

                    left+=ret;
                    p=rdBuf;
                    if (!left)
                    {
                        return 0;
                    }

                    err = AACDecode(hAACDecoder, &p, &left,&AACoutBuf[select][SRC_Num_Forehead]);
                    if (err!=0)
                    {
                        return 0;
                    }
                    else
                    {
                        memmove(rdBuf,p,left);
                        size=sizeof(rdBuf)-left;
                        pb=rdBuf+left;
                    }
                }
                else
                {
                    //DEBUG("FileInfo[(uint32)pRawFileCache].Offset  = %d \n",FileInfo[(uint32)pRawFileCache].Offset );
                    m4a_len = MovIF_AudioGetStream(p, size, 1);

                    left = left+m4a_len;
                    pb = rdBuf;
                    if (!left)
                    {
                        return 0;
                    }

                    err = AACDecode(hAACDecoder, &pb, &left, &AACoutBuf[select][SRC_Num_Forehead]);
                    if (err!=0)
                    {
                        return 0;
                    }
                    else
                              {
                                          memmove(rdBuf,pb,left);
                                          size=sizeof(rdBuf)-left;
                                          p=rdBuf+left;
                              }

                }
                return 1;
            }

            // Prepare the codec to decode a file.
        case SUBFN_CODEC_OPEN_DEC:
            {
                unsigned char flag[20];
                int fl = 0;
                rawfile=(FILE*)pRawFileCache;

                //DEBUG("raw file = %d \n",(int)rawfile);

                RKFIO_FSeek(0, 0 ,rawfile);
                RKFIO_FRead(flag,20,rawfile);
                RKFIO_FSeek(0,0 ,rawfile);
                fl = CheckID3V2Tag(flag);

                if (fl == 0)
                {
                    m4a = GetFormat(flag);
                }
                else
                {
                    fl += 10;
                    m4a = 0;
                    RKFIO_FSeek(fl,0 ,rawfile);

                }
                // DEBUG("fl = %d \n",fl);

                if (m4a==0)
                {
                    FREQ_EnterModule(FREQ_MAX);
                    //if (InitTimeForADTS( rawfile,FileInfo[(uint32)pRawFileCache].FileSize-fl,fl) == -3)
                    {
                        FREQ_ExitModule(FREQ_MAX);
                        return ERROR;
                    }
                    FREQ_ExitModule(FREQ_MAX);
                    InitAACDataTable();
                    hAACDecoder = (HAACDecoder *)AACInitDecoder();
                    if (!hAACDecoder)
                    {
                        return -1;
                    }

                    RKFIO_FSeek(fl,0 ,rawfile);
                    memset(AACoutBuf,0,sizeof(AACoutBuf));
                    select=0;
                }
                else
                {
                    {  //for m4a

                        //m4a_filelen =FileInfo[(uint32)pRawFileCache].FileSize;
                        //add by helun
                        //FileInfo[(uint32)pRawFileCache].Offset =m4a_filelen-1;
                        //DEBUG("file size = %d \n",m4a_filelen);
                        FileDev_GetFileSize(FileSysHDC,  (uint32 *)&m4a_filelen);

                        MovGlobalVarInit();

                        if (MovFileInit(rawfile))
                        {
                            MovFileClose();
                            return -100;
                        }

                        if ((err = MovFileParsing(&gMovFile.movAudBuf)) != 0)
                        {
                            MovFileClose();

                            return err;
                        }
                        MovSTseek(&gMovFile.movAudBuf, 0, 0);
                    }

                    {//for aac
                        InitAACDataTable();
                        hAACDecoder = (HAACDecoder *)AACInitDecoder();
                        if (!hAACDecoder)
                        {
                            return -20;
                        }
                        pb=p=rdBuf;
                        size = sizeof(rdBuf);
                        left=0;
                        memset(AACoutBuf,0,sizeof(AACoutBuf));
                        select=0;

                    }
                }
                return 1;
            }

            // Seek to the specified time position.
        case SUBFN_CODEC_SEEK:
            {
                if (m4a == 0)
                {
                    AACFlushCodec(hAACDecoder);

                    SeekTime_AAC( ulParam1, hAACDecoder,rawfile);
                    left=0;
                    pb=p=rdBuf;
                    size=sizeof(rdBuf);
                    select=0;
                }
                else
                {
                    //m4a does not implent
                    MovIF_SynAudio2Video((unsigned int )ulParam1 );

                    left=0;
                    pb=p=rdBuf;
                    size=sizeof(rdBuf);
                    select=0;

                }

                return 1;

            }
        case SUBFN_CODEC_DEC_GETBUFFER:
            {
                *(unsigned long *)ulParam1 = (unsigned long)(&AACoutBuf[select][SRC_Num_Forehead]);

                *(unsigned long *)ulParam2 = (unsigned long) (OUTWRITE/2);

                select += 1;
                select = select%2;
                return(1);
            }

            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {
                //DEBUG("m4a= %d,close \n",m4a);
                if (m4a == 1)
                {
                    MovFileClose();
                }

                AACFlushCodec(hAACDecoder);
                AACFreeDecoder(hAACDecoder);
                m4a = 0;
                return 1;
            }

            // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                if (m4a == 0)
                    *(int *)ulParam1 = (long long) GetCurTime_AAC();
                else
                    *(int *)ulParam1 = (int ) MovIF_AudioGetCurrentTime();

                return 1;
            }

            // Return the sample rate at which this file is encoded.
        case SUBFN_CODEC_GETSAMPLERATE:
            {
#ifdef M4A_TEST
                *(int *)ulParam1 = 44100;
                return 1;
#endif
                if (m4a ==1)
                    *(int *)ulParam1 =gMovFile.sampleRate;
                else
                    *(int *)ulParam1 = info->rate;

                return 1;
            }

            // Return the number of channels in the file.
        case SUBFN_CODEC_GETCHANNELS:
            {
#ifdef M4A_TEST
                *(int *)ulParam1 = 2;
                return 1;
#endif
                if (m4a ==1)
                    *(int *)ulParam1 =gMovFile.channelCount;
                else
                    *(int *)ulParam1 = info->channels;

                return 1;
            }

            // Return the bitrate at which this file is encoded.
        case SUBFN_CODEC_GETBITRATE:
            {
#ifdef M4A_TEST
                *(int *)ulParam1 = 256000;
                return 1;
#endif
                extern int data_size;
                if (m4a ==1 )
                {
                    *(int *)ulParam1 =((long long )data_size*8*gMovFile.audioTimeScale)/(gMovFile.uint32AudioDuration);

                    //DEBUG("bitrate = %d \n",*(int *)ulParam1);
                }
                else
                    *(int *)ulParam1 =info->bitrate*1000;

                if((*(int *)ulParam1)%1000 >=500)
                {
                    *(int *)ulParam1 += 500;
                }
                return 1;
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
#ifdef M4A_TEST
                *(int *)ulParam1 = 222000;
                return 1;
#endif
                if (m4a == 1)
                    *(int *)ulParam1 =((long long)gMovFile.uint32AudioDuration*1000/gMovFile.audioTimeScale);
                else
                    *(int *)ulParam1 =info->totaltime;
                return 1;
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                if (m4a == 1)
                    *(int *)ulParam1 =16;
                else
                    *(int *)ulParam1 =16;
                return 1;
            }

        default:
            {
                //failure
                return(0);
            }
    }
#else
    switch (ulSubFn)
    {
            // Decode a frame of data.
        case SUBFN_CODEC_DECODE:
            {
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                return 1;
            }

            // Prepare the codec to decode a file.
        case SUBFN_CODEC_OPEN_DEC:
            {

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DEC_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                gpMediaBlock.needDecode = 0;
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);

                if( gpMediaBlock.DecodeErr )    //codec decode open error
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

            // Cleanup after the codec.
        case SUBFN_CODEC_CLOSE:
            {

                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                rkos_semaphore_take(osAudioDecodeOk, MAX_DELAY);
                return 1;
            }

            // Return the current position (in milliseconds) within the file.
        case SUBFN_CODEC_GETTIME:
            {
                *(int *)ulParam1 = gpMediaBlock.CurrentPlayTime;
                return 1;
            }

            // Return the sample rate at which this file is encoded.
        case SUBFN_CODEC_GETSAMPLERATE:
            {
                *(int *)ulParam1 = gpMediaBlock.SampleRate;
                return 1;
            }

            // Return the number of channels in the file.
        case SUBFN_CODEC_GETCHANNELS:
            {
                *(int *)ulParam1 = gpMediaBlock.Channel;
                return 1;
            }

            // Return the bitrate at which this file is encoded.
        case SUBFN_CODEC_GETBITRATE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitRate;
                return 1;
            }
        case SUBFN_CODEC_GET_BIT_PER_SAMPLE:
            {
                *(int *)ulParam1 = gpMediaBlock.BitPerSample;
                return(1);
            }

            // Return the length (in milliseconds) of the file.
        case SUBFN_CODEC_GETLENGTH:
            {
                *(int *)ulParam1 = gpMediaBlock.TotalPlayTime;
                return 1;
            }
        case SUBFN_CODEC_GET_FRAME_LEN:
            {
                *(int *)ulParam1 = gpMediaBlock.OutLength;
            }

        default:
            {
                //failure
                return(0);
            }
    }

#endif

    return 0;
}


#pragma arm section code

#endif

