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

#ifndef __APP_MEDIA_MEDIATASK_H__
#define __APP_MEDIA_MEDIATASK_H__

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
typedef enum _MEDIA_CMD
{
    MEDIA_CMD_PLAY,
    MEDIA_CMD_STOP,
    MEDIA_CMD_FFW_START,
    MEDIA_CMD_FFW_END,
    MEDIA_CMD_FFD_START,
    MEDIA_CMD_FFD_END,
    MEDIA_CMD_SET_A,
    MEDIA_CMD_SET_AB,
    MEDIA_CMD_SET_TIME,
    MEDIA_CMD_SET_REC,
    MEDIA_CMD_NUM

}MEDIA_CMD;

typedef enum _MEDIDA_TYPE
{
    MEDIA_TYPE_PCM,
    MEDIA_TYPE_WAV,
    MEDIA_TYPE_MP3,
    MEDIA_TYPE_XXX,
    MEDIA_TYPE_AVI,
    MEDIA_TYPE_NUM

}MEDIDA_TYPE;

typedef enum _MEDIA_STREAM_FROM
{
    MEDIA_STREAM_FROM_WIFI,
    MEDIA_STREAM_FROM_BT,
    MEDIA_STREAM_FROM_FS,
    MEDIA_STREAM_NUM

}MEDIA_STREAM_FROM;

#define _APP_MEDIA_MEDIATASK_COMMON_  __attribute__((section("app_media_mediatask_common")))
#define _APP_MEDIA_MEDIATASK_INIT_  __attribute__((section("app_media_mediatask_common")))


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
extern rk_err_t MediaTask_GetStatus(void);
extern rk_err_t MediaTask_SetStreamFrom(uint32 StreamFrom);
extern void MediaTask_Enter(void);
extern rk_err_t MediaTask_DevInit(void *pvParameters);
extern rk_err_t MediaTask_Init(void *pvParameters);

#endif

