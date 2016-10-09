/*
********************************************************************************************
*
*        Copyright (c)Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\music\FileInfo.h
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

#ifndef __FILE_INFO_H__
#define __FILE_INFO_H__

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
typedef __packed struct
{
    uint16 DirDeep;
    uint32 DirClus[MAX_DIR_DEPTH];
    uint16 CurDirNum[MAX_DIR_DEPTH];   //该Dir在当前目录的位置,从0开始，排除文件
    uint16 TotalFile[MAX_DIR_DEPTH];
    uint16 TotalSubDir[MAX_DIR_DEPTH];
} SUB_DIR_INFO;

typedef __packed struct _FILE_TREE_BASIC
{
    uint32 dwNextBrotherID; //下一个文件或目录ID
    uint32 dwExtendTreeID;  //预留的第一个子文件或子目录ID(第4张表)
    uint32 dwBasicInfoID;   //目录本身在基础扩展表的ID(第2张表)
    uint32 dwNodeFlag;      // dir , file or cue
}
FILE_TREE_BASIC, * P_FILE_TREE_BASIC;


typedef __packed struct _FILE_TREE_EXTEND
{
    uint32 dwSonID;
}
FILE_TREE_EXTEND, * P_FILE_TREE_EXTEND;


typedef enum
{
    MEDIA_FILE_TYPE_DIR,
    MEDIA_FILE_TYPE_FILE,

#ifdef _RK_CUE_
    MEDIA_FILE_TYPE_CUE,
#endif

    MEDIA_FILE_TYPE_DELETED
}
MEDIA_FILE_TYPE;


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

#endif

