/*
********************************************************************************************
*
*                  Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\AppInclude.h
* Owner: Aaron.sun
* Date: 2014.5.28
* Time: 14:51:37
* Desc: App include
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    Aaron.sun     2014.5.28     14:51:37   1.0
********************************************************************************************
*/

#ifndef __APP_APPINCLUDE_H__
#define __APP_APPINCLUDE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _LINEIN_
#include "LineInTask.h"
#include "LineInControlTask.h"
#endif
#ifdef _RADIO_
#include "FmUiTask.h"
#include "FmControlTask.h"
#endif
#ifdef _BROWSER_
#include "BrowserUITask.h"
#endif
#ifdef _SYSSET_
#include "SystemSetWifiSelectTask.h"
#include "SystemSetTask.h"
#endif
#include "main_task.h"
#include "dlna.h"
#ifdef _WIFI_
#include "wifithread.h"
#endif
#include "http.h"
#ifdef _USE_GUI_
#include "GUITask.h"
#include "chargetask.h"
#endif
#ifdef _MUSIC_
#ifdef _USE_GUI_
#include "play_menu_task.h"
#else
#ifdef NOSCREEN_OPEN
#include "play_menu_task.h"
#endif
#endif
#endif
#ifdef _RECORD_
#include "recod_task.h"
#include "RecordControlTask.h"
#endif
#ifdef _MEDIA_MODULE_
#include "media_update.h"
#include "media_library.h"
#include "media_browser.h"
#endif
#include "StreamControlTask.h"
#include "AudioControlTask.h"
#include "MediaTask.h"
#include "FileStreamTask.h"
#include "TaskFormat.h"
#ifdef _BLUETOOTH_
#include "BlueToothControl.h"
#endif
#include "ping.h"
#include "ip_shell.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

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

