/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*File    : APP.C
* By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            DESCRIPTION
*  RK_NANO_OS is a system designed specifically for real-time embedded SOC operating system ,before using
*RK_NANO_OS sure you read the user's manual
*  The TASK NAME TABLE:
*
*
*  The DEVICE NAME TABLE:
*  "UartDevice",              Uart Serial communication devices
*  "ADCDevice",               The analog signal is converted to a digital signal device
*  "KeyDevice",               Key driver device
*
*
*
*
*
*
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "BspConfig.h"
#include "RKOS.h"
#include "stdio.h"
#include "mailbox.h"
#include "audio_globals.h"
#include "audio_main.h"
#include "audio_file_access.h"
#include "Msg.h"
#include "record_globals.h"

/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        Variable Define
*********************************************************************************************************
*/
_ATTR_BB_SYS_DATA_
uint32 gCmd = 0;
_ATTR_BB_SYS_DATA_
uint32 gData = 0;
_ATTR_BB_SYS_DATA_
uint8  gDecStatus = 0;

_ATTR_BB_SYS_DATA_
uint8  gFileOpStatus = 0;

_ATTR_BB_SYS_DATA_
uint32_t airplay_flag = 0;

extern uint8 gCmdDone;
extern uint32  CodecBufSize2;
extern uint16  gPCMDataWidth;

typedef struct _BBCore_Pcb_
{
    uint32 audio_decode_status;
    uint32 audio_decode_param;

} BbCore_PCB;

enum AUDIO_OPT_STATUS
{
    //audio decode
    AUDIO_IDLE = 0,
    AUDIO_DECODE_OPEN,
    AUDIO_DECODE_DECODE,
    AUDIO_DECODE_SEEK,
    AUDIO_DECODE_GETBUFFER,
    AUDIO_DECODE_GETTIME,
    AUDIO_DECODE_CLOSE,

    //audio file operate
    AUDIO_DECODE_FILE_OPEN,
    AUDIO_DECODE_FILE_CLOSE,
    AUDIO_ENCODE_FILE_CREATE,
    AUDIO_ENCODE_FILE_CLOSE,

    //audio encord cmd
    AUDIO_ENCODE_OPEN,
    AUDIO_ENCORD_ENCODE,
    AUDIO_ENCORD_CLOSE,
};

enum FILE_OPT_STATUS
{
    FILE_IDLE = 0,
    FILE_SEEK,
    FILE_READ,
    FILE_TELL,
    FILE_GETLENGTH,
    FILE_SEEK_CMPL,
    FILE_READ_CMPL,
    FILE_WRITE_CMPL,
    FILE_TELL_CMPL,
    FILE_GET_LEN_CMPL,
};

static BbCore_PCB pcb;
static MediaBlock gMediaBlockInfo;

static FILE_HANDLE_t * gFileHandle;
static RecFileHdl_t  * gRecFileHdl;
static RecordBlock     gRecordBlockInfo;

/*
*********************************************************************************************************
*                                      extern varible
*********************************************************************************************************
*/
extern uint32 FileTotalSize[8];
extern uint32 CurFileOffset[8];

extern uint32  AudioFileBufBusy2;
extern uint32  AudioFileBufSize2[2];
extern uint32  AudioFileWrBufID2;

extern FILE_READ_OP_t gFileReadParam;
extern uint8 gBufByPass;

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/
#ifdef _LOG_DEBUG_
//this fun just for debuging
static uint8* Cmd2String2(uint32 cmd)
{
    uint8* pstr;

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL:
            pstr = "FILE OPEN CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL:
            pstr = "FILE SEEK CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ_CMPL:
            pstr = "FILE READ CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL:
            pstr = "FILE WRITE CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL_CMPL:
            pstr = "FILE TELL CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL:
            pstr = "FILE GET_LEN CMPL";
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL:
            pstr = "FILE CLOSE CMPL";
            break;

        default:
            pstr = "NOT FOUND CMD";
            break;
    }

    return pstr;
}

void dumpMemoryByte(uint8* addr, uint32 size)
{
    uint32 index = 0;
    uint8 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            bb_printf1("\r\n");

        if (index % 16 == 0 && index != 0)
            bb_printf1("\n");

        bb_printf1("%02x", *ptemp);

        ptemp++;
    }
}

void dumpMemoryShort(uint16* addr, uint32 size)
{
    uint32 index = 0;
    uint8 data[2];
    uint16 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            bb_printf1("\r\n");

        if (index % 8 == 0 && index != 0)
            bb_printf1("\n");

        data[1] = *ptemp >> 8 & 0xff;
        data[0] = *ptemp & 0xff;
        bb_printf1("%02x ", data[1] );
        bb_printf1("%02x ", data[0] );
        ptemp++;
    }
}

void dumpMemoryInt(uint32* addr, uint32 size)
{
    uint32 index = 0, i;
    uint8 data[4];
    uint32 *ptemp;
    ptemp = addr;

    for (index = 0; index < size; index++)
    {
        if (index == 0)
            bb_printf1("\r\n");

        if (index % 4 == 0 && index != 0)
            bb_printf1("\n");

        data[3] = *ptemp >> 24 & 0xff;
        data[2] = *ptemp >> 16 & 0xff;
        data[1] = *ptemp >> 8  & 0xff;
        data[0] = *ptemp & 0xff;

        for (i = 4; i > 0; i--)
        {
            bb_printf1("%02x ", data[i - 1] );
        }

        ptemp++;
    }
}

#endif

__irq  void MailBoxDecService()
{
    uint32 cmd;
    uint32 data;
    MailBoxClearA2BInt(MAILBOX_ID_0, MAILBOX_INT_1);
    cmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_1);
    data = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_1);

    switch (cmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_OPEN:
            gFileHandle = (FILE_HANDLE_t *)data;
            pRawFileCache = (FILE *)gFileHandle->handle1;
            FileTotalSize[(uint32)pRawFileCache] = gFileHandle->filesize;
            CurFileOffset[(uint32)pRawFileCache] = gFileHandle->curfileoffset[0];
            CurrentDecCodec2 = gFileHandle->codecType;
            if(CurrentDecCodec2 != 0xff)
                CurrentDecCodec2 = 0;

#ifdef AAC_DEC_INCLUDE2
            pAacFileHandleOffset = (FILE *)gFileHandle->handle2;
            CurFileOffset[(uint32)pAacFileHandleOffset] = gFileHandle->curfileoffset[1];
            FileTotalSize[(uint32)pAacFileHandleOffset] = gFileHandle->filesize;
            pAacFileHandleSize = (FILE *)gFileHandle->handle3;
            CurFileOffset[(uint32)pAacFileHandleSize] = gFileHandle->curfileoffset[2];
            FileTotalSize[(uint32)pAacFileHandleSize] = gFileHandle->filesize;
#endif
#ifdef HIFI_AlAC_DECODE2
            pAacFileHandleOffset = (FILE *)gFileHandle->handle2;
            CurFileOffset[(uint32)pAacFileHandleOffset] = gFileHandle->curfileoffset[1];
            FileTotalSize[(uint32)pAacFileHandleOffset] = gFileHandle->filesize;
            pAacFileHandleSize = (FILE *)gFileHandle->handle3;
            CurFileOffset[(uint32)pAacFileHandleSize] = gFileHandle->curfileoffset[2];
            FileTotalSize[(uint32)pAacFileHandleSize] = gFileHandle->filesize;
#endif
#ifdef FLAC_DEC_INCLUDE2
            pFlacFileHandleBake = (FILE *)gFileHandle->handle2;
            CurFileOffset[(uint32)pFlacFileHandleBake] = gFileHandle->curfileoffset[1];
            FileTotalSize[(uint32)pFlacFileHandleBake] = gFileHandle->filesize;
#endif
            pcb.audio_decode_status = AUDIO_DECODE_FILE_OPEN;
            break;

        case MEDIA_MSGBOX_CMD_FILE_CLOSE:
            pcb.audio_decode_status = AUDIO_DECODE_FILE_CLOSE;
            break;

        case MEDIA_MSGBOX_CMD_REC_FILE_CLOSE:
            pcb.audio_decode_status = AUDIO_ENCODE_FILE_CLOSE;
            break;

        case MEDIA_MSGBOX_CMD_DEC_OPEN:
            pcb.audio_decode_status = AUDIO_DECODE_OPEN;
            //bb_printf1("STA = [DEC OPEN]");
            break;

        case MEDIA_MSGBOX_CMD_DECODE:
            pcb.audio_decode_status = AUDIO_DECODE_DECODE;
            //bb_printf1("STA = [DECODE]");
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETBUFFER:
            pcb.audio_decode_status = AUDIO_DECODE_GETBUFFER;
            //bb_printf1("STA = [DEC GET_BUFFER]");
            break;

        case MEDIA_MSGBOX_CMD_DECODE_GETTIME:
            pcb.audio_decode_status = AUDIO_DECODE_GETTIME;
            //bb_printf1("STA = [DEC GET_TIME]");
            break;

        case MEDIA_MSGBOX_CMD_DECODE_SEEK:
            pcb.audio_decode_status = AUDIO_DECODE_SEEK;
            pcb.audio_decode_param  = data;
            //bb_printf1("STA = [DECODE SEEK] data = %d",data);
            break;

        case MEDIA_MSGBOX_CMD_DECODE_CLOSE:
            pcb.audio_decode_status = AUDIO_DECODE_CLOSE;
            pcb.audio_decode_param  = data;
            //bb_printf1("STA = [DECODE CLOSE]");
            break;

//------------------------------------------------------------------------------
        case MEDIA_MSGBOX_CMD_ENCODE_OPEN:
            pcb.audio_decode_status = AUDIO_ENCODE_OPEN;
            pcb.audio_decode_param  = data;
            //bb_printf1("recive MEDIA_MSGBOX_CMD_ENCODE_INIT");
            break;

     //record file close
        case MEDIA_MSGBOX_CMD_FILE_CREATE:
            gRecFileHdl   = (RecFileHdl_t *)data;
            pRecordFile   = (FILE *)gRecFileHdl->fileHandle;
            CurFileOffset[(uint32)pRecordFile] = gRecFileHdl->fileOffset;
            FileTotalSize[(uint32)pRecordFile] = gRecFileHdl->filesize;
            CurrentEncCodec2 = gRecFileHdl->codecType;
            if(CurrentEncCodec2 != 0xff)
                CurrentEncCodec2 = 0;

            pcb.audio_decode_status = AUDIO_ENCODE_FILE_CREATE;
            break;

        case MEDIA_MSGBOX_CMD_ENCODE:
            pcb.audio_decode_status = AUDIO_ENCORD_ENCODE;
            pcb.audio_decode_param  = data;
            break;

        default:
            break;
    }
}

void RegHifiDecodeServer()
{
    IntRegister2(INT_ID_MAILBOX1 , (void*)MailBoxDecService);
    IntPendingClear2(INT_ID_MAILBOX1);
    IntEnable2(INT_ID_MAILBOX1);
    MailBoxEnableA2BInt(MAILBOX_ID_0, MAILBOX_INT_1);
}


__irq void MailBoxFileService()
{
    MailBoxClearA2BInt(MAILBOX_ID_0, MAILBOX_INT_2);

    gCmd = MailBoxReadA2BCmd(MAILBOX_ID_0, MAILBOX_CHANNEL_2);
    gData = MailBoxReadA2BData(MAILBOX_ID_0, MAILBOX_CHANNEL_2);

    //bb_printf1("A 2 B cmd is [%s]",Cmd2String2(gCmd));
    //bb_printf1("A 2 B data is 0x%08x",gData);

    switch (gCmd)
    {
        case MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL:
            gFileOpStatus = FILE_SEEK_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_READ_CMPL:
            gFileOpStatus = FILE_READ_CMPL;

            if (gBufByPass == 0)
            {
                CurFileOffset[gFileReadParam.handle] = gFileReadParam.NumBytes;
                AudioFileBufSize2[AudioFileWrBufID2] = gData;
                AudioFileBufBusy2 = 0;
                gBufByPass = 1;
            }

            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL:
            gFileOpStatus = FILE_WRITE_CMPL;
            gCmdDone = 1;

            //bb_printf1("recive MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL");
            break;

        case MEDIA_MSGBOX_CMD_FILE_TELL_CMPL:
            gFileOpStatus = FILE_TELL_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL:
            gFileOpStatus = FILE_GET_LEN_CMPL;
            gCmdDone = 1;
            break;

        case MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_CMPL:
        case MEDIA_MSGBOX_CMD_FLAC_SEEKFAST_INFO_CMPL:
            gCmdDone = 1;
            break;

        default:
            return;
    }
}

void RegHifiFileServer()
{
    bb_printf1("HifiFileInit ... ok");
    gCmdDone   = 0;
    IntRegister2(INT_ID_MAILBOX2 , (void*)MailBoxFileService);
    IntPendingClear2(INT_ID_MAILBOX2);
    IntEnable2(INT_ID_MAILBOX2);
    MailBoxEnableA2BInt(MAILBOX_ID_0, MAILBOX_INT_2);
}

/*
*********************************************************************************************************
*                                              Main(void)
*
* Description:  This Function is the first function.
*
* Argument(s) : none
*
* Return(s)   : int
*
* Note(s)     : none.
*********************************************************************************************************
*/



/*
--------------------------------------------------------------------------------
  Function name :
  Author        : ZHengYongzhi
  Description   : 模块信息表

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/

extern uint32 Image$$BB_SYS_DATA$$ZI$$Base;
extern uint32 Image$$BB_SYS_DATA$$ZI$$Length;

void ScatterLoader2(void)
{
    uint32 i, len;
    uint8  *pDest;
    //清除Bss段
    pDest = (uint8*)((uint32)(&Image$$BB_SYS_DATA$$ZI$$Base));
    len = (uint32)((uint32)(&Image$$BB_SYS_DATA$$ZI$$Length));

    for (i = 0; i < len; i++)
    {
        *pDest++ = 0;
    }
}

int Main2(void)
{
    uint outptr;
    uint OutLength;
    ScatterLoader2();
#ifdef BB_SYS_JTAG
    MailBoxWriteB2ACmd(MSGBOX_CMD_SYSTEM_START_OK, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
#endif
    BSP_Init2();
    RegHifiDecodeServer();
    RegHifiFileServer();
    ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
    MailBoxWriteB2ACmd(MSGBOX_CMD_SYSTEM_START_OK, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    MemSet2(&gMediaBlockInfo,  0, sizeof(gMediaBlockInfo));
    MemSet2(&gRecordBlockInfo, 0, sizeof(gRecordBlockInfo));
    //PWM4 (for test)
    //Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);
    //Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0,GPIO_OUT);
    //Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin0,GPIO_LOW);

    while (1)
    {
        extern uint32 MainStack2[];

        if (MainStack2[0] != 0x55aa55aa)
        {
            bb_printf1("b core stack over flow");

            while (1)
            {
                __WFI2();
            }
        }

        //process audio decode
        switch (pcb.audio_decode_status)
        {
            case AUDIO_IDLE:
#if 1
                IntMasterDisable2();

                if ((pcb.audio_decode_status == AUDIO_IDLE) && (gFileOpStatus == FILE_IDLE))
                {
                    __WFI2();
                }

                IntMasterEnable2();
#endif
                break;

            case AUDIO_DECODE_OPEN:
            {
                pcb.audio_decode_status = AUDIO_IDLE;

                if (1 != CodecOpenDec2())
                {
                    bb_printf1("###AUDIO_DECODE_OPEN error!###");
                    gMediaBlockInfo.DecodeErr = 1;
                    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteB2AData((UINT32)&gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    break;
                }

                {
                    CodecGetSampleRate2(&gMediaBlockInfo.SampleRate);
                    CodecGetChannels2(&gMediaBlockInfo.Channel);
                    CodecGetDecBitreate2(&gMediaBlockInfo.BitRate);
                    //CodecGetTime2(&gMediaBlockInfo.CurrentPlayTime);
                    CodecGetLength2(&gMediaBlockInfo.TotalPlayTime);
                    CodecGetBitPerSample2(&gMediaBlockInfo.BitPerSample);\

#ifdef  HIFI_AlAC_DECODE2
                    if( airplay_flag == 1)
                    {
                        gMediaBlockInfo.TotalPlayTime = (360000000 - 3600000);
                    }
#endif

#ifdef MP3_DEC_INCLUDE2
                    mp3_wait_synth();
#endif
                    //bb_printf1("decode open ok");
                    AudioCodecGetBufferSize2(CurrentDecCodec2, gMediaBlockInfo.SampleRate);
                    //AudioCodecGetBufferSize2(CurrentDecCodec2,gMediaBlockInfo.SampleRate);
#ifdef MP3_DEC_INCLUDE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef XXX_DEC_INCLUDE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef AAC_DEC_INCLUDE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef WAV_DEC_INCLUDE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef AMR_DEC_INCLUDE2
                   AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef APE_DEC_INCLUDE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef FLAC_DEC_INCLUDE2

#endif

#ifdef OGG_DEC_INCLUDE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef HIFI_APE_DECODE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef HIFI_FlAC_DECODE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef HIFI_AlAC_DECODE2
                    AudioFileChangeBuf2(pRawFileCache, CodecBufSize2);
#endif

#ifdef SBC_DEC_INCLUDE2

#endif

                    //bb_printf1("SampleRate = %d",gMediaBlockInfo.SampleRate);
                    //bb_printf1("Channel = %d",gMediaBlockInfo.Channel);
                    //bb_printf1("BitRate = %d",gMediaBlockInfo.BitRate);
                    //bb_printf1("CurrentPlayTime = %d",gMediaBlockInfo.CurrentPlayTime);
                    //bb_printf1("TotalPlayTime = %d",gMediaBlockInfo.TotalPlayTime);
                    //bb_printf1("BitPerSample = %d",gMediaBlockInfo.BitPerSample);
                }

                CodecGetDecBuffer2((short*)&gMediaBlockInfo.Outptr, &gMediaBlockInfo.OutLength);

                gMediaBlockInfo.DecodeErr = 0;
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DEC_OPEN_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_DECODE:
            {
                pcb.audio_decode_status = AUDIO_IDLE;

                //Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin0,GPIO_HIGH);
                if (1 != CodecDecode2())
                {
                    bb_printf1("###AUDIO_DECODE error or decode end ###");
                    gMediaBlockInfo.DecodeErr = 1;
                    MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    MailBoxWriteB2AData((UINT32)&gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                    break;
                }

                //Gpio_SetPinLevel(GPIO_CH2,GPIOPortA_Pin0,GPIO_LOW);
#ifdef MP3_DEC_INCLUDE2
                mp3_wait_synth();
#endif
                CodecGetTime2(&gMediaBlockInfo.CurrentPlayTime);
                //bb_printf1("codec decode over");
                CodecGetDecBuffer2((short*)&gMediaBlockInfo.Outptr, &gMediaBlockInfo.OutLength);
                //bb_printf1("codec get buffer ");
                gMediaBlockInfo.DecodeErr = 0;
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_SEEK:
            {
                pcb.audio_decode_status = AUDIO_IDLE;

                if ( 1 != CodecSeek2(pcb.audio_decode_param , 0))
                {
                    bb_printf1("codec seek fail");
                    //TODO...
                }

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_SEEK_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_GETBUFFER:
            {
//                CodecGetDecBuffer2((short*)&gMediaBlockInfo->Outptr,&gMediaBlockInfo->OutLength);
//
//                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_GETBUFFER_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
//                MailBoxWriteB2AData((UINT32)gMediaBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_GETTIME:
            {
                /*
                pcb.audio_decode_status = AUDIO_IDLE;
                CodecGetTime2(&gMediaBlockInfo.CurrentPlayTime);

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_GETTIME_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gMediaBlockInfo,MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                */
            }
            break;

            case AUDIO_DECODE_CLOSE:
            {
                pcb.audio_decode_status = AUDIO_IDLE;
                CodecClose2();
                //bb_printf1("AUDIO_DECODE_CLOSE");
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_DECODE_CLOSE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_FILE_OPEN:
            {
                pcb.audio_decode_status = AUDIO_IDLE;
                //bb_printf1("file open codec = %d", CurrentDecCodec2);
                AudioIntAndDmaInit2();
                AudioHWInit2();
                AudioCodecGetBufferSize2(CurrentDecCodec2, CodecFS_44100Hz);
#ifdef MP3_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef AAC_DEC_INCLUDE2
                ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
                //RKFileFuncInit2();
                //HifiFileSeek(0, SEEK_SET, pRawFileCache);
                AudioFileMhFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef HIFI_AlAC_DECODE2
                ClearMsg(MSG_AUDIO_DECODE_FILL_BUFFER);
                //RKFileFuncInit2();
                //HifiFileSeek(0, SEEK_SET, pRawFileCache);
                AudioFileMhFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
                //AudioFileMhSeek2(0, SEEK_SET, pRawFileCache);
#endif

#ifdef XXX_DEC_INCLUDE2
               AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef WAV_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef AMR_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef APE_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef FLAC_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef OGG_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef HIFI_APE_DECODE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef HIFI_FlAC_DECODE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif

#ifdef SBC_DEC_INCLUDE2
                AudioFileFuncInit2(pRawFileCache, HIFI_AUDIO_BUF_SIZE - 1024);
#endif



                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_OPEN_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            }
            break;

            case AUDIO_DECODE_FILE_CLOSE:
            {
                pcb.audio_decode_status = AUDIO_IDLE;
                //bb_printf1("file close");
                //DelayMs2(100);
                AudioIntAndDmaDeInit2();
                AudioHWDeInit2();
                while(AudioFileBufBusy2 == 1)
                {
                    __WFI2();
                }

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_CLOSE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                while (1)
                {
                    __WFI2();
                }
            }
             break;

//------------------------------------------------------------------------------
            case AUDIO_ENCODE_OPEN:
            {
                pcb.audio_decode_status = AUDIO_IDLE;

                if(CodecOpenEnc2((EncodeArg *)pcb.audio_decode_param, &gRecordBlockInfo.unenc_bufptr, &gRecordBlockInfo.unenc_length) != 1)
                {
                    gRecordBlockInfo.encodeErr = 1;
                }
                else
                {
                    CodecGetEncBitreate2(&gRecordBlockInfo.Bitrate);
                    gRecordBlockInfo.encodeErr = 0;
                }

                //bb_printf1("unenc buf = %x, unenc len = %d", gRecordBlockInfo.unenc_bufptr, gRecordBlockInfo.unenc_length);

                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_ENCODE_OPEN_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gRecordBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                break;
            }

            case AUDIO_ENCORD_ENCODE:
            {
                uint32 EncOutputDataLen;

                pcb.audio_decode_status = AUDIO_IDLE;
//              if(gRecordBlockInfo.FilterFlag == 1)
//              {
//                  NS_do(gRecordBlockInfo.unenc_bufptr,EncInputDataLen/2);
//              }

                CodecGetEncBuffer2(&gRecordBlockInfo.unenc_bufptr, &gRecordBlockInfo.unenc_length);
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_BUF_CHANGE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gRecordBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);

                gRecordBlockInfo.enc_length = CodecEncode2();
                if((int)gRecordBlockInfo.enc_length <= 0)
                {
                    gRecordBlockInfo.encodeErr = 1;
                }
                else
                {
                    gRecordBlockInfo.encodeErr = 0;
                }
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_ENCODE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData((UINT32)&gRecordBlockInfo, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                break;
            }

            //------------------------------------------------------
            case AUDIO_ENCODE_FILE_CREATE:    //used for record
                pcb.audio_decode_status = AUDIO_IDLE;
                //bb_printf1("recive MEDIA_MSGBOX_CMD_FILE_CREATE");
                AudioCodecGetBufferSize2(CurrentEncCodec2, CodecFS_44100Hz);
                RecordBufferInit2();
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                break;

            case AUDIO_ENCODE_FILE_CLOSE:
                pcb.audio_decode_status = AUDIO_IDLE;
                //bb_printf1("recive 11 AUDIO_ENCODE_FILE_CLOSE");

                //here write record left data to file tail.
                RecordWriteFileTail2();

                //bb_printf1("recive 22 AUDIO_ENCODE_FILE_CLOSE");
                MailBoxWriteB2ACmd(MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_CMPL, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteB2AData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                break;

            default :
                break;
        }

        if (GetMsg(MSG_AUDIO_DECODE_FILL_BUFFER))
        {
            AudioFileInput2(pRawFileCache);
        }

        if (GetMsg(MSG_AUDIO_ENCODE_CLEAR_BUFFER))
        {
            RecordWriteToFile2();
        }

        //process hifi file operate
        switch (gFileOpStatus)
        {
            case FILE_IDLE:
#if 1
                IntMasterDisable2();

                if ((pcb.audio_decode_status == AUDIO_IDLE) && (gFileOpStatus == FILE_IDLE))
                {
                    __WFI2();
                }

                IntMasterEnable2();
#endif
                break;

            case FILE_SEEK_CMPL:
                gFileOpStatus = FILE_IDLE;
                break;

            case FILE_READ_CMPL:
                gFileOpStatus = FILE_IDLE;
                break;

            case FILE_WRITE_CMPL:
                gFileOpStatus = FILE_IDLE;
                break;

            case FILE_TELL_CMPL:
                gFileOpStatus = FILE_IDLE;
                break;

            case FILE_GET_LEN_CMPL:
                gFileOpStatus = FILE_IDLE;
                break;
        }
    }
}


