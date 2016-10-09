/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: OS\Plugin\RKOS.h
* Owner: Aaron.sun
* Date: 2014.5.28
* Time: 14:52:29
* Desc: RKOS H
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    Aaron.sun     2014.5.28     14:52:29   1.0
********************************************************************************************
*/

#ifndef __OS_PLUGIN_RKOS_H__
#define __OS_PLUGIN_RKOS_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifndef NOT_INCLUDE_OTHER
#include "FreeRTOSConfig.h"
#include "typedef.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "DeviceManagerTask.h"
#include "ShellTask.h"
#include "UsbService.h"
#include "FwAnalysis.h"
#include "ModuleInfoTab.h"
#include "PowerManager.h"
#include "SysResume.h"
#include "DriverInclude.h"
#include "DeviceInclude.h"
#include "AppInclude.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "rk_fochannel.h"
#include "..\Resource\ImageResourceID.h"
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef void * pSemaphore;
typedef void * pQueue;
typedef void * pTimer;
typedef void (* pRkosTimerCallBack)(pTimer);
typedef void (* pRkosTaskCode)( void * );

#ifdef NOT_INCLUDE_OTHER
typedef void * HTC;
#endif

typedef  struct _MEMORY_BLOCK
{
    struct _MEMORY_BLOCK * pNext;
    uint32 hTask;
    uint32 hRkTask;
    uint32 size;
    uint32 flag;

}MEMORY_BLOCK;


typedef  struct _SEMAPHORE_QUEUE_BLOCK
{
    struct _SEMAPHORE_QUEUE_BLOCK * pNext;
    void * p;
    uint8 name[12];

}SEMAPHORE_QUEUE_BLOCK;


#define MAX_DELAY 0xffffffff

#define _OS_PLUGIN_RKOS_COMMON_ __attribute__((section("os_plugin_rkos_common")))
#define _OS_PLUGIN_RKOS_INIT_  __attribute__((section("os_plugin_rkos_init")))
#define _OS_PLUGIN_RKOS_SHELL_  __attribute__((section("os_plugin_rkos_shell")))

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern MEMORY_BLOCK * pFirstMemoryBlock;
extern uint32 UsedMemoryCnt;
extern SEMAPHORE_QUEUE_BLOCK * pFristSemaPhoreQueueBlock;
extern uint32 SemaphoreQueueCnt;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void * rkos_get_task_event(HTC hTask);
extern void rkos_memory_list_check(void);
extern uint32 rkos_GetFreeHeapSize(void);
extern void rkos_memory_check(void * buf);
extern void rkos_memory_malloc_display(uint32 display);
extern void rkos_task_delete(HTC hTask);
extern HTC rkos_task_create(pRkosTaskCode TaskCode,char *name, uint32 StatckBase, uint32 StackDeep, uint32 Priority, void * para);
extern rk_err_t rkos_queue_recive_fromisr(pQueue pQue, void * buf);
extern rk_err_t rkos_queue_send_fromisr(pQueue pQue, void * buf);
extern rk_err_t rkos_start_timer(pTimer timer);
extern rk_err_t rkos_stop_timer(pTimer timer);
void* rkos_get_timer_param(pTimer timer);
extern rk_err_t rkos_mod_timer(pTimer timer, int NewPeriod, int BlockTime);
extern void rkos_delete_timer(pTimer timer);
extern pTimer rkos_create_timer(uint32 period, uint32 reload,void* param, pRkosTimerCallBack pfCall);
extern void rkos_resume_all_task(void);
extern void rkos_suspend_all_task(void);
extern pQueue rkos_queue_create(uint32 blockcnt, uint32 blocksize);
extern void rkos_queue_delete(pQueue pQue);
extern rk_err_t rkos_queue_send(pQueue pQue, void * buf, uint32 time);
extern rk_err_t rkos_queue_receive(pQueue pQue, void * buf, uint32 time);
extern void rkos_semaphore_give_fromisr(pSemaphore pSem);
extern void rkos_start(void);
extern void rkos_init(void);
extern void rkos_memory_free(void * buf);
extern void * rkos_memory_malloc(uint32 size);
extern void * rkos_memory_realloc(void * pv, uint32 size);
extern void rkos_semaphore_delete(pSemaphore pSem);
extern pSemaphore rkos_semaphore_create(uint32 MaxCnt, uint32 InitCnt);
extern pSemaphore rkos_mutex_create();//yanghong add 2015/11/23
extern void rkos_semaphore_give(pSemaphore pSem);
extern rk_err_t rkos_semaphore_take(pSemaphore pSem, uint32 time);
extern void rkos_exit_critical(void);
extern void rkos_enter_critical(void);



#endif

