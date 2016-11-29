/*
********************************************************************************************
*
*                  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Driver\DeviceInclude.h
* Owner: Aaron.sun
* Date: 2014.5.28
* Time: 14:41:35
* Desc: Device Driver include
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    Aaron.sun     2014.5.28     14:41:35   1.0
********************************************************************************************
*/

#ifndef __DRIVER_DEVICEINCLUDE_H__
#define __DRIVER_DEVICEINCLUDE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _RADIO_
#include "FmDevice.h"
#endif
#include "PwmDevice.h"
#include "KeyDevice.h"
#include "AdcDevice.h"
#include "TimerDevice.h"
#include "SpiFlashDev.h"

#include "BcoreDevice.h"
#include "MsgDevice.h"
#ifdef _FS_
#include "DirDevice.h"
#include "PartionDevice.h"
#include "FileDevice.h"
#include "FATDevice.h"
#endif
#include "RockCodecDevice.h"
#include "I2sDevice.h"
#include "AudioDevice.h"
#ifdef _USB_
#include "UsbMscDevice.h"
#include "UsbOtgDev.h"
#endif
#include "MailBoxDevice.h"
#include "SdDevice.h"

#include "SpiDevice.h"
#include "UartDevice.h"
#include "FIFODevice.h"
#include "DeviceFormat.h"
#include "EmmcDevice.h"
#include "SdMmcDevice.h"
#include "LUNDevice.h"
#include "I2cDevice.h"
#include "RockCodecDevice.h"
#include "DmaDevice.h"
#include "SdioDevice.h"
#include "UartDevice.h"
#include "WatchDogDevice.h"
#ifdef _USE_GUI_
#include "VopDevice.h"
#include "LCDDriver.h"
#include "DisplayDevice.h"
#endif
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

