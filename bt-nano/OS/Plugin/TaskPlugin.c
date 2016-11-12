/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\Plugin\TaskPlugin.c
* Owner: aaron.sun
* Date: 2015.10.16
* Time: 16:46:40
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.16     16:46:40   1.0
********************************************************************************************
*/
#define __OS_PLUGIN_TASKPLUGIN_C__
#include "BspConfig.h"
#ifdef __OS_PLUGIN_TASKPLUGIN_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "BspConfig.h"
#include "FreeRTOS.h"
#include "RKOS.h"
#include "BSP.h"
#include "AppInclude.h"
#include "GUITask.h"
#include "DeviceInclude.h"
#include "tcpip.h"
#include "airplay.h"
#include "btPhoneVoice.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define TASK_QUEUE_SIZE         sizeof(RK_TASK_QUEUE_ITEM)
#define TASK_QUEUE_LENTH        10

#define RK_TASK_EVENT_CREATE          1
#define RK_TASK_EVENT_DELETE          2
#define RK_TASK_EVENT_CREATE_SUCCESS  3
#define RK_TASK_EVENT_DELETE_SUCCESS  4
#define RK_TASK_EVENT_CREATE_FAIL     5
#define RK_TASK_EVENT_DELETE_FAIL     6
#define RK_TASK_EVENT_IDLE_TICK       7


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//will be repeat when the overlay is success
__attribute__((section("InitTaskItem")))
const static RK_TASK_CLASS InitTaskItem[TASK_ID_NUM] =
{
    //add new task info here
    //......
#ifdef __APP_LINE_IN_LINEINCONTROLTASK_C__
    {
        .TaskInitFun   = LineInControlTask_Init,
        .TaskDeInitFun = LineInControlTask_DeInit,
        .TaskFun       = LineInControlTask_Enter,
        .TaskSuspendFun= LineInControlTask_Suspend,
        .TaskResumeFun = LineInControlTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_LININCONTROL_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_LINE_IN_CONTROL,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_LINEINCONTROL,
        .taskname      = "line_in_control",
    },
#else
    {
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
    },
#endif

#ifdef __APP_LINE_IN_LINEINTASK_C__
    {
        .TaskInitFun   = LineInTask_Init,
        .TaskDeInitFun = LineInTask_DeInit,
        .TaskFun       = LineInTask_Enter,
        .TaskSuspendFun= LineInTask_Suspend,
        .TaskResumeFun = LineInTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_LININ_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_LINE_IN,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_LINEIN,
        .taskname      = "line_in_ui",
    },
#else
    {
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
    },

#endif

#ifdef __BLUETOOTH_PHONE_TASK__
{
        .TaskInitFun   = BTPhoneVoiceTask_Init,
        .TaskDeInitFun = BTPhoneVoiceTask_DeInit,
        .TaskFun       = BTPhoneVoiceTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_BLUETOOTH_CTRL,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_BT_PHONE_VOICE,
        .taskname      = "hfp",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
{
        .TaskInitFun   = AudioDevPlayServiceTask_Init,
        .TaskDeInitFun = AudioDevPlayServiceTask_DeInit,
        .TaskFun       = AudioDevPlayServiceTask_Enter,
        .TaskSuspendFun= AudioDevPlayServiceTask_Suspend,
        .TaskResumeFun = AudioDevPlayServiceTask_Resume,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_AUDIO_PLAYER,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_AUDIODEVPLAYSERVICE,
        .taskname      = "audio_server",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_FM_FMUITASK_C__
{
        .TaskInitFun   = FmUiTask_Init,
        .TaskDeInitFun = FmUiTask_DeInit,
        .TaskFun       = FmUiTask_Enter,
        .TaskSuspendFun= FmUiTask_Suspend,
        .TaskResumeFun = FmUiTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_FMUI_TASK,//SEGMENT_ID_FMUI_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_FMUI,
        .taskname      = "fm_ui",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_FM_FMCONTROLTASK_C__
{
        .TaskInitFun   = FMControlTask_Init,
        .TaskDeInitFun = FMControlTask_DeInit,
        .TaskFun       = FMControlTask_Enter,
        .TaskSuspendFun= FMControlTask_Suspend,
        .TaskResumeFun = FMControlTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_FMCONTROL_TASK,//SEGMENT_ID_FMCONTROL_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_FM_CONTROL,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_FMCONTROL,
        .taskname      = "fm_control",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __RK_TCPHEART_TASK_C__

{
        .TaskInitFun   = RK_TcpHeartTask_Init,
        .TaskDeInitFun = RK_TcpHeartTask_Deinit,
        .TaskFun       = RK_TcpHeartTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,//SEGMENT_ID_DLNA_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_TCPHEART,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_TCPHEART_TASK,
        .taskname      = "tcpheart",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __RK_TCPCHANNEL_TASK_C__
{
        .TaskInitFun   = RK_TcpChannelTask_Init,
        .TaskDeInitFun = RK_TcpChannelTask_Deinit,
        .TaskFun       = RK_ChannelTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,//SEGMENT_ID_DLNA_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_CHANNEL,
        .TaskStackSize = 2*configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_TCPCHANNEL_TASK,
        .taskname      = "tcpchannel",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __RK_TCPCONTROL_TASK_C__
{
        .TaskInitFun   = RK_TcpControlTask_Init,
        .TaskDeInitFun = RK_TcpControlTask_Deinit,
        .TaskFun       = RK_TcpControlTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,//SEGMENT_ID_DLNA_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_TCPCONTROL,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_TCPCONTROL_TASK,
        .taskname      = "tcp_control",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __RK_TCPSTREAM_TASK_C__
{
        .TaskInitFun   = RK_TcpStreamTask_Init,
        .TaskDeInitFun = RK_TcpStreamTask_Deinit,
        .TaskFun       = RK_TcpStreamTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,//SEGMENT_ID_DLNA_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_TCPSTREAM,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE*6,
        .TaskClassID   = TASK_ID_TCPSTREAM_TASK,
        .taskname      = "tcp_stream",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __RK_BROAD_TASK_C__
{
        .TaskInitFun   = RK_TcpBroadTask_Init,
        .TaskDeInitFun = RK_TcpBroadTask_Deinit,
        .TaskFun       = RK_BroadTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,//SEGMENT_ID_DLNA_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_BROAD,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_BROAD_TASK,
        .taskname      = "broad",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_BROWSER_BROWSERUITASK_C__
    {
        .TaskInitFun   = BrowserUITask_Init,
        .TaskDeInitFun = BrowserUITask_DeInit,
        .TaskFun       = BrowserUITask_Enter,
        .TaskSuspendFun= BrowserUITask_Suspend,
        .TaskResumeFun = BrowserUITask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_BROWSERUI_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_BROWSERUI,
        .taskname      = "BrowserUITask",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

//chad.ma add
#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
    {
        .TaskInitFun   = AudioDevService_Task_Init,
        .TaskDeInitFun = AudioDevService_Task_DeInit,
        .TaskFun       = AudioDevService_Task_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_AUDIO_SERVER,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_AUDIODEVICESERVICE,
        .taskname      = "audio device thread",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

//chad.ma add
#ifdef __APP_RECORD_RECORDCONTROL_C__
    {
        .TaskInitFun   = RecordControlTask_Init,
        .TaskDeInitFun = RecordControlTask_DeInit,
        .TaskFun       = RecordControlTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_RECORDCONTROL_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_RECORD_CONTROL,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_RECORDCONTROL,
        .taskname      = "record_control",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_C__
    {
        .TaskInitFun   = SystemSetWifiSelectTask_Init,
        .TaskDeInitFun = SystemSetWifiSelectTask_DeInit,
        .TaskFun       = SystemSetWifiSelectTask_Enter,
        .TaskSuspendFun= SystemSetWifiSelectTask_Suspend,
        .TaskResumeFun = SystemSetWifiSelectTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_SYSTEMSETWIFISELECT_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_SYSTEMSETWIFISELECT,
        .taskname      = "systemSelectWifi",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_SYSTEMSET_SYSTEMSETTASK_C__
    {
        .TaskInitFun   = SystemSetTask_Init,
        .TaskDeInitFun = SystemSetTask_DeInit,
        .TaskFun       = SystemSetTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_SYSTEMSET_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_SYSTEMSET,
        .taskname      = "systemSet",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_USB_CHARGETASK_C__
    {
        .TaskInitFun   = ChargeTask_Init,
        .TaskDeInitFun = ChargeTask_DeInit,
        .TaskFun       = ChargeTask_Enter,
        .TaskSuspendFun= ChargeTask_Suspend,
        .TaskResumeFun = ChargeTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_CHARGE_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_CHARGE,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_CHARGE,
        .taskname      = "chargewin",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_MUSIC_PLAY_MENU_TASK_C__
{
        .TaskInitFun   = MusicPlayMenuTask_Init,
        .TaskDeInitFun = MusicPlayMenuTask_DeInit,
        .TaskFun       = MusicPlayMenuTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_MUSIC_PLAY_MENU_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_MUSIC_PLAY_MENU,
        .taskname      = "playmenu",

},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_RECORD_RECOD_TASK_C__
{
        .TaskInitFun   = RecodTask_Init,
        .TaskDeInitFun = RecodTask_DeInit,
        .TaskFun       = RecodTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_RECOD_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_RECOD,
        .taskname      = "recod",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_MEDIA_UPDATE_C__
{
        .TaskInitFun   = MediaUpdate_Init,
        .TaskDeInitFun = MediaUpdate_DeInit,
        .TaskFun       = MediaUpdate_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_MEDIA_UPDATE,
        .TaskFlag      = NULL,
        .TaskPriority  = 1,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 8,
        .TaskClassID   = TASK_ID_MEDIA_UPDATE,
        .taskname      = "mediaupdate",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_MEDIA_LIBRARY_C__
{
        .TaskInitFun   = MediaLibrary_Init,
        .TaskDeInitFun = MediaLibrary_DeInit,
        .TaskFun       = MediaLibrary_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_MEDIA_LIBRARY_TASK_CODE,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_MEDIA_LIBRARY,
        .taskname      = "medialibrary",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_MEDIA_BROWSER_C__
{
        .TaskInitFun   = MediaBrowser_Init,
        .TaskDeInitFun = MediaBrowser_DeInit,
        .TaskFun       = MediaBrowser_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_MEDIA_BROWSER_TASK_CODE,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_MEDIA_BROWSER,
        .taskname      = "mediabrowser",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __SYSTEM_USBSERVER_USBSERVICE_C__
{
        .TaskInitFun   = UsbService_Init,
        .TaskDeInitFun = UsbService_DeInit,
        .TaskFun       = UsbService_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_USBSERVER_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_USBSEVICE,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_USBSERVER,
        .taskname      = "usbsever",
},
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __DLNA_PLAYER_C__
    {
        .TaskInitFun   = MPLAYERTask_Init,
        .TaskDeInitFun = MPLAYERTask_DeInit,
        .TaskFun       = MPLAYERTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,//SEGMENT_ID_DLNA_INIT,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_MPLAYER,
        .TaskStackSize = configMINIMAL_STACK_SIZE*4,
        .TaskClassID   = TASK_ID_DLNA_PLAYER,
        .taskname      = "dlna_play_task",
    },

#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WIFI_DLNA_C__
    {
        .TaskInitFun   = DLNATask_Init,
        .TaskDeInitFun = DLNATask_DeInit,
        .TaskFun       = DLNATask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_DLNA_TASK_API,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_MPLAYER,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_DLNA,
        .taskname      = "dlna",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WIFI_XXX_C__
    {
        .TaskInitFun   = XXXTask_Init,
        .TaskDeInitFun = XXXTask_DeInit,
        .TaskFun       = XXXTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_XXX,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_MPLAYER,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_XXX,
        .taskname      = "XXX",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_MAIN_TASK_MAIN_TASK_C__
    {
        .TaskInitFun   = MainTask_Init,
        .TaskDeInitFun = MainTask_DeInit,
        .TaskFun       = MainTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_MAIN_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_MAINTASK,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_MAIN,
        .taskname      = "Main",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_TCPIP_C__
    {
        .TaskInitFun   = NULL,
        .TaskDeInitFun = NULL,
        .TaskFun       = tcpip_thread,
        .TaskSuspendFun= tcpip_suspend,
        .TaskResumeFun = tcpip_resume,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_TCP,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_TCPIP_THREAD,
        .taskname      = "lwip",
        .Idle1EventTime = 120 * PM_TIME,
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_WIFI_TRANS_C__
    {
        .TaskInitFun   = NULL,
        .TaskDeInitFun = NULL,
        .TaskFun       = wwdTask_Enter,
        .TaskSuspendFun= wwdTask_Suspend,
        .TaskResumeFun = wwdTask_Resume,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_WIFI,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE *2,
        .TaskClassID   = TASK_ID_WWD,
        .taskname      = "wwd",
        .Idle1EventTime = 120 * PM_TIME,
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_NETWORKING_C__
    {
        .TaskInitFun   = networkingTask_Init,
        .TaskDeInitFun = networkingTask_DeInit,
        .TaskFun       = networkingTask_Enter,
        .TaskSuspendFun= networkingTask_Suspend,
        .TaskResumeFun = networkingTask_Resume,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_WIFI,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_NETWORKING,
        .taskname      = "net_work",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_HARD_TO_WOKR_C__
    {
        .TaskInitFun   = hardware_to_workTask_Init,
        .TaskDeInitFun = hardware_to_workTask_DeInit,
        .TaskFun       = hardware_to_workTask_Enter,
        .TaskSuspendFun= hardware_to_workTask_Suspend,
        .TaskResumeFun = hardware_to_workTask_Resume,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_WIFI,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_HARDWARE_TO_WORK,
        .taskname      = "hard_to_work",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_WIFITHREAD_C__
    {
        .TaskInitFun   = wifi_applicationTask_Init,
        .TaskDeInitFun = wifi_applicationTask_DeInit,
        .TaskFun       = wifi_applicationTask_Enter,
        .TaskSuspendFun= wifi_applicationTask_Suspend,
        .TaskResumeFun = wifi_applicationTask_Resume,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_WIFI_APPTASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_WIFI_APP,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_WIFI_APPLICATION,
        .taskname      = "wifi",
        .Idle1EventTime = 10 * PM_TIME,
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_HTTP_C__
    {
        .TaskInitFun   = HTTPTask_Init,
        .TaskDeInitFun = HTTPTask_DeInit,
        .TaskFun       = HTTPTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_HTTP,
        .TaskStackSize = configMINIMAL_STACK_SIZE*5,
        .TaskClassID   = TASK_ID_HTTP,
        .taskname      = "http",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __WICE_HTTP_C__
    {
        .TaskInitFun   = NULL,
        .TaskDeInitFun = NULL,
        .TaskFun       = HTTPTaskUp_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_HTTP_UP,
        .TaskStackSize = configMINIMAL_STACK_SIZE*3,
        .TaskClassID   = TASK_ID_HTTP_UP,
        .taskname      = "httpUP",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __GUI_GUITASK_C__
    {
        .TaskInitFun   = GUITask_Init,
        .TaskDeInitFun = GUITask_DeInit,
        .TaskFun       = GUITask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_GUI_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_GUI,
        .TaskStackSize = MINIMAL_STACK_SIZE * 20,
        .TaskClassID   = TASK_ID_GUI,
        .taskname      = "gui",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __DRIVER_TIMER_TIMERDEVICE_C__
    {
        .TaskInitFun   = TimerDev_Task_Init,
        .TaskDeInitFun = TimerDev_Task_DeInit,
        .TaskFun       = TimerDev_Task_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask   = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_TIMER,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_TIMER,
        .taskname      = "timer",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif


#ifdef __DRIVER_BCORE_BCOREDEVICE_C__
    {
        .TaskInitFun   = BcoreDev_Task_Init,
        .TaskDeInitFun = BcoreDev_Task_DeInit,
        .TaskFun       = BcoreDev_Task_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_BCORE,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_BCORE,
        .taskname      = "bcore",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_FILESTREAM_STREAMCONTROLTASK_C__
    {
        .TaskInitFun   = StreamControlTask_Init,
        .TaskDeInitFun = StreamControlTask_DeInit,
        .TaskFun       = StreamControlTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_STREAMCONTROL_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_STREAM_CONTROL,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_STREAMCONTROL,
        .taskname      = "stream_control",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_AUDIO_AUDIOCONTROLTASK_C__
    {
        .TaskInitFun   = AudioControlTask_Init,
        .TaskDeInitFun = AudioControlTask_DeInit,
        .TaskFun       = AudioControlTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_AUDIOCONTROL_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_AUDIO_CONTROL,
        .TaskStackSize = MAXIMAL_STACK_SIZE * 4,
        .TaskClassID   = TASK_ID_AUDIOCONTROL,
        .taskname      = "audio_control",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __DRIVER_USBMSC_USBMSCDEVICE_C__
    {
        .TaskInitFun   = USBMSCDev_Task_Init,
        .TaskDeInitFun = USBMSCDev_Task_DeInit,
        .TaskFun       = USBMSCDev_Task_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_USBMSC,
        .TaskStackSize = MINIMAL_STACK_SIZE * 15,
        .TaskClassID   = TASK_ID_USBMSC,
        .taskname      = "usb_msc",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __DRIVER_USB_USBOTGDEV_C__
    {
        .TaskInitFun   = UsbOtgDev0_Task_Init,
        .TaskDeInitFun = UsbOtgDev0_Task_DeInit,
        .TaskFun       = UsbOtgDev0_Task_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_USBOTG,
        .TaskStackSize = MINIMAL_STACK_SIZE * 15,
        .TaskClassID   = TASK_ID_USBOTG0,
        .taskname      = "usb_otg0",
    },

    {
        .TaskInitFun   = UsbOtgHost_Task_Init,
        .TaskDeInitFun = UsbOtgHost_Task_DeInit,
        .TaskFun       = UsbOtgHost_Task_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_USBOTG,
        .TaskStackSize = MINIMAL_STACK_SIZE * 15,
        .TaskClassID   = TASK_ID_USBOTG1,
        .taskname      = "usb_otg1",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},

{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_MEDIA_MEDIATASK_C__
    {
        .TaskInitFun   = MediaTask_Init,
        .TaskDeInitFun = MediaTask_DevInit,
        .TaskFun       = MediaTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_MEDIA_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_MEDIA,
        .TaskStackSize = MINIMAL_STACK_SIZE * 360,
        .TaskClassID   = TASK_ID_MEDIA,
        .taskname      = "media",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_FILESTREAM_FILESTREAMTASK_C__
    {
        .TaskInitFun   = FileStreamTask_Init,
        .TaskDeInitFun = FileStreamTask_DevInit,
        .TaskFun       = FileStreamTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = SEGMENT_ID_FILESTREAM_TASK,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_FILE_STREAM,
        .TaskStackSize = MINIMAL_STACK_SIZE * 360,
        .TaskClassID   = TASK_ID_FILESTREAM,
        .taskname      = "file_stream",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __APP_EXAMPLE_TASKFORMAT_C__
    {
        .TaskInitFun   = TestTask_Init,
        .TaskDeInitFun = TestTask_DevInit,
        .TaskFun       = TestTask_Enter,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = 2,
        .TaskStackSize = MINIMAL_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_TEST,
        .taskname      = "test",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef _USE_SHELL_
    {
        .TaskInitFun   = ShellTaskInit,
        .TaskDeInitFun = ShellTaskDeInit,
        .TaskFun       = ShellTask,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_SHELL,
        .TaskStackSize = MAXIMAL_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_SHELL,
        .taskname      = "shell",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

#ifdef __OS_PLUGIN_DEVICEPLUGIN_C__
    {
        .TaskInitFun   = DeviceTaskInit,
        .TaskDeInitFun = DeviceTaskDeInit,
        .TaskFun       = DeviceTask,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_DEVICE_MANAGER,
        .TaskStackSize = MAXIMAL_STACK_SIZE * 2,
        .TaskClassID   = TASK_ID_DEVICE_TASK,
        .taskname      = "device_manager",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif


#ifdef __DRIVER_SDIO_SDIODEVICE_C__
    {
        .TaskInitFun   = SdioIntIrqInit,
        .TaskDeInitFun = SdioIntIrqDeInit,
        .TaskFun       = SdioIrqTask,
        .TaskSuspendFun= NULL,
        .TaskResumeFun = NULL,
        .hTask         = NULL,
        .OverlayModule = NULL,
        .TaskFlag      = NULL,
        .TaskPriority  = TASK_PRIORITY_SDIO,
        .TaskStackSize = 100*2 ,
        .TaskClassID   = TASK_ID_SDIO_IRQ_TASK,
        .taskname      = "sdio_irq",
    },
#else
{
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = 0,
        .TaskStackSize = 0,
        .TaskClassID   = 0,
        .taskname      = 0,
},
#endif

    {
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = TASK_PRIORITY_SYSTICK,
        .TaskStackSize = configMINIMAL_STACK_SIZE*2,
        .TaskClassID   = TASK_ID_SYS_TIMER,
        .taskname      = "sys_timer",
    },


    {
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = tskIDLE_PRIORITY,
        .TaskStackSize = configMINIMAL_STACK_SIZE,
        .TaskClassID   = TASK_ID_IDLE,
        .taskname      = "idle",
    },

    {
        .TaskInitFun   = 0,
        .TaskDeInitFun = 0,
        .TaskFun       = 0,
        .TaskSuspendFun= 0,
        .TaskResumeFun = 0,
        .hTask         = 0,
        .OverlayModule = 0,
        .TaskFlag      = 0,
        .TaskPriority  = TASK_PRIORITY_TASK_MANAGER,
        .TaskStackSize = configSYS_SERVICE_STACK_SIZE,
        .TaskClassID   = TASK_ID_TASK_MANAGER,
        .taskname      = "task_manager",
    },

};

RK_TASK_CLASS * TaskListHead;
uint32 TaskTotalCnt;
uint32 TaskTotalSuspendCnt;
static pQueue  TaskAskQueue;
static pQueue  TaskRespQueue;
static pSemaphore TaskSemQueue;
static pSemaphore TaskSemOper;
static void * TaskManagerHandler;


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
void RKTaskIdleCheck(void);
void vApplicationStackOverflowHook(xTaskHandle xTask,signed char *pcTaskName);
void OSSchedulerTask(void * arg);
rk_err_t RKTaskDeleteOperating(RK_TASK_CLASS * TaskHandler);
rk_err_t RKTaskCreateOperating(RK_TASK_CLASS * TaskHandler, void * arg);
rk_err_t RKTaskDeRegister(RK_TASK_CLASS * TaskHandle);
rk_err_t RKTaskRegister(RK_TASK_CLASS * TaskHandler);
rk_err_t RKGetTaskMsg(uint32 TaskClassID,RK_TASK_CLASS *pTaskItem);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RKTaskGetState
** Input:uint32 TaskClassID, uint32 TaskObjectID
** Return: uint32
** Owner:aaron.sun
** Date: 2016.9.28
** Time: 17:16:54
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API uint32 RKTaskGetState(uint32 TaskClassID, uint32 TaskObjectID)
{
    RK_TASK_CLASS * pTask;

    if (TaskListHead == NULL)
    {
        return NULL;
    }
    else
    {
        pTask = TaskListHead;
        while (pTask != NULL)
        {
            if ((pTask->TaskClassID == TaskClassID) && (pTask->TaskObjectID == TaskObjectID))
            {
                return pTask->State;
            }
            pTask = pTask->NextTCB;
        }
    }
    return NULL;

}

/*******************************************************************************
** Name: RKTaskIdleTick
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.28
** Time: 14:53:58
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API rk_err_t RKTaskIdleTick(void)
{
    RK_TASK_QUEUE_ITEM TaskItem;

    TaskItem.RkTaskEvent = RK_TASK_EVENT_IDLE_TICK;
    TaskItem.Mode = ASYNC_MODE;
    rkos_queue_send(TaskAskQueue, &TaskItem, 0);
}
/*******************************************************************************
** Name: RKTaskGetRunHandle
** Input:void
** Return: HTC
** Owner:aaron.sun
** Date: 2016.3.25
** Time: 14:03:11
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API HTC RKTaskGetRunHandle(void)
{
    RK_TASK_CLASS * pTask;

    if (TaskListHead == NULL)
    {
        return NULL;
    }
    else
    {
        pTask = TaskListHead;
        while (pTask != NULL)
        {
            if ((pTask->hTask == (HTC)pxCurrentTCB))
            {
                return  pTask;
            }
            pTask = pTask->NextTCB;
        }
    }
    return NULL;
}

/*******************************************************************************
** Name: RKTaskDelete2
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.19
** Time: 14:58:53
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API rk_err_t RKTaskDelete2(HTC hTask)
{
    RK_TASK_CLASS * pTask;

    rkos_enter_critical();
    if(hTask == NULL)
    {
        if (TaskListHead == NULL)
        {
            rkos_exit_critical();
            return RK_ERROR;
        }
        else
        {
            pTask = TaskListHead;
            while (pTask != NULL)
            {
                if ((pTask->hTask == (HTC)pxCurrentTCB))
                {
                    hTask = pTask;
                    break;
                }
                pTask = pTask->NextTCB;
            }

            if(hTask == NULL)
            {
                rkos_exit_critical();
                return RK_ERROR;
            }
        }
    }
    rkos_exit_critical();
    return RKTaskDeleteOperating((RK_TASK_CLASS *)hTask);
}

/*******************************************************************************
** Name: RKTaskCreate2
** Input:TaskCode,name, SuspendCode, ResumeCode, StatckBase,StackDeep,Priority, para
** Return: HTC
** Owner:aaron.sun
** Date: 2015.10.19
** Time: 12:04:23
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API HTC RKTaskCreate2(pTaskFunType TaskCode, pTaskSuspendFunType SuspendCode,pTaskResumeFunType ResumeCode,
                                        char *name, uint32 StackDeep, uint32 Priority, void * para)
{
    RK_TASK_CLASS * pstNewTask = NULL;

    pstNewTask = (RK_TASK_CLASS *) rkos_memory_malloc(sizeof(RK_TASK_CLASS));

    if (pstNewTask == NULL)
        return (HTC)RK_ERROR;

    pstNewTask->NextTCB = NULL;
    pstNewTask->OverlayModule = NULL;
    pstNewTask->TaskClassID = 0xffffffff;
    pstNewTask->TaskDeInitFun = NULL;
    pstNewTask->TaskInitFun = NULL;
    pstNewTask->TaskFlag = NULL;
    pstNewTask->TaskFun = TaskCode;
    pstNewTask->TaskObjectID = 0xffffffff;
    pstNewTask->TaskPriority = Priority;
    pstNewTask->TaskResumeFun = ResumeCode;
    pstNewTask->TaskSuspendFun = SuspendCode;
    pstNewTask->TaskStackSize = StackDeep;
    pstNewTask->taskname = name;
    pstNewTask->State = TASK_STATE_WORKING;
    pstNewTask->Idle1EventTime = 10 * PM_TIME;

    if(RKTaskCreateOperating(pstNewTask, para) == RK_SUCCESS)
    {
        return pstNewTask;
    }

    return (HTC)RK_ERROR;

}
/*******************************************************************************
** Name: RKTaskDelete
** Input:uint32 TaskClassID, uint32 TaskObjectID, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:21:31
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API rk_err_t RKTaskDelete(uint32 TaskClassID, uint32 TaskObjectID, uint32 Mode)
{
    RK_TASK_QUEUE_ITEM TaskItem;
    rk_err_t ret;

    if(Mode == DIRECT_MODE)
    {
        rkos_semaphore_take(TaskSemOper, MAX_DELAY);

        TaskItem.TaskHandler = RKTaskFind(TaskClassID, TaskObjectID);

        if(TaskItem.TaskHandler == NULL)
        {
            rk_printf("1task not find id = %d / %d",TaskClassID, TaskObjectID);
            rkos_semaphore_give(TaskSemOper);
            return RK_ERROR;
        }
        ret = RKTaskDeleteOperating(TaskItem.TaskHandler);
        rkos_semaphore_give(TaskSemOper);
        return ret;
    }
    else
    {
        TaskItem.TaskHandler = RKTaskFind(TaskClassID, TaskObjectID);

        if(TaskItem.TaskHandler == NULL)
        {
            rk_printf("2task not find id = %d / %d",TaskClassID, TaskObjectID);
            return RK_ERROR;
        }

        if(Mode == ASYNC_MODE)
        {
            TaskItem.RkTaskEvent = RK_TASK_EVENT_DELETE;
            TaskItem.Mode = Mode;
            rkos_queue_send(TaskAskQueue, &TaskItem, MAX_DELAY);
            return RK_SUCCESS;
        }
        else if(Mode == SYNC_MODE)
        {

            TaskItem.RkTaskEvent = RK_TASK_EVENT_DELETE;
            TaskItem.Mode = Mode;

            rkos_semaphore_take(TaskSemQueue, MAX_DELAY);

            rkos_queue_send(TaskAskQueue, &TaskItem, MAX_DELAY);

            rkos_queue_receive(TaskRespQueue, &TaskItem, MAX_DELAY);

            if(TaskItem.RkTaskEvent == RK_TASK_EVENT_DELETE_SUCCESS)
            {
                rkos_semaphore_give(TaskSemQueue);
                return RK_SUCCESS;
            }
            else
            {
                rkos_semaphore_give(TaskSemQueue);
                return RK_ERROR;
            }
        }

    }
}


/*******************************************************************************
** Name: RKTaskCreate
** Input:uint32 TaskClassID, uint32 TaskObjectID, void * arg, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:15:06
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API rk_err_t RKTaskCreate(uint32 TaskClassID, uint32 TaskObjectID, void * arg, uint32 Mode)
{
    RK_TASK_QUEUE_ITEM TaskItem;
    RK_TASK_CLASS * pstNewTask = NULL;
    uint8 name[16], i;
    uint8 * buf;
    SEGMENT_INFO_T  Segment;
    uint32 LBA, offset;
    rk_err_t ret;

    if (TaskClassID > TASK_ID_NUM)
        return RK_ERROR;

    pstNewTask = (RK_TASK_CLASS *) rkos_memory_malloc(sizeof(RK_TASK_CLASS));

    if (pstNewTask == NULL)
        return RK_ERROR;
	//rk_printf(">>>>>>>RKTaskCreate 111\n");

    RKGetTaskMsg(TaskClassID, pstNewTask);

    FW_GetSegmentInfo(SEGMENT_ID_TASK_INF,&Segment);//rk_printf(">>>>>>>RKTaskCreate 222\n");

    LBA = ((uint32)pstNewTask->taskname - Segment.CodeImageBase - sizeof(InitTaskItem)) / 512;
    offset = ((uint32)pstNewTask->taskname - Segment.CodeImageBase - sizeof(InitTaskItem)) % 512;

    buf = rkos_memory_malloc(1024);

    FW_ReadFirmwaveByByte(Segment.CodeLoadBase + sizeof(InitTaskItem) + LBA * 512, buf, 1024);

    pstNewTask->TaskObjectID = TaskObjectID;
    i = StrLenA(&buf[offset]) > 12? 12: StrLenA(&buf[offset]);
    memcpy(name, &buf[offset], i);

    rkos_memory_free(buf);

    name[i] = '//';
    i = Num2String(name + i + 1, pstNewTask->TaskObjectID);

    pstNewTask->taskname = name;


    if(Mode == DIRECT_MODE)
    {
        rkos_semaphore_take(TaskSemOper, MAX_DELAY);
        if(RKTaskFind(TaskClassID, TaskObjectID) != NULL)
        {
            rk_printf("1exist same task id = %d / %d", TaskClassID, TaskObjectID);
            rkos_memory_free(pstNewTask);
            rkos_semaphore_give(TaskSemOper);
            return RK_ERROR;
        }
        ret = RKTaskCreateOperating(pstNewTask,  arg);
        rkos_semaphore_give(TaskSemOper);
        return ret;
    }
    else
    {
        if(RKTaskFind(TaskClassID, TaskObjectID) != NULL)
        {
            rk_printf("2exist same task id = %d / %d", TaskClassID, TaskObjectID);
            rkos_memory_free(pstNewTask);
            return RK_ERROR;
        }

        if(Mode == ASYNC_MODE)
        {
            TaskItem.TaskHandler = pstNewTask;
            TaskItem.RkTaskEvent = RK_TASK_EVENT_CREATE;
            TaskItem.Mode = Mode;
            TaskItem.TaskArg = arg;
            rkos_queue_send(TaskAskQueue, &TaskItem, MAX_DELAY);
            return RK_SUCCESS;
        }
        else if(Mode == SYNC_MODE)
        {//rk_printf(">>>>>>>RKTaskCreate 333\n");
            TaskItem.TaskHandler = pstNewTask;
            TaskItem.RkTaskEvent = RK_TASK_EVENT_CREATE;
            TaskItem.Mode = Mode;
            TaskItem.TaskArg = arg;

            rkos_semaphore_take(TaskSemQueue, MAX_DELAY);

            rkos_queue_send(TaskAskQueue, &TaskItem, MAX_DELAY);

            rkos_queue_receive(TaskRespQueue, &TaskItem, MAX_DELAY);

            if(TaskItem.RkTaskEvent == RK_TASK_EVENT_CREATE_SUCCESS)
            {
                rkos_semaphore_give(TaskSemQueue);
                return RK_SUCCESS;
            }
            else
            {
                rkos_semaphore_give(TaskSemQueue);
                return RK_ERROR;
            }
        }

    }
}

/*******************************************************************************
** Name: RKTaskFind
** Input:uint32 TaskClassID, uint32 TaskObjectID
** Return: HTC
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:08:49
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON API HTC RKTaskFind(uint32 TaskClassID, uint32 TaskObjectID)
{
    RK_TASK_CLASS * pTask;

    if (TaskListHead == NULL)
    {
        return NULL;
    }
    else
    {
        pTask = TaskListHead;
        while (pTask != NULL)
        {
            if ((pTask->TaskClassID == TaskClassID) && (pTask->TaskObjectID == TaskObjectID))
            {
                return pTask;
            }
            pTask = pTask->NextTCB;
        }
    }
    return NULL;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: RKTaskIdleCheck
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.7.28
** Time: 16:29:11
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN void RKTaskIdleCheck(void)
{
    RK_TASK_CLASS * pTask;
    uint32 es;
    es = 1;
    pTask = TaskListHead;
    pTask = pTask->NextTCB;
    pTask = pTask->NextTCB;
    pTask = pTask->NextTCB;

    TaskTotalSuspendCnt = 0;

    while (pTask != NULL)
    {
        pTask->IdleTick += PM_TIME;

        if(pTask->State != TASK_STATE_WORKING)
        {
            TaskTotalSuspendCnt++;
        }

        if(es)
        {
           if((pTask->IdleTick >= pTask->Idle1EventTime)
                && (pTask->State == TASK_STATE_WORKING))
            {
                 //suspent this device
                if(pTask->TaskSuspendFun != NULL)
                {
                    if(pTask->TaskSuspendFun((void *)pTask, TASK_STATE_IDLE1) != RK_SUCCESS)
                    {
                        rk_printf("idle1 suspend fail = %s",pcTaskGetTaskName(pTask->hTask));
                    }
                    else
                    {
                        rk_printf("idle1 %s suspend ok",pcTaskGetTaskName(pTask->hTask));
                        es = 0;
                    }
                }
            }
            #if 0
            else if((pDev->IdleTick >= pDev->Idle2EventTime)
                && (pDev->State == DEV_STATE_IDLE1))
            {
                 //suspent this device
                if(pDev->suspend != NULL)
                {
                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_LoadSegment(DevInfo_Table[pDev->DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                            #endif
                        }
                    }

                    if(pDev->suspend(pDev, DEV_SATE_IDLE2) != RK_SUCCESS)
                    {
                        rk_printf("idle3 suspend fail = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }
                    else
                    {
                        rk_printf("idle3 %s[%d] suspend ok",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }

                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_RemoveSegment(DevInfo_Table[pDev->DevClassID].SegmentID);
                            #endif
                        }
                    }
                }
                es = 0;
            }
            #endif
        }

        pTask = pTask->NextTCB;
    }
}

/*******************************************************************************
** Name: vApplicationStackOverflowHook
** Input:xTaskHandle xTask,
** Return: void
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:30:07
*******************************************************************************/
extern void IntDefaultHandler_displaytasklist (void);
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{

    RK_TASK_CLASS * pTask =  TaskListHead;

    while (pTask != NULL)
    {
        if (pTask->hTask == (HTC)xTask)
        {
            printf("task ID = %d, stack overflow\n", pTask->TaskClassID);
        }

        pTask = pTask->NextTCB;
    }
    printf("pcTaskName = %s\n", pcTaskName);
    IntDefaultHandler_displaytasklist();
    while(1);
}

/*******************************************************************************
** Name: OSSchedulerTask
** Input:void * arg
** Return: void
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:28:16
*******************************************************************************/
extern xTaskHandle xIdleTaskHandle;
extern xTaskHandle xTimerTaskHandle;

_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN void OSSchedulerTask(void * arg)
{
    RK_TASK_QUEUE_ITEM TempTaskQueueItem;
    RK_TASK_CLASS * pstNewTask;

    {
        extern UINT32 __MRS_MSP(void);
        MspSave = __MRS_MSP();
    }

    pstNewTask = (RK_TASK_CLASS *) rkos_memory_malloc(sizeof(RK_TASK_CLASS));
    RKGetTaskMsg(TASK_ID_IDLE,pstNewTask);
    pstNewTask->hTask = xIdleTaskHandle;
    pstNewTask->TaskObjectID = 0;
    RKTaskRegister(pstNewTask);

    pstNewTask = (RK_TASK_CLASS *) rkos_memory_malloc(sizeof(RK_TASK_CLASS));
    RKGetTaskMsg(TASK_ID_SYS_TIMER,pstNewTask);
    pstNewTask->hTask = xTimerTaskHandle;
    pstNewTask->TaskObjectID = 0;
    RKTaskRegister(pstNewTask);

    pstNewTask = (RK_TASK_CLASS *) rkos_memory_malloc(sizeof(RK_TASK_CLASS));
    RKGetTaskMsg(TASK_ID_TASK_MANAGER, pstNewTask);
    pstNewTask->hTask = TaskManagerHandler;
    pstNewTask->TaskObjectID = 0;
    RKTaskRegister(pstNewTask);

    pstNewTask = (RK_TASK_CLASS *) rkos_memory_malloc(sizeof(RK_TASK_CLASS));
    RKGetTaskMsg(TASK_ID_DEVICE_TASK,pstNewTask);
    pstNewTask->taskname = "dm";
    pstNewTask->TaskObjectID = 0;
    RKTaskCreateOperating(pstNewTask, NULL);

    while(1)
    {
        rkos_queue_receive(TaskAskQueue, &TempTaskQueueItem, MAX_DELAY);

        if (TempTaskQueueItem.RkTaskEvent == RK_TASK_EVENT_DELETE)
        {
            rkos_semaphore_take(TaskSemOper, MAX_DELAY);

            if(RKTaskFind(TempTaskQueueItem.TaskHandler->TaskClassID, TempTaskQueueItem.TaskHandler->TaskObjectID) == NULL)
            {
                rk_printf("3task not find id = %d / %d",TempTaskQueueItem.TaskHandler->TaskClassID, TempTaskQueueItem.TaskHandler->TaskObjectID );
                TempTaskQueueItem.RkTaskEvent = RK_TASK_EVENT_DELETE_FAIL;
            }
            else
            {
                if(RKTaskDeleteOperating(TempTaskQueueItem.TaskHandler) == RK_SUCCESS)
                {
                    TempTaskQueueItem.RkTaskEvent = RK_TASK_EVENT_DELETE_SUCCESS;
                }
                else
                {
                    TempTaskQueueItem.RkTaskEvent = RK_TASK_EVENT_DELETE_FAIL;
                }
            }

            rkos_semaphore_give(TaskSemOper);

            if(TempTaskQueueItem.Mode == SYNC_MODE)
            {
                rkos_queue_send(TaskRespQueue, &TempTaskQueueItem, MAX_DELAY);
            }
        }
        else if (TempTaskQueueItem.RkTaskEvent == RK_TASK_EVENT_CREATE)
        {
            rkos_semaphore_take(TaskSemOper, MAX_DELAY);

            if(RKTaskFind(TempTaskQueueItem.TaskHandler->TaskClassID, TempTaskQueueItem.TaskHandler->TaskObjectID) != NULL)
            {
                rk_printf("3exist same task id = %d / %d",TempTaskQueueItem.TaskHandler->TaskClassID, TempTaskQueueItem.TaskHandler->TaskObjectID );
                TempTaskQueueItem.RkTaskEvent = RK_TASK_EVENT_CREATE_FAIL;
            }
            else
            {
                if(RKTaskCreateOperating(TempTaskQueueItem.TaskHandler,  TempTaskQueueItem.TaskArg) == RK_SUCCESS)
                {
                    TempTaskQueueItem.RkTaskEvent = RK_TASK_EVENT_CREATE_SUCCESS;
                }
                else
                {
                    TempTaskQueueItem.RkTaskEvent = RK_TASK_EVENT_CREATE_FAIL;
                }
            }

            rkos_semaphore_give(TaskSemOper);

            if(TempTaskQueueItem.Mode == SYNC_MODE)
            {
                rkos_queue_send(TaskRespQueue, &TempTaskQueueItem, MAX_DELAY);
            }
        }
        else if(TempTaskQueueItem.RkTaskEvent == RK_TASK_EVENT_IDLE_TICK)
        {
            RKTaskIdleCheck();
        }

    }

}
/*******************************************************************************
** Name: RKTaskDeleteOperating
** Input:RK_TASK_CLASS * TaskHandler
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:24:30
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN rk_err_t RKTaskDeleteOperating(RK_TASK_CLASS * TaskHandler)
{
    HTC hTask;

    rk_printf("delete thread classId = %d, objectid = %d, remain = %d", TaskHandler->TaskClassID, TaskHandler->TaskObjectID, rkos_GetFreeHeapSize());
    RKTaskDeRegister(TaskHandler);

    if(TaskHandler->TaskDeInitFun != NULL)
    {
        if(TaskHandler->TaskDeInitFun(NULL) !=  RK_SUCCESS)
        {
            if(TaskHandler->OverlayModule != NULL)
            {
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(TaskHandler->OverlayModule);
                #endif
            }

            return RK_ERROR;
        }
    }

    hTask = TaskHandler->hTask;
    rkos_task_delete(hTask);                //É¾³ýÈÎÎñ
    rkos_memory_free(TaskHandler);

    if(TaskHandler->OverlayModule != NULL)
    {
        #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
        FW_RemoveSegment(TaskHandler->OverlayModule);
        #endif
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: RKTaskCreateOperating
** Input:RK_TASK_CLASS * TaskHandler, void * arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:19:54
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN rk_err_t RKTaskCreateOperating(RK_TASK_CLASS * TaskHandler, void * arg)
{
    if (TaskHandler->OverlayModule != NULL)
    {
        #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
        if(FW_LoadSegment(TaskHandler->OverlayModule, SEGMENT_OVERLAY_ALL) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
        #endif
    }

    if(TaskHandler->TaskInitFun != NULL)
    {
        if (TaskHandler->TaskInitFun(TaskHandler, arg) != RK_SUCCESS)
        {
            if(TaskHandler->OverlayModule != NULL)
            {
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(TaskHandler->OverlayModule);
                #endif
            }
            return RK_ERROR;
        }
    }

    rk_printf("create thread classId = %d, objectid = %d,remain = %d", TaskHandler->TaskClassID, TaskHandler->TaskObjectID, rkos_GetFreeHeapSize());
    TaskHandler->hTask = rkos_task_create(TaskHandler->TaskFun,TaskHandler->taskname, NULL, TaskHandler->TaskStackSize, TaskHandler->TaskPriority, arg);

    if ((int32)TaskHandler->hTask > 0)
    {
        if (RKTaskRegister(TaskHandler) ==  RK_ERROR)
        {
            if(TaskHandler->OverlayModule != NULL)
            {
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(TaskHandler->OverlayModule);
                #endif
            }

            return RK_ERROR;
        }
        else
        {
            return RK_SUCCESS;
        }
    }
    else
    {
        rkos_memory_free(TaskHandler);
        if(TaskHandler->OverlayModule != NULL)
        {
            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
            FW_RemoveSegment(TaskHandler->OverlayModule);
            #endif
        }
        return RK_ERROR;

    }
}
/*******************************************************************************
** Name: RKTaskDeRegister
** Input:RK_TASK_CLASS * TaskHandle
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:05:33
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN rk_err_t RKTaskDeRegister(RK_TASK_CLASS * TaskHandle)
{
    RK_TASK_CLASS * TempTaskHandle;

    rkos_enter_critical();
    if (TaskListHead == NULL)
    {
        rkos_exit_critical();
        return RK_ERROR;
    }

    TempTaskHandle = TaskListHead;

    if(TaskListHead == TaskHandle)
    {
        TaskListHead = NULL;
    }

    while (TempTaskHandle->NextTCB != NULL)
    {
        if (TempTaskHandle->NextTCB == TaskHandle)
            break;
        TempTaskHandle = TempTaskHandle->NextTCB;
    }

    if (TempTaskHandle->NextTCB != NULL)
    {
        TempTaskHandle->NextTCB = TaskHandle->NextTCB;
        rkos_exit_critical();
        TaskTotalCnt--;
        return RK_SUCCESS;
    }
    else
    {
        rkos_exit_critical();
        return RK_ERROR;
    }
}


/*******************************************************************************
** Name: RKTaskRegister
** Input:RK_TASK_CLASS * TaskHandler
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:04:14
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN rk_err_t RKTaskRegister(RK_TASK_CLASS * TaskHandler)
{
    RK_TASK_CLASS * TempTaskHandler;
    if (TaskHandler == NULL)
    {
        return RK_ERROR;
    }

    rkos_enter_critical();
    if (TaskListHead == NULL)
    {
        TaskListHead = TaskHandler;
        TaskListHead->NextTCB = NULL;
    }
    else
    {

        TempTaskHandler = TaskListHead;
        while (TempTaskHandler->NextTCB != NULL)
            TempTaskHandler = TempTaskHandler->NextTCB;
        TempTaskHandler->NextTCB = TaskHandler;
        TaskHandler->NextTCB = NULL;
    }
    rkos_exit_critical();

    TaskTotalCnt++;
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: RKGetTaskMsg
** Input:uint32 TaskClassID,RK_TASK_CLASS *pTaskItem
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 16:59:11
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_COMMON_
COMMON FUN rk_err_t RKGetTaskMsg(uint32 TaskClassID,RK_TASK_CLASS *pTaskItem)
{
    SEGMENT_INFO_T  Segment;
    uint8 * buf;
    uint32 LBA, offset;

    if(TaskClassID == TASK_ID_DEVICE_TASK)
    {
        pTaskItem->TaskInitFun   = DeviceTaskInit;
        pTaskItem->TaskDeInitFun = DeviceTaskDeInit;
        pTaskItem->TaskFun       = DeviceTask;
        pTaskItem->TaskSuspendFun= NULL;
        pTaskItem->TaskResumeFun = NULL;
        pTaskItem->hTask         = NULL;
        pTaskItem->OverlayModule = NULL;
        pTaskItem->TaskFlag      = NULL;
        pTaskItem->TaskPriority  = TASK_PRIORITY_DEVICE_MANAGER;
        pTaskItem->TaskStackSize = MAXIMAL_STACK_SIZE * 2;
        pTaskItem->TaskClassID   = TASK_ID_DEVICE_TASK;
        pTaskItem->State = TASK_STATE_WORKING;
        return RK_SUCCESS;
    }
    else if(TaskClassID == TASK_ID_IDLE)
    {
        pTaskItem->TaskInitFun   = 0;
        pTaskItem->TaskDeInitFun = 0;
        pTaskItem->TaskFun       = 0;
        pTaskItem->TaskSuspendFun= 0;
        pTaskItem->TaskResumeFun = 0;
        pTaskItem->hTask         = 0;
        pTaskItem->OverlayModule = 0;
        pTaskItem->TaskFlag      = 0;
        pTaskItem->TaskPriority  = tskIDLE_PRIORITY;
        pTaskItem->TaskStackSize = configMINIMAL_STACK_SIZE;
        pTaskItem->TaskClassID   = TASK_ID_IDLE;
        pTaskItem->State = TASK_STATE_WORKING;
    }
    else if(TaskClassID == TASK_ID_SYS_TIMER)
    {
        pTaskItem->TaskInitFun   = 0;
        pTaskItem->TaskDeInitFun = 0;
        pTaskItem->TaskFun       = 0;
        pTaskItem->TaskSuspendFun= 0;
        pTaskItem->TaskResumeFun = 0;
        pTaskItem->hTask         = 0;
        pTaskItem->OverlayModule = 0;
        pTaskItem->TaskFlag      = 0;
        pTaskItem->TaskPriority  = TASK_PRIORITY_SYSTICK;
        pTaskItem->TaskStackSize = configMINIMAL_STACK_SIZE*2;
        pTaskItem->TaskClassID   = TASK_ID_SYS_TIMER;
        pTaskItem->State = TASK_STATE_WORKING;
    }
    else if(TaskClassID == TASK_ID_TASK_MANAGER)
    {
        pTaskItem->TaskInitFun   = 0;
        pTaskItem->TaskDeInitFun = 0;
        pTaskItem->TaskFun       = 0;
        pTaskItem->TaskSuspendFun= 0;
        pTaskItem->TaskResumeFun = 0;
        pTaskItem->hTask         = 0;
        pTaskItem->OverlayModule = 0;
        pTaskItem->TaskFlag      = 0;
        pTaskItem->TaskPriority  = TASK_PRIORITY_TASK_MANAGER;
        pTaskItem->TaskStackSize = configSYS_SERVICE_STACK_SIZE;
        pTaskItem->TaskClassID   = TASK_ID_TASK_MANAGER;
        pTaskItem->State = TASK_STATE_WORKING;
    }
    else if (TaskClassID < TASK_ID_NUM)
    {
        LBA = TaskClassID * sizeof(RK_TASK_CLASS) / 512;
        offset = TaskClassID * sizeof(RK_TASK_CLASS) % 512;
        FW_GetSegmentInfo(SEGMENT_ID_TASK_INF,&Segment);
        buf = rkos_memory_malloc(1024);
        FW_ReadFirmwaveByByte(Segment.CodeLoadBase + LBA * 512, buf, 1024);
        memcpy(pTaskItem,&buf[offset],sizeof(RK_TASK_CLASS));
        rkos_memory_free(buf);

        pTaskItem->State = TASK_STATE_WORKING;

        return RK_SUCCESS;
    }
    return RK_ERROR;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SchedulerTaskCreate
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:27:22
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_INIT_
INIT API void SchedulerTaskCreate(void)
{
    TaskManagerHandler = rkos_task_create(OSSchedulerTask, "taskm", NULL, 512, TASK_PRIORITY_TASK_MANAGER, NULL);
}

/*******************************************************************************
** Name: RKTaskListInitAll
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 16:54:16
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_INIT_
INIT FUN rk_err_t RKTaskInit(void)
{
    TaskAskQueue = rkos_queue_create(TASK_QUEUE_LENTH,TASK_QUEUE_SIZE);
    TaskRespQueue = rkos_queue_create(1,TASK_QUEUE_SIZE);
    TaskSemQueue = rkos_semaphore_create(1, 1);
    TaskSemOper = rkos_semaphore_create(1, 1);

    TaskTotalCnt = 0;
    TaskTotalSuspendCnt = 0;
    TaskListHead = NULL;

    return RK_SUCCESS;
}

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
/*******************************************************************************
** Name: RKTaskTime
** Input:HTC hTask
** Return: uint32
** Owner:aaron.sun
** Date: 2016.5.31
** Time: 15:09:18
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskTime(HTC hTask)
{
    uint32 time;
    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;
    time =  vTaskGetRunTime(p->hTask);
    time = (time * 100) / SysTickCounter;
    return time;
}
/*******************************************************************************
** Name: RKTaskGetTaskName
** Input:uint8 * taskname
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.19
** Time: 18:30:38
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API void * RKTaskGetTaskName(HTC hTask)
{
    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;
    return pcTaskGetTaskName(p->hTask);
}

/*******************************************************************************
** Name: RKTaskGetNextHandle
** Input:HTC hTask
** Return: HTC
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:14:08
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API HTC RKTaskGetNextHandle(HTC hTask, uint32 TaskClassID)
{
    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;

    if(TaskClassID == 0xffffffff)
    {
        return p->NextTCB;
    }
}

/*******************************************************************************
** Name: RKTaskGetHandle
** Input:uint32 index
** Return: HTC
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:12:00
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API HTC RKTaskGetFirstHandle(uint32 TaskClassID)
{
    if(TaskClassID == 0xffffffff)
    {
        return TaskListHead;
    }
}

/*******************************************************************************
** Name: RKTaskGetTotalCnt
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:11:29
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskGetTotalCnt(uint32 TaskClassID)
{

}

/*******************************************************************************
** Name: RKTaskHeapTotal
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:01:45
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskHeapTotal(void)
{
    return configTOTAL_APP_HEAP_SIZE;
}

/*******************************************************************************
** Name: RKTaskHeapFree
** Input:void
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 18:00:21
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskHeapFree(void)
{
    return rkos_GetFreeHeapSize();
}

/*******************************************************************************
** Name: RKTaskStackSize
** Input:HTC hTask
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 17:58:05
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskStackSize(HTC hTask)
{
    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;

    return (p->TaskStackSize * 4);
}

/*******************************************************************************
** Name: RKTaskStackRemain
** Input:HTC hTask
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 17:50:13
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskStackRemain(HTC hTask)
{
    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;
    return (uxTaskGetStackHighWaterMark(p->hTask) * sizeof(portSTACK_TYPE));
}

/*******************************************************************************
** Name: RKTaskState
** Input:HTC hTask
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 17:48:49
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskState(HTC hTask)
{
    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;
    return eTaskGetState(p->hTask);
}

/*******************************************************************************
** Name: RKTaskStackBaseAddr
** Input:HTC hTask
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.16
** Time: 17:46:07
*******************************************************************************/
_OS_PLUGIN_TASKPLUGIN_SHELL_
SHELL API uint32 RKTaskStackBaseAddr(HTC hTask)
{
    tskTCB *pxTCB;
    uint32 ucReturn;

    RK_TASK_CLASS * p = (RK_TASK_CLASS * )hTask;

    pxTCB = p->hTask;

#if portSTACK_GROWTH < 0
    {
        ucReturn = (uint32) pxTCB->pxStack;
    }
#else
    {
        ucReturn = (uint32) pxTCB->pxEndOfStack;
    }
#endif

    return ucReturn;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#endif
