/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Browser\BrowserUITask.c
* Owner: cjh
* Date: 2015.12.28
* Time: 15:14:49
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.12.28     15:14:49   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __APP_BROWSER_BROWSERUITASK_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "FileInfo.h"
#include "AddrSaveMacro.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef struct _FOLDER_INFO_STRUCT
{
    //struct      _FOLDER_INFO_STRUCT   *pPrev;        /* pPrev menu item */
    //struct      _FOLDER_INFO_STRUCT   *pNext;        /* pNext menu item */
    UINT16      CurItemCursor;                         /* the cursor in current folder*/
    UINT16      CurItemOffset;                         /* the serial number in current folder */
    UINT16      TotalItems;                            /* total number of child items.*/
    int16       TotalFiles;                            /* total number of file number,*/
    int16       TotalDirs;                             /* total number of direction number,*/
    uint16      CurDirHandlePos;
    uint32      CurFileHandlePos;

    UINT32      DirClus;                               /*for media library floder*/
} FOLDER_INFO_STRUCT;

typedef struct _BROWER_FILE_INFO
{
    FileType FileType;                                          //file type
    UINT8   *FileExtName;                                       //file extension
    UINT16  LongFileName[MAX_FILENAME_LEN];

}BROWER_FILE_INFO;
/*
typedef struct _FileListInfo
{
    UINT32 DirClus;
    UINT32 Index;

} FileListInfo;

typedef struct _FolderInfoStruct
{

    UINT16  TotalItems;                                        //total item number.
    UINT16  TotalFiles;                                        //total file number,
    UINT16  TotalDirs;                                      //total direction number,
    UINT16  Cursor;

} FolderInfoStruct;

typedef struct _BROWER_FILE_STRUCT
{

    struct _BROWER_FILE_STRUCT   *pPrev;
    struct _BROWER_FILE_STRUCT   *pNext;
    FileType                    FileType;
    //FDT                         FileFDT;                         //pointer
    UINT16                      LongFileName[MAX_FILENAME_LEN];

} BROWER_FILE_STRUCT;
*/
typedef  struct _BROWSERUITASK_RESP_QUEUE
{
    uint32 cmd;
    uint32 status;
}BROWSERUITASK_RESP_QUEUE;

typedef  struct _BROWSERUITASK_ASK_QUEUE
{
    uint32 event;
    uint32 event_type;
    uint32 offset;
    uint16 cursor;
}BROWSERUITASK_ASK_QUEUE;

typedef  struct _BROWSERUI_TASK_DATA_BLOCK
{
    pQueue                      BrowserUITaskAskQueue;
    pQueue                      BrowserUITaskRespQueue;
    uint16                      CurDiskSelect;                  //current disk select
    uint16                      PreDiskSelect;                  //it used for disk switch
    uint16                      TotleDisk;                      //all disk
    HGC                         hMsgBox;
    HGC                         hSelect;
    HDC                         hDir[MAX_DIR_DEPTH];
    HDC                         hDirForFlie[MAX_DIR_DEPTH];

    UINT16                      CurDeep;                        //direction deepth

    UINT16                      CurDirPath[MAX_DIRPATH_LEN];    //direction path
    UINT16                      Pathlen;
    FOLDER_INFO_STRUCT          FolderInfo[MAX_DIR_DEPTH];      //Dir info
    BROWER_FILE_INFO            CurFileInfo;                    //file info
    SUB_DIR_INFO                SubDirInfo;                     //Floder -> All files, Dir info
    UINT8                       *ExtName;

    UINT16                      BrowserTitleId ;                //display titile address
    int                         deleteTaskFlag;
    uint16                      SelSouceType;
    uint8                       QueueFull;
}BROWSERUI_TASK_DATA_BLOCK;

#define NEXTDIR                 1
#define LASTDIR                -1
#define CURDIR                  0

#define PLAY_WITH_FILE_NAME     5
#define PLAY_WITH_FILE_NUM      6
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static BROWSERUI_TASK_DATA_BLOCK * gpstBrowserUITaskData;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_APP_BROWSER_BROWSERUITASK_COMMON_
char  BrowserUIFileExtString[]   = "MP1MP2MP3WAVAPELACAACM4AOGGMP43GPSBCWMAAMR";

_APP_BROWSER_BROWSERUITASK_COMMON_
static char MusicFileExtString[] = "MP1MP2MP3WAVAPEFLAAACM4AOGGMP43GPSBCWMAAMR";

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint16 BrowserUITask_MediaGetTotalFiles(UINT32 DirClus);
uint16 BrowserUITask_MediaGetTotalSubDir(UINT32 DirClus);
void BrowserUITask_MediaGetLongFileName(UINT32 DirClus, int32 CurFileNum, uint16* lfName);
void BrowserUITask_MediaGetPath(UINT32 DirClus, int32 CurFileNum, uint16* path, UINT16* pathlen);
uint32 BrowserUITask_MediaGetDirClus(UINT32 DirClus, uint32 index);
void BrowserUITask_MediaVariableInit(UINT16 flag, MEDIA_FLODER_INFO_STRUCT MediaFloderInfo);
rk_err_t BrowserUITask_MediaFindFirstFile(UINT32 CurDirClus, UINT16 CurDirDeep);
void BrowserUITask_MediaGotoNextDir(void);
rk_err_t BrowserUITask_GetInfoFromPath(UINT16 *path, UINT16 deep);
rk_err_t BrowserUITask_GetDirOffset(UINT16 *dirName, UINT16 *path, UINT16 *offset);
rk_err_t BrowserUITask_ChangeSelectContent(int totalItems, int cursor, int cursorOffset);
rk_err_t BrowserUITask_GetFileOffset(UINT16 *path, UINT16 *fileName, UINT16 *offset);
rk_err_t BrowserUITask_GetPathAndCurFileName(UINT16 *path, UINT16 *fileName, UINT16 *pDeep);
rk_err_t BrowserUITask_StartPlayer(uint16 FileNum, int cmd);
rk_err_t BrowerUITask_SetPopupWindow(void * text, uint16 text_cmd);
rk_err_t BrowerUITask_GetFileType(UINT16 *path, UINT8 *pStr);
rk_err_t BrowserUITask_FileExtName(UINT16 *path, UINT8 *pStr);
rk_err_t BrowserUITask_GetFilePathByCurNum(uint32 curnum, int inOutCur);
rk_err_t BrowserUITask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc);
rk_err_t BrowserUITask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset);
rk_err_t BrowserUITask_ChangeDir(HDC hFather, UINT16 *pLongFileName, UINT16 *path, int inOutCur);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: BrowserUITaskTask_Resume
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.28
** Time: 19:41:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON API rk_err_t BrowserUITask_Resume(uint32 ObjectID)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITaskTask_Suspend
** Input:uint32 ObjectID
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.28
** Time: 19:41:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON API rk_err_t BrowserUITask_Suspend(uint32 ObjectID)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITaskTask_Enter
** Input:void * arg
** Return: void
** Owner:cjh
** Date: 2015.12.28
** Time: 19:41:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON API void BrowserUITask_Enter(void * arg)
{
#ifdef _USE_GUI_
    BROWSERUITASK_ASK_QUEUE browserUIAskQueue;
    rk_err_t ret;
    //void * text;

    if(gpstBrowserUITaskData->SelSouceType != SOURCE_FROM_DB_FLODER)
    {
        BrowserUITask_GetPathAndCurFileName(gpstBrowserUITaskData->CurDirPath
                                            , gpstBrowserUITaskData->CurFileInfo.LongFileName
                                            , &(gpstBrowserUITaskData->CurDeep));

        gpstBrowserUITaskData->Pathlen = StrLenW(gpstBrowserUITaskData->CurDirPath);
        if(StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName) > 0)
        {
            //printf("CurDirPath");
            //debug_hex((uint8 *)gpstBrowserUITaskData->CurDirPath, StrLenW(gpstBrowserUITaskData->CurDirPath)*2, 64);
            //printf("LongFileName");
            *(gpstBrowserUITaskData->CurFileInfo.LongFileName + StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName)) = 0x0000;
            //debug_hex((uint8 *)gpstBrowserUITaskData->CurFileInfo.LongFileName, StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName)*2, 64);

            BrowserUITask_GetFileOffset(gpstBrowserUITaskData->CurDirPath
                                        , gpstBrowserUITaskData->CurFileInfo.LongFileName
                                        , &(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset));
        }

        //printf("_GetInfoFromPath\n\n\n\n\n\n\n\n");
        BrowserUITask_GetInfoFromPath(gpstBrowserUITaskData->CurDirPath, gpstBrowserUITaskData->CurDeep);

        ret = BrowserUITask_ChangeDir(NULL, NULL,gpstBrowserUITaskData->CurDirPath, CURDIR);
        if(ret == RK_ERROR)
        {
            rk_printf("open root dir error.. \n");
            //MainTask_EventCallBack(MAINTASK_EVENT, MAINTASK_CALLBACK_CMD, NULL);
#ifdef __APP_BROWSER_BROWSERUITASK_C__
            MainTask_TaskSwtich(TASK_ID_BROWSERUI, 0, TASK_ID_MAIN, 0, NULL);
            while(1)
            {
                rkos_sleep(2000);
                //rk_printf("1.SEGMENT_ID_BROWSERUI_TASK\n");
            }
#endif
        }

        if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset >= gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems)
        {
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems - 1;
        }
       // printf("Offset:%d ",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
        if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset > (MAX_DISP_ITEM_NUM - 1))
        {
            if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset
                > (gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems - MAX_DISP_ITEM_NUM))
            {
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor =
                    (gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset+ MAX_DISP_ITEM_NUM)
                    - gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems;
                //printf(",>7,,Cursor = %d",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor);
            }
            else
            {
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor = 0;
            }
        }
        else
        {
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor =
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset;
            //printf(",,,Cursor = %d",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
        }
    }

    BrowserUITask_ChangeSelectContent(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems
                                    , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor
                                    , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);

    //rk_printf("BrowserUITask_Enter  \n");
    MainTask_SetStatus(MAINTASK_APP_BROWER,1);
    while (1)
    {
        rkos_queue_receive(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, MAX_DELAY);
        //rk_printf("!!BrowserUITask_Enter! event_type = 0x%x event =0x%x off =%d\n",browserUIAskQueue.event_type,browserUIAskQueue.event,browserUIAskQueue.offset);
        switch(browserUIAskQueue.event_type)
        {
            case APP_RECIVE_MSG_EVENT_KEY:
                switch(browserUIAskQueue.event)
                {
                    case KEY_VAL_PLAY_PRESS:
                        MainTask_SysEventCallBack(MAINTASK_SHUTDOWN, NULL);
                        break;

                    case KEY_VAL_ESC_SHORT_UP:
                        if(gpstBrowserUITaskData->CurDeep == 0)
                        {
#ifdef __APP_BROWSER_BROWSERUITASK_C__
                            if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_RECORD)
                            {
                                #ifdef __APP_FM_FMUITASK_C__
                                printf("playerType =%d.... \n",gpstBrowserUITaskData->SelSouceType);
                                RK_TASK_FM_ARG FmUiArg;
                                FmUiArg.source = 1;//RECORD_TYPE_LINEIN1
                                MainTask_TaskSwtich(TASK_ID_BROWSERUI, 0, TASK_ID_FMUI,0, &FmUiArg);
                                #endif

                            }
#ifdef _MEDIA_MODULE_
                            else if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_DB_FLODER)
                            {
                                RK_TASK_MEDIALIB_ARG MediaLibArg;
                                if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) == NULL)
                                    MediaLibArg.CurId = SORT_TYPE_SEL_FOLDER - 1;
                                else
                                    MediaLibArg.CurId = SORT_TYPE_SEL_FOLDER;

                                MainTask_TaskSwtich(TASK_ID_BROWSERUI, 0, TASK_ID_MEDIA_LIBRARY,0, &MediaLibArg);
                            }
#endif
                            else
                            {
                                MainTask_TaskSwtich(TASK_ID_BROWSERUI, 0, TASK_ID_MAIN, 0, NULL);
                            }
                            while(1)
                            {
                                rkos_sleep(2000);
                                rk_printf("2.SEGMENT_ID_BROWSERUI_TASK\n");
                            }
#endif
                        }
                        else
                        {
                            //rk_printf("\nESC InFatherCursor=%d InFatherOffset=%d\n",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor
                            //    ,gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
#ifdef _MEDIA_MODULE_
                            if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_DB_FLODER)
                            {
                                gpstBrowserUITaskData->CurDeep--;
                                BrowserUITask_ChangeSelectContent(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems
                                                                , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor
                                                                , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
                            }
                            else
#endif
                            {
                                ret = BrowserUITask_GetFilePathByCurNum(browserUIAskQueue.offset, LASTDIR);
                                if(ret == RK_SUCCESS)
                                {
                                    if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
                                    {
                                        ret = FileDev_CloseDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);
                                        if(ret == RK_ERROR)
                                        {
                                            rk_printf("ESC hDir Closed Error ....\n");
                                        }
                                        gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = NULL;
                                    }

                                    if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
                                    {
                                        ret = FileDev_CloseDir(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep]);
                                        if(ret == RK_ERROR)
                                        {
                                            rk_printf("ESC hDirForFlie closed Error ....CurDeep:%d\n",gpstBrowserUITaskData->CurDeep);
                                        }
                                        gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = NULL;

                                    }
                                    ret = BrowserUITask_ChangeDir(NULL, NULL,gpstBrowserUITaskData->CurDirPath, LASTDIR);
                                    if(ret != RK_ERROR)
                                    {
                                        BrowserUITask_ChangeSelectContent(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems
                                                                        , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor
                                                                        , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
                                    }
                                    else
                                    {
                                        rk_printf("ESC BrowserUITask_ChangeSelectContent Error ....\n");
                                    }
                                }
                            }
                        }
                        break;

                    default:
                        break;
                }
                gpstBrowserUITaskData->QueueFull = 0;
                break;
            case BROWSER_UPDATA:
                {
                    if(gpstBrowserUITaskData->hSelect == NULL)
                    {
                        //rk_printf("\n++++ brow show hSelect \n");
                        //GuiTask_OperWidget(gpstBrowserUITaskData->hSelect, OPERATE_SET_DISPLAY, (void *)1, SYNC_MODE);
                        //BrowserUITask_GetFilePathByCurNum(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset, CURDIR);
                        if((gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL) && (gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL))
                        {
                            ret = BrowserUITask_ChangeDir(NULL, NULL, gpstBrowserUITaskData->CurDirPath, CURDIR);
                            if(ret != RK_ERROR)
                            {
                                BrowserUITask_ChangeSelectContent(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems
                                                                , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor
                                                                , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
                            }
                        }
                    }
                }
                break;
            case BROWSER_ENTER:
#ifdef _MEDIA_MODULE_
                if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_DB_FLODER)
                {
                    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor = browserUIAskQueue.cursor;
                    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset = browserUIAskQueue.offset;

                    if((gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs > 0)
                        &&(gpstBrowserUITaskData->CurDeep > 0) && (browserUIAskQueue.offset == 0))
                    {
                        printf("\n -----All Files-----\n");
                        gSysConfig.MusicConfig.RepeatModeBak = gSysConfig.MusicConfig.RepeatMode + AUDIO_ALLONCE;

                        BrowserUITask_MediaFindFirstFile(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus, gpstBrowserUITaskData->CurDeep);

                        BrowserUITask_StartPlayer(0, PLAY_WITH_FILE_NAME);
                    }
                    else if(browserUIAskQueue.offset < gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
                    {
                        //printf("\n -----Dir Enter-----\n");

                        if(gpstBrowserUITaskData->CurDeep < (MAX_DIR_DEPTH - 1))
                        {
                            uint32 DirClus, Index;
                            DirClus = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus;
                            if(gpstBrowserUITaskData->CurDeep == 0)
                                Index = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset;
                            else
                                Index = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset - 1;

                            gpstBrowserUITaskData->CurDeep++;
                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus = BrowserUITask_MediaGetDirClus(DirClus, Index);
                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset = 0;
                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor = 0;

                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles =
                                BrowserUITask_MediaGetTotalFiles(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus);

                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs =
                                BrowserUITask_MediaGetTotalSubDir(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus);
                            if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs > 0)
                                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs++;  //add All files

                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems =
                                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles
                                + gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs;

                            BrowserUITask_ChangeSelectContent(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems,
                                                                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor,
                                                                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
                        }
                    }
                    else if(browserUIAskQueue.offset < gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems)
                    {
                        //printf("\n -----Music File Enter-----\n");
                        gSysConfig.MusicConfig.RepeatModeBak = gSysConfig.MusicConfig.RepeatMode;

                        if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs > 0)
                            browserUIAskQueue.offset -= 1;

                        BrowserUITask_MediaGetLongFileName(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus,
                            browserUIAskQueue.offset, gpstBrowserUITaskData->CurFileInfo.LongFileName);

                        BrowserUITask_MediaGetPath(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus,
                            browserUIAskQueue.offset, gpstBrowserUITaskData->CurDirPath, &(gpstBrowserUITaskData->Pathlen));

                        BrowserUITask_StartPlayer(0, PLAY_WITH_FILE_NAME);
                    }
                }
                else
#endif
                {
                    if((browserUIAskQueue.offset + 1) <= gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
                    {
                        if(gpstBrowserUITaskData->CurDeep < MAX_DIR_DEPTH)
                        {
                            //rk_printf("CurDisplayItem = %d cursor=%d\n",browserUIAskQueue.offset,browserUIAskQueue.cursor);
                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor = browserUIAskQueue.cursor;
                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset = browserUIAskQueue.offset;
                            gpstBrowserUITaskData->ExtName = NULL;
                            ret = BrowserUITask_GetFilePathByCurNum(browserUIAskQueue.offset, NEXTDIR);
                            if(ret == RK_SUCCESS)
                            {
                                ret = BrowserUITask_ChangeDir(NULL, NULL, gpstBrowserUITaskData->CurDirPath, NEXTDIR);
                                if(ret == RK_ERROR)
                                {
                                    if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
                                    {
                                        ret = FileDev_CloseDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);
                                        if(ret != RK_SUCCESS)
                                        {
                                            rk_printf("hDir Closed error ....\n");
                                        }
                                        gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = NULL;
                                    }

                                    if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
                                    {
                                        ret = FileDev_CloseDir(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep]);
                                        gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = NULL;
                                        if(ret != RK_SUCCESS);
                                        {
                                            rk_printf("hDirForFlie closed error ....\n");
                                        }
                                    }
                                    ret = BrowserUITask_GetFilePathByCurNum(browserUIAskQueue.offset, LASTDIR);
                                    if(ret == RK_SUCCESS)
                                    {
                                        gpstBrowserUITaskData->CurDeep--;
                                    }
                                    rk_printf("brow next dir open error \n");
                                }
                                else
                                {
                                    BrowserUITask_ChangeSelectContent(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems
                                                                    , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor
                                                                    , gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset);
                                }
                            }
                        }
                    }
                    else if(browserUIAskQueue.offset < gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems)
                    {
                        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor = browserUIAskQueue.cursor;
                        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset = browserUIAskQueue.offset;
                        ret = BrowserUITask_GetFilePathByCurNum(browserUIAskQueue.offset, CURDIR);
                        if(ret == RK_SUCCESS)
                        {
                            //rk_printf("Len =%d! music\n",StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName)*2);
                            ret = BrowserUITask_FileExtName(gpstBrowserUITaskData->CurFileInfo.LongFileName, BrowserUIFileExtString);
                            if(ret != RK_ERROR)
                            {
                                if(gpstBrowserUITaskData->Pathlen < 251)
                                {
                                    BrowserUITask_StartPlayer(0, PLAY_WITH_FILE_NAME);
                                    while(1)
                                    {
                                        rkos_sleep(2000);
                                        //rk_printf("SEGMENT_ID_BROWSERUI_TASK\n");
                                    }
                                }
                                else
                                {
                                    rk_printf("ERROR:Path > 251\n");
                                    //text = TEXT("Path > 251");
                                    //BrowerUITask_SetPopupWindow((void*)text, TEXT_CMD_BUF);
                                }
                            }
                            else
                            {
                                rk_printf("...FILE_FORMAT_ERROR...\n");
                                //BrowerUITask_SetPopupWindow((void*)SID_FILE_FORMAT_ERROR, TEXT_CMD_ID);
                            }
                        }
                    }
                }
                break;

            case APP_RECIVE_MSG_EVENT_WARING:
                #ifdef _USE_GUI_
                if(gpstBrowserUITaskData->hMsgBox != NULL)
                {
                    //rk_printf("gpstBrowserUITaskData->hMsgBox delete\n");
                    GuiTask_OperWidget(gpstBrowserUITaskData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
                    GuiTask_DeleteWidget(gpstBrowserUITaskData->hMsgBox);
                    gpstBrowserUITaskData->hMsgBox = NULL;

                }
                gpstBrowserUITaskData->QueueFull = 0;
                #endif
                break;
            default :
                break;
        }
    }
#endif
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _MEDIA_MODULE_

/*******************************************************************************
** Name: BrowserUITask_MediaGetTotalFiles
** Input:uint32 DirClus
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN uint16 BrowserUITask_MediaGetTotalFiles(UINT32 DirClus)
{
    uint32 offset;
    uint16 TotalFiles = 0;
    FILE_TREE_BASIC FileTreeBasic;

    if (DirClus != 0xffffffff)
    {
        for (offset = DirClus; ; offset++)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(offset), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));
#ifdef _RK_CUE_
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE || FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
#else
            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_FILE)
#endif
            {
                TotalFiles++;
            }

            if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
            {
                break;
            }
        }
    }

    return TotalFiles;
}

/*******************************************************************************
** Name: BrowserUITask_MediaGetTotalSubDir
** Input:uint32 DirClus
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN uint16 BrowserUITask_MediaGetTotalSubDir(UINT32 DirClus)
{
    uint32 offset;
    uint16 TotSubDir = 0;
    FILE_TREE_BASIC FileTreeBasic;

    if (DirClus != 0xffffffff)
    {
        for (offset = DirClus; ; offset++)
        {
            FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(offset), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));

            if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
            {
                TotSubDir++;
            }

            if (FileTreeBasic.dwNextBrotherID == 0xffffffff)
            {
                break;
            }
        }
    }

    return TotSubDir;
}

/*******************************************************************************
** Name: BrowserUITask_MediaGetLongFileName
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN void BrowserUITask_MediaGetLongFileName(UINT32 DirClus, int32 CurFileNum, uint16* lfName)
{
    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    memset(lfName, 0, MAX_FILENAME_LEN *2);

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + CurFileNum), (uint8 *)&temp, 2);
    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));

#ifdef _RK_CUE_
    if(FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_CUE)
    {
        FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ ID3_TITLE_SAVE_ADDR_OFFSET, (uint8 *)lfName, MAX_FILENAME_LEN*2);
    }
    else
#endif
    {
        FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ FILE_NAME_SAVE_ADDR_OFFSET, (uint8 *)lfName, MAX_FILENAME_LEN*2);
    }
}

/*******************************************************************************
** Name: BrowserUITask_MediaGetPath
** Input:
** Return:
** Owner:ctf
** Date: 2016.1.26
** Time: 17:15:15
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN void BrowserUITask_MediaGetPath(UINT32 DirClus, int32 CurFileNum, uint16* path, UINT16* pathlen)
{
    FILE_TREE_BASIC FileTreeBasic;
    uint16 temp;

    memset(path, 0, MAX_FILENAME_LEN *2);
    *pathlen = 0;

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + CurFileNum), (uint8 *)&temp, 2);
    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA+MUSIC_SAVE_INFO_SECTOR_START)<<9)+BYTE_NUM_SAVE_PER_FILE*(UINT32)(FileTreeBasic.dwBasicInfoID)+ DIR_PATH_SAVE_ADDR_OFFSET, (uint8 *)path, MAX_FILENAME_LEN*2);
    *pathlen = StrLenW(path);
}

/*******************************************************************************
** Name: BrowserUITask_MediaGetTotalSubDir
** Input:
** Return:
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN uint32 BrowserUITask_MediaGetDirClus(UINT32 DirClus, uint32 Index)
{
    UINT32 cluster = 0xffffffff;
    UINT16 temp;
    FILE_TREE_BASIC FileTreeBasic;

    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_SORT_INFO_SECTOR_START)<<9) + 2 * (UINT32)(DirClus + Index), (uint8 *)&temp, 2);
    FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START)<<9) + sizeof(FILE_TREE_BASIC)*(UINT32)(DirClus + temp), (uint8 *)&FileTreeBasic, sizeof(FILE_TREE_BASIC));
    if (FileTreeBasic.dwNodeFlag == MEDIA_FILE_TYPE_DIR)
    {
        FW_ReadDataBaseByByte(((UINT32)(gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START + MEDIAINFO_BLOCK_SIZE * 2)<<9) + sizeof(FILE_TREE_EXTEND)*(UINT32)(FileTreeBasic.dwExtendTreeID), (uint8 *)&cluster, 4);
    }

    return cluster;
}

/*******************************************************************************
** Name: BrowserUITask_MediaVariableInit
** Input:uint16 flag
** Return: rk_err_t
** Owner:cjh
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN void BrowserUITask_MediaVariableInit(UINT16 flag, MEDIA_FLODER_INFO_STRUCT MediaFloderInfo)
{
    int i;

    if(flag)
    {
        gpstBrowserUITaskData->CurDeep = MediaFloderInfo.MusicDirDeep;

        for(i = 0; i <= gpstBrowserUITaskData->CurDeep; i++)
        {
            gpstBrowserUITaskData->FolderInfo[i].DirClus = MediaFloderInfo.DirClus[i];
            gpstBrowserUITaskData->FolderInfo[i].CurItemOffset = MediaFloderInfo.CurItemId[i];
            gpstBrowserUITaskData->FolderInfo[i].CurItemCursor = MediaFloderInfo.Cursor[i];

            gpstBrowserUITaskData->FolderInfo[i].TotalFiles =
                BrowserUITask_MediaGetTotalFiles(gpstBrowserUITaskData->FolderInfo[i].DirClus);

            gpstBrowserUITaskData->FolderInfo[i].TotalDirs =
                BrowserUITask_MediaGetTotalSubDir(gpstBrowserUITaskData->FolderInfo[i].DirClus);

            if(i)   //CurDeep != 0
            {
                if(gpstBrowserUITaskData->FolderInfo[i].TotalDirs > 0)
                    gpstBrowserUITaskData->FolderInfo[i].TotalDirs++;  //add All files
            }

            gpstBrowserUITaskData->FolderInfo[i].TotalItems =
                gpstBrowserUITaskData->FolderInfo[i].TotalFiles
                + gpstBrowserUITaskData->FolderInfo[i].TotalDirs;

            if(gpstBrowserUITaskData->FolderInfo[i].CurItemOffset >= gpstBrowserUITaskData->FolderInfo[i].TotalItems)
            {
                gpstBrowserUITaskData->FolderInfo[i].CurItemOffset = gpstBrowserUITaskData->FolderInfo[i].TotalItems - 1;
            }
        }
    }
    else
    {
        gpstBrowserUITaskData->CurDeep = 0;
        gpstBrowserUITaskData->FolderInfo[0].DirClus = 0;
        gpstBrowserUITaskData->FolderInfo[0].CurItemOffset = 0;
        gpstBrowserUITaskData->FolderInfo[0].CurItemCursor = 0;

        gpstBrowserUITaskData->FolderInfo[0].TotalFiles =
            BrowserUITask_MediaGetTotalFiles(gpstBrowserUITaskData->FolderInfo[0].DirClus);

        gpstBrowserUITaskData->FolderInfo[0].TotalDirs =
            BrowserUITask_MediaGetTotalSubDir(gpstBrowserUITaskData->FolderInfo[0].DirClus);

        gpstBrowserUITaskData->FolderInfo[0].TotalItems =
            gpstBrowserUITaskData->FolderInfo[0].TotalFiles + gpstBrowserUITaskData->FolderInfo[0].TotalDirs;
    }
}

/*******************************************************************************
** Name: BrowserUITask_MediaFindFirstFile
** Input:
** Return: rk_err_t
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_MediaFindFirstFile(UINT32 CurDirClus, UINT16 CurDirDeep)
{
    HDC hDirDev;
    UINT16 offset = 0;
    UINT16 TotalSubDir = 0;
    UINT16 TotalFiles = 0;

    hDirDev = RKDev_Open(DEV_CLASS_DIR, 0 ,NOT_CARE);
    if (hDirDev == NULL || hDirDev == (HDC)RK_ERROR || hDirDev == (HDC)RK_PARA_ERR)
    {
        rk_printf("dir device open failure.\n");
        return RK_ERROR;
    }

    BrowserUITask_MediaGetPath(CurDirClus, 0, gpstBrowserUITaskData->CurDirPath, &(gpstBrowserUITaskData->Pathlen));

    DirDev_BuildDirInfo(hDirDev , MusicFileExtString, gpstBrowserUITaskData->CurDirPath);
    DirDev_GetTotalFile(hDirDev, (uint32 *) &(gpstBrowserUITaskData->FolderInfo[CurDirDeep].TotalFiles));
    DirDev_GetTotalSubDir(hDirDev, (uint32 *) &(gpstBrowserUITaskData->FolderInfo[CurDirDeep].TotalDirs));
    RKDev_Close(hDirDev);

    printf("\n All files, TotalFiles = %d\n", gpstBrowserUITaskData->FolderInfo[CurDirDeep].TotalFiles);
    printf("\n All files, TotalDirs = %d\n", gpstBrowserUITaskData->FolderInfo[CurDirDeep].TotalDirs);

    //gpstBrowserUITaskData->FolderInfo[CurDirDeep].TotalFiles = 25;
    //gpstBrowserUITaskData->FolderInfo[CurDirDeep].TotalDirs = 7;

    if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles <= 0)
    {
        BrowerUITask_SetPopupWindow((void *)SID_NO_MUSIC_FILE, TEXT_CMD_ID);
        rk_printf("no music files.\n");
        return RK_ERROR;
    }

    gpstBrowserUITaskData->SubDirInfo.DirDeep = CurDirDeep;
    gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep] = CurDirClus;
    gpstBrowserUITaskData->SubDirInfo.TotalFile[gpstBrowserUITaskData->SubDirInfo.DirDeep] = BrowserUITask_MediaGetTotalFiles(CurDirClus);
    gpstBrowserUITaskData->SubDirInfo.TotalSubDir[gpstBrowserUITaskData->SubDirInfo.DirDeep] = BrowserUITask_MediaGetTotalSubDir(CurDirClus);

    while(gpstBrowserUITaskData->SubDirInfo.TotalFile[gpstBrowserUITaskData->SubDirInfo.DirDeep] <= 0)
    {
        BrowserUITask_MediaGotoNextDir();   //遍历下一个目录，找完子目录再找同级目录

        if(gpstBrowserUITaskData->SubDirInfo.DirDeep == CurDirDeep)
        {
            rk_printf("find first file error.\n");
            return RK_ERROR;
        }
    }

    offset = gpstBrowserUITaskData->SubDirInfo.TotalSubDir[gpstBrowserUITaskData->SubDirInfo.DirDeep];

    //获取第一个文件的长文件名和路径
    BrowserUITask_MediaGetLongFileName(gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep],
        offset, gpstBrowserUITaskData->CurFileInfo.LongFileName);

    BrowserUITask_MediaGetPath(gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep],
        offset, gpstBrowserUITaskData->CurDirPath, &(gpstBrowserUITaskData->Pathlen));

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: BrowserUITask_MediaGotoNextDir
** Input:
** Return: void
** Owner:ctf
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN void BrowserUITask_MediaGotoNextDir(void)
{
    if ((gpstBrowserUITaskData->SubDirInfo.TotalSubDir[gpstBrowserUITaskData->SubDirInfo.DirDeep] == 0) //该目录下没有子目录, 要找父目录或同级目录
            || (gpstBrowserUITaskData->SubDirInfo.DirDeep == (MAX_DIR_DEPTH - 1))) //该目录下所有子目录查找完, 要找父目录或同级目录
    {
        while(1)
        {
            if(gpstBrowserUITaskData->SubDirInfo.DirDeep == gpstBrowserUITaskData->CurDeep) //找到All files所在层目录, 不能再往上找
            {
                return;
            }

            gpstBrowserUITaskData->SubDirInfo.DirDeep--; //即指向上一级目录

            if(gpstBrowserUITaskData->SubDirInfo.CurDirNum[gpstBrowserUITaskData->SubDirInfo.DirDeep]
                < (gpstBrowserUITaskData->SubDirInfo.TotalSubDir[gpstBrowserUITaskData->SubDirInfo.DirDeep] - 1))
            {
                gpstBrowserUITaskData->SubDirInfo.CurDirNum[gpstBrowserUITaskData->SubDirInfo.DirDeep]++;
                gpstBrowserUITaskData->SubDirInfo.DirDeep++;

                gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep]
                    = BrowserUITask_MediaGetDirClus(gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep - 1], gpstBrowserUITaskData->SubDirInfo.CurDirNum[gpstBrowserUITaskData->SubDirInfo.DirDeep - 1]);
                break;
            }
        }
    }
    else //该目录下还有子目录,要找它的第一个子目录
    {
        gpstBrowserUITaskData->SubDirInfo.CurDirNum[gpstBrowserUITaskData->SubDirInfo.DirDeep] = 0;
        gpstBrowserUITaskData->SubDirInfo.DirDeep++;

        gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep]
            = BrowserUITask_MediaGetDirClus(gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep - 1], 0);
    }

    gpstBrowserUITaskData->SubDirInfo.TotalFile[gpstBrowserUITaskData->SubDirInfo.DirDeep]
        = BrowserUITask_MediaGetTotalFiles(gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep]);

    gpstBrowserUITaskData->SubDirInfo.TotalSubDir[gpstBrowserUITaskData->SubDirInfo.DirDeep]
        = BrowserUITask_MediaGetTotalSubDir(gpstBrowserUITaskData->SubDirInfo.DirClus[gpstBrowserUITaskData->SubDirInfo.DirDeep]);
}
#endif

/*******************************************************************************
** Name: BrowserUITask_GetInfoFromPath
** Input:(UINT16 *path, UINT16 pDeep)
** Return: rk_err_t
** Owner:cjh
** Date: 2016.2.22
** Time: 10:56:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_GetInfoFromPath(UINT16 *path, UINT16 deep)
{
    uint16 pathLen;
    uint16 pathTempLen;
    uint16 backDeep = 0;
    uint16 tempPath[MAX_DIRPATH_LEN];
    uint16 floderName[MAX_DIRPATH_LEN];

    pathLen = StrLenW(path);
    pathTempLen = pathLen;

    if((pathTempLen < 3) || (pathTempLen > MAX_FILENAME_LEN))
    {
        rk_printf("GetInfo Pathlen=%d...\n",pathTempLen);
        return RK_ERROR;
    }
    if(pathTempLen == 3)
    {
        rk_printf("GetInfo c:\\ Pathlen=%d...\n",pathTempLen);
        return RK_SUCCESS;
    }

    if(deep > MAX_DIR_DEPTH)
    {
        rk_printf("GetInfo deep=%d...\n",deep);
        return RK_ERROR;
    }
    backDeep = deep;
    //rk_printf("GetInfo###deep=%d...\n",deep);
    pathTempLen--;
    while(1)
    {
        pathTempLen--;
        if(StrCmpW((path + pathTempLen), (uint16 *)L"\\", 1) == RK_SUCCESS)
        {
            if(deep < 1)
            {
                rk_printf("GetInfo c:\\ deep=%d...\n",deep);
                goto err;
            }
            deep--;
            pathTempLen++;
            //rk_printf("GetInfo Pathlen=%d pathTempLen=%d...\n",pathLen,pathTempLen);
            memcpy(floderName, path + pathTempLen, (pathLen - pathTempLen - 1)*2);
            memcpy(tempPath, path, pathTempLen*2);
            *(tempPath + pathTempLen) = 0x0000;
            //memset(tempPath + pathTempLen, 0,(pathLen - pathTempLen)*2 )
            *(floderName + (pathLen - pathTempLen - 1)) = 0x0000;
            pathLen = pathTempLen;
            pathTempLen--;

            //memset(path + pathTempLen, 0, (pathLen - pathTempLen)*2);
            //*(path + pathTempLen) = 0x0000;
            //debug_hex((char *)tempPath, StrLenW(tempPath)*2, 64);
            //rk_printf("....floderName: %d\n",StrLenW(floderName)*2);
            //debug_hex((char *)floderName, StrLenW(floderName)*2, 64);

            if(BrowserUITask_ChangeDir(NULL, NULL, tempPath, LASTDIR) == RK_ERROR)
            {
                rk_printf("LASTDIR deep=%d...\n",deep);
                goto err;
            }
            //printf("GetDirOffset..... %d\n",deep);
            BrowserUITask_GetDirOffset(floderName, tempPath, &(gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset));
            //printf("GetInfo[%d] Offset=%d \n",deep, gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset);

            if(gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset >= gpstBrowserUITaskData->FolderInfo[deep].TotalItems)
            {
                printf("GetInfo Offset:%d >= TotalItems:%d",gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset
                    ,gpstBrowserUITaskData->FolderInfo[deep].TotalItems);
                goto err;
            }
            //printf("GetInfo Offset:%d ",gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset);
            if(gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset > (MAX_DISP_ITEM_NUM - 1))
            {
                if(gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset
                    > (gpstBrowserUITaskData->FolderInfo[deep].TotalItems - MAX_DISP_ITEM_NUM))
                {
                    gpstBrowserUITaskData->FolderInfo[deep].CurItemCursor =
                        (gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset+ MAX_DISP_ITEM_NUM)
                        - gpstBrowserUITaskData->FolderInfo[deep].TotalItems;
                    //printf("GetInfo ,,,Cursor = %d",gpstBrowserUITaskData->FolderInfo[deep].CurItemCursor);
                }
                else
                {
                    gpstBrowserUITaskData->FolderInfo[deep].CurItemCursor = 0;
                }
            }
            else
            {
                gpstBrowserUITaskData->FolderInfo[deep].CurItemCursor =
                    gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset;
                //printf("GetInfo ,,,Cursor = %d",gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset);
            }
        }
        if(pathTempLen < 3)
        {
            rk_printf("GetInfo c:\\ deep=%d...\n",deep);
            goto err;
        }
    }

    err:
    //printf("over GetInfo[%d] Offset=%d \n",deep, gpstBrowserUITaskData->FolderInfo[deep].CurItemOffset);
    gpstBrowserUITaskData->CurDeep = backDeep;
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITask_GetDirOffset
** Input:UINT16 *path, UINT16 *dirName, UINT16 *offset
** Return: rk_err_t
** Owner:cjh
** Date: 2016.2.19
** Time: 17:33:58
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_GetDirOffset(UINT16 *dirName, UINT16 *path, UINT16 *offset)
{
    rk_err_t ret;
    uint16 longDirName[MAX_FILENAME_LEN];
    uint16 DirNameLen;
    uint16 DirNameCurGetLen;
    int16  totalDirs;
    //uint16 curDirNum;
    //printf("ChangeDir ok...%d\n",gpstBrowserUITaskData->CurDeep);
    DirNameLen = StrLenW(dirName);
    if(DirNameLen <= 0)
    {
        *offset = 0;
        return RK_SUCCESS;
    }
    if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
    {
        totalDirs = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs;
        //printf("DirNameLen = %d totalDirs = %d hDir=0x%x\n",DirNameLen,totalDirs,gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);
        while(totalDirs--)
        {
            //memset(longDirName, 0, MAX_FILENAME_LEN*2);
            ret = FileDev_GetFileName(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], longDirName);
            if(ret != RK_SUCCESS)
            {
                rk_printf("get FileName fail\n");
                *offset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs - totalDirs - 1;
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = *offset;
                return RK_ERROR;
            }
            DirNameCurGetLen = StrLenW(longDirName);
            //printf("folderNameCurGetLen = %d \n",DirNameCurGetLen);
            //debug_hex((uint8 *)longDirName, StrLenW(longDirName)*2, 64);
            if(DirNameCurGetLen == DirNameLen)
            {
                ret = StrCmpW(dirName, longDirName, DirNameLen);
                if(ret == RK_SUCCESS)
                {
                    *offset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs - totalDirs - 1;
                    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = *offset;
                    return RK_SUCCESS;
                }
            }
            if(FileDev_NextDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], NULL, NULL) != RK_SUCCESS)
            {
                //rk_printf("next dir\n");
                *offset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs- totalDirs - 1;
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = *offset;
                return RK_ERROR;
            }
        }
    }
    else
    {
        rk_printf("get dir  NULL err\n");
        return RK_ERROR;
    }
    //return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITask_ChangeSelectContent
** Input:int totalNum, int cursor, int cursorOffset
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.29
** Time: 14:57:38
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_ChangeSelectContent(int totalItems, int cursor, int cursorOffset)
{
#ifdef _USE_GUI_
    RKGUI_SELECT_ARG stSelectArg;
    RKGUI_SELECT_CONTENT stContentArg;

    if(gpstBrowserUITaskData->hSelect != NULL)
    {
        if(GuiTask_DeleteWidget(gpstBrowserUITaskData->hSelect) == RK_SUCCESS)
        {
            gpstBrowserUITaskData->hSelect = NULL;
            //rk_printf("--browser hSelect dele success...\n");
        }
        else
        {
            rk_printf("--browser hSelect dele faile...\n");
            return RK_ERROR;
        }
    }

    if(gpstBrowserUITaskData->hSelect == NULL)
    {
        //gpstBrowserUITaskData->CurDisplayItem = 0;
        //rk_printf("\n...hGc == NULL TotalItems=%d Cursor=%d CursorOffset=%d\n",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems,Cursor,CursorOffset);
        stSelectArg.x = 0;
        stSelectArg.y = 20;
        stSelectArg.xSize = 128;
        stSelectArg.ySize = 137;
        stSelectArg.display = 1;
        stSelectArg.level = 0;

        stSelectArg.Background = IMG_ID_BROWSER_BACKGROUND;
        stSelectArg.SeekBar = IMG_ID_BROWSER_SCOLL2;
        stSelectArg.Cursor = cursor;
        if(cursorOffset>=cursor)
        {
            stSelectArg.ItemStartOffset = cursorOffset-cursor;
        }
        else
        {
            stSelectArg.ItemStartOffset = 0;
        }
        stSelectArg.itemNum = totalItems;//gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems;
        stSelectArg.MaxDisplayItem = MAX_DISP_ITEM_NUM;
        stSelectArg.CursorStyle = IMG_ID_SEL_ICON;
        stSelectArg.IconBoxSize = 18;

        stSelectArg.pReviceMsg = BrowserUITask_SelectCallBack;

        gpstBrowserUITaskData->hSelect = GUITask_CreateWidget(GUI_CLASS_SELECT, &stSelectArg);
        if(gpstBrowserUITaskData->hSelect == NULL)
        {
            rk_printf("error:pGc == NULL\n");
            return RK_ERROR;
        }
    }
    /*else
    {
        rk_printf("SET_CONTENT...\n");
        stContentArg.icon_sel = -1;//IMG_ID_SETMENU_ICON_SEL
        stContentArg.itemNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems;
        GuiTask_OperWidget(gpstBrowserUITaskData->hSelect, OPERATE_SET_CONTENT, &stContentArg, SYNC_MODE);
    }*/
    return RK_SUCCESS;
#endif
}

/*******************************************************************************
** Name: BrowserUITask_ChangeDir
** Input:(uint32 SelectID, uint32 CursorID)
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.29
** Time: 17:27:12
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_ChangeDir(HDC hFather, UINT16 *pLongFileName, UINT16 *path, int inOutCur)
{
#ifdef _USE_GUI_
#ifdef __DRIVER_FILE_FILEDEVICE_C__
    RKGUI_SELECT_CONTENT stContentArg;
    rk_err_t ret;
    FILE_ATTR stFileAttr;

    if(hFather == NULL)
    {
        //rk_printf("hFather == NULL ....\n");
        stFileAttr.Path = path;//gpstBrowserUITaskData->CurDirPath;//L"C:\\";
        stFileAttr.FileName = NULL;
        //debug_hex((char *)stFileAttr.Path, StrLenW(stFileAttr.Path)*2, 32);
    }
    else
    {
        //rk_printf("hFather != NULL ....\n");
        stFileAttr.Path = NULL;
        stFileAttr.FileName = pLongFileName;
        //debug_hex((char *)stFileAttr.FileName, 32, 32);
    }

    if(inOutCur == NEXTDIR)
    {
        if(gpstBrowserUITaskData->CurDeep < MAX_DIR_DEPTH)
        {
            gpstBrowserUITaskData->CurDeep++;
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor = 0;
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemOffset = 0;
            //rk_printf("++CurDeep =%d\n",gpstBrowserUITaskData->CurDeep);
        }
        else
        {
            rk_printf("MAX_DIR_DEPTH\n");
            return RK_ERROR;
        }

        //rk_printf("hDir will open ....\n");
        if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
        {
            ret = FileDev_CloseDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);
            if(ret != RK_SUCCESS)
            {
                rk_printf("hDir Close1 ERROR....\n");
            }
            gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = NULL;
        }

        if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
        {
            ret = FileDev_CloseDir(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep]);
            if(ret != RK_SUCCESS)
            {
                rk_printf("hDirForFlie Close2 ERROR....\n");
            }
            gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = NULL;
        }
    }
    else if(inOutCur == LASTDIR)
    {
        if(gpstBrowserUITaskData->CurDeep > 0)
        {
            gpstBrowserUITaskData->CurDeep--;;
        }
        else
        {
            rk_printf("--1DEPTH = 0\n");
            return RK_ERROR;
        }
    }
    else if(inOutCur == CURDIR)
    {
        //rk_printf("2DEPTH = %d\n",gpstBrowserUITaskData->CurDeep);
        //gpstBrowserUITaskData->CurDeep = gpstBrowserUITaskData->CurDeep;
    }
    //rk_printf("2DEPTH = %d\n",gpstBrowserUITaskData->CurDeep);
    if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] == NULL)
    {
        //debug_hex((char *)stFileAttr.Path, StrLenW(stFileAttr.Path)*2, 52);
        gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = FileDev_OpenDir(FileSysHDC, hFather, NOT_CARE, &stFileAttr);
        if((rk_err_t)gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]  <= 0)
        {
            gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = NULL;
            rk_print_string("Browser dir open failure");
            return RK_ERROR;
        }
        //rk_printf("dir open hDir=0x%x hFileDev=0x%x ........\n",gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep],FileSysHDC);
        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs = FileDev_GetTotalDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] , NULL, NULL);//fileInfo->pExtStr
        //rk_printf("TotalDirs =%d",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs);

        if((gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs < 0) ||
           (gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs > 0x7FFE))
        {
            rk_printf("TotalDirs[%d] = RK_ERROR\n",gpstBrowserUITaskData->CurDeep);
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs = 0;
            return RK_ERROR;
        }
        //close dir --> open dir handle....

        if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs > 0)
        {
            if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
            {
                //rk_printf("hDir closed %d\n",gpstBrowserUITaskData->CurDeep);
                ret = FileDev_CloseDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);
                gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = NULL;
                if(ret != RK_SUCCESS)
                {
                    rk_printf("\n....hDir closed ret error%d\n",ret);
                    return RK_ERROR;
                }
            }
            gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = FileDev_OpenDir(FileSysHDC, hFather, NOT_CARE, &stFileAttr);
            if((rk_err_t)gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]  <= 0)
            {
                gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] = NULL;
                rk_print_string("2hDir open failure");
                return RK_ERROR;
            }

            if(FileDev_NextDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], NULL, &stFileAttr) != RK_SUCCESS)
            {
                rk_printf("FileDev_NextDir error\n");
                return RK_ERROR;
            }
            else
            {
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = 0;
                //rk_printf("....FileDev_NextDir....\n");
            }
        }
    }

    if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] == NULL)
    {
        //rk_printf(".hFather=0x%x..hFileDev=0x%x..\n", hFather, FileSysHDC);
        //debug_hex((char *)stFileAttr.Path, StrLenW(stFileAttr.Path)*2+10, 52);
        gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = FileDev_OpenDir(FileSysHDC, hFather, NOT_CARE, &stFileAttr);
        if((rk_err_t)gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep]  <= 0)
        {
            gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = NULL;
            rk_print_string("hDirForFlie open failure");
            return RK_ERROR;
        }
        //rk_printf("hDirForFlie open success");
        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles = FileDev_GetTotalFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] , NULL, NULL);//fileInfo->pExtStr
        //rk_printf("TotalFiles =%d",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles);
        if((gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles < 0) ||
           (gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles > 0x7FFE))
        {
            rk_printf("TotalFiles[%d] = 0xFFFF\n",gpstBrowserUITaskData->CurDeep);
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles = 0;
            rk_print_string("get TotalFiles failure");
            return RK_ERROR;
        }

        if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles > 0)
        {
            if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
            {
                //rk_printf("hDirForFlie closed %d\n",gpstBrowserUITaskData->CurDeep);
                ret = FileDev_CloseDir(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep]);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("\n....hDirForFlie closed error = 0x%x\n",ret);
                }
                gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = NULL;
            }
            gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = FileDev_OpenDir(FileSysHDC, hFather, NOT_CARE, &stFileAttr);
            if((rk_err_t)gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep]  <= 0)
            {
                gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] = NULL;
                rk_print_string("2hFlie open failure");
                return RK_ERROR;
            }

            if (FileDev_NextFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], 0, NULL, &stFileAttr) != RK_SUCCESS)
            {
                rk_printf("FileDev_NextDir error\n");
                return RK_ERROR;
            }
            else
            {
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = 0;
                //rk_printf("..FileDev_NextDir..\n");
            }
        }
    }

    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs + gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles;
    if(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems == 0xFFFF)
    {
        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems = 0;
    }

    //rk_printf("TotalItems[%d] = %d TotalFiles=%d TotalDirs=%d\n",gpstBrowserUITaskData->CurDeep, gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems
    //    ,gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles
    //    ,gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs);

#endif
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: BrowserUITask_GetFlieOffset
** Input:UINT16 *path, UINT16 *fileName, UINT16 *offset
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.29
** Time: 14:44:50
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_GetFileOffset(UINT16 *path, UINT16 *fileName, UINT16 *offset)
{
    rk_err_t ret;
    uint16 longFileName[MAX_FILENAME_LEN];
    uint16 fileNameLen;
    uint16 fileNameCurGetLen;
    int16  totalFiles;
    uint16 curFileNum;

    BrowserUITask_ChangeDir(NULL, NULL, gpstBrowserUITaskData->CurDirPath, CURDIR);

    fileNameLen = StrLenW(fileName);
    if(fileNameLen <= 0)
    {
        *offset = 0;
        return RK_SUCCESS;
    }
    if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
    {
        totalFiles = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles;
        //printf("fileNameLen = %d totalFiles = %d CurDeep=%d\n",fileNameLen,totalFiles,gpstBrowserUITaskData->CurDeep);
        while(totalFiles--)
        {
            //printf("totalFiles = %d\n",totalFiles);

            memset(longFileName, 0, MAX_FILENAME_LEN*2);
            ret = FileDev_GetFileName(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], longFileName);
            if(ret != RK_SUCCESS)
            {
                rk_printf("get FileName fail\n");
                *offset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems - totalFiles - 1;
                curFileNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles - totalFiles - 1;
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = curFileNum;
                return RK_ERROR;
            }
            fileNameCurGetLen = StrLenW(longFileName);
            //printf("fileNameCurGetLen = %d \n",fileNameCurGetLen);
            //debug_hex((uint8 *)longFileName, StrLenW(longFileName)*2, 64);
            //debug_hex((uint8 *)fileName, StrLenW(fileName)*2, 64);
            if(fileNameCurGetLen == fileNameLen)
            {
                ret = StrCmpW(fileName, longFileName, fileNameLen);
                if(ret == RK_SUCCESS)
                {
                    *offset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems - totalFiles - 1;
                    curFileNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles - totalFiles - 1;
                    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = curFileNum;
                    return RK_SUCCESS;
                }
            }
            if(FileDev_NextFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], 0, NULL, NULL) != RK_SUCCESS)
            {
                rk_printf("next dir\n");
                *offset = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems - totalFiles - 1;
                curFileNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles - totalFiles - 1;
                gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = curFileNum;
                return RK_ERROR;
            }
        }
    }
    else
    {
        rk_printf("get dir  NULL err\n");
        return RK_ERROR;
    }
    //return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITask_GetPathAndCurFileName
** Input:(uint32 *path, uint32 *fileName)
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.29
** Time: 13:40:28
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_GetPathAndCurFileName(UINT16 *path, UINT16 *fileName, UINT16 *pDeep)
{
    uint16 pathLen;
    uint16 pathTempLen;
    uint16 fileNameLen;
    uint16 deep = 0;

    pathLen = StrLenW(path);
    pathTempLen = pathLen;

    if((pathTempLen < 3) || (pathTempLen > MAX_FILENAME_LEN))
    {
        rk_printf("Pathlen=%d...\n",pathTempLen);
        return RK_ERROR;
    }
    if(pathTempLen == 3)
    {
        rk_printf("c:\\ Pathlen=%d...\n",pathTempLen);
        return RK_SUCCESS;
    }
    {
        while(1)
        {
            pathTempLen--;
            if(StrCmpW((path + pathTempLen), (uint16 *)L"\\", 1) == RK_SUCCESS)
            {
                pathTempLen++;
                //rk_printf("Pathlen=%d pathTempLen=%d...\n",pathLen,pathTempLen);
                memcpy(fileName, path + pathTempLen, (pathLen - pathTempLen)*2);
                //memset((uint8 *)(fileName+(pathLen - pathTempLen)), 0, 6);
                *(fileName + (pathLen - pathTempLen)) = 0x0000;
                memset(path + pathTempLen, 0, (pathLen - pathTempLen)*2);

                pathTempLen--;
                while(1)
                {
                    pathTempLen--;
                    if(StrCmpW((path + pathTempLen), (uint16 *)L"\\", 1) == RK_SUCCESS)
                    {
                        deep++;
                        if(deep > MAX_DIR_DEPTH)
                        {
                            return RK_ERROR;
                        }
                        *pDeep = deep;
                    }
                    else if(pathTempLen < 3)
                    {
                        if(deep > MAX_DIR_DEPTH)
                        {
                            return RK_ERROR;
                        }
                        *pDeep = deep;
                        //rk_printf("Final deep=%d\n",deep);
                        break;
                    }
                }
                break;
            }
            if(pathTempLen < 4)
            {
                //rk_printf("Pathlen < 4\n");
                memcpy(fileName, path + pathTempLen, (pathLen - pathTempLen)*2);
                *(fileName + (pathLen - pathTempLen)) = 0x0000;
                memset(path + pathTempLen, 0, (pathLen - pathTempLen)*2);
                break;
            }
        }
    }
}
/*******************************************************************************
** Name: BrowserUITask_StartPlayer
** Input:RK_TASK_AUDIOCONTROL_ARG Arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.13
** Time: 10:52:39
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_StartPlayer(uint16 FileNum, int cmd)
{
    RK_TASK_PLAYMENU_ARG stTaskPlayer;
    rk_err_t ret;
    uint16 fileLen;

#ifdef _MEDIA_MODULE_
    if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_DB_FLODER)
    {
        int i;

        stTaskPlayer.TotalFiles = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalFiles;
        stTaskPlayer.MediaFloderInfo.MusicDirDeep = gpstBrowserUITaskData->CurDeep;
        stTaskPlayer.MediaFloderInfo.TotalSubDir = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs;

        for(i = 0; i <= gpstBrowserUITaskData->CurDeep; i++)
        {
            stTaskPlayer.MediaFloderInfo.Cursor[i] = gpstBrowserUITaskData->FolderInfo[i].CurItemCursor;
            stTaskPlayer.MediaFloderInfo.CurItemId[i] = gpstBrowserUITaskData->FolderInfo[i].CurItemOffset;
            stTaskPlayer.MediaFloderInfo.DirClus[i] = gpstBrowserUITaskData->FolderInfo[i].DirClus;
        }
    }
#endif

    memcpy(stTaskPlayer.filepath, gpstBrowserUITaskData->CurDirPath, (gpstBrowserUITaskData->Pathlen)*2);
    if(cmd == PLAY_WITH_FILE_NAME)
    {
        fileLen = StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName);
        if(gpstBrowserUITaskData->Pathlen + fileLen < MAX_DIRPATH_LEN)
        {
            memcpy(stTaskPlayer.filepath + gpstBrowserUITaskData->Pathlen, gpstBrowserUITaskData->CurFileInfo.LongFileName, fileLen*2);
        }
        else
        {
            rk_printf("file Pathlen > 250\n");
            return RK_ERROR;
        }
    }

    *(stTaskPlayer.filepath + gpstBrowserUITaskData->Pathlen + fileLen) = 0x0000;
    stTaskPlayer.FileNum = FileNum;
    stTaskPlayer.ucSelPlayType = gpstBrowserUITaskData->SelSouceType;

    rk_printf("\n.... music  ucSelPlayType =%d......\n",gpstBrowserUITaskData->SelSouceType);
    //debug_hex((char *)stTaskPlayer.filepath, StrLenW(stTaskPlayer.filepath)*2, 32);
    MainTask_TaskSwtich(TASK_ID_BROWSERUI, 0, TASK_ID_MUSIC_PLAY_MENU, 0, &stTaskPlayer);
    //RKTaskCreate(TASK_ID_MUSIC_PLAY_MENU,0, &stTaskPlayer, SYNC_MODE);
}
/*******************************************************************************
** Name: BrowerUITask_SetPopupWindow
** Input:void * text, uint16 text_cmd
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.12
** Time: 16:27:48
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowerUITask_SetPopupWindow(void * text, uint16 text_cmd)
{
#ifdef _USE_GUI_
    RKGUI_MSGBOX_ARG pstMsgBoxArg;

    pstMsgBoxArg.cmd = MSGBOX_CMD_WARNING;
    pstMsgBoxArg.x = 4;
    pstMsgBoxArg.y = 30;
    pstMsgBoxArg.display = 1;
    pstMsgBoxArg.level = 0;
    pstMsgBoxArg.title = SID_WARNING;
    pstMsgBoxArg.text_cmd = text_cmd;//TEXT_CMD_BUF
    pstMsgBoxArg.text = text;
    pstMsgBoxArg.align= TEXT_ALIGN_Y_CENTER|TEXT_ALIGN_X_CENTER;

    if(gpstBrowserUITaskData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstBrowserUITaskData->hMsgBox, OPERATE_SET_DISPLAY, 0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstBrowserUITaskData->hMsgBox);
        gpstBrowserUITaskData->hMsgBox = NULL;
    }

    if(gpstBrowserUITaskData->hMsgBox == NULL)
    {
        gpstBrowserUITaskData->hMsgBox = GUITask_CreateWidget(GUI_CLASS_MSG_BOX, &pstMsgBoxArg);
    }
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowerUITask_GetFileType
** Input:(UINT16 *path, UINT8 *pStr)
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.12
** Time: 14:15:09
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowerUITask_GetFileType(UINT16 *path, UINT8 *pStr)
{
    UINT8 Len;
    rk_err_t Retval = RK_ERROR;
    UINT8 i;
    UINT16 pathlen;

    uint8 * pBuffer;

    i = 0;
    Len = strlen((char*)pStr);
    pathlen = StrLenW(path);
    //printf("$$$GetFileType pathlen=%d----\n",pathlen);
    pathlen *= 2;

    pathlen -= 6;

    pBuffer = ((uint8 *)path) + pathlen;

    while (i <= Len)
    {
        i += 3;
        //printf("$$$string::%s----mva:%s\n",pBuffer,pStr);
        if (((*(pBuffer + 0) == *(pStr + 0)) || ((*(pBuffer + 0) + ('a' - 'A')) == *(pStr + 0)) || ((*(pBuffer + 0) - ('a' - 'A')) == *(pStr + 0)))
                && ((*(pBuffer + 2) == *(pStr + 1)) || ((*(pBuffer + 2) + ('a' - 'A')) == *(pStr + 1)) || ((*(pBuffer + 2) - ('a' - 'A')) == *(pStr + 1)))
                && ((*(pBuffer + 4) == *(pStr + 2)) || ((*(pBuffer + 4) + ('a' - 'A')) == *(pStr + 2)) || ((*(pBuffer + 4) - ('a' - 'A')) == *(pStr + 2))))
        {
            break;
        }
        pStr += 3;
    }

    if (i <= Len)
    {
        Retval = i / 3;
    }
    return (Retval);
}
/*******************************************************************************
** Name: BrowserUITask_FileExtName
** Input:UINT16 *path, UINT8 *pStr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.1.9
** Time: 18:47:50
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_FileExtName(UINT16 *path, UINT8 *pStr)
{
    rk_err_t temp;
    //printf("\n\nBrowserUITask_FileExtName\n\n");
    temp = BrowerUITask_GetFileType(path, pStr);
    //printf(".....FileType=%d\n",temp);
    return temp;
}
/*******************************************************************************
** Name: BrowserUITask_GetFilePathByCurNum
** Input:HDC dev, uint16 * path, uint32 curnum
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.30
** Time: 16:14:42
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_GetFilePathByCurNum(uint32 curnum, int inOutCur)
{
    uint32 bCurNum;
    uint32 i;
    FILE_ATTR stFileAttr;
    rk_err_t ret;
    uint16 pathLen;
    uint16 fileLen;
    uint16 fileNum;

    if(gpstBrowserUITaskData->Pathlen < 3)
    {
        rk_printf("Pathlen=%d...\n",gpstBrowserUITaskData->Pathlen);
        return RK_ERROR;
    }
    if(gpstBrowserUITaskData->CurDeep >= MAX_DIR_DEPTH)
    {
        rk_printf("CurDeep >= MAX_DIR_DEPTH...\n");
        return RK_ERROR;
    }

    gpstBrowserUITaskData->Pathlen = StrLenW(gpstBrowserUITaskData->CurDirPath);
    if(inOutCur == NEXTDIR)
    {
        if(gpstBrowserUITaskData->CurDeep >= (MAX_DIR_DEPTH - 1))
        {
            rk_printf("NEXTDIR CurDeep >= MAX_DIR_DEPTH...\n");
            return RK_ERROR;
        }
        if(curnum >= gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
        {
            rk_printf("TotalDirs=%d...\n",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs);
            return RK_ERROR;
        }

        bCurNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos;
        if(curnum > bCurNum)
        {
            for(i = 0; i < (curnum - bCurNum); i++)
            {
                if(FileDev_NextDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], NULL, &stFileAttr) != RK_SUCCESS)
                {
                    rk_printf("Dir error\n");
                }
                else
                {
                    //rk_printf("..NextDir..\n");
                }

            }
        }
        else
        {
            for(i = 0; i < (bCurNum - curnum); i++)
            {
                if (FileDev_PrevDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], 0, NULL) != RK_SUCCESS)
                {
                    rk_printf("!.. PrevDir Error ..! \n");
                }
                else
                {
                    //rk_printf("!..PrevDir ..!\n");
                }
            }
        }
        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = curnum;

        //rk_printf("curnum=%d bCurNum=%d hDir[%d]=0x%x....\n",curnum,bCurNum,gpstBrowserUITaskData->CurDeep,gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);

        if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
        {
            ret = FileDev_GetFileName(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], gpstBrowserUITaskData->CurFileInfo.LongFileName);
            if(ret != RK_SUCCESS)
            {
                rk_printf("get dir fail\n");
                return RK_ERROR;
            }
        }
        else
        {
            rk_printf("get dir  NULL err\n");
            return RK_ERROR;
        }
        fileLen = StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName);
        if(gpstBrowserUITaskData->Pathlen + fileLen < MAX_FILENAME_LEN)
        {
            if(StrCmpW((gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen-1), (uint16 *)L"\\", 1) != RK_SUCCESS)
            {
                //rk_printf("add \\.....  ");
                memcpy(&gpstBrowserUITaskData->CurDirPath[gpstBrowserUITaskData->Pathlen], L"\\", 2);
                gpstBrowserUITaskData->Pathlen++;
            }
            else
            {
                //rk_printf("exist \\.....  ");
            }
            memcpy(gpstBrowserUITaskData->CurDirPath + gpstBrowserUITaskData->Pathlen, gpstBrowserUITaskData->CurFileInfo.LongFileName, fileLen*2);
            gpstBrowserUITaskData->Pathlen += fileLen;
            //debug_hex((char * )(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen-1), 2, 2);
            if(StrCmpW((gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen-1), (uint16 *)L"\\", 1) != RK_SUCCESS)
            {
                //rk_printf("add 2 \\.....  ");
                memcpy(&gpstBrowserUITaskData->CurDirPath[gpstBrowserUITaskData->Pathlen], L"\\", 2);
                gpstBrowserUITaskData->Pathlen++;
            }
            else
            {
                //rk_printf("exist 2 \\.....  ");
            }
            *(gpstBrowserUITaskData->CurDirPath + gpstBrowserUITaskData->Pathlen) = 0x0000;
        }
        else
        {
            rk_printf("Pathlen > 255\n");
            return RK_ERROR;
        }
    }
    else if(inOutCur == LASTDIR)
    {
        if(gpstBrowserUITaskData->CurDeep == 0)
        {
            rk_printf("CurDeep = 0...\n");
            return RK_ERROR;
        }
        pathLen = gpstBrowserUITaskData->Pathlen;
        //printf("old pathLen=%d\n",pathLen);
        //debug_hex((char *)gpstBrowserUITaskData->CurDirPath, pathLen*2, 32);
        gpstBrowserUITaskData->Pathlen--;
        while(1)
        {
            gpstBrowserUITaskData->Pathlen--;
            //rk_printf("LASTDIR Pathlen=%d ...\n",gpstBrowserUITaskData->Pathlen);
            //debug_hex((char *)L"\\", 2, 2);
            //rk_printf("...=? ");
            //debug_hex((char *)(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen), 2, 2);

            if(StrCmpW((gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen), (uint16 *)L"\\", 1) == RK_SUCCESS)
            {
                gpstBrowserUITaskData->Pathlen++;
                if(pathLen < MAX_DIRPATH_LEN)
                {
                    memset(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen, 0, (pathLen-gpstBrowserUITaskData->Pathlen)*2);
                }
                else
                {
                    rk_printf("error pathLen >= 259...\n");
                    return RK_ERROR;
                }
                break;
            }
            if(gpstBrowserUITaskData->Pathlen < 4)
            {
                memset((gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen), 0, (pathLen-gpstBrowserUITaskData->Pathlen)*2);
                //rk_printf("Pathlen < 4\n");
                break;
            }
        }
        //rk_printf("LASTDIR ...\n");
        //debug_hex((char *)gpstBrowserUITaskData->CurDirPath, gpstBrowserUITaskData->Pathlen*2, 32);
    }
    else if(inOutCur == CURDIR)
    {
/*
        {
            pathLen = gpstBrowserUITaskData->Pathlen;
            while(1)
            {
                if(StrCmpA((uint8 *)(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen), (uint8 *)L"\\", 2) == RK_SUCCESS)
                {
                    gpstBrowserUITaskData->Pathlen++;
                    if(pathLen < 259)
                    {
                        memset(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen, 0, (pathLen-gpstBrowserUITaskData->Pathlen)*2);
                    }
                    else
                    {
                        rk_printf("CURDIR error pathLen >= 259...\n");
                        return RK_ERROR;
                    }
                    break;
                }
                if(gpstBrowserUITaskData->Pathlen < 4)
                {
                    memset((gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen), 0, (pathLen-gpstBrowserUITaskData->Pathlen)*2);
                    rk_printf("CURDIR Pathlen < 4\n");
                    break;
                }
                gpstBrowserUITaskData->Pathlen--;
            }
            rk_printf("CURDIR LASTDIR ...\n");
        }
*/
        if(curnum >= gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems)
        {
            rk_printf("CURDIR TotalDirs=%d...\n",gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems);
            return RK_ERROR;
        }

        bCurNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos;
        if(curnum >= gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
        {
            fileNum = curnum-gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs;
            //rk_printf("fileNum=%d bCurNum =%d\n",fileNum,bCurNum);
            if(fileNum > bCurNum)
            {
                for(i = 0; i < (fileNum - bCurNum); i++)
                {
                    if(FileDev_NextFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], NULL, NULL, &stFileAttr) != RK_SUCCESS)
                    {
                        rk_printf("hDirForFlie error\n");
                        return RK_ERROR;
                    }
                    else
                    {
                        //rk_printf("..hDirForFlie..\n");
                    }

                }
            }
            else
            {
                for(i = 0; i < (bCurNum - fileNum); i++)
                {
                    if (FileDev_PrevFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], 0, NULL, NULL) != RK_SUCCESS)
                    {
                        rk_printf("!.. hDirForFlie Error ..! \n");
                        return RK_ERROR;
                    }
                    else
                    {
                        //rk_printf("!..hDirForFlie ..!\n");
                    }
                }
            }
            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = fileNum;

            //rk_printf("curnum=%d bCurNum=%d hDir[%d]=0x%x....\n",fileNum,bCurNum,gpstBrowserUITaskData->CurDeep,gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep]);

            if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
            {
                ret = FileDev_GetFileName(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], gpstBrowserUITaskData->CurFileInfo.LongFileName);
                if(ret != RK_SUCCESS)
                {
                    rk_printf("get dir fail\n");
                    return RK_ERROR;
                }
            }
            else
            {
                rk_printf("get dir  NULL err\n");
                return RK_ERROR;
            }
            fileLen = StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName);
            if((gpstBrowserUITaskData->Pathlen + fileLen) < MAX_FILENAME_LEN)
            {
#if 0
                debug_hex((char * )(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen-1), 2, 2);
                if(StrCmpA((uint8 *)(gpstBrowserUITaskData->CurDirPath+gpstBrowserUITaskData->Pathlen-1), (uint8 *)L"\\", 2) != RK_SUCCESS)
                {
                    rk_printf("add for file \\.....  \n");
                    memcpy(&gpstBrowserUITaskData->CurDirPath[gpstBrowserUITaskData->Pathlen], L"\\", 2);
                    gpstBrowserUITaskData->Pathlen++;
                }
                else
                {
                    rk_printf("exist for file \\.....  \n");
                }
                memcpy(gpstBrowserUITaskData->CurDirPath + gpstBrowserUITaskData->Pathlen, gpstBrowserUITaskData->CurFileInfo.LongFileName, fileLen*2);
                gpstBrowserUITaskData->Pathlen += fileLen; //StrLenW(gpstBrowserUITaskData->CurDirPath);
#endif
            }
            else
            {
                rk_printf("file Pathlen > 250\n");
                return RK_ERROR;
            }
        }
        else
        {
            rk_printf("! faile\n");
            return RK_ERROR;
        }
    }

    #if 0
    {
        Unicode2Ascii(fileName, gpstBrowserUITaskData->CurFileInfo.LongFileName, StrLenW(gpstBrowserUITaskData->CurFileInfo.LongFileName));
        printf("......floder name %s\n", fileName);
        debug_hex((char *)gpstBrowserUITaskData->CurFileInfo.LongFileName, 50, 50);

        Unicode2Ascii(path, gpstBrowserUITaskData->CurDirPath, gpstBrowserUITaskData->Pathlen);
        printf("......new path %s len=%d\n", path,gpstBrowserUITaskData->Pathlen);
        debug_hex((char *)gpstBrowserUITaskData->CurDirPath, gpstBrowserUITaskData->Pathlen*2, 32);
    }
    #endif

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITask_ButtonCallBack
** Input:APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.29
** Time: 18:00:52
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN rk_err_t BrowserUITask_ButtonCallBack(APP_RECIVE_MSG_EVENT event_type, uint32 event, void * arg, HGC pGc)
{
#ifdef _USE_GUI_
    rk_err_t ret = 0;
    BROWSERUITASK_ASK_QUEUE browserUIAskQueue;
    BROWSERUITASK_ASK_QUEUE browserUIAskQueue_tmp;
    //printf("Browser event_type = 0x%x event=0x%x\n",event_type, event);

    if ((gpstBrowserUITaskData->QueueFull == 1) && ((event & KEY_STATUS_LONG_UP) != KEY_STATUS_LONG_UP) && ((event & KEY_STATUS_SHORT_UP) != KEY_STATUS_SHORT_UP))
    {
        rk_printf("lose key BrowserQueueFull=%d\n",gpstBrowserUITaskData->QueueFull);
        return RK_SUCCESS;
    }

    if(gpstBrowserUITaskData->deleteTaskFlag != 1)
    {
        switch(event_type)
        {
            case APP_RECIVE_MSG_EVENT_KEY:
            {
                browserUIAskQueue.event = event;
                browserUIAskQueue.event_type = event_type;
                browserUIAskQueue.offset = NULL;
                gpstBrowserUITaskData->QueueFull = 1;
                ret = rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
                if (ret == RK_ERROR)
                {
                        if((event==KEY_VAL_FFW_PRESS)||(event==KEY_VAL_FFD_PRESS)||(event==KEY_VAL_HOLD_PRESS))
                        {
                            gpstBrowserUITaskData->QueueFull = 0;
                            return RK_SUCCESS;
                        }
                        rkos_queue_receive(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue_tmp, 0);
                        if(browserUIAskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                        {
                            rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue_tmp, 0);
                            rkos_queue_receive(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue_tmp, 0);
                            if(browserUIAskQueue_tmp.event_type != APP_RECIVE_MSG_EVENT_KEY)
                            {
                                rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue_tmp, 0);
                                gpstBrowserUITaskData->QueueFull = 0;
                            }
                            else
                            {
                                ret = rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
                                if (ret == RK_ERROR)
                                {
                                    rk_printf("Send Key Failure 0\n");
                                    gpstBrowserUITaskData->QueueFull = 0;
                                }
                            }
                        }
                        else
                        {
                            ret = rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
                            if (ret == RK_ERROR)
                            {
                                rk_printf("Send Key Failure 1\n");
                                gpstBrowserUITaskData->QueueFull = 0;
                            }
                        }

                    }
            }
            break;

            case APP_RECIVE_MSG_EVENT_WARING:
            {
                browserUIAskQueue.event = event;
                browserUIAskQueue.event_type = event_type;
                browserUIAskQueue.offset = NULL;
                gpstBrowserUITaskData->QueueFull = 1;
                ret = rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
                if(ret == RK_ERROR)
                {
                    rk_printf("-------lose Browser_MsgBoxCallBack 0\n");
                    rkos_queue_receive(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue_tmp, 0);//MAX_DELAY
                    rk_printf ("-------lose Browser_MsgBoxCallBack 1\n");
                    rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
                }
            }
            break;

            case APP_RECIVE_MSG_EVENT_DIALOG:
            {
                browserUIAskQueue.event = event;
                browserUIAskQueue.event_type = event_type;
                browserUIAskQueue.offset = NULL;
                ret = rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
            }
            break;

            default:
            break;
        }
    }
#endif
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITask_SelectCallBack
** Input:HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset
** Return:  rk_err_t
** Owner:cjh
** Date: 2015.12.29
** Time: 17:59:39
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_COMMON_
COMMON FUN  rk_err_t BrowserUITask_SelectCallBack(HGC pGc, eSELECT_EVENT_TYPE event_type, void * arg, int offset)
{
#ifdef _USE_GUI_
    RKGUI_SELECT_ITEM * item;
    rk_err_t ret = 0;
    FILE_ATTR stFileAttr;
    int curPos;
    BROWSERUITASK_ASK_QUEUE browserUIAskQueue;
    BROWSERUITASK_ASK_QUEUE browserUIAskQueue_tmp;
    //char * pTempPritnf;
    int i;
    uint32 bCurNum;
    int fileOffset;
    uint8 floderName[256];
    uint8 fileName[256];

    stFileAttr.Path = gpstBrowserUITaskData->CurDirPath;//L"C:\\";
    stFileAttr.FileName = NULL;

    //printf("Browser offset = %d event_type=0x%x arg=0x%x\n",offset,event_type, (uint32)arg);

    switch(event_type)
    {
        case SELECT_ENVEN_ENTER:
        {
            browserUIAskQueue.event = NULL;
            browserUIAskQueue.event_type = BROWSER_ENTER;
            browserUIAskQueue.offset = offset;
            browserUIAskQueue.cursor = (uint32)arg;
            //printf("brower send offset %d cursor=%d  0x%x\n",browserUIAskQueue.offset,browserUIAskQueue.cursor,browserUIAskQueue.event_type);
            ret = rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
            if (ret == RK_ERROR)
            {
                printf("send enter enven\n");
                rkos_queue_receive(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue_tmp, 0);
                rkos_queue_send(gpstBrowserUITaskData->BrowserUITaskAskQueue, &browserUIAskQueue, 0);
            }
        }
        break;

        case SELECT_ENVEN_UPDATA:
        {
            item = (RKGUI_SELECT_ITEM *)arg;
            item->cmd = TEXT_CMD_BUF;
            item->sel_icon = -1;

            //printf("TotalItems=%d TotalDirs=%d\n", gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems, gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs);
            if(offset > gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalItems)
            {
                printf("offset flow over!!\n");
                goto error;
            }

#ifdef _MEDIA_MODULE_
            if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_DB_FLODER)
            {
                if((gpstBrowserUITaskData->CurDeep > 0) && (gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs > 0))
                {
                    if(offset == 0)
                    {
                        item->unsel_icon = IMG_ID_LIST;
                        memcpy(item->text, L"All files\0", 20);
                    }
                    else
                    {
                        if(offset < gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
                            item->unsel_icon = IMG_ID_LIST;
                        else
                            item->unsel_icon = IMG_ID_MUSIC_ICON;

                        BrowserUITask_MediaGetLongFileName(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus, offset-1, item->text);
                    }
                }
                else
                {
                    if(offset < gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
                        item->unsel_icon = IMG_ID_LIST;
                    else
                        item->unsel_icon = IMG_ID_MUSIC_ICON;

                    BrowserUITask_MediaGetLongFileName(gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].DirClus, offset, item->text);
                }

                *(item->text+StrLenW(item->text)) = 0x0000;
            }
            else
#endif
            {
                if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] == NULL)
                {
                    printf("hDir NULL!!\n");
                    goto error;
                }
                if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] == NULL)
                {
                    printf("hDirForFlie NULL!!\n");
                    goto error;
                }

                //for(gpstBrowserUITaskData->CurDisplayItem; gpstBrowserUITaskData->CurDisplayItem < offset; gpstBrowserUITaskData->CurDisplayItem++)
                {
                    if(offset < gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
                    {
                        if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
                        {
                            bCurNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos;
                            if(offset > bCurNum)
                            {
                                for(i = 0; i < (offset - bCurNum); i++)
                                {
                                    if (FileDev_NextDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], 0, NULL) != RK_SUCCESS)
                                    {
                                        printf("NextDir Error[%d]!!,offset=%d TotalDirs=%d %d\n ",gpstBrowserUITaskData->CurDeep, offset, gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs);
                                        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = bCurNum + 1 + i;
                                        goto error;
                                    }
                                    else
                                    {
                                        //printf("NextDir !!%d\n",bCurNum + 1 + i);
                                    }
                                }
                            }
                            else
                            {
                                for(i = 0; i < (bCurNum - offset); i++)
                                {
                                    if (FileDev_PrevDir(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], 0, NULL) != RK_SUCCESS)
                                    {
                                        printf("!! PrevDir Error!! offset = %d\n",offset);
                                        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = bCurNum - 1 - i;
                                        goto error;
                                    }
                                    else
                                    {
                                        //printf("!!PrevDir !!%d\n",bCurNum - 1 - i);
                                    }
                                }
                            }
                            gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurDirHandlePos = offset;
                        }
                        else
                        {
                            printf("hDir = NULL !!\n");
                            goto error;
                        }
                    }
                    else
                    {
                        bCurNum = gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos;
                        fileOffset = offset - gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs;
                        if(fileOffset > bCurNum)
                        {
                            for(i = 0; i < (fileOffset - bCurNum); i++)
                            {
                                if (FileDev_NextFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], 0, NULL, NULL) != RK_SUCCESS)
                                {
                                    printf("!!121 NextFile Error!! offset = %d\n",offset);
                                    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = bCurNum + 1 + i;
                                    goto error;
                                }
                                else
                                {
                                    //printf("!!NextFile !! %d\n",bCurNum + 1 + i);
                                }
                            }
                        }
                        else
                        {
                            for(i = 0; i < (bCurNum - fileOffset); i++)
                            {
                                if (FileDev_PrevFile(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], 0, NULL, NULL) != RK_SUCCESS)
                                {
                                    printf("!!131 PrevFile Error!! offset =%d\n",offset);
                                    gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = bCurNum - 1 - i;
                                    goto error;
                                }
                                else
                                {
                                    //printf("!!PrevFile !!%d\n",bCurNum - 1 - i);
                                }
                            }
                        }
                        gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurFileHandlePos = fileOffset;
                    }
                }

                if(offset+1 <= gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].TotalDirs)
                {
                    item->unsel_icon = IMG_ID_LIST;
                    if(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep] != NULL)
                    {
                        //curPos = offset
                        //printf("11Folders FileDev_GetFileName \n");
                        ret = FileDev_GetFileName(gpstBrowserUITaskData->hDir[gpstBrowserUITaskData->CurDeep], item->text);
                        if(ret != RK_SUCCESS)
                        {
                            printf("Folders FileDev_GetFileName fail\n");
                        }

                        //Unicode2Ascii(floderName, item->text, StrLenW(item->text));
                        //printf("floder name[%d] %s\n",StrLenW(item->text), floderName);
                        //debug_hex((char *)item->text, StrLenW(item->text)*2, 50);
                    }
                    else
                    {
                        printf("gpstBrowserUITaskData->hDir = NULL\n");
                        goto error;
                    }
                }
                else
                {
                    if(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep] != NULL)
                    {
                        //curPos = offset
                        //printf("22hFlie FileDev_GetFileName \n");
                        ret = FileDev_GetFileName(gpstBrowserUITaskData->hDirForFlie[gpstBrowserUITaskData->CurDeep], item->text);
                        if(ret != RK_SUCCESS)
                        {
                            printf("hDirForFlie FileDev_GetFileName fail\n");
                            goto error;
                        }
                        //Unicode2Ascii(fileName, item->text, StrLenW(item->text));
                        //printf("flie name[%d] %s\n",StrLenW(item->text), fileName);
                        //debug_hex((char *)item->text, StrLenW(item->text)*2, 50);
                    }
                    else
                    {
                        printf("gpstBrowserUITaskData->hDir = NULL\n");
                        goto error;
                    }
                    ret = BrowserUITask_FileExtName(item->text, BrowserUIFileExtString);
                    if(ret != RK_ERROR)
                    {
                        item->unsel_icon = IMG_ID_MUSIC_ICON;
                    }
                    else
                    {
                        item->unsel_icon = IMG_ID_BROWSER_FILETYPE_OTHER;
                    }
                }

                //printf("StrLenW=%d\n",StrLenW(item->text));
                *(item->text+StrLenW(item->text)) = 0x0000;
            }

            return RK_SUCCESS;
error:
            item->unsel_icon = IMG_ID_JEPG_ICON;
            memcpy(item->text, L"ERROR!", sizeof(L"ERROR!"));
            *(item->text+StrLenW(item->text)) = 0x0000;
            return RK_ERROR;
        }
        break;

        default:
        break;
    }
    //printf("over over\n");

#endif
    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: BrowserUITaskTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.28
** Time: 19:41:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_INIT_
INIT API rk_err_t BrowserUITask_DeInit(void *pvParameters)
{
#ifdef _USE_GUI_
    uint32 i;
    gpstBrowserUITaskData->deleteTaskFlag = 1;
    printf("\n... BrowserUITask_DeInit \n");
    if(gpstBrowserUITaskData->hMsgBox != NULL)
    {
        GuiTask_OperWidget(gpstBrowserUITaskData->hMsgBox, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstBrowserUITaskData->hMsgBox);
        gpstBrowserUITaskData->hMsgBox = NULL;
    }
    if(gpstBrowserUITaskData->hSelect != NULL)
    {
        printf("\n- brow del hSelect \n");
        //GuiTask_OperWidget(gpstBrowserUITaskData->hSelect, OPERATE_SET_DISPLAY, (void *)0, SYNC_MODE);
        GuiTask_DeleteWidget(gpstBrowserUITaskData->hSelect);
        gpstBrowserUITaskData->hSelect = NULL;
    }
    MainTask_SetStatus(MAINTASK_APP_BROWER,0);
    GuiTask_AppUnReciveMsg(BrowserUITask_ButtonCallBack);
    for(i=0; i<MAX_DIR_DEPTH; i++)
    {
        if(gpstBrowserUITaskData->hDirForFlie[i] != NULL)
        {
            FileDev_CloseDir(gpstBrowserUITaskData->hDirForFlie[i]);
            gpstBrowserUITaskData->hDirForFlie[i] = NULL;
        }
        if(gpstBrowserUITaskData->hDir[i] != NULL)
        {
            FileDev_CloseDir(gpstBrowserUITaskData->hDir[i]);
            gpstBrowserUITaskData->hDir[i] = NULL;
        }
    }
    rkos_queue_delete(gpstBrowserUITaskData->BrowserUITaskAskQueue);
    rkos_queue_delete(gpstBrowserUITaskData->BrowserUITaskRespQueue);
    rkos_memory_free(gpstBrowserUITaskData);
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_BROWSERUI_TASK);
#endif
    gpstBrowserUITaskData = NULL;
#endif
    printf("BrowserUITask_DeInit Success\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: BrowserUITaskTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2015.12.28
** Time: 19:41:40
*******************************************************************************/
_APP_BROWSER_BROWSERUITASK_INIT_
INIT API rk_err_t BrowserUITask_Init(void *pvParameters, void *arg)
{
    RK_TASK_CLASS * pBrowserUITaskTask = (RK_TASK_CLASS*)pvParameters;
    RK_TASK_BROWSER_ARG * pArg = (RK_TASK_BROWSER_ARG *)arg;
    BROWSERUI_TASK_DATA_BLOCK * pBrowserUITaskData;
    FILE_ATTR stFileAttr;
    uint32 i;
    if (pBrowserUITaskTask == NULL)
    {
        return RK_PARA_ERR;
    }
    pBrowserUITaskData = rkos_memory_malloc(sizeof(BROWSERUI_TASK_DATA_BLOCK));
    if(pBrowserUITaskData == NULL)
    {
        printf("pSystemSetTaskData NULL\n");
        return RK_ERROR;
    }
    memset(pBrowserUITaskData, NULL, sizeof(BROWSERUI_TASK_DATA_BLOCK));
    pBrowserUITaskData->BrowserUITaskAskQueue = rkos_queue_create(1, sizeof(BROWSERUITASK_ASK_QUEUE));
    pBrowserUITaskData->BrowserUITaskRespQueue = rkos_queue_create(1, sizeof(BROWSERUITASK_RESP_QUEUE));

    //pBrowserUITaskData->CurDisplayItem = 0;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_LoadSegment(SEGMENT_ID_BROWSERUI_TASK, SEGMENT_OVERLAY_ALL);
#endif
    gpstBrowserUITaskData = pBrowserUITaskData;

    gpstBrowserUITaskData->CurDeep = 0;
    for(i=0; i<MAX_DIR_DEPTH; i++)
    {
        gpstBrowserUITaskData->hDirForFlie[i] = NULL;
        gpstBrowserUITaskData->hDir[i] = NULL;
    }
    gpstBrowserUITaskData->hSelect = NULL;

    memset(gpstBrowserUITaskData->CurDirPath, 0, 518);
    memset(gpstBrowserUITaskData->CurFileInfo.LongFileName, 0, MAX_FILENAME_LEN);
    memcpy(gpstBrowserUITaskData->CurDirPath, pArg->filepath, StrLenW(pArg->filepath) * 2);
    *(gpstBrowserUITaskData->CurDirPath + StrLenW(pArg->filepath)) = 0x0000;
    gpstBrowserUITaskData->SelSouceType = pArg->SelSouceType;

#ifdef _MEDIA_MODULE_
    if(gpstBrowserUITaskData->SelSouceType == SOURCE_FROM_DB_FLODER)
        BrowserUITask_MediaVariableInit(pArg->Flag, pArg->MediaFloderInfo);
#endif

#ifdef _USE_GUI_
    GuiTask_AppReciveMsg(BrowserUITask_ButtonCallBack);
#endif
    gpstBrowserUITaskData->deleteTaskFlag = 0;
    //printf("BrowserUITask_Init Success deep=%d len=%d Cursor=%d\n",gpstBrowserUITaskData->CurDeep, gpstBrowserUITaskData->Pathlen,gpstBrowserUITaskData->FolderInfo[gpstBrowserUITaskData->CurDeep].CurItemCursor);
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#endif
