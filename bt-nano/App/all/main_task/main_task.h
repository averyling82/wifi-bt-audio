/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\main_task\main_task.h
* Owner: aaron.sun
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: main task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.8.20     17:29:00   1.0
********************************************************************************************
*/


#ifndef __APP_MAIN_TASK_MAIN_TASK_H__
#define __APP_MAIN_TASK_MAIN_TASK_H__

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

typedef struct _RK_TASK_MAIN_ARG
{
    uint32 xxx;

}RK_TASK_MAIN_ARG;

#define _APP_MAIN_TASK_MAIN_TASK_COMMON_  __attribute__((section("app_main_task_main_task_common")))
#define _APP_MAIN_TASK_MAIN_TASK_MENU_  __attribute__((section("app_main_task_main_task_menu")))
#define _APP_MAIN_TASK_MAIN_TASK_INIT_    __attribute__((section("app_main_task_main_task_common")))
#define _APP_MAIN_TASK_MAIN_TASK_SHELL_   __attribute__((section("app_main_task_main_task_shell")))


typedef enum _MAIN_TASK_EVENT
{
    MAINTASK_SYS_EVENT,
    MAINTASK_KEY_EVENT,
    MAINTASK_AUDIO_EVENT,
    MAINTASK_TASK_SWTICH,
    MAINTASK_USBCREATE,
    MAINTASK_USBDELETE,
    MAINTASK_SHUTDOWN,
    MAINTASK_AUDIO_ERROR,
    MAINTASK_SUSPEND_WIFI,
    MAINTASK_RESUME_WIFI,
    MAINTASK_EVENT_MAX

}MAIN_TASK_EVENT;


typedef  struct _TASK_SWTICH_ARG
{
   uint32 CurClassId;
   uint32 CurObjectId;
   uint32 TargetClassId;
   uint32 TargetObjectId;
   void * targ;

}TASK_SWTICH_ARG;

typedef rk_err_t (*pKeyFunc)(uint32);

#ifdef NOSCREEN_USE_LED
#define MAINTASK_LED1          1
#define MAINTASK_LED2          2
#define MAINTASK_LED_ENABLE    1
#define MAINTASK_LED_ON        2
#define MAINTASK_LED_OFF       3
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
extern void      MainTask_Enter(void);
extern rk_err_t  MainTask_DeInit(void *pvParameters);
extern rk_err_t  MainTask_Init(void *pvParameters, void *arg);
extern rk_err_t  MainTask_SysEventCallBack(uint32 event, void*arg);
extern uint8     MainTask_GetStatus (uint32 StatusID);
extern uint8     MainTask_SetStatus (uint32 StatusID, uint8 status);
extern void      MainTask_AudioCallBack(uint32 audio_state);
extern int32     MainTask_SetTopIcon (uint32 StatusID);
extern int32     MainTask_DeleteTopIcon (uint32 StatusID);
extern int32     MainTask_TaskSwtich (uint32 CurClassId, uint32 CurObjectId, uint32 TargetClassId, uint32 TargetObjectId,  void *targ);
#ifndef _USE_GUI_
extern rk_err_t  MainTask_KeyCallBack(uint32 KeyValue);
extern rk_err_t  MainTask_RegisterKey(pKeyFunc func);
extern rk_err_t  MainTask_UnRegisterKey(void);
#ifdef NOSCREEN_USE_LED
extern uint8     MainTask_SetLED (int8 led,int8 led_state);
#endif
#endif
#endif
