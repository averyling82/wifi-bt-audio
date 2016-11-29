/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name：   audio_file_access.c
*
* Description:  Audio File Operation Interface
*
* History:      <author>          <time>        <version>
*             Vincent Hsiung    2009-01-08         1.0
*    desc:    ORG.
********************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "typedef.h"
#include "audio_main.h"
#include "audio_globals.h"
#include "audio_file_access.h"
#include "mailbox.h"
#include "Msg.h"
#ifdef SBC_DEC_INCLUDE2
#include "sbc.h"
#endif

#ifdef _RECORD_
#include "record_globals.h"
#endif


/*
*-------------------------------------------------------------------------------
*
*                           type define
*
*-------------------------------------------------------------------------------
*/
typedef unsigned int size_t;

//size_t   (*RKFIO_FOpen2)(uint8 * /*shortname*/, int32 /*DirClus*/, int32 /*Index*/, FS_TYPE /*FsType*/, uint8* /*Type*/) ;
size_t   (*RKFIO_FOpen)();
size_t   (*RKFIO_FRead)(void * /*buffer*/, size_t /*length*/,FILE *) ;
int      (*RKFIO_FSeek)(long int /*offset*/, int /*whence*/ , FILE * /*stream*/);
long int (*RKFIO_FTell)(FILE * /*stream*/);
size_t   (*RKFIO_FWrite)(void * /*buffer*/, size_t /*length*/,FILE * /*stream*/);
unsigned long (*RKFIO_FLength)(FILE *in /*stream*/);
int      (*RKFIO_FClose)(FILE * /*stream*/);
int (*RKFIO_FEof)(FILE *);
void (*RKFIO_BWrite)(char *DataBuf, size_t DataLen);

FILE *pRawFileCache,*pFlacFileHandleBake,*pAacFileHandleSize,*pAacFileHandleOffset, *pRecordFile;

/*
*-------------------------------------------------------------------------------
*
*                           AudioFile Buffer define
*
*-------------------------------------------------------------------------------
*/

unsigned long SRC_Num_Forehead;
extern  short   *gAmrOutbuf[2];
extern  short   *outbuf[2];
extern  short   *gWmaOutputPtr[2];
extern short     *gSbcOutputPtr[2];
extern short    *AACoutBuf[2];
extern short    *WavoutBuf[2];
extern short    *gAPEPingPangBuf[2];
extern unsigned short *g_FlacCodecBuffer[2];
extern short *OggPcmOut[2];
extern unsigned short      *WavEncInput;
extern unsigned short      *WavEncOutPut;
extern unsigned short      *Mp3EncInput;
extern unsigned short      *Mp3EncOutPut;
extern unsigned short      *AmrEncInput;
extern unsigned short      *AmrEncOutPut;

extern uint8 *gHAPEPingPangBuf[2];
extern uint8 *gAlacPingPangBuf[2];
extern uint8 *gFlacPingPangBuf[2];


#if defined(__arm__) && defined(__ARMCC_VERSION)
__align(4)
#endif
_ATTR_AUDIO_BSS_ uint8   AudioBuffer2[HIFI_AUDIO_BUF_SIZE]
#if defined(__arm__) && defined(__GNUC__)
__attribute__ ((aligned (4)))
#endif
;
_ATTR_AUDIO_BSS_ uint8  *AudioFileBuf2;
_ATTR_AUDIO_BSS_ char   *AudioCodecBuf2;
_ATTR_AUDIO_BSS_ uint32  CodecBufSize2;
_ATTR_AUDIO_BSS_ uint32  AudioFilePIPOBufSize2;     //分配给文件缓冲PIPO Buffer 的大小,总大小AudioFilePIPOBufSize x 2
//不同的文件，其大小不一样
_ATTR_AUDIO_BSS_ uint32  AudioFileBufBusy2;
_ATTR_AUDIO_BSS_ uint32  AudioFileBufSize2[2];
_ATTR_AUDIO_BSS_ uint32  AudioFileBufPos2;
_ATTR_AUDIO_BSS_ uint32  AudioFileRdBufID2;
_ATTR_AUDIO_BSS_ uint32  AudioFileWrBufID2;

extern uint32 gCmd;
extern uint32 gData;

extern int CurrentDecCodec2;

_ATTR_BB_SYS_DATA_
uint8 gCmdDone = 0;

_ATTR_BB_SYS_DATA_
uint8 gBufByPass = 0;

_ATTR_BB_SYS_DATA_
FILE_SEEK_OP_t gFileSeekParam;

_ATTR_BB_SYS_DATA_
FILE_READ_OP_t gFileReadParam;

_ATTR_BB_SYS_DATA_
FILE_WRITE_OP_t gFileWriteParam;

_ATTR_BB_SYS_DATA_
FLAC_SEEKFAST_OP_t gFlacSeekParam;


_ATTR_BB_SYS_DATA_
uint32 FileTotalSize[8];

_ATTR_BB_SYS_DATA_
uint32 CurFileOffset[8];

#ifdef _RECORD_
WAV_WRITE_BUFFER_STRUCT  WriteBuffer;
#endif

extern MediaBlock gMediaBlockInfo;

/*
--------------------------------------------------------------------------------
  Function name : File access interface
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                                    2009/02/20         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
size_t HifiFileOpen()
{
    while (!gCmdDone)
    {
        __WFI2();
    }

    gCmdDone = 0;
    return 0;
}

bool HifiFileEof(FILE *in)
{
    if (CurFileOffset[(uint32)in] == FileTotalSize[(uint32)in])
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}

uint32 HifiRKFLength(FILE * in)
{

#if 0
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_GET_LENGTH,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteB2AData((int)in, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gCmdDone)
    {
        __WFI2();
    }

    gCmdDone = 0;
    return gData;
#else
    return FileTotalSize[(uint32)in];
#endif
}

unsigned long HifiRKFTell(FILE * in)
{
#if 0
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_TELL,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteB2AData((int)in, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gCmdDone)
    {
        __WFI2();
    }

    gCmdDone = 0;
    return gData;
#else
    return CurFileOffset[(uint32)in];
#endif
}

uint8 HifiFileSeek(int32 offset, uint8 Whence, FILE * Handle)
{
    gFileSeekParam.offset = offset;
    gFileSeekParam.whence = Whence;
    gFileSeekParam.handle = (uint32)Handle;
    gCmdDone = 0;
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_SEEK,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteB2AData((uint32)&gFileSeekParam, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gCmdDone)
    {
        __WFI2();
    }

    CurFileOffset[(uint32)gFileSeekParam.handle] = gFileSeekParam.offset;

    gCmdDone = 0;
    return gData;
}

uint32 HifiFileRead(uint8 *pData, uint32 NumBytes, FILE * Handle)
{
    gFileReadParam.pData    = pData;
    gFileReadParam.NumBytes = NumBytes;
    gFileReadParam.handle   = (uint32)Handle;

    gCmdDone = 0;
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_READ,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteB2AData((uint32)&gFileReadParam, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    if(gBufByPass)
    {
        while (!gCmdDone)
        {
            __WFI2();
        }

        CurFileOffset[(uint32)gFileReadParam.handle] = gFileReadParam.NumBytes;

        gCmdDone = 0;

        return gData;   //return read data length
    }
    else
    {
        return 0;
    }
}

uint32 HifiFileWrite(uint8 *Buf, uint32 Size, FILE * Handle)
{
    gFileWriteParam.buf         = Buf;
    gFileWriteParam.size        = Size;
    gFileWriteParam.handle      = (uint32)Handle;
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_WRITE,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteB2AData((uint32)&gFileWriteParam, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    if(gBufByPass)
    {
        while (!gCmdDone)
        {
            __WFI2();
        }

        gCmdDone = 0;
        return gData;
    }
    else
    {
        return 0;
    }
}

uint8 HifiFileClose(FILE * Handle)
{
    return 0;

    #if 0
    gCmdDone = 0;
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_CLOSE,MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    MailBoxWriteB2AData((uint32)Handle, MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    while (!gCmdDone)
    {
        __WFI2();
    }

    gCmdDone = 0;
    return gData;
    #endif
}


void RKFileFuncInit2(void)
{
    gBufByPass = 1;

    RKFIO_FOpen   = HifiFileOpen;
    RKFIO_FLength = HifiRKFLength;
    RKFIO_FRead   = HifiFileRead;
    RKFIO_FWrite  = HifiFileWrite;
    RKFIO_FSeek   = HifiFileSeek;
    RKFIO_FTell   = HifiRKFTell;
    RKFIO_FClose  = HifiFileClose;
    RKFIO_FEof    = HifiFileEof;
}

#ifdef _RECORD_
void RecordCopyEncData2Buf2(UINT8 *DataBuf, uint32 encodeDataLen)
{
    UINT16  uTail, i;

    for (i = 0; i < encodeDataLen; i++)
    {
        uTail = WriteBuffer.uTail;
        WriteBuffer.Data[uTail] = DataBuf[i];
        WriteBuffer.uTail = (uTail + 1) % ENCODER_WRITE_BUFFER_LENGTH2;
        WriteBuffer.uCounter++;
    }

    if (WriteBuffer.uCounter >= ENCODER_WRITE2FLASH_LENGTH2 * 6)
    {
        SendMsg(MSG_AUDIO_ENCODE_CLEAR_BUFFER);
    }
}

BOOLEAN RecordWriteToFile2(void)
{
    UINT16  uHead;
    UINT8  *pWriteBuffer;
    UINT16  uWriteBytes, i;
    uint16  TempLength;



    TempLength = ENCODER_WRITE2FLASH_LENGTH2 * 6;


    if (WriteBuffer.uCounter >= TempLength)     //write 2 k data every time.
    {
        //bb_printf1("WriteBuffer.uCounter = %d, head = %d", WriteBuffer.uCounter, WriteBuffer.uHead);
        uHead           = WriteBuffer.uHead;
        pWriteBuffer    = &(WriteBuffer.Data[uHead]);

        if ((ENCODER_WRITE_BUFFER_LENGTH2 - uHead) > TempLength)
        {
            gBufByPass = 1;
            uWriteBytes = HifiFileWrite(pWriteBuffer, TempLength, pRecordFile);
            if(uWriteBytes != TempLength)
            {
                bb_printf1("record  .wav file write error 11 XXXXXXXXXXXX ");
                return FALSE;
            }
        }
        else
        {
            uint32 ret;
            uint32 temp = ENCODER_WRITE_BUFFER_LENGTH2 - uHead;

            gBufByPass = 1;
            uWriteBytes = HifiFileWrite(pWriteBuffer,temp, pRecordFile);

            if(uWriteBytes != temp)
            {
                bb_printf1("record  .wav file write error 22 XXXXXXXXXXXXX ");
                return FALSE;
            }

            pWriteBuffer    = &(WriteBuffer.Data[0]);
            temp            = TempLength - temp;

            gBufByPass = 1;
            ret = HifiFileWrite( pWriteBuffer,temp,pRecordFile);
            if(ret != temp)
            {
                bb_printf1("record  .wav file write error 44 XXXXXXXXXXXXX");
                return FALSE;
            }
            uWriteBytes     += ret;
        }

        if (TempLength == uWriteBytes)
        {
            //RecFileOffset += uWriteBytes;
            //RecordFileOffset += uWriteBytes;
        }
        else
        {
            bb_printf1("write file err   44 XXXXXXXXXXXXX");
            return FALSE;
        }

        WriteBuffer.uCounter  -= TempLength;
        WriteBuffer.uHead     = (uHead + TempLength) % ENCODER_WRITE_BUFFER_LENGTH2;

        //return TRUE;
    }
    return TRUE;
}

BOOLEAN RecordWriteFileTail2(void)
{
    UINT16  i, j;
    UINT16  uHead;
    UINT8  *pWriteBuffer;
    UINT16  uWriteBytes;
    uint16  TempLength;


    TempLength = ((WriteBuffer.uCounter) / 512) * 512;
    bb_printf1("TempLength = %d",TempLength);
    uHead           = WriteBuffer.uHead;
    bb_printf1("uHead = %d",uHead);
    pWriteBuffer    = &(WriteBuffer.Data[uHead]);

    if(TempLength != 0)
    {
        gBufByPass = 1;
        uWriteBytes = HifiFileWrite(pWriteBuffer,TempLength, pRecordFile);
        if(uWriteBytes != TempLength)
        {
            bb_printf1("record  .mp3 file Endinfo data write error");
            return RK_ERROR;
        }
    }
    return RK_SUCCESS;

}

void RecordBufferInit2()
{
    uint32 i;

    // wirte buf init...
    WriteBuffer.uHead     = 0;
    WriteBuffer.uTail     = 0;
    WriteBuffer.uCounter  = 0;
    WriteBuffer.Data = &AudioBuffer2[CodecBufSize2];

    for (i = 0; i < ENCODER_WRITE_BUFFER_LENGTH2; i++)
    {
        WriteBuffer.Data[i] = 0;
    }

    RKFIO_BWrite = RecordCopyEncData2Buf2;
}
#endif

/*
--------------------------------------------------------------------------------
  Function name : AudioFileBufferInit with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileBufferInit2(FILE * Handle)
{
    AudioFileBufPos2      = 0;
    AudioFileRdBufID2     = 0;
    AudioFileBufBusy2 = 0;

    gBufByPass = 1;

    AudioFileBufSize2[AudioFileRdBufID2]
    = HifiFileRead((uint8*)(&AudioFileBuf2[AudioFileRdBufID2 * AudioFilePIPOBufSize2]),
                   AudioFilePIPOBufSize2, Handle);

    AudioFileWrBufID2     = 1;
    AudioFileBufSize2[AudioFileWrBufID2]  = 0;

    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileInput with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint32 AudioFileInput2(FILE * Handle)
{
    {

       while(AudioFileBufBusy2 == 1)
       {
           __WFI2();
       }


        gBufByPass = 0;

        AudioFileBufBusy2 = 1;

        AudioFileBufSize2[AudioFileWrBufID2]
        = HifiFileRead((uint8*)(&AudioFileBuf2[AudioFileWrBufID2 * AudioFilePIPOBufSize2]),
                       AudioFilePIPOBufSize2, Handle);
    }
    return 0;
}



/*
--------------------------------------------------------------------------------
  Function name : AudioFileBufferSwitch with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileBufferSwitch2(FILE * Handle)
{
    //if the second buffer empty, fill buffer

    while(AudioFileBufBusy2 == 1)
    {
        __WFI2();
        bb_printf1("wait file stream...2");
    }

    AudioFileBufPos2 = 0;
    AudioFileBufSize2[AudioFileRdBufID2] = 0;
    AudioFileWrBufID2 = AudioFileRdBufID2;
    AudioFileRdBufID2 = 1 - AudioFileRdBufID2;

    if (AudioFileBufSize2[AudioFileRdBufID2] == 0)
    {
        gBufByPass = 1;

        bb_printf1("wait file stream...");

        AudioFileBufSize2[AudioFileRdBufID2]
        = HifiFileRead((uint8*)(&AudioFileBuf2[AudioFileRdBufID2 * AudioFilePIPOBufSize2]),
                       AudioFilePIPOBufSize2, Handle);
    }

    #if 1
    ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
    AudioFileInput2(pRawFileCache);
    #else
    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
    #endif
}


/*
--------------------------------------------------------------------------------
  Function name : AudioFileRead with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint32 AudioFileRead2(uint8 *pData, uint32 NumBytes, FILE * Handle)
{
    uint32 remain;
    uint32 retval;
    uint32 readcnt;
    uint32 temp;

    readcnt = 0;

    temp = 0;

    while (NumBytes)
    {


        /*
        if ((AudioFileBufSize[AudioFileRdBufID] < AudioFilePIPOBufSize2) && (AudioFileBufPos >= AudioFileBufSize[AudioFileRdBufID]))
        {
            break;
        }
        */


retry:
        remain = AudioFileBufSize2[AudioFileRdBufID2] - AudioFileBufPos2;

        //printf("\nremain = %d", remain);

        if (NumBytes <= remain)
        {
            memcpy(pData, &AudioFileBuf2[AudioFileRdBufID2 * AudioFilePIPOBufSize2 + AudioFileBufPos2], NumBytes);
            readcnt         += NumBytes;
            pData           += NumBytes;
            AudioFileBufPos2 += NumBytes;
            if (AudioFileBufPos2 >= AudioFileBufSize2[AudioFileRdBufID2])
            {
                AudioFileBufPos2 = 0;

                temp++;
                if(temp == 2)
                {
                    bb_printf1("want = %d, p = %d", NumBytes + readcnt, AudioFilePIPOBufSize2);
                }

                //bb_printf1("want = %d, p = %d", NumBytes, AudioFilePIPOBufSize2);

                AudioFileBufferSwitch2(Handle);
            }
            break;
        }
        else
        {
            //read buffer remain data, buffer empty
            memcpy(pData, &AudioFileBuf2[AudioFileRdBufID2 * AudioFilePIPOBufSize2 + AudioFileBufPos2], remain);
            NumBytes        -= remain;
            readcnt         += remain;
            pData           += remain;
            AudioFileBufPos2 += remain;
            if (AudioFileBufPos2 >= AudioFileBufSize2[AudioFileRdBufID2])
            {
                AudioFileBufPos2 = 0;

                temp++;
                if(temp == 2)
                {
                    bb_printf1("want11= %d, p = %d", NumBytes + readcnt, AudioFilePIPOBufSize2);
                }

                //bb_printf1("want12 = %d, p = %d", NumBytes, AudioFilePIPOBufSize2);

                AudioFileBufferSwitch2(Handle);
                if (AudioFileBufSize2[AudioFileRdBufID2] == 0)
                {
                    bb_printf1("file end...");
                    break;
                }
            }

            goto retry;
        }

    }



    return readcnt;

}
/*
--------------------------------------------------------------------------------
  Function name : AudioFileRead with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint32 AudioFileMhRead2(uint8 *pData, uint32 NumBytes, FILE * Handle)
{
    if(Handle == 0)
    {
        return AudioFileRead2(pData, NumBytes, Handle);
    }
    else
    {
       while(AudioFileBufBusy2 == 1)
       {
           __WFI2();
       }
        return HifiFileRead(pData, NumBytes, Handle);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileTell with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
unsigned long AudioFileTell2(FILE * in)
{
    uint32 offset;
    uint32 ret;
    offset = HifiRKFTell(in);
    ret = (offset - ((AudioFileBufSize2[0] + AudioFileBufSize2[1]) - AudioFileBufPos2));

    //bb_printf1("hifirkTell = %d",ret);
    return ret;
}

/*
--------------------------------------------------------------------------------
  Function name : AudioFileTell with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
unsigned long AudioFileMhTell2(FILE * in)
{
    if(in == 0)
    {
        return AudioFileTell2(in);
    }
    else
    {
        return HifiRKFTell(in);
    }
}


/*
--------------------------------------------------------------------------------
  Function name :  with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
bool AudioFileEof2(FILE *in)
{
    if (HifiRKFTell((HANDLE)in) == HifiRKFLength((HANDLE)in))
    {
        if (((AudioFileBufSize2[0] + AudioFileBufSize2[1]) - AudioFileBufPos2) == 0)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}


/*
--------------------------------------------------------------------------------
  Function name :  with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
bool AudioFileMhEof2(FILE *in)
{
    if(in == 0)
    {
        return AudioFileEof2(in);
    }
    else
    {
        return HifiFileEof(in);
    }

}


/*
--------------------------------------------------------------------------------
  Function name : AudioFileSeek with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint8 AudioFileSeek2(int32 offset, uint8 Whence, FILE * Handle)
{
    uint8 ret = RETURN_OK;
    uint32 FileBuffStart, FileBufcurPos, FileBufferEnd;
    uint32 temp;
    uint32 fileoffset;

    while(AudioFileBufBusy2 == 1)
    {
        __WFI2();
    }

    fileoffset = HifiRKFTell(Handle);

    if (Whence == SEEK_SET)
    {
        //printf("\n audio file seek = whence ---%d, %d",Whence, offset);
        FileBufferEnd = fileoffset;
        FileBuffStart = fileoffset - (AudioFileBufSize2[0] + AudioFileBufSize2[1]);

        //bb_printf1("FileBuffStart = %d FileBufferEnd = %d",FileBuffStart,FileBufferEnd);

        if ((FileBuffStart <= offset) && (offset < FileBufferEnd))
        {
            temp = offset - FileBuffStart;
            if (temp < AudioFileBufSize2[AudioFileRdBufID2])
            {
                AudioFileBufPos2 = temp;
            }
            else
            {
                temp -= AudioFileBufSize2[AudioFileRdBufID2];
                AudioFileBufPos2 = temp;

                AudioFileBufSize2[AudioFileRdBufID2] = 0;
                AudioFileWrBufID2 = AudioFileRdBufID2;
                AudioFileRdBufID2 = 1 - AudioFileRdBufID2;
                SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
            }
        }
        else
        {
            ret = HifiFileSeek((offset / 512) * 512, Whence, Handle);
            AudioFileBufferInit2(Handle);
            AudioFileBufPos2 += (offset % 512);

            //bb_printf1("filebufPos = %d",AudioFileBufPos2);
        }

    }
    else if (Whence == SEEK_CUR)
    {
        temp = AudioFileBufPos2 + offset;
        if ((0 <= temp) && (temp < (AudioFileBufSize2[0] + AudioFileBufSize2[1])))
        {
            if (offset > 0)
            {
                AudioFileBufPos2 += offset;
                if (AudioFileBufPos2 > AudioFileBufSize2[AudioFileRdBufID2])
                {
                    AudioFileBufPos2 -= AudioFileBufSize2[AudioFileRdBufID2];
                    AudioFileBufSize2[AudioFileRdBufID2] = 0;
                    AudioFileWrBufID2 = AudioFileRdBufID2;
                    AudioFileRdBufID2 = 1 - AudioFileRdBufID2;
                    SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
                }
            }
            else if (offset < 0)
            {
                AudioFileBufPos2 += offset;
            }
        }
        else
        {
            FileBuffStart = fileoffset - (AudioFileBufSize2[0] + AudioFileBufSize2[1]);

            FileBuffStart = FileBuffStart + temp;

            ret = HifiFileSeek((FileBuffStart / 512) * 512, 0, Handle);
            AudioFileBufferInit2(Handle);
            AudioFileBufPos2 += (FileBuffStart % 512);
        }
    }
    else
    {
        ret = HifiFileSeek((offset / 512) * 512, Whence, Handle);
        AudioFileBufferInit2(Handle);
        AudioFileBufPos2 += (offset % 512);
    }

    return ret;
}


/*
--------------------------------------------------------------------------------
  Function name : AudioFileSeek with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
uint8 AudioFileMhSeek2(int32 offset, uint8 Whence, FILE * Handle)
{
    if(Handle == 0)
    {
        return AudioFileSeek2(offset, Whence, Handle);
    }
    else
    {
        while(AudioFileBufBusy2 == 1)
       {
           __WFI2();
       }

        return HifiFileSeek(offset, Whence, Handle);
    }

}


/*
--------------------------------------------------------------------------------
  Function name : File access with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileFuncInit2(FILE * Handle, uint32 CodecBufSize)
{
    RKFIO_FOpen   = HifiFileOpen;
    RKFIO_FLength = HifiRKFLength;
    RKFIO_FRead   = AudioFileRead2;
    RKFIO_FSeek   = AudioFileSeek2;
    RKFIO_FTell   = AudioFileTell2;
    RKFIO_FClose  = HifiFileClose;
    RKFIO_FEof    = AudioFileEof2;

    AudioFileBuf2  = (uint8 *)AudioBuffer2;
    AudioFilePIPOBufSize2 = ((HIFI_AUDIO_BUF_SIZE - CodecBufSize) / 1024) * 1024;
    AudioFilePIPOBufSize2 = AudioFilePIPOBufSize2 >> 1;

    AudioFileBufferInit2(Handle);
}

/*
--------------------------------------------------------------------------------
  Function name : File access with Buffer
  Author        :
  Description   :
  Input         :
  Return        :
  History       : <author>         <time>         <version>
                       zyz            2013/11/07         Ver1.0
  desc          :   ORG
--------------------------------------------------------------------------------
*/
_ATTR_AUDIO_TEXT_
void AudioFileMhFuncInit2(FILE * Handle, uint32 CodecBufSize)
{
    RKFIO_FOpen   = HifiFileOpen;
    RKFIO_FLength = HifiRKFLength;
    RKFIO_FRead   = AudioFileMhRead2;
    RKFIO_FSeek   = AudioFileMhSeek2;
    RKFIO_FTell   = AudioFileMhTell2;
    RKFIO_FClose  = HifiFileClose;
    RKFIO_FEof    = AudioFileMhEof2;

    AudioFileBuf2  = (uint8 *)AudioBuffer2;
    AudioFilePIPOBufSize2 = ((HIFI_AUDIO_BUF_SIZE - CodecBufSize) / 1024) * 1024;
    AudioFilePIPOBufSize2 = AudioFilePIPOBufSize2 >> 1;
    //bb_printf1("2AudioFilePIPOBufSize2 = %d, %x CodecBufSize = %d", AudioFilePIPOBufSize2, &AudioFilePIPOBufSize2, CodecBufSize);
    AudioFileBufferInit2(Handle);
}


_ATTR_AUDIO_TEXT_
void AudioFileChangeBuf2(FILE * Handle, uint32 CodecBufSize)
{
    uint32 fileoffset;

    while(AudioFileBufBusy2 == 1)
    {
        __WFI2();
    }

    fileoffset = HifiRKFTell(Handle);

    AudioFilePIPOBufSize2 = ((HIFI_AUDIO_BUF_SIZE - CodecBufSize) / 1024) * 1024;
    AudioFilePIPOBufSize2 = AudioFilePIPOBufSize2 >> 1;

    if(gMediaBlockInfo.directplay == 0)
    {
        HifiFileSeek((fileoffset - AudioFileBufSize2[0] - AudioFileBufSize2[1]), SEEK_SET, Handle);


        gBufByPass = 1;
        AudioFileBufSize2[AudioFileRdBufID2]
        = HifiFileRead((uint8*)(&AudioFileBuf2[AudioFileRdBufID2 * AudioFilePIPOBufSize2]),
                       AudioFilePIPOBufSize2, Handle);

        AudioFileBufSize2[AudioFileWrBufID2]  = 0;

        rk_printf2("AudioFilePIPOBufSize2 = %d, AudioFileBufSize2[0] = %d, AudioFileBufSize2[1] = %d", AudioFilePIPOBufSize2, AudioFileBufSize2[0], AudioFileBufSize2[1]);

        SendMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
    }
    else
    {
        memcpy(&AudioFileBuf2[AudioFilePIPOBufSize2],&AudioFileBuf2[512], 512);
    }
}

uint32 AudioCodecGetBufferSize2(int codec, int samplerate)
{
    //Get Codecs output buffersize
#ifdef DC_FILTER

    if ((samplerate == FS_16KHz) ||

            (samplerate == FS_8000Hz) || (samplerate == FS_32KHz) )
    {

#ifdef MP3_DEC_INCLUDE2
        CodecBufSize2 = 1152 * 12+320*4;                   // 1152*4*2*1.5 = 13824
        SRC_Num_Forehead = 1152+320;
#endif

#ifdef XXX_ENC_INCLUDE2
        CodecBufSize2 = 1152 * 12+320*4;                   // 1152*12+320*4 = 15104
        SRC_Num_Forehead = 1152+320;
#endif


#ifdef SBC_DEC_INCLUDE2
        CodecBufSize2 = 2048 * 12+320*4;                   // 1152*4*2*1.5 = 13824
        SRC_Num_Forehead = 2048+320;
#endif

#ifdef XXX_DEC_INCLUDE2
        CodecBufSize2 = 2048 * 12+320*4;                   // 2048*4*2*1.5 = 24576
        SRC_Num_Forehead = 2048+320;
#endif

#ifdef WAV_DEC_INCLUDE2
        CodecBufSize2 = 2730 * 6+320*4;                    // 2730*4*2*1.5 = 32760 / 2
        SRC_Num_Forehead = 2730+320;
#endif

#ifdef WAV_ENC_INCLUDE2
        CodecBufSize2 = 2730 * 6+320*4;                    // 2730*4*2*1.5 = 32760 / 2
        SRC_Num_Forehead = 2730+320;
#endif

#ifdef AMR_DEC_INCLUDE2
       CodecBufSize2 = CodecBufSize2 + 1024*2;
       SRC_Num_Forehead = 0;
#endif

#ifdef AMR_ENC_INCLUDE2
       CodecBufSize2 = CodecBufSize2 + 1024*2;
       SRC_Num_Forehead = 0;
#endif

#ifdef  AAC_DEC_INCLUDE2
        CodecBufSize2 = 1024 * 12+320*4;                    // 1024*4*2*1.5 = 32760 / 2
        SRC_Num_Forehead = 1024+320;
#endif

#ifdef  APE_DEC_INCLUDE2
        CodecBufSize2 = 1152 * 12+320*4;                    // 1152*4*2*1.5 = 32760 / 2
        SRC_Num_Forehead = 1152+320;
#endif

#ifdef FLAC_DEC_INCLUDE2
        CodecBufSize2 = 2320 * 12+320*4;                    // 2320*4*2*1.5 = 32760 / 2
        SRC_Num_Forehead = 2320+320;
#endif

#ifdef OGG_DEC_INCLUDE2
        CodecBufSize2 = 1024 * 12+320*4;                    // 2320*4*2*1.5 = 32760 / 2
        SRC_Num_Forehead = 1024+320;
#endif

    }
    else
#endif //DC_FILTER
    {
#ifdef MP3_DEC_INCLUDE2
        CodecBufSize2 = 1152 * 16;                    // 1152*4*2 = 9216
#endif

#ifdef XXX_ENC_INCLUDE2
        CodecBufSize2 = 1152 * 8;                    // 1152*4*2 = 9216  //1152*32=36864
#endif

#ifdef SBC_DEC_INCLUDE2
        CodecBufSize2 = 2048 * 8;                    //1024*4*2 = 8192
#endif

#ifdef XXX_DEC_INCLUDE2
        CodecBufSize2 = 2048 * 8;                    // 2048*4*2 = 16384
#endif

#ifdef WAV_DEC_INCLUDE2
        #ifdef CODEC_24BIT
        CodecBufSize2 = 1024*32;                    // 2730*4*2 = 21840 / 2
        #else
        CodecBufSize2 = 2730 * 4;                    // 2730*4*2 = 21840 / 2
        #endif
#endif
#ifdef AMR_DEC_INCLUDE2
        CodecBufSize2 = 2048;
#endif

#ifdef WAV_ENC_INCLUDE2
        #ifdef CODEC_24BIT
        CodecBufSize2 = 2730 * 8;                    // 2730*4*2 = 21840 / 2
        #else
        CodecBufSize2 = 2730 * 4;                    // 2730*4*2 = 21840 / 2
        #endif
#endif

#ifdef AMR_ENC_INCLUDE2
        CodecBufSize2 = 1024*10;
#endif


#ifdef  AAC_DEC_INCLUDE2
        CodecBufSize2 = 1024* 8;                     // 1024*4*2 = 8192
#endif

#ifdef APE_DEC_INCLUDE2
        CodecBufSize2 = 1152 * 8;                    // 1152*4*2 = 9216
#endif

#ifdef FLAC_DEC_INCLUDE2
        CodecBufSize2 = 2320 * 12;          // 2320*4*2= 18560  该值/8应为四字节对齐
#endif

#ifdef OGG_DEC_INCLUDE2
        CodecBufSize2 = 1024*8;
#endif
#ifdef HIFI_APE_DECODE2
        CodecBufSize2 = 1024*48;
#endif
#ifdef HIFI_AlAC_DECODE2
        CodecBufSize2 = 1024*48;
#endif
#ifdef HIFI_FlAC_DECODE2
        CodecBufSize2 = 1024*48; // 4096*4*2*1.5  4608*4*2*1.5
#endif

#ifdef CODEC_24BIT
        CodecBufSize2 = (CodecBufSize2 )*2;
        SRC_Num_Forehead = 0;
#else
#ifdef MP3_DEC_INCLUDE2

             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4)*2;
             SRC_Num_Forehead = (64+128+320)*2;
             #else
             CodecBufSize2 = (CodecBufSize2 + 0)*2;
             SRC_Num_Forehead = 0;
             #endif

#endif

#ifdef XXX_ENC_INCLUDE2
             CodecBufSize2 = ((CodecBufSize2 + (64+128+320)* 4)*2)+2048;//(9216+(64+128+320)*4)*2=22528  //(36864+(64+128+320)*4)*2=77842
             SRC_Num_Forehead = (64+128+320)*2;
#endif


#ifdef WAV_DEC_INCLUDE2

             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4)*2;
             SRC_Num_Forehead = (64+128+320)*2;
             #else
             CodecBufSize2 = (CodecBufSize2 + 0)*2;
             SRC_Num_Forehead = 0;
             #endif

#endif

#ifdef WAV_ENC_INCLUDE2

             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4)*2;
             SRC_Num_Forehead = (64+128+320)*2;

#endif

#ifdef AMR_DEC_INCLUDE2
             CodecBufSize2 = CodecBufSize2 + 1024*2;
             SRC_Num_Forehead = 0;
#endif

#ifdef AMR_ENC_INCLUDE2
             CodecBufSize2 = CodecBufSize2 + 1024*2;
             SRC_Num_Forehead = 0;
#endif

#ifdef HIFI_APE_DECODE2

             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4)*2;
             SRC_Num_Forehead = (64+128+320)*2;
             #else
             CodecBufSize2 = (CodecBufSize2 + 0)*2;
             SRC_Num_Forehead = 0;
             #endif

#endif

#ifdef HIFI_ALAC_DECODE2

             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4)*2;
             SRC_Num_Forehead = (64+128+320)*2;
             #else
             CodecBufSize2 = (CodecBufSize2 + 0)*2;
             SRC_Num_Forehead = 0;
             #endif

#endif

#ifdef HIFI_FlAC_DECODE2

             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4)*2;
             SRC_Num_Forehead = (64+128+320)*2;
             #else
             CodecBufSize2 = (CodecBufSize2 + 0)*2;
             SRC_Num_Forehead = 0;
             #endif

#endif
#ifdef SBC_DEC_INCLUDE2
             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4);
             SRC_Num_Forehead = (64+128+320);
             #else
             CodecBufSize2 = (CodecBufSize2 + 0);
             SRC_Num_Forehead = 0;
             #endif
#endif

#ifdef XXX_DEC_INCLUDE2
             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4);
             SRC_Num_Forehead = (64+128+320);
             #else
             CodecBufSize2 = (CodecBufSize2 + 0);
             SRC_Num_Forehead = 0;
             #endif
#endif

#ifdef AAC_DEC_INCLUDE2
             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4);
             SRC_Num_Forehead = (64+128+320);
             #else
             CodecBufSize2 = (CodecBufSize2 + 0);
             SRC_Num_Forehead = 0;
             #endif
#endif

#ifdef FLAC_DEC_INCLUDE2
             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4);
             SRC_Num_Forehead = (64+128+320);
             #else
             CodecBufSize2 = (CodecBufSize2 + 0);
             SRC_Num_Forehead = 0;
             #endif
#endif

#ifdef APE_DEC_INCLUDE2
             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4);
             SRC_Num_Forehead = (64+128+320);
             #else
             CodecBufSize2 = (CodecBufSize2 + 0);
             SRC_Num_Forehead = 0;
             #endif
#endif

#ifdef OGG_DEC_INCLUDE2
             #ifdef DC_FILTER
             CodecBufSize2 = (CodecBufSize2 + (64+128+320)* 4);
             SRC_Num_Forehead = (64+128+320);
             #else
             CodecBufSize2 = (CodecBufSize2 + 0);
             SRC_Num_Forehead = 0;
             #endif
#endif

#endif

    }

    AudioCodecBuf2 = AudioBuffer2 + HIFI_AUDIO_BUF_SIZE - CodecBufSize2;

#ifdef MP3_DEC_INCLUDE2
        outbuf[0] = (short*)AudioCodecBuf2;
        outbuf[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif

#ifdef XXX_ENC_INCLUDE2
        Mp3EncInput = (short*)AudioCodecBuf2;
        Mp3EncOutPut = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif



#ifdef SBC_DEC_INCLUDE2

            gSbcOutputPtr[0] = (short*)AudioCodecBuf2;
            gSbcOutputPtr[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif


#ifdef XXX_DEC_INCLUDE2
        gWmaOutputPtr[0] = (short*)AudioCodecBuf2;
        gWmaOutputPtr[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif

#ifdef WAV_DEC_INCLUDE2
        WavoutBuf[0] = (short*)AudioCodecBuf2;
        WavoutBuf[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif

#ifdef WAV_ENC_INCLUDE2
        WavEncInput = (short*)AudioCodecBuf2;
        WavEncOutPut = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif

#ifdef AMR_DEC_INCLUDE2
        gAmrOutbuf[0] = (short*)AudioCodecBuf2;
        gAmrOutbuf[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif

#ifdef AMR_ENC_INCLUDE2
        AmrEncInput = (short*)AudioCodecBuf2;
        AmrEncOutPut = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);
#endif

#ifdef  AAC_DEC_INCLUDE2

        AACoutBuf[0] = (short*)AudioCodecBuf2;
        AACoutBuf[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif

#ifdef APE_DEC_INCLUDE2

        gAPEPingPangBuf[0] = (short*)AudioCodecBuf2;
        gAPEPingPangBuf[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif

#ifdef FLAC_DEC_INCLUDE2

        //extern unsigned char FlacOutputBuff[4608*4];;
        g_FlacCodecBuffer[0] = (short*)AudioCodecBuf2;
        g_FlacCodecBuffer[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif

#ifdef  OGG_DEC_INCLUDE2

        OggPcmOut[0] = (short*)AudioCodecBuf2;
        OggPcmOut[1] = (short*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif
#ifdef HIFI_APE_DECODE2

        gHAPEPingPangBuf[0] = (uint8*)AudioCodecBuf2;
        gHAPEPingPangBuf[1] = (uint8*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif
#ifdef HIFI_AlAC_DECODE2

        gAlacPingPangBuf[0] = (uint8*)AudioCodecBuf2;
        gAlacPingPangBuf[1] = (uint8*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif
#ifdef HIFI_FlAC_DECODE2

        gFlacPingPangBuf[0] = (uint8*)AudioCodecBuf2;
        gFlacPingPangBuf[1] = (uint8*)(&AudioCodecBuf2[CodecBufSize2>>1]);

#endif

    return CodecBufSize2;
}

extern void imdct36_handler();
extern void synth_handler();
void AudioIntAndDmaInit2(void)
{
    /*---------clear interrupt flag.------------*/
#ifdef MP3_DEC_INCLUDE2
    IntPendingClear2(INT_ID_IMDCT);
    IntPendingClear2(INT_ID_SYNTH);
#endif

#ifdef MP3_DEC_INCLUDE2
    IntRegister2(INT_ID_IMDCT, imdct36_handler);
    IntRegister2(INT_ID_SYNTH, synth_handler);
#endif


#ifdef MP3_DEC_INCLUDE2
    IntEnable2(INT_ID_IMDCT);
    IntEnable2(INT_ID_SYNTH);
#endif
}

void AudioIntAndDmaDeInit2(void)
{
    /*---------clear interrupt enable------------*/
#ifdef MP3_DEC_INCLUDE2
#if(MP3_EQ_WAIT_SYNTH)
    mp3_wait_synth();
#endif
    IntDisable2(INT_ID_IMDCT);
    IntDisable2(INT_ID_SYNTH);
#endif

    /*---------clear interrupt Pending---------*/
#ifdef MP3_DEC_INCLUDE2
    IntPendingClear2(INT_ID_IMDCT);
    IntPendingClear2(INT_ID_SYNTH);
#endif

    /*-----------interrupt callback auti-register--------------*/
#ifdef MP3_DEC_INCLUDE2
    IntUnregister2(INT_ID_IMDCT);
    IntUnregister2(INT_ID_SYNTH);
#endif

}

extern void AcceleratorHWInit(void);
extern void AcceleratorHWExit();
void AudioHWInit2(void)
{
#ifdef MP3_DEC_INCLUDE2
     AcceleratorHWInit();
#endif
}

void AudioHWDeInit2(void)
{
#ifdef MP3_DEC_INCLUDE2
    AcceleratorHWExit();
#endif
}

#ifdef FLAC_DEC_INCLUDE2
_ATTR_FLACDEC_TEXT_
int FLAC_FileSeekFast(int offset, int clus, FILE * in)
{
    gFlacSeekParam.offset = offset;
    gFlacSeekParam.clus   = clus;
    gFlacSeekParam.fp     = (int)in;

    gCmdDone = 0;

    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FLAC_SEEKFAST,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteB2AData((uint32)&gFlacSeekParam, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    while (!gCmdDone)
    {
        __WFI2();
    }

    gCmdDone = 0;
  //FileInfo[(int)in].Offset = offset;
  //FileInfo[(int)in].Clus   = clus;
    return 0;
}
_ATTR_FLACDEC_TEXT_
int FLAC_FileGetSeekInfo(int *pOffset, int *pClus, FILE * in)
{
    FLAC_SEEKFAST_OP_t* ptmep;

    gCmdDone = 0;
    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FLAC_GETSEEK_INFO,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    MailBoxWriteB2AData((int)in, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    while (!gCmdDone)
    {
        __WFI2();
    }

    gCmdDone = 0;
    ptmep = (FLAC_SEEKFAST_OP_t*)gData;

    *pOffset = ptmep->offset;
    *pClus   = ptmep->clus;

    //*pOffset = FileInfo[(int)in].Offset;
    //*pClus   = FileInfo[(int)in].Clus;
    return 0;
}
#endif
/*
********************************************************************************
*
*                         End of Audio_file_access.c
*
********************************************************************************
*/



