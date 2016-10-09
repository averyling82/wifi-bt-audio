/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\FileStream\StreamControlTask.h
* Owner: aaron.sun
* Date: 2015.6.18
* Time: 16:04:00
* Version: 1.0
* Desc: stream control task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.18     16:04:00   1.0
********************************************************************************************
*/


#ifndef __APP_FILESTREAM_STREAMCONTROLTASK_H__
#define __APP_FILESTREAM_STREAMCONTROLTASK_H__

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

typedef struct _RK_TASK_STREAMCONTROL_ARG
{
    uint32 xxx;
}RK_TASK_STREAMCONTROL_ARG;

#define _APP_FILESTREAM_STREAMCONTROLTASK_COMMON_  __attribute__((section("app_filestream_streamcontroltask_common")))
#define _APP_FILESTREAM_STREAMCONTROLTASK_INIT_  __attribute__((section("app_filestream_streamcontroltask_common")))
#define _APP_FILESTREAM_STREAMCONTROLTASK_SHELL_  __attribute__((section("app_filestream_streamcontroltask_shell")))
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
extern rk_err_t StreamControlTask_SendFileHandle(HDC hFile, uint32 FileNum);
extern rk_err_t StreamControlTask_Resume(void);
extern rk_err_t StreamControlTask_Suspend(void);
extern void StreamControlTask_Enter(void);
extern rk_err_t StreamControlTask_DeInit(void *pvParameters);
extern rk_err_t StreamControlTask_Init(void *pvParameters, void *arg);



#endif
