/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Codecs\Audio\Encode\include\record_globals.h
* Owner: chad.ma
* Date: 2015.12.30
* Time: 11:34:41
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.ma     2015.12.30     11:34:41   1.0
********************************************************************************************
*/


#ifndef __CODECS_AUDIO_ENCODE_INCLUDE_RECORD_GLOBALS_H__
#define __CODECS_AUDIO_ENCODE_INCLUDE_RECORD_GLOBALS_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//RecordType
#define     RECORD_TYPE_MIC_STERO               0
#define     RECORD_TYPE_LINEIN1                 1
#define     RECORD_TYPE_LINEIN2                 2
#define     RECORD_TYPE_MIC1_MONO               3
#define     RECORD_TYPE_MIC2_MONO               4
#define     RECORD_TYPE_NULL                    5

//RecordChannel
#define     RECORD_CHANNEL_STERO                2
#define     RECORD_CHANNEL_MONO                 1

//RecordData_Width
#define     RECORD_DATAWIDTH_16BIT              16
#define     RECORD_DATAWIDTH_24BIT              24
#define     RECORD_DATAWIDTH_32BIT              32

//record code type.
#define     RECORD_ENCODE_TYPE_WAV              0
#define     RECORD_ENCODE_TYPE_XXX              1
#define     RECORD_ENCODE_TYPE_AMR              2


//definition of sampling rate.
#define     RECORD_SAMPLE_FREQUENCY_8KHZ        8000    //8KHz
#define     RECORD_SAMPLE_FREQUENCY_11KHZ       11025   //11KHz
#define     RECORD_SAMPLE_FREQUENCY_16KHZ       16000   //16KHz
#define     RECORD_SAMPLE_FREQUENCY_22KHZ       22050   //22KHz
#define     RECORD_SAMPLE_FREQUENCY_32KHZ       32000   //32KHz
#define     RECORD_SAMPLE_FREQUENCY_44_1KHZ     44100   //44.1KHz
#define     RECORD_SAMPLE_FREQUENCY_48KHZ       48000   //48KHz

#define     RECORD_PCM                          1
#define     RECORD_ADPCM                        2

#define     ENCODER_WRITE2FLASH_LENGTH      2048
#define     ENCODER_BUFFER_NUM              12//24  //chad.ma modified because B core data more than 0x01060000
#define     ENCODER_WRITE_BUFFER_LENGTH     (ENCODER_WRITE2FLASH_LENGTH * ENCODER_BUFFER_NUM)

#define     ENCODER_WRITE2FLASH_LENGTH2     2048
#define     ENCODER_BUFFER_NUM2             12//24  //chad.ma modified because B core data more than 0x01060000
#define     ENCODER_WRITE_BUFFER_LENGTH2    (ENCODER_WRITE2FLASH_LENGTH * ENCODER_BUFFER_NUM)



#define     WAV_HEAD_BUFFER_SIZE            4096
#define     AD_PIPO_BUFFER_SIZE             6144



typedef enum _MEDIA_MSGBOX_ENCODE_CMD
{
    MEDIA_MSGBOX_CMD_ENCODE_NULL = 200,

    MEDIA_MSGBOX_CMD_FILE_CREATE,
    MEDIA_MSGBOX_CMD_FILE_CREATE_CMPL,

    MEDIA_MSGBOX_CMD_ENCODE_OPEN,
    MEDIA_MSGBOX_CMD_ENCODE_OPEN_CMPL,

    MEDIA_MSGBOX_CMD_ENCODE,
    MEDIA_MSGBOX_CMD_ENCODE_CMPL,      /*完成一次编码操作*/

    MEDIA_MSGBOX_CMD_BUF_CHANGE,

    MEDIA_MSGBOX_CMD_REC_FILE_CLOSE,
    MEDIA_MSGBOX_CMD_REC_FILE_CLOSE_CMPL,    //211

    MEDIA_MSGBOX_CMD_ENCODE_NUM

}MEDIA_MSGBOX_ENCODE_CMD;


typedef enum Record_FS
{
    RECORD_FS_8000Hz   = 8000,
    RECORD_FS_11025Hz  = 11025,
    RECORD_FS_12KHz    = 12000,
    RECORD_FS_16KHz    = 16000,
    RECORD_FS_22050Hz  = 22050,
    RECORD_FS_24KHz    = 24000,
    RECORD_FS_32KHz    = 32000,
    RECORD_FS_44100Hz  = 44100,
    RECORD_FS_48KHz    = 48000,
    RECORD_FS_64KHz    = 64000,
    RECORD_FS_88200Hz   = 88200,
    RECORD_FS_96KHz    = 96000,
    RECORD_FS_128KHz   = 128000,
    RECORD_FS_1764KHz  = 176400,
    RECORD_FS_192KHz   = 192000,
    RECORD_FSSTOP      = 192000
} RecordFS_en_t;

typedef struct _EncodeArg
{
    unsigned int sampleRate;        //encode samplerate
    unsigned int channel;           //ebcode channel
    unsigned int dataWidth;         //data width
    unsigned int encodeType;        //encode type
    unsigned int encodeSubType;

}EncodeArg;

typedef struct _RecordBlock
{
    unsigned int Bitrate;           //encode type
    unsigned int enc_bufptr;        //encoded data buffer pointer
    unsigned int unenc_bufptr;      //unencode data buffer pointer
    unsigned int enc_length;        //encode data buffer length
    unsigned int unenc_length;      //unencode data buffer length
    unsigned int FilterFlag;        //FilterFlag
    unsigned int encodeOver;        //flag of encode if or not done
    unsigned int encodeErr;         //flag of encode error status
    unsigned int needencode;

}RecordBlock;

// 写文件缓冲器.
typedef struct
{
    //write buffer must algin 4
    UINT8 *   Data; //WriteBuffer memory space.

    UINT16  uHead;        //the head of cycle.
    UINT16  uTail;        //the end of cycle.
    UINT16  uCounter;     //the valid data in current WriteBuffer.

} WAV_WRITE_BUFFER_STRUCT;


#define PRINTF_CHAR_DATA    0
#define PRINTF_HEX_DATA     1
#define PRINTF_DEC_DATA     2
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
