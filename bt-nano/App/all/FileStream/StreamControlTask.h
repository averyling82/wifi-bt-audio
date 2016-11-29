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
