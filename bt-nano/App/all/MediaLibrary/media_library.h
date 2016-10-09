/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\music\media_library.h
* Owner: ctf
* Date: 2016.1.26
* Time: 17:29:00
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*      ctf     2016.1.26     17:29:00       1.0
********************************************************************************************
*/


#ifndef __APP_MEDIA_LIBRARY_H__
#define __APP_MEDIA_LIBRARY_H__

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
#define _APP_MEDIA_LIBRARY_COMMON_  __attribute__((section("app_media_task_common")))
#define _APP_MEDIA_LIBRARY_INIT_    __attribute__((section("app_media_task_common")))

#define     SORT_TYPE_SEL_NOW_PLAY      0   //正在播放
#define     SORT_TYPE_SEL_ID3TITLE      1
#define     SORT_TYPE_SEL_ID3SINGER     2
#define     SORT_TYPE_SEL_ID3ALBUM      3
#define     SORT_TYPE_SEL_GENRE         4
#define     SORT_TYPE_SEL_FOLDER        5   //目录列表
#define     MUSIC_TYPE_SEL_MYFAVORITE   6   //收藏夹
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
extern rk_err_t MediaLibrary_Init(void *pvParameters, void *arg);
extern rk_err_t MediaLibrary_DeInit(void *pvParameters);
extern void MediaLibrary_Enter(void);

#endif

