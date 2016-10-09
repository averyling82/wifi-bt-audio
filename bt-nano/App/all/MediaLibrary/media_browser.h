/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\music\media_browser.h
* Owner: ctf
* Date: 2016.1.26
* Time: 17:29:00
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*      ctf      2016.1.26    17:29:00      1.0
********************************************************************************************
*/


#ifndef __APP_MEDIA_BROWSER_H__
#define __APP_MEDIA_BROWSER_H__

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
#define _APP_MEDIA_BROWSER_COMMON_  __attribute__((section("app_media_browser_task_common")))
#define _APP_MEDIA_BROWSER_INIT_    __attribute__((section("app_media_browser_task_common")))

#define MAX_MUSIC_DIR_DEPTH 4

#define MEDIA_PLAY_WITH_FILE_NAME     0
#define MEDIA_PLAY_WITH_FILE_NUM      1

#define  FIND_SUM_STARTFILEID       0 //获取文件的起始id号
#define  FIND_SUM_SORTSTART         1 //菜单显示条目的起始文件序号
#define  FIND_SUM_ITEMNUM           2 //菜单拥有的条目总数
#define  FIND_SUM_FILENUM           4 //获取文件总数

typedef struct _SORT_INFO_ADDR_STRUCT
{ 
    UINT32 ulFileFullInfoSectorAddr;    //保存文件详细信息的起始sector地址(基础信息表其实地址)
    UINT16 uiSortInfoAddrOffset[4];     //同一文件不同ID3信息在基础信息表中的偏移地址
    UINT32 ulFileSortInfoSectorAddr;    //保存文件排序信息的起始sector地址(存储已排序的歌曲在基础信息表中的ID)
    UINT32 ulSortSubInfoSectorAddr[3];  //ID3信息分类保存地址(各ID3分类的SORTINFO_STRUCT结构体信息)
}SORT_INFO_ADDR_STRUCT;

typedef struct _MUSIC_DIR_TREE_STRUCT{

    UINT16 MusicDirTotalItem;
    UINT16 MusicDirBaseSortId[MAX_MUSIC_DIR_DEPTH];
    UINT16 MusicDirDeep;
    UINT16 Cursor[MAX_MUSIC_DIR_DEPTH];    //光标位置: 0~7
    UINT16 CurItemId[MAX_MUSIC_DIR_DEPTH]; //光标所指向的item在当前目录下的文件号
}MUSIC_DIR_TREE_STRUCT;
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
extern rk_err_t MediaBrowser_Init(void *pvParameters, void *arg);
extern rk_err_t MediaBrowser_DeInit(void *pvParameters);
extern void MediaBrowser_Enter(void);
extern void GetMediaItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag);
extern UINT16 GetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType, UINT16 SortInfoAddrOffset, UINT16 MusicDirDeep);

#endif

