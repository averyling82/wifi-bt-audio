/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Record\recod_task.h
* Owner: aaron.sun
* Date: 2016.3.11
* Time: 17:16:26
* Version: 1.0
* Desc: record ui
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.3.11     17:16:26   1.0
********************************************************************************************
*/


#ifndef __APP_RECORD_RECOD_TASK_H__
#define __APP_RECORD_RECOD_TASK_H__

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

#define _APP_RECORD_RECOD_TASK_COMMON_  __attribute__((section("app_record_recod_task_common")))
#define _APP_RECORD_RECOD_TASK_INIT_  __attribute__((section("app_record_recod_task_common")))
#define _APP_RECORD_RECOD_TASK_SHELL_  __attribute__((section("app_record_recod_task_shell")))
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
extern rk_err_t RecodTask_Init(void *pvParameters, void *arg);
extern rk_err_t RecodTask_DeInit(void *pvParameters);
extern void RecodTask_Enter(void);


#endif
