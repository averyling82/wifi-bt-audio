/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\MediaLibrary\media_library.c
* Owner: ctf
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: music play menu task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*      ctf      2016.1.26    17:29:00       1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_MEDIA_LIBRARY_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "Bsp.h"
#include "global.h"
#include "main_task.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "LCDDriver.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "KeyDevice.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"
#include "AddrSaveMacro.h"
#include "AudioControlTask.h"
#include "media_library.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    MEDIA_LIBRARY_UPDATA = 0x11,
    MEDIA_LIBRARY_ENTER,
    MEDIA_LIBRARY_MOVE,
} MEDIA_LIBRARY_TYPE;

typedef struct _MEDIA_LIBRARY_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;
    uint32 offset;
}MEDIA_LIBRARY_ASK_QUEUE;

typedef  struct _MEDIA_LIBRARY_TASK_DATA_BLOCK
{
    pQueue              AskQueue;
    HGC                 hSelect;
    HGC                 hMsgBox;
    uint32              CurId;
    uint8               QueueFull;
    RKGUI_SELECT_ITEM   *pstitem;
    RKGUI_SELECT_ITEM   item[8];
}MEDIA_LIBRARY_TASK_DATA_BLOCK;

static MEDIA_LIBRARY_TASK_DATA_BLOCK * gpstMediaLibDataBlock;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/


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
rk_err_t MediaLibraryTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc);
rk_err_t MediaLibrary_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset);
rk_err_t MediaLibrary_SetPopupWindow(void * text, uint16 text_cmd);
rk_err_t MediaLibrary_KeyEventHD(uint32 event);
rk_err_t MediaLibrary_SelectEnter(uint32 offset);
rk_err_t MediaLibrary_DeleteMsgbox(void);
rk_err_t MediaLibrary_GuiInit(void);
rk_err_t MediaLibrary_GuiInit(void);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MediaLibraryTask_ButtonCallBack
** Input:APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 18:00:52
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON FUN rk_err_t MediaLibraryTask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
    printf("\n MediaLibraryTask_ButtonCallBack \n");
#ifdef _USE_GUI_
    rk_err_t ret = 0;
    MEDIA_LIBRARY_ASK_QUEUE AskQueue;
    MEDIA_LIBRARY_ASK_QUEUE AskQueue_tmp;

    if ((gpstMediaLibDataBlock->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP) && ((event & KEY_STATUS_SHORT_UP) != KEY_STATUS_SHORT_UP))
    {
        rk_printf("lose key MediaLibQueueFull=%d\n", gpstMediaLibDataBlock->QueueFull);
        return RK_SUCCESS;
    }

    switch(event_type)
    {
        case APP_RECIVE_MSG_EVENT_KEY:
        {
            AskQueue.event_type = event_type;
            AskQueue.event  = event;
            AskQueue.offset = 0;
            gpstMediaLibDataBlock->QueueFull = 1;

            rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
            if (ret == RK_ERROR)
            {
                if((event==KEY_VAL_FFW_PRESS)||(event==KEY_VAL_FFD_PRESS)||(event==KEY_VAL_HOLD_PRESS))
                {
                    gpstMediaLibDataBlock->QueueFull = 0;
                    return RK_SUCCESS;
                }

                rkos_queue_receive(gpstMediaLibDataBlock->AskQueue, &AskQueue_tmp, 0);
                if (AskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                {
                    rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue_tmp, 0);
                    rkos_queue_receive(gpstMediaLibDataBlock->AskQueue, &AskQueue_tmp, 0);
                    if (AskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                    {
                        rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue_tmp, 0);
                        gpstMediaLibDataBlock->QueueFull = 0;
                    }
                    else
                    {
                        ret = rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
                        if (ret == RK_ERROR)
                        {
                            rk_printf("MediaLib Send Key Failure 0\n");
                            gpstMediaLibDataBlock->QueueFull = 0;
                        }
                    }
                }
                else
                {
                    ret = rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
                    if (ret == RK_ERROR)
                    {
                        rk_printf("MediaLib Send Key Failure 1\n");
                        gpstMediaLibDataBlock->QueueFull = 0;
                    }
                }
            }

            break;
        }

        case APP_RECIVE_MSG_EVENT_WARING:
        {
            AskQueue.event = event;
            AskQueue.event_type = event_type;
            AskQueue.offset = 0;
            gpstMediaLibDataBlock->QueueFull = 1;

            ret = rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
            if(ret == RK_ERROR)
            {
                rk_printf("\n lose MediaLib_MsgBoxCallBack\n");
                rkos_queue_receive(gpstMediaLibDataBlock->AskQueue, &AskQueue_tmp, 0);//MAX_DELAY
                rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
            }

            break;
        }

        case APP_RECIVE_MSG_EVENT_DIALOG:
        {
            break;
        }

        default:
            break;
    }

#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: BrowserUITask_SelectCallBack
** Input:HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset
** Return:  rk_err_t
** Owner:ctf
** Date: 2015.12.29
** Time: 17:59:39
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON FUN  rk_err_t MediaLibrary_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset)
{
    rk_err_t ret;
    MEDIA_LIBRARY_ASK_QUEUE AskQueue;
    MEDIA_LIBRARY_ASK_QUEUE AskQueue_tmp;

    switch(event_type)
    {
        case SELECT_ENVEN_UPDATA:
            RKGUI_SELECT_ITEM * item= (RKGUI_SELECT_ITEM *)arg;

            item->cmd= gpstMediaLibDataBlock->pstitem[offset].cmd;
            item->text_id= gpstMediaLibDataBlock->pstitem[offset].text_id;
            item->sel_icon= gpstMediaLibDataBlock->pstitem[offset].sel_icon;
            item->unsel_icon= gpstMediaLibDataBlock->pstitem[offset].unsel_icon;
            break;

        case SELECT_ENVEN_ENTER:
#ifdef _USE_GUI_
            AskQueue.event = 0;
            AskQueue.event_type = MEDIA_LIBRARY_ENTER;
            AskQueue.offset = offset;

            rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
            if (ret == RK_ERROR)
            {
                printf("MediaLib send enter envet\n");
                rkos_queue_receive(gpstMediaLibDataBlock->AskQueue, &AskQueue_tmp, 0);
                rkos_queue_send(gpstMediaLibDataBlock->AskQueue, &AskQueue, 0);
            }
#endif
            break;

        default:
            break;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaLibrary_SetPopupWindow
** Input:
** Return: rk_err_t
** Owner:ctf
** Date: 2015.12.22
** Time: 10:15:31
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON FUN rk_err_t MediaLibrary_SetPopupWindow(void * text, uint16 text_cmd)
{
    RKGUI_MSGBOX_ARG pstMsgBoxArg;

    pstMsgBoxArg.cmd = MSGBOX_CMD_WARNING;
    pstMsgBoxArg.x = 4;
    pstMsgBoxArg.y = 30;
    pstMsgBoxArg.display = 1;
    pstMsgBoxArg.level = 0;
    pstMsgBoxArg.title = SID_WARNING;
    pstMsgBoxArg.text_cmd = text_cmd;
    pstMsgBoxArg.text = text;
    pstMsgBoxArg.align= TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;

    if(gpstMediaLibDataBlock->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstMediaLibDataBlock->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstMediaLibDataBlock->hMsgBox);
        gpstMediaLibDataBlock->hMsgBox = NULL;
    }

    if(gpstMediaLibDataBlock->hMsgBox == NULL)
    {
        gpstMediaLibDataBlock->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaLibrary_KeyEventHD
** Input:uint32 event
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON FUN rk_err_t MediaLibrary_KeyEventHD(uint32 event)
{
    switch(event)
    {
        case KEY_VAL_ESC_SHORT_UP:
            //MediaLibrary_GuiDeInit();
            MainTask_TaskSwtich(TASK_ID_MEDIA_LIBRARY, 0, TASK_ID_MAIN, 0, NULL);
            while(1)
            {
                rkos_sleep(2000);
            }
            break;

        case KEY_VAL_PLAY_PRESS:
            MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
            break;

        default :
            break;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaLibrary_SelectEnter
** Input:uint32 offset
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON FUN rk_err_t MediaLibrary_SelectEnter(uint32 offset)
{
    uint16 MediaTypeSelID;

    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) == NULL)
        MediaTypeSelID = offset + 1;

    switch(MediaTypeSelID)
    {
        case SORT_TYPE_SEL_NOW_PLAY:
        {
            if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
            {
                RK_TASK_PLAYMENU_ARG stTaskPlayer;
                stTaskPlayer.ucSelPlayType = SOURCE_FROM_CUR_AUDIO;
                MainTask_TaskSwtich(TASK_ID_MEDIA_LIBRARY, 0, TASK_ID_MUSIC_PLAY_MENU, 0, &stTaskPlayer);
            }
            break;
        }

        case SORT_TYPE_SEL_ID3TITLE:
        case SORT_TYPE_SEL_ID3SINGER :
        case SORT_TYPE_SEL_ID3ALBUM :
        case SORT_TYPE_SEL_GENRE :
        {
            if(RKTaskFind(TASK_ID_MEDIA_UPDATE, 0) == NULL)
            {
                if (gSysConfig.MedialibPara.gMusicFileNum != 0)
                {
                    RK_TASK_MEDIABRO_ARG MediaBro;
                    MediaBro.MediaTypeSelID = MediaTypeSelID;
                    MediaBro.Flag = 0;
                    MainTask_TaskSwtich(TASK_ID_MEDIA_LIBRARY, 0, TASK_ID_MEDIA_BROWSER, 0, &MediaBro);
                }
                else
                {
                    MediaLibrary_SetPopupWindow((void *)SID_NO_MUSIC_FILE, TEXT_CMD_ID);
                }
            }
            else
            {
                printf("\n updating media library, please wait...\n");
            }
            break;
        }

        case SORT_TYPE_SEL_FOLDER:
        {
            if (gSysConfig.MedialibPara.gMusicFileNum != 0)
            {
#ifdef __APP_BROWSER_BROWSERUITASK_C__
                RK_TASK_BROWSER_ARG Arg;
                Arg.SelSouceType = SOURCE_FROM_DB_FLODER;
                Arg.Flag = 0;
                MainTask_TaskSwtich(TASK_ID_MEDIA_LIBRARY, 0, TASK_ID_BROWSERUI, 0, &Arg);
#endif
            }
            else
            {
                MediaLibrary_SetPopupWindow((void *)SID_NO_MUSIC_FILE, TEXT_CMD_ID);
            }
            break;
        }

        case MUSIC_TYPE_SEL_MYFAVORITE :
        {
            if (gSysConfig.MedialibPara.gMyFavoriteFileNum)
            {
                RK_TASK_MEDIABRO_ARG MediaBro;
                MediaBro.MediaTypeSelID = MediaTypeSelID;
                MediaBro.Flag = 0;
                MainTask_TaskSwtich(TASK_ID_MEDIA_LIBRARY, 0, TASK_ID_MEDIA_BROWSER, 0, &MediaBro);
            }
            else
            {
                MediaLibrary_SetPopupWindow((void *)SID_NO_MUSIC_FILE, TEXT_CMD_ID);
            }

            break;
        }

        default :
            break;
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: MediaLibrary_DeleteMsgbox
** Input:
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON FUN rk_err_t MediaLibrary_DeleteMsgbox(void)
{
    if(gpstMediaLibDataBlock->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstMediaLibDataBlock->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstMediaLibDataBlock->hMsgBox);
        gpstMediaLibDataBlock->hMsgBox = NULL;
    }
}

/*******************************************************************************
** Name: MediaLibrary_GuiInit
** Input:void
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_INIT_
INIT FUN rk_err_t MediaLibrary_GuiInit(void)
{
    RKGUI_SELECT_ARG stSelectArg;

    gpstMediaLibDataBlock->item[0].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[0].text_id= SID_PLAYING;
    gpstMediaLibDataBlock->item[0].sel_icon= -1;
    gpstMediaLibDataBlock->item[0].unsel_icon= IMG_ID_NOWPLAY;

    gpstMediaLibDataBlock->item[1].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[1].text_id= SID_ALL_MUISC_FILE;
    gpstMediaLibDataBlock->item[1].sel_icon= -1;
    gpstMediaLibDataBlock->item[1].unsel_icon= IMG_ID_ALLMUSIC;

    gpstMediaLibDataBlock->item[2].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[2].text_id= SID_ARTIST;
    gpstMediaLibDataBlock->item[2].sel_icon= -1;
    gpstMediaLibDataBlock->item[2].unsel_icon= IMG_ID_MEDIA_RECORDFILE;

    gpstMediaLibDataBlock->item[3].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[3].text_id= SID_ALBUM;
    gpstMediaLibDataBlock->item[3].sel_icon= -1;
    gpstMediaLibDataBlock->item[3].unsel_icon= IMG_ID_ABLUM;

    gpstMediaLibDataBlock->item[4].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[4].text_id= SID_GENRE;
    gpstMediaLibDataBlock->item[4].sel_icon= -1;
    gpstMediaLibDataBlock->item[4].unsel_icon= IMG_ID_GENRE;

    gpstMediaLibDataBlock->item[5].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[5].text_id= SID_DIR_LIST;
    gpstMediaLibDataBlock->item[5].sel_icon= -1;
    gpstMediaLibDataBlock->item[5].unsel_icon= IMG_ID_LIST;

    gpstMediaLibDataBlock->item[6].cmd= TEXT_CMD_ID;
    gpstMediaLibDataBlock->item[6].text_id= SID_MY_FAVORITE;
    gpstMediaLibDataBlock->item[6].sel_icon= -1;
    gpstMediaLibDataBlock->item[6].unsel_icon= IMG_ID_LIST;

    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
    {
        gpstMediaLibDataBlock->pstitem = &gpstMediaLibDataBlock->item[0];
        stSelectArg.itemNum= 7;
    }
    else
    {
        gpstMediaLibDataBlock->pstitem = &gpstMediaLibDataBlock->item[1];
        stSelectArg.itemNum= 6;
    }

    stSelectArg.x= 0;
    stSelectArg.y= 20;
    stSelectArg.xSize= 128;
    stSelectArg.ySize= 137;
    stSelectArg.display = 1;
    stSelectArg.level= 0;

    stSelectArg.Background= IMG_ID_BROWSER_BACKGROUND;
    stSelectArg.SeekBar= IMG_ID_BROWSER_SCOLL2;
    stSelectArg.Cursor= gpstMediaLibDataBlock->CurId;
    stSelectArg.ItemStartOffset= 0;
    stSelectArg.MaxDisplayItem= MAX_DISP_ITEM_NUM;
    stSelectArg.IconBoxSize= 18;

    stSelectArg.pReviceMsg= MediaLibrary_SelectCallBack;
    stSelectArg.CursorStyle= IMG_ID_SEL_ICON;

    gpstMediaLibDataBlock->hSelect = GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
    if(gpstMediaLibDataBlock->hSelect == NULL)
    {
        rk_printf("error:pGc == NULL\n");
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaLibrary_GuiDeInit
** Input:void
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_INIT_
INIT FUN rk_err_t MediaLibrary_GuiDeInit(void)
{
    GuiTask_ScreenLock();
    if( gpstMediaLibDataBlock->hSelect!= NULL )
    {
        GuiTask_DeleteWidget(gpstMediaLibDataBlock->hSelect);
    }
    GuiTask_ScreenUnLock();
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaLibrary_Enter
** Input:void
** Return: void
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_COMMON_
COMMON API void MediaLibrary_Enter(void)
{
    MEDIA_LIBRARY_ASK_QUEUE MediaAskQueue;

    //MediaLibrary_GuiInit();
    while(1)
    {
        rkos_queue_receive(gpstMediaLibDataBlock->AskQueue, &MediaAskQueue, MAX_DELAY);
        switch(MediaAskQueue.event_type)
        {
            case APP_RECIVE_MSG_EVENT_KEY:
                MediaLibrary_KeyEventHD(MediaAskQueue.event);
                gpstMediaLibDataBlock->QueueFull = 0;
                break;

            case MEDIA_LIBRARY_ENTER:
                MediaLibrary_SelectEnter(MediaAskQueue.offset);
                break;

            case APP_RECIVE_MSG_EVENT_WARING:
                #ifdef _USE_GUI_
                MediaLibrary_DeleteMsgbox();
                gpstMediaLibDataBlock->QueueFull = 0;
                #endif
                break;
            default :
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
** Name: MediaLibrary_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_INIT_
INIT API rk_err_t MediaLibrary_Init(void *pvParameters, void *arg)
{
    RK_TASK_MEDIALIB_ARG * pArg = (RK_TASK_MEDIALIB_ARG *)arg;

    gpstMediaLibDataBlock = (MEDIA_LIBRARY_TASK_DATA_BLOCK *)rkos_memory_malloc(sizeof(MEDIA_LIBRARY_TASK_DATA_BLOCK));
    if(gpstMediaLibDataBlock == NULL)
    {
        return RK_ERROR;
    }

    gpstMediaLibDataBlock->AskQueue = rkos_queue_create(1, sizeof(MEDIA_LIBRARY_ASK_QUEUE));
    if( gpstMediaLibDataBlock->AskQueue == NULL )
    {
        rkos_memory_free(gpstMediaLibDataBlock);
        return RK_ERROR;
    }

    gpstMediaLibDataBlock->hSelect = NULL;
    gpstMediaLibDataBlock->hMsgBox = NULL;
    gpstMediaLibDataBlock->QueueFull = 0;
    gpstMediaLibDataBlock->CurId = pArg->CurId;

#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(MediaLibraryTask_ButtonCallBack);
    MediaLibrary_GuiInit();
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaLibrary_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_LIBRARY_INIT_
INIT API rk_err_t MediaLibrary_DeInit(void *pvParameters)
{
#ifdef _USE_GUI_
    if(gpstMediaLibDataBlock->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstMediaLibDataBlock->hMsgBox, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstMediaLibDataBlock->hMsgBox);
        gpstMediaLibDataBlock->hMsgBox = NULL;
    }

    MediaLibrary_GuiDeInit();
    GuiTask_AppUnReciveMsg(MediaLibraryTask_ButtonCallBack);
#endif

    rkos_queue_delete(gpstMediaLibDataBlock->AskQueue);
    rkos_memory_free(gpstMediaLibDataBlock);
    return RK_SUCCESS;
}
#endif
