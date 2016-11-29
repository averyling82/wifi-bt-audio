/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\music\play_menu_task.c
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


#include "BspConfig.h"
#ifdef __APP_MUSIC_PLAY_MENU_TASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#ifdef _USE_GUI_
#include "GUITask.h"
#include "GUIManager.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"
#endif



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _MUSIC_PLAY_MENU_RESP_QUEUE
{
    uint32 cmd;
    uint32 type;
    uint32 err_code;

}MUSIC_PLAY_MENU_RESP_QUEUE;

typedef  struct _MUSIC_PLAY_MENU_ASK_QUEUE
{
    uint32 cmd;
    uint32 type;

}MUSIC_PLAY_MENU_ASK_QUEUE;

#define MUSIC_PLAY_MENU_QUEUE 4

typedef  struct _MUSIC_PLAY_MENU_TASK_DATA_BLOCK
{
    pQueue  PlayMenuAskQueue;
    pQueue  PlayMenuRespQueue;
    pTimer  Timer;

    #ifdef _USE_GUI_
    HGC hBackdrop;
    HGC hLoop;
    HGC hOrder;
    HGC hPlayerState;
    HGC hPage;
    HGC hProgressBar;
    HGC hVolumeBar;
    HGC hVolumeValue;
    HGC hMusicTotalTimer;
    HGC hMusicProgressTimer;
    HGC hBitrate;
    HGC hEQMode;
    HGC hMusicName;
    HGC hPlayer;
    HGC hMsgBox;
    HGC hSpecrm;
    #endif

    #ifndef _USE_GUI_
    pSemaphore osPlayKeySem;
    pSemaphore osStartPlayerSem;
    uint32  keyvalue;
    uint8   keycontrol;
    #ifdef _WIFI_
    uint8   WIFIControl;
    uint8   smartconfig;
    uint8   startplayer;
    int8    TimerCount;
    uint8   Closetimer;
    #endif
    #endif

    AUDIO_INFO stAudioInfo;
    uint32 PlayerState;
    uint32 Volume;
    uint32 MusicTotalTimer;
    uint32 MusicProgressTimer;
    uint32 playerType;
    uint8  KeyQueueCnt;
    RK_TASK_AUDIOCONTROL_ARG Arg;

}PLAY_MENU_TASK_DATA_BLOCK;

#define MaximumVolume                   30


#define WARING_WINDOW                   1



#define PLAYMENU_DEBUG(format,...) printf("\n[play_menu_task.c]:(Line=%d)--------"format, __LINE__, ##__VA_ARGS__)
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static PLAY_MENU_TASK_DATA_BLOCK * gpstPlayMenuData;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t PlayMenuCheckIdle(HTC hTask);
#if 1
rk_err_t MusicPlay_DisplayInit();
rk_err_t MusicPlay_DeletePlayer(uint32 SourecePlayer);
rk_err_t MusicPlay_StartPlayer(uint32 SourecePlayer);
rk_err_t MusicPlay_SelectExePlayer(uint32 SourecePlayer);
#ifdef _USE_GUI_
rk_err_t MusicPlayMenuTask_GuiCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc);
#endif
void     MusicPlay_DeleteGuiHandle(void);
void     MusicPlay_DisplayMsg(uint32 Sid);
int      MusicPlay_SetPopupWindow(int type);
int      MusicPlay_InitParams(int params);
int      MusicPlay_PlayerOpen(HTC hTask);
int      MusicPlay_NoSong(int type);
void     MusicPlay_MusicNumDisplay(void);
void     MusicPlay_VolumeDisplay(void);
void     MusicPlay_PlayOrderDisplay(void);
void     MusicPlay_RepeatModeDisplay(void);
void     MusicPlay_SpectrumDisplay(void);
void     MusicPlay_StateDisplay();
void     MusicPlay_BitrateDisplay(void);
void     MusicPlay_PlayProcessDisplay(void);
int      MusicPlayMenuTask_KeyEvent(uint32 KeyVal);
int      MusicPlayMenuTask_AudioEvent(uint32 AudioState);
int      MusicPlayMenuTask_MsgBoxEvent(uint32 cmd, int type);
void     MusicPlay_SetMusicPath(RK_TASK_PLAYMENU_ARG * Arg);
void     MusicPlay_BackGroundDisplay(void);
#endif

#ifndef _USE_GUI_
rk_err_t MusicPlay_NoScreenKeyRev(uint32 keyvalue);
int      MusicPlayMenuTask_NoScreenEvent(uint32 cmd, int type);
void     MusicPlayMenuTask_StartTimer(void);
void     MusicPlayMenuTask_CloseTimer(void);
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PlayMenuCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 19:48:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t PlayMenuCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        pTask->State = TASK_STATE_WORKING;
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlayMenuTask_AudioEvent
** Input:int type
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlayMenuTask_MsgBoxEvent(uint32 cmd, int type)
{
#ifdef _USE_GUI_
    uint32 msg;

    switch (cmd)
    {
        case KEY_VAL_ESC_SHORT_UP:
            if (gpstPlayMenuData->hMsgBox != NULL)
            {
                GuiTask_DeleteWidget(gpstPlayMenuData->hMsgBox);
                gpstPlayMenuData->hMsgBox=NULL;
            }
            if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD)
            {
                MusicPlayMenuTask_DeleteSelf(1);
            }
            else
            {
                MusicPlayMenuTask_DeleteSelf(0);
            }
            while(1)
            {
                rkos_sleep(2000);
            }
            break;

        case KEY_VAL_ESC_LONG_UP:
            if (gpstPlayMenuData->hMsgBox != NULL)
            {
                GuiTask_DeleteWidget(gpstPlayMenuData->hMsgBox);
                gpstPlayMenuData->hMsgBox=NULL;
            }

            MusicPlayMenuTask_DeleteSelf(1);
            while(1)
            {
                rkos_sleep(2000);
            }
            break;

        case KEY_VAL_FFD_SHORT_UP:
        case KEY_VAL_FFD_LONG_UP:
            if (gpstPlayMenuData->hMsgBox != NULL)
            {
                GuiTask_OperWidget(gpstPlayMenuData->hMsgBox, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
                GuiTask_DeleteWidget(gpstPlayMenuData->hMsgBox);
                gpstPlayMenuData->hMsgBox=NULL;
            }

            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
               ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
               ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
               ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
                AudioControlTask_SendCmd(AUDIO_CMD_PREVFILE, NULL, SYNC_MODE);
            }
            break;

        case KEY_VAL_FFW_SHORT_UP:
        case KEY_VAL_FFW_LONG_UP:
            if (gpstPlayMenuData->hMsgBox != NULL)
            {
                GuiTask_OperWidget(gpstPlayMenuData->hMsgBox, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
                GuiTask_DeleteWidget(gpstPlayMenuData->hMsgBox);
                gpstPlayMenuData->hMsgBox=NULL;
            }

            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
                AudioControlTask_SendCmd(AUDIO_CMD_NEXTFILE, NULL, SYNC_MODE);
            }
            break;

        default:
            break;

    }
#endif
    return RK_SUCCESS;
}

#ifdef __SHELL_SWITCH_PLAYER_C__
/*******************************************************************************
** Name: SwitchToPlayer
** Input: uint32 PlayerType
** Return: int
** Owner:jjjhhh
** Date: 2016.10.16
** Time: 22:06:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_//_SWITCH_PLAYER_SHELL_
int SwitchToPlayer(uint32 PlayerType)
{

#if defined(_ENABLE_WIFI_BLUETOOTH)

		if (gpstPlayMenuData->startplayer == 1)//It is screen player now.
        {   
            rk_printf("It is screen player now.-------------------------------\n");
			return -1;
		}
		/*
        -->SOURCE_FROM_HTTP->(SOURCE_FROM_XXX)->SOURCE_FROM_FILE_BROWSER->SOURCE_FROM_BT-
        |                                                                              |
        <---------------------<----------------------------<-------------------<----<-
		*/
		
			switch (gSysConfig.PlayerType)
			{
				case SOURCE_FROM_HTTP:
		#ifdef NOSCREEN_USE_LED
					{
						MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
						MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
						//test
						MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
					}
		#endif
					if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)==1)//STOP DLNA
					{
						if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
						{
							if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER_START)==0)
							{
					#ifdef __WIFI_DLNA_C__
								rk_dlna_end();
								RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
					#endif
								rkos_sleep(3000);
							}
						}
						MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
						gSysConfig.PlayerType = SOURCE_FROM_XXX;
					}
			
					MusicPlay_StartPlayer(SOURCE_FROM_XXX);//START XXX
		#ifdef __WIFI_XXX_C__
					if (RKTaskFind(TASK_ID_XXX, 0) != NULL)
					{
						while (XXX_init_state() == 0)
						{
							rkos_sleep(1000);
						}
					}
		#endif
		#ifdef NOSCREEN_USE_LED
					{
						MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
						MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
					}
		#endif
					break;
				case SOURCE_FROM_XXX:
		#ifdef NOSCREEN_USE_LED
					{
						MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
						MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
					}
		#endif
					if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)==1)//STOP XXX
					{
						if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
						{
							if(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER_START)==0)
							{
					#ifdef __WIFI_XXX_C__
								XXX_end();
								RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
					#endif
								rkos_sleep(2000);
							}
						}
						MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
						gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;
					}
					if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) != NULL)//STOP WIFI
					{
						MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 0);
						rk_wifi_deinit();
						RKTaskDelete(TASK_ID_WIFI_APPLICATION, 0, SYNC_MODE);
						rk_printf ("Delete wifi OK\n");
					}
			
			
					MusicPlay_StartPlayer(SOURCE_FROM_FILE_BROWSER);//START SD PLAYER
		#ifdef NOSCREEN_USE_LED
					{
						MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
						MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
					}
		#endif
					break;
				case SOURCE_FROM_FILE_BROWSER:
		#ifdef NOSCREEN_USE_LED
					{
						MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
						//test
						/*MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 0);
						MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);*/
						MainTask_SetStatus(MAINTASK_BT_START, 1);
					}
		#endif
					//rk_printf("11111111111111 -------------------------\n");
					if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)//STOP SD PLAYER
					{
						AudioControlTask_SetStateChangeFunc(MusicPlay_AudioCallBack, NULL);
						RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
						//等待本地Audio删除
					}
					MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);//rk_printf("3333333333 -------------------------\n");
					//关闭定时器(停止WIFI 配置)
					{
						rk_printf ("SUSPEND WIFI/WIFICONFIG&TIMER\n");
						MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 1);
						rk_wifi_deinit();
						RKTaskDelete(TASK_ID_WIFI_APPLICATION,0,SYNC_MODE);
					}

					//MainTask_DeleteAllApp();//enable BT
					if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
					{
						RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
					}
			
					rk_printf("switch BT,sleep 2S......\n");
					rkos_sleep(2000);
			
					//rk_printf("44444444444 -------------------------\n");
					MainTask_StartSubThread(FM_SUB_THREAD); //打开BT音乐线程
					//rk_printf("55555555555555 -------------------------\n");
			
					//START BT
					gSysConfig.PlayerType = SOURCE_FROM_BT;
					MusicPlay_StartPlayer(SOURCE_FROM_BT);
					//rk_printf("66666666666666 -------------------------\n");
		#ifdef NOSCREEN_USE_LED
					{
						//LED Display
						MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
						MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
						rkos_sleep(1000);
					}
		#endif
			
					break;
				case SOURCE_FROM_BT:
					if (gSysConfig.BtControl==1)//STOP BT
					{
						bluetooth_stop();
						FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
						gSysConfig.BtControl = 0;
					}
					if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)//DELETE AUDIOCONTROL
					{
						RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
					}
			
					rk_printf("switch to DLNA,sleep 2S......\n");
					rkos_sleep(2000);//wait 2S ???
			
					//gpstPlayMenuData->smartconfig=1; //WIFI 还在配置状态标志
					//MainTask_SetStatus(MAINTASK_WIFICONFIG,1);
					//启动WIFI
					RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_STA, SYNC_MODE);
					while (wifi_init_flag() == WICED_FALSE)
					{
						rkos_sleep(1000);
					}
					rk_printf("wifi init ok-------------------------------\n");
					gpstPlayMenuData->WIFIControl = 1;
					MusicPlayMenuTask_StartTimer();
			
					gSysConfig.PlayerType = SOURCE_FROM_HTTP;//start DLNA
					MusicPlay_StartPlayer(SOURCE_FROM_HTTP);
		#ifdef NOSCREEN_USE_LED
					{
						MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
						MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
					}
		#endif
			
					break;
				default:
					break;
			}


	#ifdef NOSCREEN_USE_LED//update led
		{
			if (MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)!=1)
			{
				if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
				}
				if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_HTTP)
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
					//test
					MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
				}
				
				if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_XXX)
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
					//test
					MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
				}
			}
		}
	#endif
	return 0;
#else
	return 0;
#endif//#if define(_ENABLE_WIFI_BLUETOOTH) SwitchToPlayer
	
}
#endif //__SHELL_SWITCH_PLAYER_C__



/*******************************************************************************
** Name: MusicPlayMenuTask_AudioEvent
** Input: uint32 AudioState
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlayMenuTask_AudioEvent(uint32 AudioState)
{
#ifdef _USE_GUI_
    if(gpstPlayMenuData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstPlayMenuData->hMsgBox, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstPlayMenuData->hMsgBox);
        gpstPlayMenuData->hMsgBox=NULL;
    }
#endif
    switch (AudioState)
    {
        case AUDIO_STATE_PAUSE: //play or pause
            gpstPlayMenuData->PlayerState = AUDIO_STATE_PAUSE;
            MusicPlay_StateDisplay();
            break;

        case AUDIO_STATE_PLAY: //play or pause
            gpstPlayMenuData->PlayerState = AUDIO_STATE_PLAY;
            MusicPlay_StateDisplay();
            break;

        case AUDIO_STATE_STOP:
            gpstPlayMenuData->PlayerState = AUDIO_STATE_STOP;
            MusicPlay_StateDisplay();
            break;

        case AUDIO_STATE_TIME_CHANGE://update time
            MusicPlay_PlayProcessDisplay();
            break;

        case AUDIO_STATE_VOLUME_CHANGE://update volume
            MusicPlay_VolumeDisplay();
            break;

        case AUDIO_STATE_MUSIC_CHANGE: //update
            gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
            MusicPlay_DisplayAll();
            break;

        case AUDIO_STATE_ERROR:
            MusicPlay_SetPopupWindow(WARING_WINDOW);
            break;

        case AUDIO_STATE_UPDATA_SPECTRUM:
            MusicPlay_SpectrumDisplay();
            break;

        case AUDIO_STATE_CREATE:
            AudioControlTask_SetStateChangeFunc(NULL, MusicPlay_AudioCallBack);
            AudioPlayer_SetSpectrumEn(1);
            break;

        default:
            break;


    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlayMenuTask_KeyEvent
** Input:uint32 KeyVal
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlayMenuTask_KeyEvent(uint32 KeyVal)
{
    #ifdef _USE_GUI_
    if(gpstPlayMenuData->hMsgBox != NULL)
    {
        return RK_SUCCESS;
    }
    #endif

    switch (KeyVal)
    {
        case KEY_VAL_PLAY_SHORT_UP://无屏操作切换播放器
#ifdef _USE_GUI_

#else
            #if !defined(_ENABLE_WIFI_BLUETOOTH) && defined(_BLUETOOTH_)
            {
                if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) == NULL)
                {
                    MusicPlay_StartPlayer(gSysConfig.PlayerType);
                    break;
                }
                switch (gSysConfig.PlayerType)
                {
                    case SOURCE_FROM_BT:
                        if (gSysConfig.BtControl==1)
                        {
                            bluetooth_stop();
                            FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
                            gSysConfig.BtControl = 0;
                        }
                        MusicPlay_StartPlayer(SOURCE_FROM_FILE_BROWSER);
                        #ifdef NOSCREEN_USE_LED
                        {
                            //LED Display
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                            rkos_sleep(1000);
                        }
                        #endif
                        break;
                    case SOURCE_FROM_FILE_BROWSER:
                        if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
                        {
                            AudioControlTask_SetStateChangeFunc(MusicPlay_AudioCallBack, NULL);
                            RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
                        }
                        MusicPlay_StartPlayer(SOURCE_FROM_BT);
                        #ifdef NOSCREEN_USE_LED
                        {
                            //LED Display
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                            rkos_sleep(1000);
                        }
                        #endif

                        break;
                    default:
                        break;
                }
            }
            #endif //!defined(_ENABLE_WIFI_BLUETOOTH) && defined(_BLUETOOTH_)

	#if defined(_WIFI_) && !defined(_ENABLE_WIFI_BLUETOOTH)//#ifdef _WIFI_
            {
                if (gpstPlayMenuData->startplayer == 1)//It is screen player now.
                {
                    break;
                }
                switch (gSysConfig.PlayerType)
                {
                    case SOURCE_FROM_HTTP:
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                            //test
                            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
                        }
                        #endif
                        if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)==1)
                        {
                            if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
                            {
                                if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER_START)==0)
                                {
                                    #ifdef __WIFI_DLNA_C__
                                    rk_dlna_end();
                                    RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
                                    #endif
                                    rkos_sleep(3000);
                                }
                            }
                            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
                            gSysConfig.PlayerType = SOURCE_FROM_XXX;
                        }

                        MusicPlay_StartPlayer(SOURCE_FROM_XXX);
                        #ifdef __WIFI_XXX_C__
                        if (RKTaskFind(TASK_ID_XXX, 0) != NULL)
                        {
                            while (XXX_init_state() == 0)
                            {
                                rkos_sleep(1000);
                            }
                        }
                        #endif
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                        }
                        #endif
                        break;
                    case SOURCE_FROM_XXX:
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
                        }
                        #endif
                        if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)==1)
                        {
                            if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
                            {
                                if(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER_START)==0)
                                {
                                    #ifdef __WIFI_XXX_C__
                                    XXX_end();
                                    RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
                                    #endif
                                    rkos_sleep(2000);
                                }
                            }
                            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
                            gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;
                        }

                        MusicPlay_StartPlayer(SOURCE_FROM_FILE_BROWSER);
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                        }
                        #endif
                        break;
                    case SOURCE_FROM_FILE_BROWSER:
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                            //test
                            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 0);
                            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
                        }
                        #endif
                        if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
                        {
                            AudioControlTask_SetStateChangeFunc(MusicPlay_AudioCallBack, NULL);
                            RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
                            //等待本地Audio删除
                            rkos_sleep(2000);
                        }
                        MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);
                        gSysConfig.PlayerType = SOURCE_FROM_HTTP;
                        MusicPlay_StartPlayer(SOURCE_FROM_HTTP);
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                        }
                        #endif
                        break;
                    default:
                        break;
                }
            }
	/****************_ENABLE_WIFI_BLUETOOTH start**********************/
	#elif defined(_ENABLE_WIFI_BLUETOOTH)
		if ((gpstPlayMenuData->startplayer == 1) || (MainTask_GetStatus(MAINTASK_SYS_UPDATE_FW)==1))//It is screen player now.
		{
			break;
		}
		/*
        -->SOURCE_FROM_HTTP->(SOURCE_FROM_XXX)->SOURCE_FROM_FILE_BROWSER->SOURCE_FROM_BT-
        |                                                                              |
        <---------------------<----------------------------<-------------------<----<-
		*/
		switch (gSysConfig.PlayerType)
		{
#if 0//dlna->airplay->sd
			case SOURCE_FROM_HTTP:
				if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)==1)//STOP DLNA
				{
					MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
					if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
					{
						if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER_START)==0)
						{
							#ifdef __WIFI_DLNA_C__
							rk_dlna_end();
							RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
							#endif
							rkos_sleep(3000);
						}
					}
					MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
					gSysConfig.PlayerType = SOURCE_FROM_XXX;
				}

				MusicPlay_StartPlayer(SOURCE_FROM_XXX);//START XXX
				#ifdef __WIFI_XXX_C__
				if (RKTaskFind(TASK_ID_XXX, 0) != NULL)
				{
					while (XXX_init_state() == 0)
					{
						rkos_sleep(1000);
					}
				}
				#endif
			/*	#ifdef NOSCREEN_USE_LED
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
				}
				#endif*/
				break;
			case SOURCE_FROM_XXX:
				if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)==1)//STOP XXX
				{
					MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
					if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
					{
						if(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER_START)==0)
						{
							#ifdef __WIFI_XXX_C__
							XXX_end();
							RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
							#endif
							rkos_sleep(2000);
						}
					}
					MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
					gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;
				}
				if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) != NULL)//STOP WIFI
				{
					//rk_printf ("Delete wifi 111\n");
					MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 0);
					rk_wifi_deinit();//rk_printf ("Delete wifi 222\n");
					RKTaskDelete(TASK_ID_WIFI_APPLICATION, 0, SYNC_MODE);
					
					//MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 1);
					rk_printf ("Delete wifi OK\n");
				}
				//MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 0);//just for stoping wifi timer
				MusicPlay_StartPlayer(SOURCE_FROM_FILE_BROWSER);//START SD PLAYER
				break;
#else//dlna->sd
			case SOURCE_FROM_HTTP:
				if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)==1)//STOP DLNA
				{
					MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
					if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
					{
						if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER_START)==0)
						{
							#ifdef __WIFI_DLNA_C__
							rk_dlna_end();
							RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
							#endif
							rkos_sleep(3000);
						}
				}
				MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
				gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;
				}
				if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) != NULL)//STOP WIFI
				{
					//rk_printf ("Delete wifi 111\n");
					MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 0);
					rk_wifi_deinit();//rk_printf ("Delete wifi 222\n");
					RKTaskDelete(TASK_ID_WIFI_APPLICATION, 0, SYNC_MODE);
		
					//MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 1);
					rk_printf ("Delete wifi OK\n");
				}
				//MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 0);//just for stoping wifi timer
				MusicPlay_StartPlayer(SOURCE_FROM_FILE_BROWSER);//START SD PLAYER
				break;

#endif//dlna->sd end
			case SOURCE_FROM_FILE_BROWSER:
				//rk_printf("11111111111111 -------------------------\n");
				MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);
				if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)//STOP SD PLAYER
				{
					AudioControlTask_SetStateChangeFunc(MusicPlay_AudioCallBack, NULL);
					RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
					//等待本地Audio删除
				}
				MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);//rk_printf("3333333333 -------------------------\n");
				MainTask_SetStatus(MAINTASK_BT_START, 1);
				
				//MainTask_DeleteAllApp();//enable BT
				if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
				{
					RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
				}

				rk_printf("switch BT,sleep 2S......\n");
				rkos_sleep(2000);

				//rk_printf("44444444444 -------------------------\n");
				MainTask_StartSubThread(FM_SUB_THREAD); //打开BT音乐线程
				//rk_printf("55555555555555 -------------------------\n");

				//START BT
				gSysConfig.PlayerType = SOURCE_FROM_BT;
				MusicPlay_StartPlayer(SOURCE_FROM_BT);
				//rk_printf("66666666666666 -------------------------\n");
				break;
			case SOURCE_FROM_BT:
				MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 0);
				if (gSysConfig.BtControl==1)//STOP BT
				{
					bluetooth_stop();
					FW_RemoveSegment(SEGMENT_ID_BLUETOOTH);
					gSysConfig.BtControl = 0;
				}
				MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 0);
				if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)//DELETE AUDIOCONTROL
				{
					RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
				}

				rk_printf("switch to DLNA,sleep 2S......\n");
				rkos_sleep(2000);//wait 2S ???

				//reinit WIFI
				MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 1);
				MainTask_SetStatus(MAINTASK_WIFI_SUSPEND, 0);
				//gpstPlayMenuData->smartconfig=1; //WIFI 还在配置状态标志
				//MainTask_SetStatus(MAINTASK_WIFICONFIG,1);
				//启动WIFI
				RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_STA, SYNC_MODE);
				while (wifi_init_flag() == WICED_FALSE)
				{
					rkos_sleep(1000);
				}
				rk_printf("wifi init ok-------------------------------\n");
				gpstPlayMenuData->WIFIControl = 1;
				MusicPlayMenuTask_StartTimer();

                gSysConfig.PlayerType = SOURCE_FROM_HTTP;//start DLNA
				break;
			default:
				break;
		}

		#if 0 //def NOSCREEN_USE_LED//update led
		{
			if (MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)!=1)
			{
				if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
				}
				if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_HTTP)
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
					//test
					MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
				}
				
				if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_XXX)
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
					//test
					MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
				}
			}
		}
		#endif
	#endif//defined(_WIFI_) && !defined(_ENABLE_WIFI_BLUETOOTH) end
	/******************_ENABLE_WIFI_BLUETOOTH end*********************************/

#endif//#ifdef _USE_GUI_ end
            break;
        case KEY_VAL_ESC_SHORT_UP:  //退出播放器界面，但保留播放器
#ifdef _USE_GUI_
            if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD)
            {
                PLAYMENU_DEBUG ("SOURCE_FROM_RECORD Delete player not store\n");
                MusicPlayMenuTask_DeleteSelf(1);
                while(1)
                {
                    rkos_sleep(2000);
                }
            }
            else
            {
                if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
                {
                    AudioControlTask_SetStateChangeFunc(MusicPlay_AudioCallBack, MainTask_AudioCallBack);
                    MainTask_AudioCallBack(AUDIO_STATE_MUSIC_CHANGE);
                }

                MusicPlay_DeleteGuiHandle();
                if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER) || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
                {
                    #ifdef __APP_BROWSER_BROWSERUITASK_C__
                    RK_TASK_BROWSER_ARG Arg;
                    Arg.SelSouceType = gpstPlayMenuData->Arg.ucSelPlayType;
                    Arg.Flag = 0;
                    memcpy(Arg.filepath, gpstPlayMenuData->stAudioInfo.path, StrLenW(gpstPlayMenuData->stAudioInfo.path)*2);
                    Arg.filepath[StrLenW(gpstPlayMenuData->stAudioInfo.path)] = 0x0000;
                    MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
                    #endif
                }
#ifdef _MEDIA_MODULE_
                else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                {
                    RK_TASK_MEDIABRO_ARG MediaBro;
                    MediaBro.Flag = 1;
                    MediaBro.MediaTypeSelID = gSysConfig.MediaDirTreeInfo.MediaType;
                    memcpy((UINT8*) &(MediaBro.MediaDirTreeInfo),
                           (UINT8*) &(gSysConfig.MediaDirTreeInfo),
                           sizeof(MEDIABRO_DIR_TREE_STRUCT));

                    MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_MEDIA_BROWSER, 0, &MediaBro);
                }
                else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                {
                    RK_TASK_BROWSER_ARG Arg;
                    Arg.Flag = 1;
                    Arg.SelSouceType = gpstPlayMenuData->Arg.ucSelPlayType;

                    AudioPlayer_SetFloderInfo();
                    AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
                    memcpy((UINT8*) &(Arg.MediaFloderInfo), (UINT8*) &(gpstPlayMenuData->stAudioInfo.MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));

                    MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
                }
#endif
                else
                {
                    MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_MAIN, 0, NULL);
                }

                while(1)
                {
                    rkos_sleep(2000);
                    PLAYMENU_DEBUG("play_menu 1\n");
                }
            }
#else

#endif
            break;
        case KEY_VAL_ESC_PRESS_START:
#ifdef _USE_GUI_

#else
            #ifdef _WIFI_//SmartConfig 配置
            {
                if (gpstPlayMenuData->WIFIControl==1)
                {
                    //删除 DLNA or XXX 播放器
                    if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)==1)
                    {
                        if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
                        {
                            #ifdef __WIFI_DLNA_C__
                            rk_dlna_end();
                            RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
                            #endif
                        }
                        MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
                    }
                    if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)==1)
                    {
                        if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
                        {
                            #ifdef __WIFI_XXX_C__
                            XXX_end();
                            RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
                            #endif
                        }
                        MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
                    }
                    if (MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER)==1)
                    {
                        RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
                        MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);
                    }

                    if (gpstPlayMenuData->smartconfig==0)
                    {
                        rk_printf ("wificonfig---------------------------------------on");
                        gpstPlayMenuData->smartconfig=1;
                        MainTask_SetStatus(MAINTASK_WIFICONFIG,1);
                        rk_wifi_smartconfig();
                        MusicPlayMenuTask_StartTimer();
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                        }
                        #endif
                        //防止WIFIConfig开关太快导致WIFI配置出问题
                        rkos_sleep(5000);
                    }
                    else
                    {
                        rk_printf ("wificonfig---------------------------------------off");
                        gpstPlayMenuData->smartconfig=0;
                        rk_easy_smartconfig_stop();
                        #ifdef NOSCREEN_USE_LED
                        {
                            MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                            MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                        }
                        #endif
                        MainTask_SetStatus(MAINTASK_WIFICONFIG,0);
                        //防止WIFIConfig开关太快导致WIFI配置出问题
                        rkos_sleep(5000);
                    }
                }
            }
            #endif //_WIFI_ end
#endif
            break;

        case KEY_VAL_ESC_PRESS: //退出播放器界面，不保留播放器
#ifdef _USE_GUI_
            MusicPlayMenuTask_DeleteSelf(1);
            while(1)
            {
                rkos_sleep(2000);
            }
#else

#endif
            break;


        case KEY_VAL_PLAY_PRESS_START://关机
#ifdef _USE_GUI_
#else
            MainTask_UnRegisterKey();
            MainTask_SetStatus(MAINTASK_APP_PLAYMENU,0);
            MusicPlayMenuTask_CloseTimer();//SaveSysInformation(0);//----jjjhhh 20161105
#endif
            MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
            break;

        case KEY_VAL_FFW_SHORT_UP: //Next Song
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))

            {
                AudioControlTask_SendCmd(AUDIO_CMD_NEXTFILE, (void *)1, SYNC_MODE);
                #ifndef _USE_GUI_
                rkos_sleep(1000);
                #endif
            }
            else if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
            {
                PLAYMENU_DEBUG ("BlueToothKeyPlayNext Song\n");
                #ifdef _BLUETOOTH_
                ct_next();
                #endif
                break;
            }
            break;

        case KEY_VAL_FFD_SHORT_UP: // Last Song
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
                AudioControlTask_SendCmd(AUDIO_CMD_PREVFILE, NULL, SYNC_MODE);
                #ifndef _USE_GUI_
                rkos_sleep(1000);
                #endif
                break;
            }
            if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
            {
                PLAYMENU_DEBUG ("BlueToothKeyPlayLast Song\n");
                #ifdef _BLUETOOTH_
                ct_previous();
                #endif
                break;
            }
            break;

        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_SHORT_UP: //Volume +
            //PLAYMENU_DEBUG ("KEY_UP\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT))
            {
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                {
                    if(gpstPlayMenuData->Volume >= MaximumVolume)
                    {
                        break;
                    }

                    gpstPlayMenuData->Volume++;
                    AudioControlTask_SendCmd(AUDIO_CMD_VOLUMESET, (void *)gpstPlayMenuData->Volume, SYNC_MODE);
                    MusicPlay_VolumeDisplay();
                }
            }
            break;

        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_SHORT_UP: //Volume -
            //PLAYMENU_DEBUG ("KEY_DOWN\n");
            //local
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT))
            {
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                {
                    if (gpstPlayMenuData->Volume== 0)
                    {
                        break;
                    }
                    gpstPlayMenuData->Volume--;
                    AudioControlTask_SendCmd(AUDIO_CMD_VOLUMESET, (void *)gpstPlayMenuData->Volume, SYNC_MODE);
                    MusicPlay_VolumeDisplay();
                }
            }
            break;

        case KEY_VAL_MENU_SHORT_UP: //Start and Stop
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
#ifdef _USE_GUI_
#else
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                {
                    gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
                }
#endif
                if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PAUSE) //play->stop
                {
                    gpstPlayMenuData->PlayerState = AUDIO_STATE_PLAY;
                    MusicPlay_StateDisplay();
                    AudioControlTask_SendCmd(AUDIO_CMD_RESUME, NULL, SYNC_MODE);
                }
                else if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PLAY)
                {
                    gpstPlayMenuData->PlayerState = AUDIO_STATE_PAUSE;
                    MusicPlay_StateDisplay();
                    AudioControlTask_SendCmd(AUDIO_CMD_PAUSE, NULL, SYNC_MODE);
                }

            }
            else if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
            {
                PLAYMENU_DEBUG ("BlueToothKeyPlayPause\n");
                #ifdef _BLUETOOTH_
                BlueToothKeyPlayPause();
                #endif
                break;
            }
            else if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_HTTP)
                    ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_XXX))
            {

            }
            break;

        case KEY_VAL_FFD_PRESS:
            //PLAYMENU_DEBUG ("\n快退开始\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
#ifdef _USE_GUI_
#else
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                {
                    gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
                }
#endif
                if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PAUSE)
                {
                    PLAYMENU_DEBUG ("pause=%d\n",gpstPlayMenuData->PlayerState);
                    //stall state
                    AudioControlTask_SendCmd(AUDIO_CMD_PREVFILE, NULL, SYNC_MODE);
                    #ifndef _USE_GUI_
                    rkos_sleep(1000);
                    #endif
                    break;
                }
                else
                {
                    if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                    {
                        AudioControlTask_SendCmd(AUDIO_CMD_FFW, (void *)5000, SYNC_MODE);
                        MusicPlay_PlayProcessDisplay();
                        #ifdef _USE_GUI_
                        ;
                        #else
                        AudioPlayer_GetCurTime(&gpstPlayMenuData->MusicProgressTimer);
                        AudioPlayer_GetTotalTime(&gpstPlayMenuData->MusicTotalTimer);
                        #endif
                        if (gpstPlayMenuData->MusicProgressTimer <= 1)
                        {
                            PLAYMENU_DEBUG ("Ready Prev song\n");
                            AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_PrevFile, SYNC_MODE);
                        }
                    }
                }

            }
            break;


        case KEY_VAL_FFW_PRESS:
            //printf ("快进开始\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
#ifdef _USE_GUI_
#else
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                {
                    gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
                }
#endif
                if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PAUSE)
                {
                    //stall state
                    PLAYMENU_DEBUG ("pause=%d\n",gpstPlayMenuData->PlayerState);
                    AudioControlTask_SendCmd(AUDIO_CMD_NEXTFILE, NULL, SYNC_MODE);
                    #ifndef _USE_GUI_
                    rkos_sleep(1000);
                    #endif
                }
                else
                {
                    if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                    {
                        AudioControlTask_SendCmd(AUDIO_CMD_FFD, (void *)5000, SYNC_MODE);
                        MusicPlay_PlayProcessDisplay();
                        #ifdef _USE_GUI_
                        ;
                        #else
                        AudioPlayer_GetCurTime(&gpstPlayMenuData->MusicProgressTimer);
                        AudioPlayer_GetTotalTime(&gpstPlayMenuData->MusicTotalTimer);
                        #endif
                        if (gpstPlayMenuData->MusicProgressTimer >= gpstPlayMenuData->MusicTotalTimer)
                        {
                            PLAYMENU_DEBUG ("Ready next song\n");
                            AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_NextFile, SYNC_MODE);
                        }
                    }
                }
            }

            break;

        case KEY_VAL_FFD_LONG_UP:  //rewind end
            //PLAYMENU_DEBUG ("快退结束\n");
            //printf ("\n----------FFD LONG PRESS end\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
#ifdef _USE_GUI_
#else
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                    gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
#endif
                if(gpstPlayMenuData->PlayerState == AUDIO_STATE_FFW)
                {
                    AudioControlTask_SendCmd(AUDIO_CMD_FF_STOP, NULL, SYNC_MODE);
                }
            }
            break;

        case KEY_VAL_FFW_LONG_UP: //fast forward end
            //printf ("快进结束\n");
            //PLAYMENU_DEBUG ("-----FFW LONG PRESS end\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
#ifdef _USE_GUI_
#else
                if (gpstPlayMenuData->PlayerState != AUDIO_STATE_ERROR)
                    gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
#endif

                if(gpstPlayMenuData->PlayerState == AUDIO_STATE_FFD)
                {
                    AudioControlTask_SendCmd(AUDIO_CMD_FF_STOP, NULL, SYNC_MODE);
                }
            }
            break;
        case KEY_VAL_FFD_PRESS_START:// rewind Start
#ifdef _USE_GUI_
            //PLAYMENU_DEBUG ("\n快退开始\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {

                if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PAUSE)
                {
                    PLAYMENU_DEBUG ("pause=%d\n",gpstPlayMenuData->PlayerState);
                    //stall state
                    AudioControlTask_SendCmd(AUDIO_CMD_PREVFILE, NULL, SYNC_MODE);
                    break;
                }
                else
                {
                    gpstPlayMenuData->PlayerState = AUDIO_STATE_FFW;
                    AudioControlTask_SendCmd(AUDIO_CMD_FFW, (void *)5000, SYNC_MODE);
                    MusicPlay_PlayProcessDisplay();
                    MusicPlay_StateDisplay();

                    if (gpstPlayMenuData->MusicProgressTimer <= 1)
                    {
                        PLAYMENU_DEBUG ("Ready Prev song\n");
                        AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_PrevFile, SYNC_MODE);
                    }
                }

            }
#endif
            break;

        case KEY_VAL_FFW_PRESS_START://fast forward start
#ifdef _USE_GUI_
             //printf ("快进开始\n");
            if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                ||(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
            {
                if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PAUSE)
                {
                    //stall state
                    PLAYMENU_DEBUG ("pause=%d\n",gpstPlayMenuData->PlayerState);
                    AudioControlTask_SendCmd(AUDIO_CMD_NEXTFILE, NULL, SYNC_MODE);
                }
                else
                {
                    gpstPlayMenuData->PlayerState = AUDIO_STATE_FFD;
                    AudioControlTask_SendCmd(AUDIO_CMD_FFD, (void *)5000, SYNC_MODE);
                    MusicPlay_PlayProcessDisplay();
                    MusicPlay_StateDisplay();

                    if (gpstPlayMenuData->MusicProgressTimer >= gpstPlayMenuData->MusicTotalTimer)
                    {
                        PLAYMENU_DEBUG ("Ready next song\n");
                        AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_NextFile, SYNC_MODE);
                    }
                }
            }
#endif
            break;
        default:
            break;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MusicPlay_MusicNumDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_MusicNumDisplay(void)
{
#ifdef _USE_GUI_
    uint32 i;
    char buf[10];
    Ucs2 PageHead[10];


    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;


    sprintf(buf, "%04d/%04d", gpstPlayMenuData->stAudioInfo.CurrentFileNum + 1, gpstPlayMenuData->stAudioInfo.TotalFiles);

    for (i= 0; i< sizeof(buf); i++)
    {
        PageHead[i] = (Ucs2)buf[i];
    }

    PageHead[i] = 0;


    if(gpstPlayMenuData->hPage == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 75;
        pstTextArg.y= 125;
        pstTextArg.xSize= 52;
        pstTextArg.ySize= -1;
        pstTextArg.text= PageHead;
        pstTextArg.BackdropX= 75;
        pstTextArg.BackdropY= 125;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstPlayMenuData->hPage = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        playerTextContentArg.opercmd= TEXT_SET_CONTENT;
        playerTextContentArg.text = PageHead;
        GuiTask_OperWidget(gpstPlayMenuData->hPage, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }
#endif
}

#ifdef _RK_EQ_
/*******************************************************************************
** Name: MusicPlay_MusicNumDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_MusicEqDisplay(void)
{
#ifdef _USE_GUI_
    uint32 i;
    uint32 SidEq[9] = {SID_EQ_HEAVY,SID_EQ_POP,SID_EQ_JAZZ, SID_EQ_UNIQUE, SID_EQ_SELECT, SID_EQ_BASS, 0, SID_EQ_CUSTOM, SID_EQ_NONE};

    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    if(gpstPlayMenuData->hEQMode == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_ID;
        pstTextArg.x= 80;
        pstTextArg.y= 96;//113
        pstTextArg.xSize= 46;
        pstTextArg.ySize= -1;
        pstTextArg.text= NULL;
        pstTextArg.BackdropX= 80;
        pstTextArg.BackdropY= 96;//113
        pstTextArg.resource = SidEq[gpstPlayMenuData->stAudioInfo.EqMode - EQ_HEAVY];
        pstTextArg.align= TEXT_ALIGN_X_RIGHT;
        gpstPlayMenuData->hEQMode = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        playerTextContentArg.opercmd = TEXT_SET_CONTENT;
        playerTextContentArg.resource = SidEq[gpstPlayMenuData->stAudioInfo.EqMode - EQ_HEAVY];
        GuiTask_OperWidget(gpstPlayMenuData->hEQMode, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }
#endif
}
#endif


/*******************************************************************************
** Name: MusicPlay_BackGroundDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_BackGroundDisplay(void)
{
#ifdef _USE_GUI_
    RKGUI_ICON_ARG pstIconArg;

    pstIconArg.resource= IMG_ID_MUSIC_BACKGROUND;
    pstIconArg.x= 0;
    pstIconArg.y= 0;
    pstIconArg.level = 0;
    pstIconArg.display= 1;
    gpstPlayMenuData->hBackdrop = GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
#endif
}
/*******************************************************************************
** Name: MusicPlay_VolumeDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_VolumeDisplay(void)
{
#ifdef _USE_GUI_
    int i;
    char buf[3];
    Ucs2 VolumeValue[3];

    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    RKGUI_TEXT_ARG pstTextArg;
    RKGUI_PROGRESSBAR_ARG pstProgressArg;

    sprintf(buf, "%02d", gpstPlayMenuData->Volume);
    //printf ("Volume buf=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        VolumeValue[i]= (Ucs2)buf[i];
    }
    VolumeValue[i] = 0;

    if(gpstPlayMenuData->hVolumeValue == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 114;
        pstTextArg.y= 143;
        pstTextArg.xSize= 15;
        pstTextArg.ySize= -1;
        pstTextArg.text= VolumeValue;
        pstTextArg.BackdropX= 114;
        pstTextArg.BackdropY= 143;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstPlayMenuData->hVolumeValue = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        playerTextContentArg.opercmd = TEXT_SET_CONTENT;
        playerTextContentArg.text = VolumeValue;
        GuiTask_OperWidget(gpstPlayMenuData->hVolumeValue, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }

    if(gpstPlayMenuData->hVolumeBar == NULL)
    {
        pstProgressArg.x= 70;
        pstProgressArg.y= 147;
        pstProgressArg.display = 1;
        pstProgressArg.level= 0;

        pstProgressArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstProgressArg.BackdropX= 70;
        pstProgressArg.BackdropY= 147;

        pstProgressArg.Bar= IMG_ID_VOLUME_BAR;
        pstProgressArg.percentage= gpstPlayMenuData->Volume* 100/ MaximumVolume;
        gpstPlayMenuData->hVolumeBar = GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        pstProgressBarContent.cmd= PROGRESSBAR_SET_CONTENT;
        pstProgressBarContent.percentage = gpstPlayMenuData->Volume* 100/ MaximumVolume;
        //PLAYMENU_DEBUG("percentage=%d\n",pstProgressBarContent.percentage);
        GuiTask_OperWidget(gpstPlayMenuData->hVolumeBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
    return ;
}

/*******************************************************************************
** Name: MusicPlay_PlayOrderDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_PlayOrderDisplay(void)
{
#ifdef _USE_GUI_
    uint32 Order;
    RKGUI_ICON_ARG pstIconArg;

    if (gpstPlayMenuData->stAudioInfo.PlayOrder)
    {
        Order = IMG_ID_MUSIC_ORDER_MODE01;
    }
    else
    {
        Order = IMG_ID_MUSIC_ORDER_MODE02;
    }

    if(gpstPlayMenuData->hOrder == NULL)
    {
        pstIconArg.resource = Order;
        pstIconArg.x= 24;
        pstIconArg.y= 141;
        pstIconArg.level = 0;
        pstIconArg.display= 1;
        gpstPlayMenuData->hOrder= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);

    }
    else
    {
        GuiTask_OperWidget(gpstPlayMenuData->hOrder, OPERATE_SET_CONTENT, (void *)Order, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: MusicPlay_RepeatModeDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_RepeatModeDisplay(void)
{
#ifdef _USE_GUI_
    uint32 RepeatMode;
    RKGUI_ICON_ARG pstIconArg;

    switch (gpstPlayMenuData->stAudioInfo.RepeatMode)
    {
        case AUDIO_FOLDER_ONCE:
            RepeatMode= IMG_ID_MUSIC_LOOP_MODE03;
            break;

        case AUIDO_FOLDER_REPEAT:
            RepeatMode= IMG_ID_MUSIC_LOOP_MODE04;
            break;

        case AUDIO_REPEAT:
            RepeatMode= IMG_ID_MUSIC_LOOP_MODE02;
            break;

        case AUDIO_ALLONCE:
            RepeatMode= IMG_ID_MUSIC_LOOP_MODE05;
            break;

        case AUDIO_ALLREPEAT:
            RepeatMode= IMG_ID_MUSIC_LOOP_MODE06;
            break;

        case AUDIO_REPEAT1:
            RepeatMode= IMG_ID_MUSIC_LOOP_MODE02;
            break;
    }

    if(gpstPlayMenuData->hLoop == NULL)
    {
        pstIconArg.resource = RepeatMode;
        pstIconArg.x = 3;
        pstIconArg.y = 141;
        pstIconArg.level = 0;
        pstIconArg.display = 1;
        gpstPlayMenuData->hLoop= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);

    }
    else
    {
        GuiTask_OperWidget(gpstPlayMenuData->hLoop, OPERATE_SET_CONTENT, (void *)RepeatMode, SYNC_MODE);
    }
#endif
}


/*******************************************************************************
** Name: MusicPlay_SpectrumDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_SpectrumDisplay(void)
{
#ifdef _USE_GUI_
    RKGUI_SPECTRUM_ARG pstSpectrumArg;
    RKGUI_SPECTRUM_ARG pstSpectrumContent;

    uint8 buf[12]= {0,0,0,0,0,0,0,0,0,0,0,0};

    if(gpstPlayMenuData->hSpecrm == NULL)
    {
        pstSpectrumArg.x= 23;
        pstSpectrumArg.y= 29;
        pstSpectrumArg.level= 0;
        pstSpectrumArg.display= 1;
        pstSpectrumArg.StripNum= 12;
        pstSpectrumArg.StripStyle= IMG_ID_MUSIC_SPECTRUM20;
        pstSpectrumArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstSpectrumArg.BackdropX= 23;
        pstSpectrumArg.BackdropY= 29;
        pstSpectrumArg.StripDat= buf;
        gpstPlayMenuData->hSpecrm = GUITask_CreateWidget(GUI_CLASS_SPECTRUM, &pstSpectrumArg);
    }
    else
    {
        pstSpectrumContent.cmd= SPECTRUM_SET_CONTENT;
        AudioPlayer_GetCurSpectrum(&pstSpectrumContent.StripDat);
        GuiTask_OperWidget(gpstPlayMenuData->hSpecrm, OPERATE_SET_CONTENT, &pstSpectrumContent, SYNC_MODE);

    }
#endif
}

/*******************************************************************************
** Name: MusicPlay_StateDisplay
** Input:uint32 state
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_StateDisplay()
{
#ifdef _USE_GUI_
    uint32 PlayerState;
    RKGUI_ICON_ARG pstIconArg;

    if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PAUSE)
    {
        PlayerState = IMG_ID_MUSIC_PLAY_STATE;
    }
    else if (gpstPlayMenuData->PlayerState == AUDIO_STATE_PLAY)
    {
        PlayerState = IMG_ID_MUSIC_PAUSE_STATE;
    }
    else if (gpstPlayMenuData->PlayerState == AUDIO_STATE_FFW)
    {
        PlayerState = IMG_ID_MUSIC_FFW_STATE;
    }
    else if (gpstPlayMenuData->PlayerState == AUDIO_STATE_FFD)
    {
        PlayerState = IMG_ID_MUSIC_FFD_STATE;
    }
    else if (gpstPlayMenuData->PlayerState == AUDIO_STATE_STOP)
    {
        PlayerState = IMG_ID_MUSIC_STOP_STATE;
    }

    if(gpstPlayMenuData->hPlayerState == NULL)
    {
        pstIconArg.resource = PlayerState;//IMG_ID_MUSIC_PLAY_STATE
        pstIconArg.x= 45;
        pstIconArg.y= 141;
        pstIconArg.level = 0;
        pstIconArg.display= 1;
        gpstPlayMenuData->hPlayerState= GUITask_CreateWidget(GUI_CLASS_ICON, &pstIconArg);
    }
    else
    {
        GuiTask_OperWidget(gpstPlayMenuData->hPlayerState, OPERATE_SET_CONTENT, (void *)PlayerState, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: MusicPlay_BitrateDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_BitrateDisplay(void)
{
#ifdef _USE_GUI_
    int i;
    char buf[9];
    Ucs2 Bitrate[9];

    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;

    sprintf(buf, "%dkbps", gpstPlayMenuData->stAudioInfo.Bitrate / 1000);

    for (i= 0; i< sizeof(buf); i++)
    {
        Bitrate[i]= (Ucs2)buf[i];
    }

    Bitrate[i] = 0;

    if(gpstPlayMenuData->hBitrate == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 4;
        pstTextArg.y= 125;
        pstTextArg.xSize= 45;
        pstTextArg.ySize= -1;
        pstTextArg.text = Bitrate;
        pstTextArg.BackdropX= 4;
        pstTextArg.BackdropY= 125;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstPlayMenuData->hBitrate = GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        playerTextContentArg.opercmd= TEXT_SET_CONTENT;
        playerTextContentArg.text = Bitrate;
        GuiTask_OperWidget(gpstPlayMenuData->hBitrate, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }
#endif
}


/*******************************************************************************
** Name: MusicPlay_MusicNameDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_MusicNameDisplay(void)
{
#ifdef _USE_GUI_
    Ucs2 * path, * MusicName;
    uint8 i=0;

    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_TEXT_ARG pstTextArg;



    path= gpstPlayMenuData->stAudioInfo.path;
    MusicName= path;
    while ( *path!= 0x0000 )
    {
        if (*path++== 0x005C)
        {
            MusicName= path;
        }
    }

    if (gpstPlayMenuData->hMusicName == NULL)
    {
        pstTextArg.display= 1;
        pstTextArg.level = 0;
        pstTextArg.lucency= OPACITY;
        pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstTextArg.ForegroundR= 0;
        pstTextArg.ForegroundG= 0;
        pstTextArg.ForegroundB= 0;
        pstTextArg.BackgroundR= 255;
        pstTextArg.BackgroundG= 255;
        pstTextArg.BackgroundB= 255;

        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 30;
        pstTextArg.y= 109;//102
        pstTextArg.xSize= 90;
        pstTextArg.ySize= -1;
        pstTextArg.BackdropX= 30;
        pstTextArg.BackdropY= 109;//102
        pstTextArg.text= MusicName;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstPlayMenuData->hMusicName= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
        GuiTask_OperWidget(gpstPlayMenuData->hMusicName, OPERATE_START_MOVE, (void*)10, SYNC_MODE);

    }
    else
    {
        playerTextContentArg.opercmd= TEXT_SET_CONTENT;
        playerTextContentArg.text = MusicName;
        GuiTask_OperWidget(gpstPlayMenuData->hMusicName, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }
#endif
}

/*******************************************************************************
** Name: MusicPlay_PlayProcessDisplay
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_PlayProcessDisplay(void)
{
#ifdef _USE_GUI_
    int i;
    char buf[9];
    Ucs2 Time[9];
    RKGUI_PROGRESSBAR_ARG pstProgressArg;
    RKGUI_TEXT_ARG pstTextArg;

    uint8 Sec, Min, Hour;

    #if 1
    RKGUI_TEXT_ARG playerTextContentArg;
    RKGUI_PROGRESSBAR_ARG pstProgressBarContent;
    #endif

    AudioPlayer_GetCurTime(&gpstPlayMenuData->MusicProgressTimer);
    AudioPlayer_GetTotalTime(&gpstPlayMenuData->MusicTotalTimer);

    Sec = (gpstPlayMenuData->MusicTotalTimer / 1000) % 60;
    Min = (gpstPlayMenuData->MusicTotalTimer / 60000) % 60;
    Hour = gpstPlayMenuData->MusicTotalTimer / 3600000;

    sprintf(buf, "%02d:%02d:%02d", Hour, Min, Sec);
    //printf ("TotalTime=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        Time[i]= (Ucs2)buf[i];
    }

    Time[i] = 0;

    pstTextArg.display= 1;
    pstTextArg.level = 0;
    pstTextArg.lucency= OPACITY;
    pstTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
    pstTextArg.ForegroundR= 0;
    pstTextArg.ForegroundG= 0;
    pstTextArg.ForegroundB= 0;
    pstTextArg.BackgroundR= 255;
    pstTextArg.BackgroundG= 255;
    pstTextArg.BackgroundB= 255;


    if(gpstPlayMenuData->hMusicTotalTimer == NULL)
    {
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 84;
        pstTextArg.y= 85;
        pstTextArg.xSize= 42;
        pstTextArg.ySize= -1;
        pstTextArg.text= Time;
        pstTextArg.BackdropX= 84;
        pstTextArg.BackdropY= 85;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstPlayMenuData->hMusicTotalTimer= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);
    }
    else
    {
        playerTextContentArg.opercmd= TEXT_SET_CONTENT;
        playerTextContentArg.text = Time;

        GuiTask_OperWidget(gpstPlayMenuData->hMusicTotalTimer, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }

    Sec = (gpstPlayMenuData->MusicProgressTimer / 1000) % 60;
    Min = (gpstPlayMenuData->MusicProgressTimer / 60000) % 60;
    Hour = gpstPlayMenuData->MusicProgressTimer / 3600000;

    sprintf(buf, "%02d:%02d:%02d", Hour, Min, Sec);
    //printf ("CurrentTime=%s\n",buf);
    for (i= 0; i< sizeof(buf); i++)
    {
        Time[i]= (Ucs2)buf[i];
    }

    Time[i] = 0;


    if(gpstPlayMenuData->hMusicProgressTimer == NULL)
    {
        pstTextArg.cmd= TEXT_CMD_BUF;
        pstTextArg.x= 4;
        pstTextArg.y= 85;
        pstTextArg.xSize= 43;
        pstTextArg.ySize= -1;
        pstTextArg.text= Time;
        pstTextArg.BackdropX= 4;
        pstTextArg.BackdropY= 85;
        pstTextArg.align= TEXT_ALIGN_X_LEFT;
        gpstPlayMenuData->hMusicProgressTimer= GUITask_CreateWidget(GUI_CLASS_TEXT, &pstTextArg);

    }
    else
    {
        playerTextContentArg.opercmd= TEXT_SET_CONTENT;
        playerTextContentArg.text = Time;
        GuiTask_OperWidget(gpstPlayMenuData->hMusicProgressTimer, OPERATE_SET_CONTENT, &playerTextContentArg, SYNC_MODE);
    }

    if (gpstPlayMenuData->MusicTotalTimer== 0)return;

    pstProgressBarContent.cmd = PROGRESSBAR_SET_CONTENT;
    pstProgressBarContent.percentage = gpstPlayMenuData->MusicProgressTimer* 100 / gpstPlayMenuData->MusicTotalTimer;

    if(gpstPlayMenuData->hProgressBar == NULL)
    {
        pstProgressArg.x= 7;
        pstProgressArg.y= 78;
        pstProgressArg.display = 1;
        pstProgressArg.level = 0;

        pstProgressArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
        pstProgressArg.BackdropX= 7;
        pstProgressArg.BackdropY= 78;

        pstProgressArg.Bar= IMG_ID_PROGRESS_BAR;
        pstProgressArg.percentage = pstProgressBarContent.percentage;
        gpstPlayMenuData->hProgressBar= GUITask_CreateWidget(GUI_CLASS_PROGRESSBAR, &pstProgressArg);
    }
    else
    {
        //rk_printf("pstProgressBarContent.percentage = %d", pstProgressBarContent.percentage);
        GuiTask_OperWidget(gpstPlayMenuData->hProgressBar, OPERATE_SET_CONTENT, &pstProgressBarContent, SYNC_MODE);
    }
#endif
}


/*******************************************************************************
** Name: MusicPlay_DisplayMusicInfo
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t MusicPlay_DisplayMusicInfo(void)
{
    MusicPlay_MusicNameDisplay();
    MusicPlay_MusicNumDisplay();
    MusicPlay_BitrateDisplay();
    MusicPlay_PlayProcessDisplay();
    MusicPlay_StateDisplay();
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlay_PlayerOpen
** Input:int type
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlay_PlayerOpen(HTC hTask)
{
    int ret = 0;
    MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue;
    ret = MusicPlay_SelectExePlayer(gpstPlayMenuData->Arg.ucSelPlayType);
    if (ret == -1)  //播放器打开失败
    {

        PLAYMENU_DEBUG ("player err\n");

        while (1)
        {
            rkos_queue_receive(gpstPlayMenuData->PlayMenuAskQueue, &PlayMenuAskQueue, MAX_DELAY);
            if(PlayMenuCheckIdle(hTask) != RK_SUCCESS)
            {
                continue;
            }

            if((PlayMenuAskQueue.type == PLAYMENU_KEY_EVENT) ||
                (PlayMenuAskQueue.type == PLAYMENU_MSGBOX_EVENT))
            {
                gpstPlayMenuData->KeyQueueCnt = 0;
            }

            switch (PlayMenuAskQueue.cmd)
            {
                case KEY_VAL_ESC_SHORT_UP:
                    MusicPlay_DeleteGuiHandle();
                    if((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER) || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
                    {
#ifdef __APP_BROWSER_BROWSERUITASK_C__
                        RK_TASK_BROWSER_ARG Arg;
                        Arg.SelSouceType = gpstPlayMenuData->Arg.ucSelPlayType;
                        Arg.Flag = 0;
                        memcpy(Arg.filepath, gpstPlayMenuData->stAudioInfo.path, StrLenW(gpstPlayMenuData->stAudioInfo.path)*2);
                        Arg.filepath[StrLenW(gpstPlayMenuData->stAudioInfo.path)] = 0x0000;
                        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
#endif
                        //memcpy(Arg.filepath, L"C:\\", 8);
                        //MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
                    }
#ifdef _MEDIA_MODULE_
                    else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                    {
                        RK_TASK_MEDIABRO_ARG MediaBro;
                        MediaBro.Flag = 1;
                        MediaBro.MediaTypeSelID = gSysConfig.MediaDirTreeInfo.MediaType;
                        memcpy((UINT8*) &(MediaBro.MediaDirTreeInfo),
                                   (UINT8*) &(gSysConfig.MediaDirTreeInfo),
                                   sizeof(MEDIABRO_DIR_TREE_STRUCT));

                        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_MEDIA_BROWSER, 0, &MediaBro);
                    }
                    else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                    {
                        RK_TASK_BROWSER_ARG Arg;
                        Arg.SelSouceType = gpstPlayMenuData->Arg.ucSelPlayType;
                        Arg.Flag = 1;

                        AudioPlayer_SetFloderInfo();
                        AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
                        memcpy((UINT8*) &(Arg.MediaFloderInfo), (UINT8*) &(gpstPlayMenuData->stAudioInfo.MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));

                        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
                    }
#endif
                    else
                    {
                        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_MAIN, 0, NULL);
                    }

                    while(1)
                    {
                        rkos_sleep(2000);
                    }
                    break;

                default:
                    break;
            }
        }
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlay_InitParams
** Input:int params
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlay_InitParams(int params)
{
    //Local params init
    gpstPlayMenuData->KeyQueueCnt = 0;
    gpstPlayMenuData->Volume = gSysConfig.OutputVolume;
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MusicPlay_SetPopupWindow
** Input:int type
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlay_SetPopupWindow(int type)
{
#ifdef _USE_GUI_
    GUI_CONTROL_CLASS * hGc;
    RKGUI_MSGBOX_ARG pstMsgBoxArg;
    switch (type)
    {
        case WARING_WINDOW:
            gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
            MusicPlay_DisplayAll();
            pstMsgBoxArg.cmd= MSGBOX_CMD_WARNING;
            pstMsgBoxArg.x= 4;
            pstMsgBoxArg.y= 36;
            pstMsgBoxArg.display = 1;
            pstMsgBoxArg.level= 0;
            pstMsgBoxArg.title= SID_WARNING;
            pstMsgBoxArg.text_cmd= TEXT_CMD_ID;
            pstMsgBoxArg.text= (void*)SID_FILE_FORMAT_ERROR;
            pstMsgBoxArg.align = TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;
            gpstPlayMenuData->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
            break;

        default:
            break;
    }
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlay_DisplayMsg
** Input:int type, int status
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_DisplayMsg(uint32 Sid)
{
#ifdef _USE_GUI_
    RKGUI_TEXT_ARG DlnaTextArg;

    DlnaTextArg.cmd= TEXT_CMD_ID;
    DlnaTextArg.lucency= OPACITY;
    DlnaTextArg.Backdrop= IMG_ID_MUSIC_BACKGROUND;
    DlnaTextArg.ForegroundR= 0;
    DlnaTextArg.ForegroundG= 0;
    DlnaTextArg.ForegroundB= 0;
    DlnaTextArg.BackgroundR= 255;
    DlnaTextArg.BackgroundG= 255;
    DlnaTextArg.BackgroundB= 255;
    DlnaTextArg.align= TEXT_ALIGN_X_LEFT;
    DlnaTextArg.display = 1;
    DlnaTextArg.level = 0;
    DlnaTextArg.BackdropX= 25;
    DlnaTextArg.BackdropY= 103;
    DlnaTextArg.x= 25;
    DlnaTextArg.y= 103;
    DlnaTextArg.xSize= 103;
    DlnaTextArg.ySize= -1;

    DlnaTextArg.resource = Sid;
    gpstPlayMenuData->hMusicName = GUITask_CreateWidget(GUI_CLASS_TEXT, &DlnaTextArg);
#endif
}

/*******************************************************************************
** Name: MusicPlay_DeleteGuiHandle
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_DeleteGuiHandle(void)
{
#ifdef _USE_GUI_
    if (gpstPlayMenuData->hMusicName != NULL)
    {
        //GuiTask_OperWidget(gpstPlayMenuData->hDlna, OPERATE_SET_DISPLAY, (void*)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstPlayMenuData->hMusicName);
        gpstPlayMenuData->hMusicName = NULL;
    }

    if (gpstPlayMenuData->hPlayer != NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hPlayer);
        gpstPlayMenuData->hPlayer = NULL;
    }

    if (gpstPlayMenuData->hBackdrop!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hBackdrop);
        gpstPlayMenuData->hBackdrop = NULL;
    }

    if (gpstPlayMenuData->hLoop!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hLoop);
        gpstPlayMenuData->hLoop = NULL;
    }

    if (gpstPlayMenuData->hOrder!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hOrder);
        gpstPlayMenuData->hOrder = NULL;
    }

    if (gpstPlayMenuData->hPlayerState!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hPlayerState);
        gpstPlayMenuData->hPlayerState = NULL;
    }

    if (gpstPlayMenuData->hPage!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hPage);
        gpstPlayMenuData->hPage = NULL;
    }

    if (gpstPlayMenuData->hProgressBar!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hProgressBar);
        gpstPlayMenuData->hProgressBar = NULL;
    }

    if (gpstPlayMenuData->hVolumeBar!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hVolumeBar);
        gpstPlayMenuData->hVolumeBar = NULL;
    }

    if (gpstPlayMenuData->hVolumeValue!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hVolumeValue);
        gpstPlayMenuData->hVolumeValue = NULL;
    }

    if (gpstPlayMenuData->hMusicTotalTimer!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hMusicTotalTimer);
        gpstPlayMenuData->hMusicTotalTimer = NULL;
    }

    if (gpstPlayMenuData->hMusicProgressTimer!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hMusicProgressTimer);
        gpstPlayMenuData->hMusicProgressTimer = NULL;
    }

    if (gpstPlayMenuData->hBitrate!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hBitrate);
        gpstPlayMenuData->hBitrate = NULL;
    }

    if (gpstPlayMenuData->hEQMode!= NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hEQMode);
        gpstPlayMenuData->hEQMode = NULL;
    }

    if (gpstPlayMenuData->hMsgBox != NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hMsgBox);
        gpstPlayMenuData->hMsgBox = NULL;
    }

    if(gpstPlayMenuData->hSpecrm != NULL)
    {
        GuiTask_DeleteWidget(gpstPlayMenuData->hSpecrm);
        gpstPlayMenuData->hSpecrm = NULL;
    }
#endif
}


/*******************************************************************************
** Name: MusicPlay_DisplayInit
** Input:uint32 SourecePlayer
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t MusicPlay_DisplayInit()
{
#ifdef _USE_GUI_
    GuiTask_ScreenLock();
#endif
    MusicPlay_BackGroundDisplay();
    MusicPlay_VolumeDisplay();

    #ifdef _RK_EQ_
    MusicPlay_MusicEqDisplay();
    #endif

    MusicPlay_DisplayAll();

    if(gpstPlayMenuData->stAudioInfo.playerr)
    {
        MusicPlay_SetPopupWindow(WARING_WINDOW);
    }
#ifdef _USE_GUI_
    GuiTask_ScreenUnLock();
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlay_DeletePlayer
** Input:uint32 SourecePlayer
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t MusicPlay_DeletePlayer(uint32 SourecePlayer)
{
    //Delete localplayer
    if ((SourecePlayer == SOURCE_FROM_FILE_BROWSER)
         ||(SourecePlayer == SOURCE_FROM_MEDIA_LIBRARY)
         ||(SourecePlayer == SOURCE_FROM_DB_FLODER)
         ||(SourecePlayer == SOURCE_FROM_RECORD))
    {
        RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
        return RK_SUCCESS;
    }

    //Delete dlnaplayer
    if (SourecePlayer == SOURCE_FROM_HTTP)
    {
        #ifdef __WIFI_DLNA_C__
        rk_dlna_end();
        RKTaskDelete(TASK_ID_DLNA, 0, SYNC_MODE);
        #endif
        //Notification MainTask DlnaPlayer already Deleted
        MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER,0);
        return RK_SUCCESS;
    }

    //Delete btplayer
    if (SourecePlayer == SOURCE_FROM_BT)
    {
        return RK_SUCCESS;
    }

    //Delete XXX
    if (SourecePlayer == SOURCE_FROM_XXX)
    {
        PLAYMENU_DEBUG("Delete XXX\n");
        #ifdef __WIFI_XXX_C__
        XXX_end();
        RKTaskDelete(TASK_ID_XXX,0,SYNC_MODE);
        #endif
        MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER,0);
        return RK_SUCCESS;
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: MusicPlay_StartPlayer
** Input:uint32 SourecePlayer
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t MusicPlay_StartPlayer(uint32 SourecePlayer)
{
#ifdef _USE_GUI_
    int ret;
    RK_TASK_AUDIOCONTROL_ARG Arg;
    AUDIO_INFO AudioInfo;

    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
    {
        MusicPlay_BackGroundDisplay();
        MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
        return RK_ERROR;
    }
    else if (SourecePlayer == SOURCE_FROM_FILE_BROWSER || SourecePlayer == SOURCE_FROM_MEDIA_LIBRARY
                || SourecePlayer == SOURCE_FROM_DB_FLODER|| (SourecePlayer == SOURCE_FROM_RECORD))
    {
        Arg.FileNum = gpstPlayMenuData->Arg.FileNum;
        Arg.TotalFiles = gpstPlayMenuData->Arg.TotalFiles;
        memcpy(Arg.filepath, gpstPlayMenuData->Arg.filepath, sizeof(Arg.filepath));
        memcpy((UINT8*) &(Arg.MediaFloderInfo), (UINT8*) &(gpstPlayMenuData->Arg.MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));

        Arg.ucSelPlayType = SourecePlayer;
        Arg.SaveMemory = 0;
        Arg.DirectPlay = 0;

        Arg.pfAudioState = MusicPlay_AudioCallBack;
        if (RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
        {
            PLAYMENU_DEBUG("Audio control task create failure\n");
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
            return RK_ERROR;
        }

        //Player Menu GUI Init1

        gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(NULL);

        while(gpstPlayMenuData->PlayerState != AUDIO_STATE_PLAY)
        {
             gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(NULL);
             rkos_sleep(10);
        }

        gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);

        if(gpstPlayMenuData->stAudioInfo.TotalFiles == 0)
        {
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
            RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
            return RK_ERROR;
        }
        return RK_SUCCESS;

    }
    else if (SourecePlayer == SOURCE_FROM_HTTP)
    {
        #ifdef _DRIVER_WIFI__
        if ((MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK) == 1)
            || (MainTask_GetStatus(MAINTASK_WIFI_AP_CONNECT_OK) == 1))
        {
            #ifdef __WIFI_DLNA_C__
            if (RKTaskCreate(TASK_ID_DLNA, 0, NULL, SYNC_MODE) != RK_SUCCESS)
            {
                PLAYMENU_DEBUG("Dlna task create failure\n");
                MusicPlay_BackGroundDisplay();
                MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
                return RK_ERROR;
            }

            rk_dlna_start();
            #endif
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 1);
            return RK_SUCCESS;
        }
        else
        {
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_WIFI_TURN_ON);
            return RK_ERROR;
        }
        #endif

    }
    else if (SourecePlayer == SOURCE_FROM_BT)
    {
        if(MainTask_GetStatus(MAINTASK_APP_BT_PLAYER) == 1)
        {
            PLAYMENU_DEBUG ("Already Create Bt Player\n");
            return RK_SUCCESS;
        }
        else
        {
            PLAYMENU_DEBUG ("Not Create Bt Player\n");
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_BT_TURN_ON);
            return RK_ERROR;
        }

    }
    else if (SourecePlayer == SOURCE_FROM_XXX)
    {
        #ifdef _DRIVER_WIFI__
        if ((MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK) == 1)
            || (MainTask_GetStatus(MAINTASK_WIFI_AP_CONNECT_OK) == 1))
        {
            #ifdef __WIFI_XXX_C__
            if (RKTaskCreate(TASK_ID_XXX, 0, NULL, SYNC_MODE) != RK_SUCCESS)
            {
                PLAYMENU_DEBUG("XXX task create failure\n");
                MusicPlay_BackGroundDisplay();
                MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
                return RK_ERROR;
            }
            #endif
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER,1);
            return RK_SUCCESS;

        }
        else
        {
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_WIFI_TURN_ON);
            return RK_ERROR;
        }
        #endif

    }

    return RK_ERROR;
#else //无屏幕操作打开播放器
    RK_TASK_AUDIOCONTROL_ARG Arg;
    PLAYMENU_DEBUG("SourecePlayer =0x%x\n",SourecePlayer);
    rkos_semaphore_take(gpstPlayMenuData->osStartPlayerSem, MAX_DELAY);
    //启动本地播放
    if((SourecePlayer == SOURCE_FROM_FILE_BROWSER)
        &&(MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER)!=1))

    {//PLAYMENU_DEBUG("Music start22222222\n");
        Arg.ucSelPlayType = SOURCE_FROM_FILE_BROWSER;
        Arg.FileNum = 0;
        Arg.TotalFiles = 0;
        Arg.SaveMemory = 0;
        Arg.DirectPlay = 0;
        Arg.pfAudioState = MusicPlay_AudioCallBack;
        //Arg.pfAudioState = NULL;
        memcpy(Arg.filepath, L"C:\\", 8);
		//PLAYMENU_DEBUG("Music start33333333\n");
        if(RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
        {
             rk_printf("Audio control task create failure");
             rkos_semaphore_give(gpstPlayMenuData->osStartPlayerSem);
             return RK_SUCCESS;
        }
        PLAYMENU_DEBUG("Music start\n");
        gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;
        gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_FILE_BROWSER;
        MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 1);
    }
    #if !defined(_ENABLE_WIFI_BLUETOOTH) && defined(_BLUETOOTH_)
    {
        if(SourecePlayer == SOURCE_FROM_BT) //Bt
        {
            gSysConfig.BtControl = 1;
            FW_LoadSegment(SEGMENT_ID_BLUETOOTH, SEGMENT_OVERLAY_ALL);
            if(bluetooth_start() == RK_SUCCESS)
            {
                gSysConfig.PlayerType = SOURCE_FROM_BT;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_BT;
                rk_printf ("BT Success....\n");
            }
        }
    }
    #endif//!defined(_ENABLE_WIFI_BLUETOOTH) && defined(_BLUETOOTH_)
	
    #ifdef _WIFI_
    {
        //启动DLNA 播放器
        if((SourecePlayer == SOURCE_FROM_HTTP)
            &&(MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)!=1))
        {
            if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK) == 1)
            {
                #ifdef __WIFI_DLNA_C__
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
                if (RKTaskCreate(TASK_ID_DLNA, 0, NULL, SYNC_MODE) != RK_SUCCESS)
                {
                    PLAYMENU_DEBUG("Dlna task create failure\n");
                    MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
                    rkos_semaphore_give(gpstPlayMenuData->osStartPlayerSem);
                    return RK_ERROR;
                }
                PLAYMENU_DEBUG("DLNA start\n");
                rk_dlna_start();
                gSysConfig.PlayerType = SOURCE_FROM_HTTP;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_HTTP;
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 1);
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
                //等待DLNA创建完成
                rkos_sleep(2000);
                #endif
            }
            else
            {
                PLAYMENU_DEBUG("DLNA virtual start\n");
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
                gSysConfig.PlayerType = SOURCE_FROM_HTTP;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_HTTP;
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
            }
        }
		
#ifdef _ENABLE_WIFI_BLUETOOTH
        if(SourecePlayer == SOURCE_FROM_BT) //Bt
        {
            gSysConfig.BtControl = 1;
            FW_LoadSegment(SEGMENT_ID_BLUETOOTH, SEGMENT_OVERLAY_ALL);
            if(bluetooth_start() == RK_SUCCESS)
            {
                gSysConfig.PlayerType = SOURCE_FROM_BT;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_BT;
                rk_printf ("BT Success....\n");
            }
			MainTask_SetStatus(MAINTASK_APP_BT_PLAYER, 1);
        }
#endif//_ENABLE_WIFI_BLUETOOTH
		
        //XXX
        if((SourecePlayer == SOURCE_FROM_XXX)
            &&(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER)!=1))
        {
            if(MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK) == 1)
            {
                MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
                #ifdef __WIFI_XXX_C__
                if (RKTaskCreate(TASK_ID_XXX, 0, NULL, SYNC_MODE) != RK_SUCCESS)
                {
                    PLAYMENU_DEBUG("XXX task create failure\n");
                    rkos_semaphore_give(gpstPlayMenuData->osStartPlayerSem);
                    return RK_ERROR;
                }
                #endif
                gSysConfig.PlayerType = SOURCE_FROM_XXX;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_XXX;
                MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 1);
                MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 0);
            }
            else
            {
                PLAYMENU_DEBUG("XXX virtual start\n");
                MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
                gSysConfig.PlayerType = SOURCE_FROM_XXX;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_XXX;
                MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
            }
        }
    }
    #endif//#ifdef _WIFI_
    rkos_semaphore_give(gpstPlayMenuData->osStartPlayerSem);
#endif//#ifdef _USE_GUI_
}
#ifndef _USE_GUI_
/*******************************************************************************
** Name: MusicPlayMenuTask_StartTimer
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API void MusicPlayMenuTask_StartTimer(void)
{
#ifdef _WIFI_
    if (gpstPlayMenuData->TimerCount == 0)
    {
        gpstPlayMenuData->TimerCount++;
        rkos_start_timer(gpstPlayMenuData->Timer);
    }
#endif
    return ;
}

/*******************************************************************************
** Name: MusicPlayMenuTask_CloseTimer
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API void MusicPlayMenuTask_CloseTimer(void)
{
#ifdef _WIFI_
    if (gpstPlayMenuData->TimerCount == 1)
    {
        gpstPlayMenuData->TimerCount--;
        rkos_stop_timer(gpstPlayMenuData->Timer);
    }
#endif
    return ;
}


/*******************************************************************************
** Name: MusicPlayMenuTask_Timer
** Input:uint32 event
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API void MusicPlayMenuTask_Timer(void * handle)
{
#ifdef _WIFI_
    //如果WIFI休眠，侧关闭定时器(停止WIFI 配置)
    if (MainTask_GetStatus(MAINTASK_WIFI_SUSPEND)==1 ||
		(NULL == RKTaskFind(TASK_ID_WIFI_APPLICATION, 0)))
    {
        rk_printf ("PlayMenuTask_Timer---WIFI suspend");
        gpstPlayMenuData->smartconfig=0;
        MainTask_SetStatus(MAINTASK_WIFICONFIG,0);
        MusicPlayMenuTask_CloseTimer();
        return ;
    }

    //WIFI 连接成功
    if (MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)==1)
    {
        rk_printf ("PlayMenuTask_Timer---WIFI connect ok");
        #if 0
        //WIFI 稳定时间
        rkos_sleep(6000);
        if (MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)!=1)
        {
            rk_printf ("PlayMenuTask_Timer---WIFI connecting");
            //WIFI 配置中，继续定时检测
            rkos_start_timer(gpstPlayMenuData->Timer);
            return;
        }
        #endif
        MainTask_SetStatus(MAINTASK_WIFICONFIG,0);
        if (MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER) != 1)
        {
            MusicPlayMenuTask_SendStartPlayer(0);
        }
        gpstPlayMenuData->smartconfig=0;
        MusicPlayMenuTask_CloseTimer();
    }
    else
    {
        rk_printf ("PlayMenuTask_Timer---WIFI connecting");
        //WIFI 配置中，继续定时检测
        if (gpstPlayMenuData->TimerCount == 1)
            rkos_start_timer(gpstPlayMenuData->Timer);
    }
#endif
}


/*******************************************************************************
** Name: MusicPlay_NoScreenKeyHandle
** Input:uint32 event
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API int MusicPlayMenuTask_SendStartPlayer(uint8 deleteplayer)
{
#ifdef _WIFI_
    MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue;
    PlayMenuAskQueue.cmd  = 0; //start player
    PlayMenuAskQueue.type = PLAYMENU_NOSCREEN_EVENT;

    if (gpstPlayMenuData->startplayer == 1)
        return 0;
    gpstPlayMenuData->startplayer = 1;

    rkos_queue_send(gpstPlayMenuData->PlayMenuAskQueue, &PlayMenuAskQueue, 0);
#endif
}

/*******************************************************************************
** Name: MusicPlayMenuTask_AudioEvent
** Input:int type
** Return: int
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN int MusicPlayMenuTask_NoScreenEvent(uint32 cmd, int type)
{
    switch (cmd)
    {
        case 0:
#ifdef _WIFI_
			rk_printf("1.wifi connect ok:start palyer=0x%x",gSysConfig.PlayerType);
			MusicPlay_StartPlayer(gSysConfig.PlayerType);
			//CheckOTAandUpdateFw();//check ota and update fw //jjjhhh 20161105 must place here
#endif
            break;
        default:
            break;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlay_NoScreenKeyRev
** Input:uint32 event
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t MusicPlay_NoScreenKeyRev(uint32 keyvalue)
{
    rk_err_t ret;
    MUSIC_PLAY_MENU_ASK_QUEUE PMAskQueue;

    //printf ("--------app is: playmenu\n");
    rkos_semaphore_take(gpstPlayMenuData->osPlayKeySem, MAX_DELAY);
    if (keyvalue==0)
    {
        if (gpstPlayMenuData->keyvalue == 0)
        {
            rkos_semaphore_give(gpstPlayMenuData->osPlayKeySem);
            //rk_printf ("--------app is: playmenu 1\n");
            return RK_SUCCESS;
        }
        else
        {
            goto NEXT;
        }
    }


    gpstPlayMenuData->keyvalue = keyvalue;
    if (gpstPlayMenuData->keycontrol == 1)
    {
        rkos_semaphore_give(gpstPlayMenuData->osPlayKeySem);
        //rk_printf ("--------app is: playmenu 2\n");
        return RK_SUCCESS;
    }

NEXT:
    PMAskQueue.cmd = gpstPlayMenuData->keyvalue;
    PMAskQueue.type = PLAYMENU_KEY_EVENT;
    gpstPlayMenuData->keyvalue = 0;
    gpstPlayMenuData->keycontrol = 1;
    ret = rkos_queue_send(gpstPlayMenuData->PlayMenuAskQueue, &PMAskQueue, 0);
    if (ret == RK_ERROR)
    {
        gpstPlayMenuData->keycontrol = 0;
        //rk_printf ("--------app is: playmenu 3\n");
    }

    rkos_semaphore_give(gpstPlayMenuData->osPlayKeySem);
    //rk_printf ("--------app is: playmenu 4\n");
    return RK_SUCCESS;
}
#endif

#ifdef _USE_GUI_
/*******************************************************************************
** Name: MusicPlayMenuTask_GuiCallBack
** Input:uint32 evnet_type,uint32 event,void * arg,uint32 mode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN rk_err_t MusicPlayMenuTask_GuiCallBack(APP_RECIVE_MSG_EVENT evnet_type, uint32 event, void * arg, HGC pGc)
{
    rk_err_t ret = 0;
    MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue;

    if ((gpstPlayMenuData->KeyQueueCnt == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP))
    {
        return RK_SUCCESS;
    }

    if(evnet_type == APP_RECIVE_MSG_EVENT_KEY)
    {
        PlayMenuAskQueue.cmd  = event;
        PlayMenuAskQueue.type = PLAYMENU_KEY_EVENT;
        gpstPlayMenuData->KeyQueueCnt = 1;
        ret = rkos_queue_send(gpstPlayMenuData->PlayMenuAskQueue, &PlayMenuAskQueue, 0);
        if (ret == RK_ERROR)
        {
            gpstPlayMenuData->KeyQueueCnt = 0;
            rk_printf("lose key event = %d", event);
            return RK_SUCCESS;
        }

    }
    else if(evnet_type == APP_RECIVE_MSG_EVENT_WARING)
    {
        rk_err_t ret;
        MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue;

        PlayMenuAskQueue.type = PLAYMENU_MSGBOX_EVENT;
        PlayMenuAskQueue.cmd = event;
        gpstPlayMenuData->KeyQueueCnt = 1;
        ret = rkos_queue_send(gpstPlayMenuData->PlayMenuAskQueue, &PlayMenuAskQueue, 0);
        if (ret == RK_ERROR)
        {
            gpstPlayMenuData->KeyQueueCnt = 0;
            rk_printf("lose msgbox event = %d", event);
            return RK_SUCCESS;
        }


    }
    return RK_SUCCESS;
}
#endif

/*******************************************************************************
** Name: MusicPlay_AudioCallBack
** Input:uint32 audio_state
** Return: void
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON FUN void MusicPlay_AudioCallBack(uint32 audio_state)
{
#ifdef _USE_GUI_
    rk_err_t ret = 0;
    MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue;
    MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue_tmp;

    if(gpstPlayMenuData->PlayerState == audio_state)
    {
        return;
    }
    else if(audio_state == AUDIO_STATE_VOLUME_CHANGE)
    {
        gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
        if(gpstPlayMenuData->stAudioInfo.PlayVolume == gpstPlayMenuData->Volume)
        {
            return;
        }
        else
        {
           gpstPlayMenuData->Volume = gpstPlayMenuData->stAudioInfo.PlayVolume;
        }
    }

    PlayMenuAskQueue.type = PLAYMENU_AUDIO_EVENT;

    PlayMenuAskQueue.cmd = audio_state;

retry:
    ret = rkos_queue_send(gpstPlayMenuData->PlayMenuAskQueue, &PlayMenuAskQueue, 0);
#else
    if (audio_state == AUDIO_STATE_ERROR)
    {
        gpstPlayMenuData->PlayerState = AUDIO_STATE_ERROR;
        printf("audio call back...ERROR\n");
    }
    else
    {
        gpstPlayMenuData->PlayerState = audio_state;
    }
#endif

}

/*******************************************************************************
** Name: MusicPlay_DisplayAll
** Input:void
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
static COMMON FUN rk_err_t MusicPlay_DisplayAll()
{
#ifdef _USE_GUI_
    GuiTask_ScreenLock();
#endif
    MusicPlay_PlayOrderDisplay();
    MusicPlay_RepeatModeDisplay();
    MusicPlay_DisplayMusicInfo();
#ifdef _USE_GUI_
    GuiTask_ScreenUnLock();
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlay_SelectExePlayer
** Input:uint32 SourecePlayer
** Return: rk_err_t
** Owner:wrm
** Date: 2015.11.03
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
static COMMON FUN rk_err_t MusicPlay_SelectExePlayer(uint32 SourecePlayer)
{
#ifdef _USE_GUI_
    int ret, samesong;
    uint32 new_pathlen, old_pathlen;

    RK_TASK_AUDIOCONTROL_ARG Arg;

    //Local Player From MainTask
    if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_CUR_AUDIO)
    {
        if(MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)
            || MainTask_GetStatus(MAINTASK_APP_BT_PLAYER)
            || MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER))
        {
            MainTask_AudioCallBack(AUDIO_STATE_CUSTOM);
            AudioControlTask_SetStateChangeFunc(MainTask_AudioCallBack, MusicPlay_AudioCallBack);
            gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
            gpstPlayMenuData->Arg.ucSelPlayType = gpstPlayMenuData->stAudioInfo.AudioSource;

            if(gpstPlayMenuData->stAudioInfo.playerr)
            {
                MusicPlay_SetPopupWindow(WARING_WINDOW);
            }
            return RK_SUCCESS;
        }
        else if(MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER))
        {
            gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
            gpstPlayMenuData->Arg.ucSelPlayType = gpstPlayMenuData->stAudioInfo.AudioSource;
            memcpy(gpstPlayMenuData->Arg.filepath, gpstPlayMenuData->stAudioInfo.path, MAX_FILENAME_LEN * 2);
            memcpy((UINT8*) &(gpstPlayMenuData->Arg.MediaFloderInfo), (UINT8*) &(gpstPlayMenuData->stAudioInfo.MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));
            AudioControlTask_SetStateChangeFunc(MainTask_AudioCallBack, MusicPlay_AudioCallBack);
            MainTask_AudioCallBack(AUDIO_STATE_CUSTOM);
            return RK_SUCCESS;
        }
        else
        {
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
            return RK_ERROR;
        }

    }
    else if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER
                || gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY
                || gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER
                || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
    {
        if(MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER)
            || MainTask_GetStatus(MAINTASK_APP_BT_PLAYER)
            || MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER))
        {
            MusicPlay_BackGroundDisplay();
            MusicPlay_DisplayMsg(SID_PLAYER_FAILED);
            return RK_ERROR;
        }
        else if (MainTask_GetStatus (MAINTASK_APP_LOCAL_PLAYER))
        {
            AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);

            if(gpstPlayMenuData->stAudioInfo.AudioSource == gpstPlayMenuData->Arg.ucSelPlayType)
            {
                memcpy(Arg.filepath, gpstPlayMenuData->Arg.filepath, sizeof(Arg.filepath));
                //判断歌曲是否一样
                new_pathlen = StrLenW(Arg.filepath);
                old_pathlen = StrLenW(gpstPlayMenuData->stAudioInfo.path);
                if (new_pathlen != old_pathlen)
                {
                    samesong = -1;
                }
                else
                {
                    samesong = StrCmpW(Arg.filepath, gpstPlayMenuData->stAudioInfo.path, new_pathlen);
                }

#ifdef _MEDIA_MODULE_
                if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
                {
                    if((gpstPlayMenuData->stAudioInfo.TotalFiles != gpstPlayMenuData->Arg.TotalFiles)
                        ||(gpstPlayMenuData->stAudioInfo.BaseID != gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[gSysConfig.MediaDirTreeInfo.MusicDirDeep]))
                    {
                        samesong = -1;
                    }
                }
                else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
                {
                    if(gSysConfig.MusicConfig.RepeatModeBak > AUDIO_REPEAT) //All files
                       samesong = -1;
                }
#endif
                if (samesong == 0) //歌曲一样
                {
                    gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);

                    AudioControlTask_SetStateChangeFunc(MainTask_AudioCallBack, MusicPlay_AudioCallBack);
                    MainTask_AudioCallBack(AUDIO_STATE_CUSTOM);

                    if (gpstPlayMenuData->PlayerState == AUDIO_STATE_ERROR)
                    {
                        MusicPlay_SetPopupWindow(WARING_WINDOW);
                    }
                }
                else
                {
                    //Delete Local Player
                    MainTask_AudioCallBack(AUDIO_STATE_CUSTOM);
                    MusicPlay_DeletePlayer(gpstPlayMenuData->Arg.ucSelPlayType);
                    return MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
                }
            }
            else
            {
                //Delete Local Player
                MainTask_AudioCallBack(AUDIO_STATE_CUSTOM);
                MusicPlay_DeletePlayer(gpstPlayMenuData->Arg.ucSelPlayType);
                return MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
            }
        }
        else
        {
            ret = MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
            if(ret == RK_SUCCESS)
            {
                 MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 1);
            }
        }

    }
    else if((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_HTTP)
            || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
            || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_XXX))
    {
        uint32 app;

        if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_HTTP)
        {
            app = MAINTASK_APP_DLNA_PLAYER;
        }
        else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
        {
            app = MAINTASK_APP_BT_PLAYER;
        }
        else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_XXX)
        {
            app = MAINTASK_APP_XXX_PLAYER;
        }

        if (MainTask_GetStatus (app) == 1)
        {
            MainTask_AudioCallBack(AUDIO_STATE_CUSTOM);
            AudioControlTask_SetStateChangeFunc(MainTask_AudioCallBack, MusicPlay_AudioCallBack);
            gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);

            if(gpstPlayMenuData->stAudioInfo.playerr)
            {
                MusicPlay_SetPopupWindow(WARING_WINDOW);
            }
            return RK_SUCCESS;
        }
        else
        {
            ret = MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
            if(ret == RK_SUCCESS)
            {
                while(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) == NULL)
                {
                    rkos_delay(10);
                }
                AudioControlTask_SetStateChangeFunc(NULL, MusicPlay_AudioCallBack);
                gpstPlayMenuData->PlayerState = AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
            }
            return ret;
        }

    }
#else //无屏幕操作打开播放器
    RK_TASK_AUDIOCONTROL_ARG Arg;
    if(SourecePlayer == SOURCE_FROM_FILE_BROWSER) //Local
    {
        Arg.ucSelPlayType = SOURCE_FROM_FILE_BROWSER;
        Arg.FileNum = 0;
        Arg.TotalFiles = 0;
        Arg.SaveMemory = 0;
        Arg.DirectPlay = 0;
        Arg.pfAudioState = NULL;
        memcpy(Arg.filepath, L"C:\\", 8);

        if(RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
        {
             rk_printf("Audio control task create failure");
             return RK_SUCCESS;
        }
        gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;
        gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_FILE_BROWSER;
        MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 1);
    }
    #ifdef _BLUETOOTH_
        if(SourecePlayer == SOURCE_FROM_BT) //Bt
        {
            gSysConfig.BtControl = 1;
            FW_LoadSegment(SEGMENT_ID_BLUETOOTH, SEGMENT_OVERLAY_ALL);
            if(bluetooth_start() == RK_SUCCESS)
            {
                gSysConfig.PlayerType = SOURCE_FROM_BT;
                gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_BT;
                rk_printf ("BT Success....\n");
            }
        }
    #endif
    #ifdef _WIFI_
        if(SourecePlayer == SOURCE_FROM_HTTP) //DLNA
        {
            #ifdef __WIFI_DLNA_C__
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
            if (RKTaskCreate(TASK_ID_DLNA, 0, NULL, SYNC_MODE) != RK_SUCCESS)
            {
                PLAYMENU_DEBUG("Dlna task create failure\n");
                MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
                return RK_ERROR;
            }
            rk_dlna_start();
            gSysConfig.PlayerType = SOURCE_FROM_HTTP;
            gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_HTTP;
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 1);
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
            #endif
        }
        if(SourecePlayer == SOURCE_FROM_XXX) //XXX
        {
            #ifdef __WIFI_XXX_C__
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
            if (RKTaskCreate(TASK_ID_XXX, 0, NULL, SYNC_MODE) != RK_SUCCESS)
            {
                PLAYMENU_DEBUG("XXX task create failure\n");
                return RK_ERROR;
            }
            gSysConfig.PlayerType = SOURCE_FROM_XXX;
            gpstPlayMenuData->Arg.ucSelPlayType = SOURCE_FROM_XXX;
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 1);
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 0);
            #endif
        }
    #endif

#endif
    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MusicPlay_SetMusicPath
** Input:RK_TASK_AUDIOCONTROL_ARG * Arg
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API void MusicPlay_SetMusicPath(RK_TASK_PLAYMENU_ARG * Arg)
{
    gpstPlayMenuData->Arg.ucSelPlayType = Arg->ucSelPlayType;
    if(gpstPlayMenuData->Arg.ucSelPlayType != SOURCE_FROM_CUR_AUDIO)
    {
        gpstPlayMenuData->Arg.FileNum = Arg->FileNum;
        gpstPlayMenuData->Arg.TotalFiles= Arg->TotalFiles;
    }

    if((gpstPlayMenuData->Arg.ucSelPlayType != SOURCE_FROM_HTTP)
        && (gpstPlayMenuData->Arg.ucSelPlayType != SOURCE_FROM_BT)
        && (gpstPlayMenuData->Arg.ucSelPlayType != SOURCE_FROM_CUR_AUDIO))
    {
        memcpy(gpstPlayMenuData->Arg.filepath, Arg->filepath, StrLenW(Arg->filepath)*2);
        gpstPlayMenuData->Arg.filepath[StrLenW(Arg->filepath)] = 0x0000;
        memcpy((UINT8*) &(gpstPlayMenuData->Arg.MediaFloderInfo), (UINT8*) &(Arg->MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));
    }
}

/*******************************************************************************
** Name: MusicPlayMenuTask_Enter
** Input:int flag, uint8 deleteplayer
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API int MusicPlayMenuTask_DeleteSelf(uint8 deleteplayer)
{
    MusicPlay_DeleteGuiHandle();

    if (deleteplayer == 1)
    {
        MusicPlay_DeletePlayer(gpstPlayMenuData->Arg.ucSelPlayType);
        if((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
            || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
            || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
            || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
        {
            MainTask_SetStatus(MAINTASK_APP_LOCAL_PLAYER, 0);
        }
    }

    if ((gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER) || (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_RECORD))
    {
        RK_TASK_BROWSER_ARG Arg;
        Arg.SelSouceType = gpstPlayMenuData->Arg.ucSelPlayType;
        Arg.Flag = 0;
        memcpy(Arg.filepath, gpstPlayMenuData->stAudioInfo.path, StrLenW(gpstPlayMenuData->stAudioInfo.path)*2);
        Arg.filepath[StrLenW(gpstPlayMenuData->stAudioInfo.path)] = 0x0000;
        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
    }
#ifdef _MEDIA_MODULE_
    else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_MEDIA_LIBRARY)
    {
        RK_TASK_MEDIABRO_ARG MediaBro;
        MediaBro.Flag = 1;
        MediaBro.MediaTypeSelID = gSysConfig.MediaDirTreeInfo.MediaType;
        memcpy((UINT8*) &(MediaBro.MediaDirTreeInfo),
                   (UINT8*) &(gSysConfig.MediaDirTreeInfo),
                   sizeof(MEDIABRO_DIR_TREE_STRUCT));

        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_MEDIA_BROWSER, 0, &MediaBro);
    }
    else if(gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_DB_FLODER)
    {
        RK_TASK_BROWSER_ARG Arg;
        Arg.SelSouceType = gpstPlayMenuData->Arg.ucSelPlayType;
        Arg.Flag = 1;

        AudioPlayer_SetFloderInfo();
        AudioPlayer_GetAudioInfo(&gpstPlayMenuData->stAudioInfo);
        memcpy((UINT8*) &(Arg.MediaFloderInfo), (UINT8*) &(gpstPlayMenuData->stAudioInfo.MediaFloderInfo), sizeof(MEDIA_FLODER_INFO_STRUCT));

        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_BROWSERUI, 0, &Arg);
    }
#endif
    else
    {
        MainTask_TaskSwtich(TASK_ID_MUSIC_PLAY_MENU, 0, TASK_ID_MAIN, 0, NULL);
    }

   return RK_SUCCESS;
}

/*******************************************************************************
** Name: MusicPlayMenuTask_Enter
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API void MusicPlayMenuTask_Enter(void)
{
    MUSIC_PLAY_MENU_ASK_QUEUE PlayMenuAskQueue;
    HTC hSelf;
    hSelf = RKTaskGetRunHandle();


    MusicPlay_InitParams(0);
#ifdef _USE_GUI_
    //Player Open
    MusicPlay_PlayerOpen(hSelf); //若播放器打开失败，则不会再往下执行

    AudioPlayer_SetSpectrumEn(1);

    MusicPlay_DisplayInit();
#else
    while (1)
    {
        //选择默认播放器
        if ((gSysConfig.PlayerType>SOURCE_FROM_NET)||(gSysConfig.PlayerType<SOURCE_FROM_FILE_BROWSER))
        {
            //printf ("2:------------gSysConfig.PlayerType=0x%x\n",gSysConfig.PlayerType);
            gpstPlayMenuData->Arg.ucSelPlayType = gSysConfig.PlayerType;
            break;
        }
        rkos_sleep(1000);
    }
    //启动蓝牙播放器
    #if !defined(_ENABLE_WIFI_BLUETOOTH) && defined(_BLUETOOTH_)
    {
        #ifdef NOSCREEN_USE_LED
        {
            if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
            {
                MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
            }
            if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
            {
                MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
            }
        }
        #endif
        MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
    }
    #endif

    //启动WIFI播放器
    #ifdef _WIFI_
#ifdef _ENABLE_WIFI_BLUETOOTH
	if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
    {
        /*#ifdef NOSCREEN_USE_LED
        {
            if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_BT)
            {
                MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
            }
            if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
            {
                MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
            }
        }
        #endif*/
        MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
    }
	else
#endif//_ENABLE_WIFI_BLUETOOTH
    {
        /*#ifdef NOSCREEN_USE_LED
        {
            if (MainTask_GetStatus(MAINTASK_WIFI_CONNECT_OK)!=1)
            {
                if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                }
                if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_HTTP)
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                    //test
                    MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 1);
                }

                if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_XXX)
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                    //test
                    MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 1);
                }
            }
        }
        #endif*/
        if (gpstPlayMenuData->Arg.ucSelPlayType == SOURCE_FROM_FILE_BROWSER)
            MusicPlay_StartPlayer(gpstPlayMenuData->Arg.ucSelPlayType);
        else
        {
            //gpstPlayMenuData->smartconfig=1; //WIFI 还在配置状态标志
            MainTask_SetStatus(MAINTASK_WIFICONFIG,1);
        }

        //启动WIFI
        RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_STA, SYNC_MODE);
        while (wifi_init_flag() == WICED_FALSE)
        {
            rkos_sleep(1000);
        }
        rk_printf("wifi init ok-------------------------------\n");
        gpstPlayMenuData->WIFIControl = 1;

        if(gpstPlayMenuData->Arg.ucSelPlayType != SOURCE_FROM_FILE_BROWSER)
        {
            MusicPlayMenuTask_StartTimer();
        }
    }
    #endif
#endif
    while(1)
    {
        rkos_queue_receive(gpstPlayMenuData->PlayMenuAskQueue, &PlayMenuAskQueue, MAX_DELAY);

        if(PlayMenuCheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

        switch (PlayMenuAskQueue.type)
        {
            case PLAYMENU_KEY_EVENT:
                MusicPlayMenuTask_KeyEvent(PlayMenuAskQueue.cmd);
                #ifdef _USE_GUI_
                gpstPlayMenuData->KeyQueueCnt = 0;
                #else
                gpstPlayMenuData->keycontrol = 0;
                MusicPlay_NoScreenKeyRev(0);
                #endif
                break;

            case PLAYMENU_AUDIO_EVENT:
                MusicPlayMenuTask_AudioEvent(PlayMenuAskQueue.cmd);
                break;

            case PLAYMENU_MSGBOX_EVENT:
                MusicPlayMenuTask_MsgBoxEvent(PlayMenuAskQueue.cmd, 0);
                gpstPlayMenuData->KeyQueueCnt = 0;
                break;
            case PLAYMENU_NOSCREEN_EVENT:
                #ifndef _USE_GUI_
                #ifdef _WIFI_
                //PLAYMENU_DEBUG("1:setup_flag=%d\n",wifi_easy_setup_flag());
                MusicPlayMenuTask_NoScreenEvent(PlayMenuAskQueue.cmd, 0);
                gpstPlayMenuData->startplayer = 0;
                #endif
                #endif
                break;
            default:
                PLAYMENU_DEBUG ("default\n");
                break;
        }
    }
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MusicPlayMenuTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_COMMON_
COMMON API rk_err_t MusicPlayMenuTask_DeInit(void *pvParameters)
{
#ifdef _USE_GUI_
    MusicPlay_DeleteGuiHandle();
    GuiTask_AppUnReciveMsg(MusicPlayMenuTask_GuiCallBack);
#else
    MainTask_UnRegisterKey();
    #ifdef _WIFI_
    rkos_delete_timer(gpstPlayMenuData->Timer);
    #endif
    rkos_semaphore_delete(gpstPlayMenuData->osPlayKeySem);
#endif
    rkos_queue_delete(gpstPlayMenuData->PlayMenuAskQueue);
    rkos_queue_delete(gpstPlayMenuData->PlayMenuRespQueue);

    rkos_memory_free(gpstPlayMenuData);
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_MUSIC_PLAY_MENU_TASK);
#endif


    PLAYMENU_DEBUG("Delete Music Play Menu Task Success\n");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: PlayMenuTaskResume
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN rk_err_t PlayMenuTaskResume(HTC hTask)
{
    RK_TASK_CLASS*   pPlayMenuTask = (RK_TASK_CLASS*)hTask;
    pPlayMenuTask->State = TASK_STATE_WORKING;
#ifndef _USE_GUI_
#ifdef _WIFI_
    if (gpstPlayMenuData->Closetimer == 1)
        MusicPlayMenuTask_StartTimer();
#endif
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: PlayMenuTaskSuspend
** Input: HTC hTask, uint32  Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MAIN_TASK_MAIN_TASK_COMMON_
COMMON FUN rk_err_t PlayMenuTaskSuspend(HTC hTask, uint32  Level)
{
    RK_TASK_CLASS*   pPlayMenuTask = (RK_TASK_CLASS*)hTask;
    //rk_printf ("PlayMenuTask Suspend");

    if(Level == TASK_STATE_IDLE1)
    {
        pPlayMenuTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pPlayMenuTask->State = TASK_STATE_IDLE2;
    }
#ifndef _USE_GUI_
#ifdef _WIFI_
    if (gpstPlayMenuData->TimerCount == 1)
    {
        gpstPlayMenuData->Closetimer = 1;
        MusicPlayMenuTask_CloseTimer();
    }
#endif
#endif
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MusicPlayMenuTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:wrm
** Date: 2015.8.20
** Time: 17:30:03
*******************************************************************************/
_APP_MUSIC_PLAY_MENU_TASK_INIT_
INIT API rk_err_t MusicPlayMenuTask_Init(void *pvParameters, void *arg)
{
    PLAY_MENU_TASK_DATA_BLOCK*  hPlayMenuTaskData;
    pTimer TimerHandler = NULL;
    RK_TASK_PLAYMENU_ARG * stTaskPlayer = (RK_TASK_PLAYMENU_ARG *)arg;


    RK_TASK_CLASS*   pPlayMenuTask = (RK_TASK_CLASS*)pvParameters;
    pPlayMenuTask->Idle1EventTime = 10 * PM_TIME;
    pPlayMenuTask->TaskSuspendFun = PlayMenuTaskSuspend;
    pPlayMenuTask->TaskResumeFun = PlayMenuTaskResume;

    hPlayMenuTaskData = rkos_memory_malloc(sizeof(PLAY_MENU_TASK_DATA_BLOCK));
    memset(hPlayMenuTaskData, NULL, sizeof(PLAY_MENU_TASK_DATA_BLOCK));

    hPlayMenuTaskData->PlayMenuAskQueue = rkos_queue_create(MUSIC_PLAY_MENU_QUEUE, sizeof(MUSIC_PLAY_MENU_ASK_QUEUE));
    hPlayMenuTaskData->PlayMenuRespQueue = rkos_queue_create(MUSIC_PLAY_MENU_QUEUE, sizeof(MUSIC_PLAY_MENU_RESP_QUEUE));


#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_MUSIC_PLAY_MENU_TASK, SEGMENT_OVERLAY_ALL);
#endif

#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(MusicPlayMenuTask_GuiCallBack);
#else
    #ifndef _USE_GUI_
    {
        hPlayMenuTaskData->osPlayKeySem  = rkos_semaphore_create(1,1);
        hPlayMenuTaskData->osStartPlayerSem  = rkos_semaphore_create(1,1);
        MainTask_RegisterKey(MusicPlay_NoScreenKeyRev);
        hPlayMenuTaskData->keyvalue = 0;
        hPlayMenuTaskData->keycontrol = 0;
        #ifdef _BLUETOOTH_
            gSysConfig.BtOpened=0;
        #endif
        #ifdef _WIFI_
            TimerHandler = rkos_create_timer(100, 100, NULL, MusicPlayMenuTask_Timer);
            hPlayMenuTaskData->WIFIControl = 0;
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER_START, 0);
            MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER_START, 0);
            MainTask_SetStatus(MAINTASK_APP_XXX_PLAYER, 0);
            hPlayMenuTaskData->smartconfig = 0;
            hPlayMenuTaskData->startplayer = 0;
            hPlayMenuTaskData->TimerCount = 0;
            hPlayMenuTaskData->Closetimer = 0;
            hPlayMenuTaskData->Timer = TimerHandler;
            MainTask_SetStatus(MAINTASK_APP_PLAYMENU,1);
        #endif
    }
    #endif
#endif

    gpstPlayMenuData = hPlayMenuTaskData;

    #ifdef _USE_GUI_
    MusicPlay_SetMusicPath(stTaskPlayer);
    #else
    gpstPlayMenuData->Arg.ucSelPlayType = gSysConfig.PlayerType;
    #endif

    return RK_SUCCESS;

}
#endif
