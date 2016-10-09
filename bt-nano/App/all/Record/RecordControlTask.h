/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Record\RecordControlTask.h
* Owner: chad.ma
* Date: 2015.12.22
* Time: 16:55:21
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.ma     2015.12.22     16:55:21   1.0
********************************************************************************************
*/


#ifndef __APP_RECORD_RECORDCONTROL_H__
#define __APP_RECORD_RECORDCONTROL_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define     MIN_RECORD_RESERVE_MEMORY           ((UINT32)512 * 1024)  //the smallest memory space is 512k.

//RecordServiceFlag
#define     RECORD_STA_NULL                     0x0000
#define     RECORD_STA_PCMBUF_EMPTY             (0x0001 << 0)
#define     RECORD_STA_PCMBUF_FULL              (0x0001 << 1)
#define     RECORD_STA_DISP_INIT                (0X0001 << 2)
#define     RECORD_STA_TIME_UPDATE              (0X0001 << 3)
#define     RECORD_STA_FILEBUF_FULL             (0x0001 << 4)
#define     RECORD_STA_DISK_FULL                (0x0001 << 5)
#define     RECORD_STA_DECODE_MUTE_ON           (0x0001 << 6)
#define     RECORD_STA_DECODE_MUTE_OFF          (0x0001 << 7)

//RecordConFlag
#define     RECORD_CON_NULL                     0x0000
#define     RECORD_CON_ENCODE_INIT              (0x0001 << 0)
#define     RECORD_CON_ENCODE_ENABLE            (0x0001 << 1)


#define    RECORD_STARTNOISEBLOCK    19
#define    RECORD_KEYNOISEBLOCK      15

typedef enum _RECORD_STATE
{
    RECORD_STATE_PREPARE,
    RECORD_STATE_BEING,
    RECORD_STATE_PAUSE,
    RECORD_STATE_STOP,
    RECORD_STATE_TIME_CHANGE,
    RECORD_STATE_ERROR,
    RECORD_STATE_NOT_SPACE,
    RECORD_STATE_MAX

}RECORD_STATE;


typedef enum _RECORD_CMD
{
    RECORD_CMD_PREPARE,
    RECORD_CMD_START,
    RECORD_CMD_PAUSE,
    RECORD_CMD_RESUME,
    RECORD_CMD_STOP,
    RECORD_CMD_GETINFO,
    RECORD_CMD_GETPCM_PROCESS,
    RECORD_CMD_MAX

}RECORD_CMD;

typedef  struct _RECORD_INFO
{
    uint32 LineInInputType;
    uint32 TotalFiles;
    uint32 RecordSource;
    uint32 Samplerate;
    uint32 Bitrate;
    uint32 channels;
    uint32 bitpersample;
    uint8  FileName[12];
    uint32 EncodeType;
    uint32 RecordQuality;
    uint32 recorderr;

}RECORD_INFO;

#define _APP_RECORD_RECORDCONTROLTASK_COMMON_  __attribute__((section("app_record_recordcontroltask_common")))
#define _APP_RECORD_RECORDCONTROLTASK_INIT_  __attribute__((section("app_record_recordcontroltask_common")))
#define _APP_RECORD_RECORDCONTROLTASK_SHELL_  __attribute__((section("app_record_recordcontroltask_shell")))
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define _APP_RECORD_RECORDCONTROLTASK_DATA_ _APP_RECORD_RECORDCONTROLTASK_COMMON_
#elif defined(__arm__) && defined(__GNUC__)
#define _APP_RECORD_RECORDCONTROLTASK_DATA_  __attribute__((section("app_record_recordcontroltask_data")))
#else
#error Unknown compiling tools.
#endif

typedef void (* P_RECORD_CALLBACK)(uint32 record_state);      //record callback funciton

typedef  struct _RECORD_CALLBACK
{
    struct _RECORD_CALLBACK * pNext;
    P_RECORD_CALLBACK pfRecordState;

}RECORD_CALLBACK;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define RECORD_DMACHANNEL_IIS    (DMA_CHN_MAX - 1)


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
void RecordPcmInput(uint8 * buf, uint32 Samples);
extern rk_err_t Recorder_ChangeCallBack(P_RECORD_CALLBACK old, P_RECORD_CALLBACK new);
extern rk_err_t Recorder_GetTotalTime(uint32 * time);
extern rk_err_t Recorder_GetCurTime(uint32 * time);
extern uint32 Recorder_GetRecordInf(RECORD_INFO * pRecordInf);
extern void dumpMemoryCharA(uint8 * addr,uint32 size, uint8 data_type);
extern void dumpMemoryShort2Char(uint16* memory,uint32 size);
extern rk_err_t RecordControlTask_SendCmd(uint32 Cmd, void * msg, uint32 Mode);
extern void DeRegMBoxRecEncodeSvc(void);
extern void RegMBoxRecEncodeSvc(void);
extern void RecordHWInit(void);
extern BOOL RecordServiceGetTotalTime(void);
extern void RecordGetInfo(void);
extern BOOL RecordGetFileName(void);
extern void RecordControlTask_Enter(void * arg);
extern rk_err_t RecordControlTask_DeInit(void *pvParameters);
extern rk_err_t RecordControlTask_Init(void *pvParameters, void *arg);



#endif
