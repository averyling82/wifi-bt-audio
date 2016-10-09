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
typedef enum _MEDIA_MSGBOX_FILE_CMD
{
    MEDIA_MSGBOX_CMD_FILE_NULL = 100,

    MEDIA_MSGBOX_CMD_FILE_SEEK,
    MEDIA_MSGBOX_CMD_FILE_SEEK_CMPL,

    MEDIA_MSGBOX_CMD_FILE_READ,
    MEDIA_MSGBOX_CMD_FILE_READ_CMPL,

    MEDIA_MSGBOX_CMD_FILE_WRITE,
    MEDIA_MSGBOX_CMD_FILE_WRITE_CMPL,

    MEDIA_MSGBOX_CMD_FILE_TELL,
    MEDIA_MSGBOX_CMD_FILE_TELL_CMPL,

    MEDIA_MSGBOX_CMD_FILE_GET_LENGTH,
    MEDIA_MSGBOX_CMD_FILE_GET_LENGTH_CMPL,

}MEDIA_MSGBOX_FILE_CMD;



typedef enum _FILE_STREAM_CMD
{
    FILE_STREAM_CMD_OPEN,
    FILE_STREAM_CMD_CLOSE,
    FILE_STREAM_CMD_SEEK,
    FILE_STREAM_CMD_GET,
    FILE_STREAM_CMD_LOSE,

    FILE_STREAM_CMD_SET_SEARCH_AREA,
    FILE_STREAM_CMD_SET_SEARCH_MODE,
    FILE_STREAM_CMD_SEARCH_NEXT,
    FILE_STREAM_CMD_SEARCH_PREV,

    FILE_STREAM_CMD_NUM

}FILE_STREAM_CMD;

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

