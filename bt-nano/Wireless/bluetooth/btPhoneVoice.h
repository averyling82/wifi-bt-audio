/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\App\Media\MediaTask.h
* Owner: Aaron.sun
* Date: 2014.5.7
* Time: 17:41:58
* Desc: Media Task
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.5.7     17:41:58   1.0
********************************************************************************************
*/

#include "bt_config.h"

#ifndef __BLUETOOTH_PHONE_VOICE_H__
#define __BLUETOOTH_PHONE_VOICE_H__

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

#ifdef _HFP_
#define __BLUETOOTH_PHONE_TASK__
#endif

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
extern rk_err_t BTPhoneVoiceTask_Init(void *pvParameters, void *arg);
extern rk_err_t BTPhoneVoiceTask_DeInit(void *pvParameters);
extern void BTPhoneVoiceTask_Enter(void);
extern void BTPhoneVoiceStreamNofity(uint8 *buf, uint32 len);
extern void VoiceNotifyService(void);
extern rk_err_t VoiceNotifyDeInit(void *pvParameters);
extern rk_err_t VoiceNotifyInit(void *pvParameters);

#endif


