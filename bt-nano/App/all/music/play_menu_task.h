/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\music\play_menu_task.h
* Owner: wrm
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: music play menu task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm        2015.8.20     17:29:00   1.0
********************************************************************************************
*/


#ifndef __APP_MUSIC_PLAY_MENU_TASK_H__
#define __APP_MUSIC_PLAY_MENU_TASK_H__

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
#define _APP_MUSIC_PLAY_MENU_TASK_COMMON_  __attribute__((section("app_music_play_menu_task_common")))
#define _APP_MUSIC_PLAY_MENU_TASK_INIT_    __attribute__((section("app_music_play_menu_task_common")))
#define _APP_MUSIC_PLAY_MENU_TASK_SHELL_   __attribute__((section("app_music_play_menu_task_shell")))

typedef enum _PLAY_MENU_EVENT
{
    PLAYMENU_KEY_EVENT,
    PLAYMENU_AUDIO_EVENT,
    PLAYMENU_MSGBOX_EVENT,
    PLAYMENU_NOSCREEN_EVENT,
    PLAYMENU_EVENT_MAX

}PLAY_MENU_EVENT;






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

extern rk_err_t MusicPlayMenuTask_Resume(void);
extern rk_err_t MusicPlayMenuTask_Suspend(void);
extern void     MusicPlayMenuTask_Enter(void);
extern rk_err_t MusicPlayMenuTask_DeInit(void *pvParameters);
extern rk_err_t MusicPlayMenuTask_Init(void *pvParameters, void *arg);
extern void     MusicPlay_AudioCallBack(uint32 audio_state);
extern int      MusicPlayMenuTask_DeleteSelf(uint8 deleteplayer);
#ifndef _USE_GUI_
extern int      MusicPlayMenuTask_SendStartPlayer(uint8 deleteplayer);
#endif
#endif

