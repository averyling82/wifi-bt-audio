/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\MediaLibrary\media_browser.c
* Owner: ctf
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: media library browser
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*      ctf      2016.1.26    17:29:00      1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_MEDIA_BROWSER_C__

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
#include "AddrSaveMacro.h"
#include "AudioControlTask.h"
#include "media_library.h"
#include "media_browser.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"
#include "MsgDevice.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    MEDIA_BROWSER_UPDATA = 0x11,
    MEDIA_BROWSER_ENTER,
    MEDIA_BROWSER_MOVE,
} MEDIA_BROWSER_TYPE;

typedef struct _MEDIA_BROWSER_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;
    uint32 offset;
    uint16 cursor;
}MEDIA_BROWSER_ASK_QUEUE;

typedef  struct _MEDIA_BROWSER_TASK_DATA_BLOCK
{
    pQueue                  AskQueue;
    HGC                     hSelect;
    HGC                     hMsgBox;
    uint16                  MediaTypeSelID;
    uint8                   QueueFull;
    UINT16                  CurDirPath[MAX_FILENAME_LEN];    //direction path, exclusive LongFileName
    UINT16                  Pathlen;
    UINT16                  LongFileName[MAX_FILENAME_LEN];
    MUSIC_DIR_TREE_STRUCT   MusicDirTreeInfo;
    SORT_INFO_ADDR_STRUCT   SortInfoAddr;
}MEDIA_BROWSER_TASK_DATA_BLOCK;

static MEDIA_BROWSER_TASK_DATA_BLOCK * gpstMediaBroDataBlock;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_APP_MEDIA_BROWSER_COMMON_
static char MediaFileExtString[] = "MP1MP2MP3WMAWAVAPELACAACM4AOGGMP43GPSBCFLA";

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
rk_err_t MediaBrowser_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc);
rk_err_t MediaBrowser_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset);
rk_err_t MediaBrowser_ChangeSelectContent(int totalItems, int cursor, int CurItemId);
rk_err_t MediaBrowser_SetPopupWindow(void * text, uint16 text_cmd);
rk_err_t MediaBrowser_KeyEventHD(uint32 event);
rk_err_t MediaBrowser_SelectUpdata(uint32 offset);
rk_err_t MediaBrowser_SelectEnter(uint32 offset, uint16 cursor);
rk_err_t MediaBrowser_SaveAndPostMusicPlayInfo(int cmd, uint16 All_Album);
void MediaBrowser_DeleteMsgbox(void);
void MediaBrowser_GetPathAndLongFileName(uint32 uiListNO);
uint16 MediaBrowser_GetSelPlayType(void);
void MediaBrowser_FomatTheOtherItem(UINT16 *ucFileName);
bool MediaBrowser_FileExtNameRemove(uint16 *LongFileName, uint8 *Filter);
UINT16 MediaBrowser_GetListItem(UINT16 *pListName, UINT16 uiListNO);
UINT16 MediaBrowser_GetBaseId(void);
UINT16 MediaBrowser_GetCurItemNum(void);
void MediaBrowser_SortInfoAddrInit(uint32 MediaTypeSelID);
void MediaBrowser_MusicDirInit(void);
void MediaBrowser_MusicBroVariableInit(RK_TASK_MEDIABRO_ARG * pArg);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MediaBrowserTask_ButtonCallBack
** Input:APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 18:00:52
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
#ifdef _USE_GUI_
    rk_err_t ret = 0;
    MEDIA_BROWSER_ASK_QUEUE AskQueue;
    MEDIA_BROWSER_ASK_QUEUE AskQueue_tmp;

    if ((gpstMediaBroDataBlock->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP) && ((event & KEY_STATUS_SHORT_UP) != KEY_STATUS_SHORT_UP))
    {
        rk_printf("lose key MediaLibQueueFull=%d\n", gpstMediaBroDataBlock->QueueFull);
        return RK_SUCCESS;
    }

    switch(event_type)
    {
        case APP_RECIVE_MSG_EVENT_KEY:
        {
            AskQueue.event_type = event_type;
            AskQueue.event  = event;
            AskQueue.offset = 0;
            gpstMediaBroDataBlock->QueueFull = 1;

            rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
            if (ret == RK_ERROR)
            {
                if((event==KEY_VAL_FFW_PRESS)||(event==KEY_VAL_FFD_PRESS)||(event==KEY_VAL_HOLD_PRESS))
                {
                    gpstMediaBroDataBlock->QueueFull = 0;
                    return RK_SUCCESS;
                }

                rkos_queue_receive(gpstMediaBroDataBlock->AskQueue, &AskQueue_tmp, 0);
                if (AskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                {
                    rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue_tmp, 0);
                    rkos_queue_receive(gpstMediaBroDataBlock->AskQueue, &AskQueue_tmp, 0);
                    if (AskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                    {
                        rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue_tmp, 0);
                        gpstMediaBroDataBlock->QueueFull = 0;
                    }
                    else
                    {
                        ret = rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
                        if (ret == RK_ERROR)
                        {
                            rk_printf("MediaLib Send Key Failure 0\n");
                            gpstMediaBroDataBlock->QueueFull = 0;
                            return RK_ERROR;
                        }
                    }
                }
                else
                {
                    ret = rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
                    if (ret == RK_ERROR)
                    {
                        rk_printf("MediaLib Send Key Failure 1\n");
                        gpstMediaBroDataBlock->QueueFull = 0;
                        return RK_ERROR;
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
            gpstMediaBroDataBlock->QueueFull = 1;

            ret = rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
            if(ret == RK_ERROR)
            {
                rkos_queue_receive(gpstMediaBroDataBlock->AskQueue, &AskQueue_tmp, 0);//MAX_DELAY
                rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
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
** Name: MediaBrowser_SelectCallBack
** Input:HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset
** Return:  rk_err_t
** Owner:ctf
** Date: 2015.12.29
** Time: 17:59:39
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN  rk_err_t MediaBrowser_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset)
{
    rk_err_t ret;
    MEDIA_BROWSER_ASK_QUEUE AskQueue;
    MEDIA_BROWSER_ASK_QUEUE AskQueue_tmp;

#ifdef _USE_GUI_
    switch(event_type)
    {
        case SELECT_ENVEN_UPDATA:
        {
            RKGUI_SELECT_ITEM * item= (RKGUI_SELECT_ITEM *)arg;
            UINT16 LongFileName[MAX_FILENAME_LEN];

            if(offset > gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem)
            {
                printf("offset flow over!!\n");
                return RK_ERROR;
            }

            item->cmd = TEXT_CMD_BUF;
            item->sel_icon = -1;
            item->unsel_icon = IMG_ID_MUSIC_ICON;

            memset(LongFileName, 0, MAX_FILENAME_LEN*2);
            if ((gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET)
                    &&  (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep != 0))
            {
                if(offset == 0)
                {
                    memcpy(LongFileName, L"All Album\0", 20);
                }
                else
                {
                    MediaBrowser_GetListItem(LongFileName, offset-1);
                    MediaBrowser_FomatTheOtherItem(LongFileName);
                }
            }
            else
            {
                MediaBrowser_GetListItem(LongFileName, offset);
                MediaBrowser_FomatTheOtherItem(LongFileName);
            }

            memcpy((uint8 *)item->text, LongFileName, StrLenW(LongFileName)*2);
            *(item->text + StrLenW(LongFileName)*2) = 0x0000;

            break;
        }

        case SELECT_ENVEN_ENTER:
        {
            AskQueue.event = 0;
            AskQueue.event_type = MEDIA_BROWSER_ENTER;
            AskQueue.offset = offset;
            AskQueue.cursor = (uint32)arg;

            rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
            if (ret == RK_ERROR)
            {
                rkos_queue_receive(gpstMediaBroDataBlock->AskQueue, &AskQueue_tmp, 0);
                rkos_queue_send(gpstMediaBroDataBlock->AskQueue, &AskQueue, 0);
            }
            break;
        }

#if 0
        case SELECT_ENVEN_MOVE:
        {
            printf("\n SELECT_ENVEN_MOVE \n");
            gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = offset;
            break;
        }
#endif

        default:
            break;
    }
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_ChangeSelectContent
** Input:
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_ChangeSelectContent(int totalItems, int cursor, int CurItemId)
{
    RKGUI_SELECT_ARG stSelectArg;

    if(gpstMediaBroDataBlock->hSelect != NULL)
    {
        if(GuiTask_DeleteWidget(gpstMediaBroDataBlock->hSelect) == RK_SUCCESS)
        {
            gpstMediaBroDataBlock->hSelect = NULL;
        }
        else
        {
            rk_printf("MediaBrowser hSelect delete failed...\n");
            return RK_ERROR;
        }
    }

    if(gpstMediaBroDataBlock->hSelect == NULL)
    {
        stSelectArg.x = 0;
        stSelectArg.y = 20;
        stSelectArg.xSize = 128;
        stSelectArg.ySize = 137;
        stSelectArg.display = 1;
        stSelectArg.level = 0;

        stSelectArg.Background = IMG_ID_BROWSER_BACKGROUND;
        stSelectArg.SeekBar = IMG_ID_BROWSER_SCOLL2;
        stSelectArg.itemNum = totalItems;
        stSelectArg.Cursor = cursor;

        if(CurItemId > cursor)
            stSelectArg.ItemStartOffset = CurItemId - cursor;
        else
            stSelectArg.ItemStartOffset = 0;

        stSelectArg.MaxDisplayItem = MAX_DISP_ITEM_NUM;
        stSelectArg.IconBoxSize = 18;

        stSelectArg.pReviceMsg = MediaBrowser_SelectCallBack;
        stSelectArg.CursorStyle = IMG_ID_SEL_ICON;

        gpstMediaBroDataBlock->hSelect = GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
        if(gpstMediaBroDataBlock->hSelect == NULL)
        {
            rk_printf("error, gpstMediaBroDataBlock->hSelect == NULL\n");
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_SetPopupWindow
** Input:
** Return: rk_err_t
** Owner:ctf
** Date: 2015.12.22
** Time: 10:15:31
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_SetPopupWindow(void * text, uint16 text_cmd)
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

    if(gpstMediaBroDataBlock->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstMediaBroDataBlock->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstMediaBroDataBlock->hMsgBox);
        gpstMediaBroDataBlock->hMsgBox = NULL;
    }

    if(gpstMediaBroDataBlock->hMsgBox == NULL)
    {
        gpstMediaBroDataBlock->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_KeyEventHD
** Input:uint32 event
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_KeyEventHD(uint32 event)
{
    switch(event)
    {
        case KEY_VAL_ESC_SHORT_UP:
            if (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep == 0)
            {
                RK_TASK_MEDIALIB_ARG MediaLibArg;
                if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) == NULL)
                {
                    MediaLibArg.CurId = gpstMediaBroDataBlock->MediaTypeSelID - 1;
                }
                else
                {
                    MediaLibArg.CurId = gpstMediaBroDataBlock->MediaTypeSelID;
                }

                MainTask_TaskSwtich(TASK_ID_MEDIA_BROWSER, 0, TASK_ID_MEDIA_LIBRARY,0, &MediaLibArg);
            }
            else
            {
                gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep--;
                MediaBrowser_MusicDirInit();
                MediaBrowser_ChangeSelectContent(gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem,
                    gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep],
                    gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep]);

            }

            break;

        case KEY_VAL_PLAY_SHORT_UP:    //media browser for Favorite(短按)
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
** Name: MediaBrowser_SelectUpdata
** Input:uint32 offset
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_SelectUpdata(uint32 offset)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_SelectEnter
** Input:uint32 offset
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_SelectEnter(uint32 offset, uint16 cursor)
{
    gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = offset;
    gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = cursor;

    if (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep == 0)
    {
        switch (gpstMediaBroDataBlock->MediaTypeSelID)
        {
            case SORT_TYPE_SEL_ID3TITLE:
            case MUSIC_TYPE_SEL_MYFAVORITE:
                MediaBrowser_GetPathAndLongFileName(offset + gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep]);
                MediaBrowser_SaveAndPostMusicPlayInfo(MEDIA_PLAY_WITH_FILE_NAME, 0);
                break;

            case SORT_TYPE_SEL_ID3SINGER:
            case SORT_TYPE_SEL_ID3ALBUM:
            case SORT_TYPE_SEL_GENRE:
                gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep++;
                gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                MediaBrowser_MusicDirInit();
                MediaBrowser_ChangeSelectContent(gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem, 0, 0);
                break;

            default:
                break;
        }
    }
    else if (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep == 1)
    {
        switch (gpstMediaBroDataBlock->MediaTypeSelID)
        {
            case SORT_TYPE_SEL_ID3SINGER:
                if (gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] == 0)
                {
                    // Aritist->All Album
                    MediaBrowser_SaveAndPostMusicPlayInfo(MEDIA_PLAY_WITH_FILE_NAME, 1);
                }
                else
                {
                    gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep++;
                    gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                    gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                    MediaBrowser_MusicDirInit();
                    MediaBrowser_ChangeSelectContent(gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem, 0, 0);
                }
                break;

            case SORT_TYPE_SEL_GENRE:
                gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep++;
                gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                MediaBrowser_MusicDirInit();
                MediaBrowser_ChangeSelectContent(gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem, 0, 0);
                break;

            case SORT_TYPE_SEL_ID3ALBUM:
                MediaBrowser_GetPathAndLongFileName(offset + gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep]);
                MediaBrowser_SaveAndPostMusicPlayInfo(MEDIA_PLAY_WITH_FILE_NAME, 0);
                break;

            default:
                break;
        }
    }
    else if (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep == 2)
    {
        switch (gpstMediaBroDataBlock->MediaTypeSelID)
        {
            case SORT_TYPE_SEL_GENRE:
                if (gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] == 0)
                {
                    //Genre->All Album
                    MediaBrowser_SaveAndPostMusicPlayInfo(MEDIA_PLAY_WITH_FILE_NAME, 1);
                }
                else
                {
                    gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep++;
                    gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                    gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
                    MediaBrowser_MusicDirInit();
                    MediaBrowser_ChangeSelectContent(gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem, 0, 0);
                }
                break;

            case SORT_TYPE_SEL_ID3SINGER:
                MediaBrowser_GetPathAndLongFileName(offset + gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep]);
                MediaBrowser_SaveAndPostMusicPlayInfo(MEDIA_PLAY_WITH_FILE_NAME, 0);
                break;

            default:
                break;
        }
    }
    else if (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep == 3)
    {
        switch (gpstMediaBroDataBlock->MediaTypeSelID)
        {
            case SORT_TYPE_SEL_GENRE:
                MediaBrowser_GetPathAndLongFileName(offset + gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep]);
                MediaBrowser_SaveAndPostMusicPlayInfo(MEDIA_PLAY_WITH_FILE_NAME, 0);
                break;

            default:
                break;
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_DeleteMsgbox
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN void MediaBrowser_DeleteMsgbox(void)
{
    if(gpstMediaBroDataBlock->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstMediaBroDataBlock->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstMediaBroDataBlock->hMsgBox);
        gpstMediaBroDataBlock->hMsgBox = NULL;
    }
}

/*******************************************************************************
** Name: MediaBrowser_GetPathAndLongFileName
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN void MediaBrowser_GetPathAndLongFileName(uint32 uiListNO)
{
    UINT16 i;
    UINT16 temp;
    UINT8  ucBufTemp[8];
    UINT32 AddrOffset;
    UINT8  FileInfoBuf[MAX_FILENAME_LEN *2];
    memset(FileInfoBuf, 0, MAX_FILENAME_LEN *2);

    FW_ReadDataBaseByByte((gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiListNO*2), ucBufTemp, 2);
    temp = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); //获得对应的文件保存号

    AddrOffset = (UINT32)(temp)*BYTE_NUM_SAVE_PER_FILE + FILE_NAME_SAVE_ADDR_OFFSET;
    FW_ReadDataBaseByByte((gpstMediaBroDataBlock->SortInfoAddr.ulFileFullInfoSectorAddr<<9)+AddrOffset, FileInfoBuf, MAX_FILENAME_LEN*2);
    for(i=0; i<MAX_FILENAME_LEN; i++)
    {
        gpstMediaBroDataBlock->LongFileName[i] = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }

    memset(FileInfoBuf, 0, MEDIA_ID3_SAVE_CHAR_NUM *2);
    AddrOffset = (UINT32)(temp)*BYTE_NUM_SAVE_PER_FILE + DIR_PATH_SAVE_ADDR_OFFSET;
    FW_ReadDataBaseByByte((gpstMediaBroDataBlock->SortInfoAddr.ulFileFullInfoSectorAddr<<9)+AddrOffset, FileInfoBuf, MAX_FILENAME_LEN*2);
    for(i=0; i<MAX_FILENAME_LEN; i++)
    {
        gpstMediaBroDataBlock->CurDirPath[i] = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }

    gpstMediaBroDataBlock->Pathlen = StrLenW(gpstMediaBroDataBlock->CurDirPath);
}

/*******************************************************************************
** Name: MediaBrowser_GetSelPlayType
** Input:void
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN uint16 MediaBrowser_GetSelPlayType(void)
{
    uint16 SelPlayType = 0xff;

    switch(gpstMediaBroDataBlock->MediaTypeSelID)
    {
        case SORT_TYPE_SEL_ID3TITLE:
        case SORT_TYPE_SEL_ID3SINGER:
        case SORT_TYPE_SEL_ID3ALBUM:
        case SORT_TYPE_SEL_GENRE:
            SelPlayType = SOURCE_FROM_MEDIA_LIBRARY;
            break;

        case MUSIC_TYPE_SEL_MYFAVORITE:
            SelPlayType = SOURCE_FROM_DB_MYFAVORITE;
            break;

        default:
            break;
    }

    return SelPlayType;
}

/*******************************************************************************
** Name: MediaBrowser_SaveAndPostMusicPlayInfo
** Input:
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN rk_err_t MediaBrowser_SaveAndPostMusicPlayInfo(int cmd, uint16 All_Album)
{
    uint16 i;
    uint16 TotalNum = 0;
    uint16 BaseID = 0;
    uint16 fileLen = 0;

    RK_TASK_PLAYMENU_ARG stTaskPlayer;
    UINT16 MusicDirDeep = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep;

    if(gpstMediaBroDataBlock->Pathlen > 251)
    {
        rk_printf("Path > 251\n");
        return RK_ERROR;
    }

    if(All_Album)
    {
        for (i = 0; i < gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem - 1; i++)
        {
            TotalNum += (GetSummaryInfo(gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirDeep],
                            gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[MusicDirDeep] + i,
                            FIND_SUM_ITEMNUM, gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[MusicDirDeep],
                            MusicDirDeep) - 1);
        }
        //printf("All Album, TotalNum = %d", TotalNum);

        BaseID = GetSummaryInfo(gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirDeep],
                    gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[MusicDirDeep],
                    FIND_SUM_SORTSTART, gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[MusicDirDeep],
                    MusicDirDeep); // first Song;
        //printf("All Album, BaseID = %d", BaseID);

        MediaBrowser_GetPathAndLongFileName(BaseID);

        gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem =  TotalNum;
        gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[MusicDirDeep] = BaseID;
    }

    gSysConfig.MediaDirTreeInfo.MusicDirDeep = MusicDirDeep;
    gSysConfig.MediaDirTreeInfo.MediaType = gpstMediaBroDataBlock->MediaTypeSelID;
    gSysConfig.MediaDirTreeInfo.ulFileFullInfoSectorAddr = gpstMediaBroDataBlock->SortInfoAddr.ulFileFullInfoSectorAddr;
    gSysConfig.MediaDirTreeInfo.ulFileSortInfoSectorAddr = gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr;
    for(i = 0; i < MAX_MUSIC_DIR_DEPTH; i++)
    {
        gSysConfig.MediaDirTreeInfo.MusicDirBaseSortId[i] = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[i];
        gSysConfig.MediaDirTreeInfo.CurItemId[i]= gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[i];
        gSysConfig.MediaDirTreeInfo.Cursor[i] = gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[i];
    }

    memcpy(stTaskPlayer.filepath, gpstMediaBroDataBlock->CurDirPath, (gpstMediaBroDataBlock->Pathlen)*2);
    if(cmd == MEDIA_PLAY_WITH_FILE_NAME)
    {
        fileLen = StrLenW(gpstMediaBroDataBlock->LongFileName);
        if(gpstMediaBroDataBlock->Pathlen + fileLen < MAX_DIRPATH_LEN)
        {
            memcpy(stTaskPlayer.filepath + gpstMediaBroDataBlock->Pathlen, gpstMediaBroDataBlock->LongFileName, fileLen*2);
        }
        else
        {
            printf("file filepath >= 259\n");
            return RK_ERROR;
        }
    }

    *(stTaskPlayer.filepath + gpstMediaBroDataBlock->Pathlen + fileLen) = 0x0000;
    stTaskPlayer.FileNum = gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[MusicDirDeep];
    stTaskPlayer.TotalFiles = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem;
    stTaskPlayer.ucSelPlayType = MediaBrowser_GetSelPlayType();

    printf("switch play_menu_task, FileNum= %d, TotalFiles = %d\n", stTaskPlayer.FileNum, stTaskPlayer.TotalFiles);

    MainTask_TaskSwtich(TASK_ID_MEDIA_BROWSER, 0, TASK_ID_MUSIC_PLAY_MENU, 0, &stTaskPlayer);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_FomatTheOtherItem
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN void MediaBrowser_FomatTheOtherItem(UINT16 *ucFileName)
{
    unsigned int j;

    if (ucFileName[0] == 0) // (ucFileName[0]==0x20)
    {
        for (j = 0; j < MAX_FILENAME_LEN; j++)
        {
            if (ucFileName[j] != 0)
            {
                break;
            }
        }

        if (j == MAX_FILENAME_LEN)
        {
            //ucFileName[0]=0x003c; // '<'
            //ucFileName[1]=0x004F; // 'O'
            //ucFileName[2]=0x0074; // 't'
            //ucFileName[3]=0x0068; // 'h'
            //ucFileName[4]=0x0065; // 'e'
            //ucFileName[5]=0x0072; // 'r'
            //ucFileName[6]=0x003e; // '>'
            ucFileName[0] = 'U'; // '<'
            ucFileName[1] = 'n'; // 'O'
            ucFileName[2] = 'K'; // 't'
            ucFileName[3] = 'n'; // 'h'
            ucFileName[4] = 'o'; // 'e'
            ucFileName[5] = 'w'; // 'r'
            ucFileName[6] = 'n'; // '>'
        }
    }
}

/*******************************************************************************
** Name: MediaBrowser_FileExtNameRemove
** Description  :filter the file extension.
** Input:
** Return: bool
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN bool MediaBrowser_FileExtNameRemove(uint16 *LongFileName, uint8 *Filter)
{
    uint16 i;
    uint16 j = MAX_FILENAME_LEN;
    uint16 TempBuf[4];

    for (i = 0; (i < MAX_FILENAME_LEN) || (LongFileName[i] == '\0'); i++)
    {
        if (LongFileName[i] == '.')
        {
            j = i;
        }
    }

    if ( j < (MAX_FILENAME_LEN - 4))
    {
        for (i = 0; i < 3; i++)
        {
            TempBuf[i] = LongFileName[j+i+1];

            if (TempBuf[i] >= 'a' && TempBuf[i] <= 'z')
            {
                TempBuf[i] = (TempBuf[i] - 'a' + 'A');
            }
        }

        while (*Filter != '\0')
        {
            if (TempBuf[0] == Filter[0])
            {
                if (TempBuf[1] == Filter[1])
                {
                    if (TempBuf[2] == Filter[2])
                    {
                        LongFileName[j] = '\0';
                    }
                }
            }
            Filter += 3;
        }
    }
    else if ((j >= (MAX_FILENAME_LEN - 4)) && (j < MAX_FILENAME_LEN))
    {
        LongFileName[j] = '\0';
    }

    return (TRUE);
}

/*******************************************************************************
** Name: MediaBrowser_GetListItem
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN UINT16 MediaBrowser_GetListItem(UINT16 *pListName, UINT16 uiListNO)
{
    unsigned int i;

    switch (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep)
    {
        case 0://最外层(流派, 歌手, 专辑, 标题)
        {
            switch (gpstMediaBroDataBlock->MediaTypeSelID)
            {
                case SORT_TYPE_SEL_ID3TITLE:
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 1);
                    break;

                case SORT_TYPE_SEL_ID3ALBUM:
                case SORT_TYPE_SEL_ID3SINGER:
                case SORT_TYPE_SEL_GENRE:
                    uiListNO += gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 0);
                    break;

                case MUSIC_TYPE_SEL_MYFAVORITE:
#ifdef FAVOSUB
                    //GetFavoInfo(&stFindData, uiListNO, pListName);
#endif
                    break;

                default:
                    for (i = 0; i < MAX_FILENAME_LEN; i++)
                    {
                        *pListName++ = 0;
                    }

                    break;
            }

            break;
        }

        case 1://次外层(流派-歌手, 歌手-专辑, 专辑-标题)
        {
            switch (gpstMediaBroDataBlock->MediaTypeSelID)
            {
                case SORT_TYPE_SEL_ID3ALBUM:    //获取最外层的专辑信息
                    uiListNO += gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 1);
                    break;

                case SORT_TYPE_SEL_ID3SINGER:   //获取最外层的歌手信息
                case SORT_TYPE_SEL_GENRE:       //获取最外层的流派信息
                    uiListNO += gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 0);
                    break;

                default:
                    break;
            }

            break;
        }

        case 2://内层(流派-歌手-专辑, 歌手-专辑-标题)
        {
            switch (gpstMediaBroDataBlock->MediaTypeSelID)
            {
                case SORT_TYPE_SEL_ID3SINGER:    //获取最外层的专辑信息
                    uiListNO += gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 1);
                    break;

                case SORT_TYPE_SEL_GENRE:       //获取最外层的流派信息
                    uiListNO += gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 0);
                    break;

                default:
                    break;
            }

            break;
        }

        case 3://最内层(流派-歌手-专辑-标题)
        {
            switch (gpstMediaBroDataBlock->MediaTypeSelID)
            {
                case SORT_TYPE_SEL_GENRE:       //获取最外层的流派信息
                    uiListNO += gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep];
                    GetMediaItemInfo(pListName, gpstMediaBroDataBlock->SortInfoAddr, uiListNO, MAX_FILENAME_LEN, gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep, 1);
                    break;

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }

    MediaBrowser_FileExtNameRemove(pListName, MediaFileExtString);
    return RETURN_OK;
}

/*******************************************************************************
** Name: MediaBrowser_GetBaseId
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN UINT16 MediaBrowser_GetBaseId(void)
{
    UINT16 uiBaseId;
    UINT16 MusicDirDeep = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep;
    UINT16 uiSortInfoAddrOffset = gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[MusicDirDeep];
    UINT32 ulSortSubInfoSectorAddr = gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirDeep - 1];
    UINT16 CurItemId = gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[MusicDirDeep - 1];
    UINT16 MusicDirBaseSortId = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[MusicDirDeep - 1];

    if (MusicDirDeep == 0)
    {
        uiBaseId = 0;
    }
    else
    {
        if ((uiSortInfoAddrOffset == ID3_TITLE_SAVE_ADDR_OFFSET)
            && ((gpstMediaBroDataBlock->MediaTypeSelID == SORT_TYPE_SEL_ID3SINGER)
            || (gpstMediaBroDataBlock->MediaTypeSelID == SORT_TYPE_SEL_GENRE)))
        {
            uiBaseId = GetSummaryInfo(ulSortSubInfoSectorAddr, CurItemId - 1 + MusicDirBaseSortId, FIND_SUM_SORTSTART, uiSortInfoAddrOffset, MusicDirDeep);
        }
        else
        {
            uiBaseId = GetSummaryInfo(ulSortSubInfoSectorAddr, CurItemId + MusicDirBaseSortId, FIND_SUM_SORTSTART, uiSortInfoAddrOffset, MusicDirDeep);
        }
    }

    return uiBaseId;
}

/*******************************************************************************
** Name: MediaBrowser_GetCurItemNum
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON FUN UINT16 MediaBrowser_GetCurItemNum(void)
{
    UINT16  uiTotalItemNum;

    UINT16 MusicDirDeep = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep;
    UINT16 uiSortInfoAddrOffset = gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[MusicDirDeep];
    UINT32 ulSortSubInfoSectorAddr = gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[MusicDirDeep - 1];
    UINT16 CurItemId = gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[MusicDirDeep - 1];
    UINT16 MusicDirBaseSortId = gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[MusicDirDeep - 1];

    if (MusicDirDeep == 0)
    {
        switch (gpstMediaBroDataBlock->MediaTypeSelID)
        {
            case SORT_TYPE_SEL_ID3TITLE:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3TitleFileNum;
                break;

            case SORT_TYPE_SEL_ID3SINGER:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3ArtistFileNum;
                break;

            case SORT_TYPE_SEL_ID3ALBUM:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3AlbumFileNum;
                break;

            case SORT_TYPE_SEL_GENRE:
                uiTotalItemNum = gSysConfig.MedialibPara.gID3GenreFileNum;
                break;

            case MUSIC_TYPE_SEL_MYFAVORITE:
                uiTotalItemNum = gSysConfig.MedialibPara.gMyFavoriteFileNum;
                break;

            default:
                uiTotalItemNum = 0;
                break;
        }
    }
    else    //获取专辑或艺术家下的文件数
    {
        if ((uiSortInfoAddrOffset == ID3_TITLE_SAVE_ADDR_OFFSET)
            && ((gpstMediaBroDataBlock->MediaTypeSelID == SORT_TYPE_SEL_ID3SINGER)
            || (gpstMediaBroDataBlock->MediaTypeSelID == SORT_TYPE_SEL_GENRE)))
        {
            uiTotalItemNum = GetSummaryInfo(ulSortSubInfoSectorAddr, CurItemId - 1 + MusicDirBaseSortId, FIND_SUM_ITEMNUM, uiSortInfoAddrOffset, MusicDirDeep);
        }
        else
        {
            uiTotalItemNum = GetSummaryInfo(ulSortSubInfoSectorAddr, CurItemId + MusicDirBaseSortId, FIND_SUM_ITEMNUM, uiSortInfoAddrOffset, MusicDirDeep);
        }
    }

    return uiTotalItemNum;
}
/*******************************************************************************
** Name: MediaBrowser_SortInfoAddrInit
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_INIT_
INIT FUN void MediaBrowser_SortInfoAddrInit(uint32 MediaTypeSelID)
{
    gpstMediaBroDataBlock->SortInfoAddr.ulFileFullInfoSectorAddr = gSysConfig.MediaLibrayStartLBA + MUSIC_SAVE_INFO_SECTOR_START;

    switch (MediaTypeSelID)
    {

        case SORT_TYPE_SEL_ID3TITLE:
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_TITLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr = gSysConfig.MediaLibrayStartLBA + ID3TITLE_SORT_INFO_SECTOR_START;
            break;

        case SORT_TYPE_SEL_ID3SINGER:
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_SINGLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[1] = ID3_ALBUM_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[2] = ID3_TITLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr = gSysConfig.MediaLibrayStartLBA + ID3ARTIST_SORT_INFO_SECTOR_START;
            gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[0] = gSysConfig.MediaLibrayStartLBA + ID3ARTIST_SORT_SUB_SECTOR_START;//歌手分类信息存放地址
            gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[1] = gSysConfig.MediaLibrayStartLBA + ID3ARTIST_ALBUM_SORT_SUB_SECTOR_START;//歌手-专辑分类信息存放地址
            break;

        case SORT_TYPE_SEL_ID3ALBUM:
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_ALBUM_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[1] = ID3_TITLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr = gSysConfig.MediaLibrayStartLBA + ID3ALBUM_SORT_INFO_SECTOR_START;
            gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[0] = gSysConfig.MediaLibrayStartLBA + ID3ALBUM_SORT_SUB_SECTOR_START; //专辑分类信息存放地址
            break;

        case SORT_TYPE_SEL_GENRE:
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_GENRE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[1] = ID3_SINGLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[2] = ID3_ALBUM_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[3] = ID3_TITLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr = gSysConfig.MediaLibrayStartLBA +  ID3GENRE_SORT_INFO_SECTOR_START; //排序后的文件顺序列表
            gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[0] = gSysConfig.MediaLibrayStartLBA + ID3GENRE_SORT_SUB_SECTOR_START; //流派分类信息存放地址
            gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[1] = gSysConfig.MediaLibrayStartLBA + ID3GENRE_ARTIST_SORT_SUB_SECTOR_START;//流派-歌手分类信息存放地址
            gpstMediaBroDataBlock->SortInfoAddr.ulSortSubInfoSectorAddr[2] = gSysConfig.MediaLibrayStartLBA + ID3GENRE_ALBUM_SORT_SUB_SECTOR_START;//流派-歌手-专辑分类信息存放地址
            break;

        case MUSIC_TYPE_SEL_MYFAVORITE:
            gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[0] = ID3_TITLE_SAVE_ADDR_OFFSET;
            gpstMediaBroDataBlock->SortInfoAddr.ulFileSortInfoSectorAddr = gSysConfig.MediaLibrayStartLBA +  FAVORITE_MUSIC_INFO_SECTOR_START;
            break;

        default:
            break;
    }
}

/*******************************************************************************
** Name: MediaBrowser_MusicBroVariableInit
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_INIT_
INIT FUN void MediaBrowser_MusicBroVariableInit(RK_TASK_MEDIABRO_ARG * pArg)
{
    uint32 Flag = pArg->Flag;
    int i;

    if(Flag)
    {
        gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep = pArg->MediaDirTreeInfo.MusicDirDeep;

        for (i = 0; i <= gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep; i++)
        {
            gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[i] = pArg->MediaDirTreeInfo.MusicDirBaseSortId[i];
            gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[i] = pArg->MediaDirTreeInfo.CurItemId[i];
            gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[i] = pArg->MediaDirTreeInfo.Cursor[i];
        }

        //All Album
        if ((gpstMediaBroDataBlock->SortInfoAddr.uiSortInfoAddrOffset[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] == ID3_ALBUM_SAVE_ADDR_OFFSET) &&  (gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep != 0) )
        {
            gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
            gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = 0;
        }
    }

}

/*******************************************************************************
** Name: MediaBrowser_MusicDirInit
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_INIT_
INIT FUN void MediaBrowser_MusicDirInit(void)
{
    gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem = MediaBrowser_GetCurItemNum();
    gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirBaseSortId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep] = MediaBrowser_GetBaseId();
}

/*******************************************************************************
** Name: MediaBrowser_Enter
** Input:void
** Return: void
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_COMMON_
COMMON API void MediaBrowser_Enter(void)
{
    MEDIA_BROWSER_ASK_QUEUE MediaAskQueue;
    while(1)
    {
        rkos_queue_receive(gpstMediaBroDataBlock->AskQueue, &MediaAskQueue, MAX_DELAY);
        switch(MediaAskQueue.event_type)
        {
            case APP_RECIVE_MSG_EVENT_KEY:
                MediaBrowser_KeyEventHD(MediaAskQueue.event);
                gpstMediaBroDataBlock->QueueFull = 0;
                break;

            case MEDIA_BROWSER_UPDATA:
                MediaBrowser_SelectUpdata(MediaAskQueue.offset);
                break;

            case MEDIA_BROWSER_ENTER:
                MediaBrowser_SelectEnter(MediaAskQueue.offset, MediaAskQueue.cursor);
                break;

            case APP_RECIVE_MSG_EVENT_WARING:
                #ifdef _USE_GUI_
                MediaBrowser_DeleteMsgbox();
                gpstMediaBroDataBlock->QueueFull = 0;
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
** Name: MediaBrowser_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_INIT_
INIT API rk_err_t MediaBrowser_Init(void *pvParameters, void *arg)
{
    RK_TASK_MEDIABRO_ARG * pArg = (RK_TASK_MEDIABRO_ARG *)arg;

    gpstMediaBroDataBlock = (MEDIA_BROWSER_TASK_DATA_BLOCK *)rkos_memory_malloc(sizeof(MEDIA_BROWSER_TASK_DATA_BLOCK));
    if(gpstMediaBroDataBlock == NULL)
    {
        return RK_ERROR;
    }
    memset(gpstMediaBroDataBlock, 0, sizeof(MEDIA_BROWSER_TASK_DATA_BLOCK));

    gpstMediaBroDataBlock->AskQueue = rkos_queue_create(1, sizeof(MEDIA_BROWSER_ASK_QUEUE));
    if( gpstMediaBroDataBlock->AskQueue == NULL )
    {
        rkos_memory_free(gpstMediaBroDataBlock);
        return RK_ERROR;
    }

    gpstMediaBroDataBlock->hSelect = NULL;
    gpstMediaBroDataBlock->hMsgBox = NULL;
    gpstMediaBroDataBlock->QueueFull = 0;
    gpstMediaBroDataBlock->MediaTypeSelID = pArg->MediaTypeSelID;

    MediaBrowser_SortInfoAddrInit(gpstMediaBroDataBlock->MediaTypeSelID);
    MediaBrowser_MusicBroVariableInit(pArg);
    MediaBrowser_MusicDirInit();

    MediaBrowser_ChangeSelectContent(gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirTotalItem,
                                        gpstMediaBroDataBlock->MusicDirTreeInfo.Cursor[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep],
                                        gpstMediaBroDataBlock->MusicDirTreeInfo.CurItemId[gpstMediaBroDataBlock->MusicDirTreeInfo.MusicDirDeep]);

#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(MediaBrowser_ButtonCallBack);
#endif
    MainTask_SetStatus(MAINTASK_APP_MEDIA_BROWSER, 1);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MediaBrowser_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_MEDIA_BROWSER_INIT_
INIT API rk_err_t MediaBrowser_DeInit(void *pvParameters)
{
#ifdef _USE_GUI_
    if(gpstMediaBroDataBlock->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstMediaBroDataBlock->hMsgBox, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstMediaBroDataBlock->hMsgBox);
        gpstMediaBroDataBlock->hMsgBox = NULL;
    }

    GuiTask_ScreenLock();
    if( gpstMediaBroDataBlock->hSelect!= NULL )
    {
        GuiTask_DeleteWidget(gpstMediaBroDataBlock->hSelect);
    }
    GuiTask_ScreenUnLock();

    GuiTask_AppUnReciveMsg(MediaBrowser_ButtonCallBack);
#endif

    rkos_queue_delete(gpstMediaBroDataBlock->AskQueue);
    rkos_memory_free(gpstMediaBroDataBlock);
    MainTask_SetStatus(MAINTASK_APP_MEDIA_BROWSER, 0);
    return RK_SUCCESS;
}
#endif

