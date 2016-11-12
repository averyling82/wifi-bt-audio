/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\MSG\MsgDevice.h
* Owner: aaron.sun
* Date: 2015.6.23
* Time: 15:58:38
* Version: 1.0
* Desc: Msg Device Class
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.23     15:58:38   1.0
********************************************************************************************
*/


#ifndef __DRIVER_MSG_MSGDEVICE_H__
#define __DRIVER_MSG_MSGDEVICE_H__

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
typedef  struct _MSG_DEV_ARG
{
    uint32 Channel;

}MSG_DEV_ARG;

#define _DRIVER_MSG_MSGDEVICE_COMMON_  __attribute__((section("driver_msg_msgdevice_common")))
#define _DRIVER_MSG_MSGDEVICE_INIT_  __attribute__((section("driver_msg_msgdevice_init")))
#define _DRIVER_MSG_MSGDEVICE_SHELL_  __attribute__((section("driver_msg_msgdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_MSG_MSGDEVICE_SHELL_DATA_      _DRIVER_MSG_MSGDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_MSG_MSGDEVICE_SHELL_DATA_      __attribute__((section("driver_msg_msgdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

typedef enum
{
    //MAIN TASK message
    MAINTASK_BT_START,
    MAINTASK_APP_DLNA_PLAYER_START,
    MAINTASK_APP_XXX_PLAYER_START,
    MAINTASK_BT_OK,
    MAINTASK_WIFI_OPEN_OK,
    MAINTASK_WIFI_CONNECT_OK,
    MAINTASK_WIFI_CONNECTING,
    MAINTASK_WIFI_SUSPEND,
    MAINTASK_WIFI_AP_SUSPEND,
    MAINTASK_WIFI_AP_OPEN_OK,
    MAINTASK_WIFI_AP_CONNECT_OK,
    MAINTASK_CHARGE,
    MAINTASK_APP_USB_START,
    MAINTASK_APP_USB_OK,
    MAINTASK_BATTERY,
    MAINTASK_BATTERY_LEVEL,
    MAINTASK_APP_PLAYMENU,
    MAINTASK_APP_LOCAL_PLAYER,
    MAINTASK_APP_DLNA_PLAYER,
    MAINTASK_APP_BT_PLAYER,
    MAINTASK_APP_XXX_PLAYER,
    MAINTASK_APP_CHANNEL_PLAYER,
    MAINTASK_APP_RECORD,
    MAINTASK_APP_SYSTEMSET,
    MAINTASK_APP_BROWER,
    MAINTASK_WIFICONFIG,
    MAINTASK_APP_FMUI,
    MAINTASK_APP_LINE_IN,
    MAINTASK_SYS_MEDIA_UPDATE,
    MAINTASK_SYS_READY_SD,
    MAINTASK_APP_MEDIA_BROWSER,
    MAINTASK_APP_MEDIA_LIBRAY,
    MAINTASK_APP_PLAY_WINDOW,
    MAINTASK_SYS_UPDATE_FW,
    MSGDEV_MAX

}MSGDEV_ID;

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
extern rk_err_t MsgDev_ClearMsg(HDC dev, uint32 MsgID);
extern rk_err_t MsgDev_CheckMsg(HDC dev,  uint32 MsgID);
extern rk_err_t MsgDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t MsgDev_SendMsg(HDC dev, uint32 MsgID);
extern rk_err_t MsgDev_GetMsg(HDC dev, uint32 MsgID);
extern rk_err_t MsgDev_Delete(uint32 DevID, void * arg);
extern HDC MsgDev_Create(uint32 DevID, void * arg);



#endif
