/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\App\FileStream\FileStreamTask.h
* Owner: Aaron.sun
* Date: 2014.5.7
* Time: 17:43:30
* Desc: File Stream
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.5.7     17:43:30   1.0
********************************************************************************************
*/

#ifndef __APP_FILESTREAM_FILESTREAMTASK_H__
#define __APP_FILESTREAM_FILESTREAMTASK_H__

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
#define _APP_FILESTREAM_FILESTREAMTASK_COMMON_  __attribute__((section("app_filestream_filestreamtask_common")))
#define _APP_FILESTREAM_FILESTREAMTASK_INIT_  __attribute__((section("app_filestream_filestreamtask_common")))

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
extern void FileStreamTask_Enter(void);
extern rk_err_t FileStreamTask_DevInit(void *pvParameters);
extern rk_err_t FileStreamTask_Init(void *pvParameters);



#endif

