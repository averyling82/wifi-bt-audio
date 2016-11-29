/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\Plugin\TaskPlugin.h
* Owner: aaron.sun
* Date: 2015.10.16
* Time: 17:34:44
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.16     17:34:44   1.0
********************************************************************************************
*/


#ifndef __OS_PLUGIN_TASKPLUGIN_H__
#define __OS_PLUGIN_TASKPLUGIN_H__

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

#define _OS_PLUGIN_TASKPLUGIN_COMMON_  __attribute__((section("os_plugin_taskplugin_common")))
#define _OS_PLUGIN_TASKPLUGIN_INIT_  __attribute__((section("os_plugin_taskplugin_init")))
#define _OS_PLUGIN_TASKPLUGIN_SHELL_  __attribute__((section("os_plugin_taskplugin_shell")))


typedef enum _TASK_ID
{
    //add new task info here
    //......
    TASK_ID_LINEINCONTROL,
    TASK_ID_LINEIN,
    TASK_ID_BT_PHONE_VOICE,
    TASK_ID_AUDIODEVPLAYSERVICE,
    TASK_ID_FMUI,
    TASK_ID_FMCONTROL,
    TASK_ID_TCPHEART_TASK,
    TASK_ID_TCPCHANNEL_TASK,
    TASK_ID_TCPCONTROL_TASK,
    TASK_ID_TCPSTREAM_TASK,
    TASK_ID_BROAD_TASK,
    TASK_ID_BROWSERUI,
    TASK_ID_AUDIODEVICESERVICE,
    TASK_ID_RECORDCONTROL,
    TASK_ID_SYSTEMSETWIFISELECT,
    TASK_ID_SYSTEMSET,
    TASK_ID_CHARGE,
    TASK_ID_MUSIC_PLAY_MENU,
    TASK_ID_RECOD,
    TASK_ID_MEDIA_UPDATE,
    TASK_ID_MEDIA_LIBRARY,
    TASK_ID_MEDIA_BROWSER,
    TASK_ID_USBSERVER,
    TASK_ID_DLNA_PLAYER,
    TASK_ID_DLNA,
    TASK_ID_XXX,
    TASK_ID_MAIN,
    TASK_ID_TCPIP_THREAD,
    TASK_ID_WWD,
    TASK_ID_NETWORKING,
    TASK_ID_HARDWARE_TO_WORK,
    TASK_ID_WIFI_APPLICATION,
    TASK_ID_HTTP,
    TASK_ID_HTTP_UP,
    TASK_ID_GUI,
    TASK_ID_TIMER,
    TASK_ID_BCORE,
    TASK_ID_STREAMCONTROL,
    TASK_ID_AUDIOCONTROL,
    TASK_ID_USBMSC,
    TASK_ID_USBOTG0,
    TASK_ID_USBOTG1,
    TASK_ID_MEDIA,
    TASK_ID_FILESTREAM,
    TASK_ID_TEST,
    TASK_ID_SHELL,
    TASK_ID_DEVICE_TASK,
    TASK_ID_SDIO_IRQ_TASK,
    TASK_ID_SYS_TIMER,
    TASK_ID_IDLE,
    TASK_ID_TASK_MANAGER,
    TASK_ID_NUM

}TASK_ID;

typedef enum _TASK_STATE
{
    TASK_STATE_RUNING,
    TASK_STATE_READY,
    TASK_STATE_BLOCKED,
    TASK_STATE_SUSPEND,
    TASK_STATE_DELETED,
    TASK_STATE_WORKING,
    TASK_STATE_IDLE1,
    TASK_STATE_IDLE2,
    TASK_STATE_ERROR,
    TASK_STATE_NUM

}TASK_STATE;
typedef rk_err_t (*pTaskInitFunType)(void * , void *);
typedef rk_err_t (*pTaskDeInitFunType)(void * );
typedef void (*pTaskFunType)(void *);
typedef rk_err_t (*pTaskSuspendFunType)(void *, uint32);
typedef rk_err_t (*pTaskResumeFunType)(void *);

#ifndef NOT_INCLUDE_OTHER
typedef void * HTC;
#endif


/* 任务控制块结构 */
typedef struct _RK_TASK_CLASS
{
    struct _RK_TASK_CLASS * NextTCB;
    pTaskInitFunType     TaskInitFun;
    pTaskFunType         TaskFun;
    pTaskDeInitFunType   TaskDeInitFun;
    pTaskSuspendFunType  TaskSuspendFun;
    pTaskResumeFunType   TaskResumeFun;
    HTC        hTask;/*任务传参回传的任务句柄*/
    uint8  TaskFlag;               /*认为状态信息标志位*/
    uint32 OverlayModule;          /*Overlay模块编号*/
    uint32 TaskPriority;           /*任务优先级*/
    uint32 TaskStackSize;          /*系统保存恢复堆栈大小*/
    uint32 TaskClassID;
    uint32 TaskObjectID;
    uint8 * taskname;
    uint32 TotalMemory;
    uint32 State;
    uint32 IdleTick;
    uint32 Idle1EventTime;
    uint32 Idle2EventTime;

}RK_TASK_CLASS;

typedef struct _RK_TASK_FM_ARG
{
    uint32 source;

}RK_TASK_FM_ARG;


typedef struct _RK_TASK_MEDIA_ARG
{
    uint32 file;

}RK_TASK_MEDIA_ARG;

typedef struct _RK_TASK_MEDIALIB_ARG
{
    uint32 CurId;
} RK_TASK_MEDIALIB_ARG;

typedef struct _RK_TASK_MEDIABRO_ARG
{
    uint16 Flag;    //是否需要使用回传的MEDIABRO_DIR_TREE_STRUCT结构体数据恢复资源信息的标记
    uint16 MediaTypeSelID;
    MEDIABRO_DIR_TREE_STRUCT MediaDirTreeInfo;
} RK_TASK_MEDIABRO_ARG;

typedef struct _RK_TASK_BROWSER_ARG
{
    uint16 Flag;    //是否需要使用回传的MEDIA_FLODER_INFO_STRUCT结构体数据恢复资源信息的标记
    uint16 SelSouceType;
    uint16 filepath[MAX_FILENAME_LEN];
    MEDIA_FLODER_INFO_STRUCT MediaFloderInfo;
}RK_TASK_BROWSER_ARG;

typedef struct _RK_TASK_AUDIOCONTROL_ARG
{
    uint16 FileNum;
    uint16 TotalFiles;
    uint16 filepath[MAX_FILENAME_LEN];
    uint32 ucSelPlayType;
    uint32 SaveMemory;
    uint32 DirectPlay;
    uint32 CodecType;
    void * pfAudioState;
    MEDIA_FLODER_INFO_STRUCT MediaFloderInfo;
}RK_TASK_AUDIOCONTROL_ARG;

typedef struct _RECORD_ARG
{
    uint32 RecordType;               //Record Mode : 0 : MIC_STERO / 1 : LINE1 / 2 : LINE2 / 3 : MIC1_MONO / 4 : MIC2_MONO
    uint32 RecordSampleRate;        //record Samplerate  8K - 192K : 8000 - 192000;
    uint32 RecordBitrate;
    uint32 RecordChannel;            //record Channels : 1 : mono / 2 : stero
    uint32 RecordDataWidth;         //Record DataWidth : 0x0F : 16bit / 0x17 : 24bit
    uint32 RecordEncodeType;        //Record EncodeType : 0 : wav / 1 : mp3 / 2 : amr
    uint32 RecordEncodeSubType;     //Record EncodeSubType
    uint32 RecordSource;            //Record Source : 0 : MIC LineIn / 1 : FM
    void * pfRecordState;

}RK_TASK_RECORDCONTROL_ARG;

typedef struct _RK_TASK_FMCONTROL_ARG
{
    UINT32    FmArea;                           // SID_RadioListAreaChina,  SID_RadioListAreaJapan, SID_RadioListAreaUSA, SID_RadioListAreaEurope
    BOOLEAN   FmStereo;
    uint32    LRChannel;
    uint32    FMInputType;
    uint32    SamplesPerBlock;
    void * pfmStateCallBcakForAPP;
}RK_TASK_FMCONTROL_ARG;

typedef struct _RK_TASK_LINEINCONTROL_ARG
{
    uint32    LRChannel;
    uint32    LineInInputType;
    uint32    SamplesPerBlock;
    uint32    Samples;
    uint32    bits;
    void *    pLineInStateCallBcakForAPP;
}RK_TASK_LINEINCONTROL_ARG;

typedef struct _RK_TASK_LINEIN_ARG
{
    uint32 source;

}RK_TASK_LINEIN_ARG;

typedef struct _RK_TASK_PLAYMENU_ARG
{
    uint16 ucSelPlayType;
    uint16 FileNum;
    uint16 TotalFiles;
    uint16 filepath[MAX_DIRPATH_LEN];
    MEDIA_FLODER_INFO_STRUCT MediaFloderInfo;
}RK_TASK_PLAYMENU_ARG;


/* 任务控制块结构 */
typedef union _RK_TASK_ARG
{
    RK_TASK_MEDIA_ARG stTaskMediaArg;
    RK_TASK_AUDIOCONTROL_ARG stTaskAudioControlArg;
    RK_TASK_BROWSER_ARG stBrowserArg;
    RK_TASK_PLAYMENU_ARG stPlayMenuArg;
    RK_TASK_LINEINCONTROL_ARG stLineInControlArg;
    RK_TASK_LINEIN_ARG stLineInArg;

}RK_TASK_ARG;


/* 任务队列元素结构 */
typedef struct _RK_TASK_QUEUE_ITEM
{
    RK_TASK_CLASS * TaskHandler;
    RK_TASK_ARG * TaskArg;
    uint32         RkTaskEvent;
    uint32  Mode;

}RK_TASK_QUEUE_ITEM;

#define MINIMAL_DATA_SIZE 32
#define MAXIMAL_DATA_SIZE 512

#define MINIMAL_STACK_SIZE 32
#define MAXIMAL_STACK_SIZE 512

//TaskPriority
//[20 =< real time task < 30]
#define TASK_PRIORITY_LINE_IN_CONTROL (configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_FM_CONTROL (configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_AUDIO_PLAYER (configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_TIMER (configMAX_PRIORITIES - 2)
#define TASK_PRIORITY_AUDIO_CONTROL (configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_AUDIO_SERVER (configMAX_PRIORITIES - 3)
#define TASK_PRIORITY_RECORD_CONTROL (configMAX_PRIORITIES - 4)
#define TASK_PRIORITY_MEDIA (configMAX_PRIORITIES - 4)
#define TASK_PRIORITY_SYSTICK (configMAX_PRIORITIES - 5)
#define TASK_PRIORITY_TASK_MANAGER (configMAX_PRIORITIES - 6)
#define TASK_PRIORITY_DEVICE_MANAGER (configMAX_PRIORITIES - 7)

//[10 =< protocal task < 20]
#define TASK_PRIORITY_UPNP_SSDPRESPONSE_SERVER 18
#define TASK_PRIORITY_UPNP_SERVICE_CREATENOTIFY 18
#define TASK_PRIORITY_UPNP_DEVICE_ADVERTISER 18
#define TASK_PRIORITY_HTTP_CLIENT 18
#define TASK_PRIORITY_HTTP_SERVER 18
#define TASK_PRIORITY_SSDP_SERVER 18
#define TASK_PRIORITY_MPLAYER 18
#define TASK_PRIORITY_STREAM_CONTROL 18
#define TASK_PRIORITY_FILE_STREAM 18
#define TASK_PRIORITY_HTTP 18
#define TASK_PRIORITY_HTTP_UP 18
#define TASK_PRIORITY_TCP  17
#define TASK_PRIORITY_WIFI 16
#define TASK_PRIORITY_WIFI_APP 10

#define BT_HCI_DATA_PROCESS_TASK_PRIORITY   (TASK_PRIORITY_STREAM_CONTROL -1)
#define BT_HCI_H5_DATA_SEND_TASK_PRIORITY   (BT_HCI_DATA_PROCESS_TASK_PRIORITY -1)
#define TASK_PRIORITY_BLUETOOTH_CTRL        (BT_HCI_H5_DATA_SEND_TASK_PRIORITY -1)


#define TASK_PRIORITY_CHANNEL  18
#define TASK_PRIORITY_BROAD    18
#define TASK_PRIORITY_TCPSTREAM 18
#define TASK_PRIORITY_TCPCONTROL 19
#define TASK_PRIORITY_TCPHEART  19

//XXX
#define TASK_PRIORITY_TINYSVC_MDNS 16   // == TASK_PRIORITY_TCP
#define TASK_PRIORITY_RTSP 17
#define TASK_PRIORITY_RTP 17

#define TASK_PRIORITY_SDIO   12
#define BT_HCI_DATA_RECV_TASK_PRIORITY   12
#define TASK_PRIORITY_USBOTG 12
#define TASK_PRIORITY_USBMSC 11

#define TASK_PRIORITY_SHELL  10

//[0 =< device task < 10]
#define TASK_PRIORITY_LINE_IN 2
#define TASK_PRIORITY_BCORE  TASK_PRIORITY_AUDIO_CONTROL
#define TASK_PRIORITY_USBSEVICE 2
#define TASK_PRIORITY_GUI    2
#define TASK_PRIORITY_MAINTASK 2
#define TASK_PRIORITY_CHARGE 2
#define TASK_PRIORITY_IDLE   0

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern uint32 TaskTotalCnt;
extern uint32 TaskTotalSuspendCnt;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern uint32 RKTaskGetState(uint32 TaskClassID, uint32 TaskObjectID);
extern rk_err_t RKTaskIdleTick(void);
extern uint32 RKTaskTime(HTC hTask);
extern HTC RKTaskGetRunHandle(void);
extern void * RKTaskGetTaskName(HTC hTask);
extern rk_err_t RKTaskDelete2(HTC hTask);
extern HTC RKTaskCreate2(pTaskFunType TaskCode, pTaskSuspendFunType SuspendCode,pTaskResumeFunType ResumeCode,
                                        char *name, uint32 StackDeep, uint32 Priority, void * para);
extern void SchedulerTaskCreate(void);
extern rk_err_t RKTaskDelete(uint32 TaskClassID, uint32 TaskObjectID, uint32 Mode);
extern rk_err_t RKTaskCreate(uint32 TaskClassID, uint32 TaskObjectID, void * arg, uint32 Mode);
extern HTC RKTaskGetNextHandle(HTC hTask, uint32 TaskClassID);
extern HTC RKTaskGetFirstHandle(uint32 TaskClassID);
extern uint32 RKTaskGetTotalCnt(uint32 TaskClassID);
extern HTC RKTaskFind(uint32 TaskClassID, uint32 TaskObjectID);
extern uint32 RKTaskHeapTotal(void);
extern uint32 RKTaskHeapFree(void);
extern uint32 RKTaskStackSize(HTC hTask);
extern uint32 RKTaskStackRemain(HTC hTask);
extern uint32 RKTaskState(HTC hTask);
extern uint32 RKTaskStackBaseAddr(HTC hTask);
extern rk_err_t RKTaskInit(void);
#endif
