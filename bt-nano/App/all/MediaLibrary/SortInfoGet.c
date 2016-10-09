/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\MediaLibrary\SortInfoGet.c
* Owner: ctf
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: 媒体库ui时调用
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    ctf        2016.1.26     17:29:00   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef _MEDIA_MODULE_

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "Bsp.h"
#include "global.h"
#include "AddrSaveMacro.h"
#include "media_browser.h"

//tiantian, 未实现MDReadData, 暂时用FW_ReadFirmwaveByByte替换MDReadData

#if 0
/*
--------------------------------------------------------------------------------
  Function name : void GetSavedMusicPath(UINT8 *pPathBuffer, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId)

  Author        : anzhiguo
  Description   : 获取路径信息，，文件号，以及长文件名

  Input         : pPathBuffer -- 获取的路径信息存放buf
                  ulFullInfoSectorAddr --- flash中存放详细文件信息的起始地址
                  ulSortSectorAddr --- flash中存放分类排序信息的起始地址
                  uiSortId -- 需要获取的文件在分类信息中的排序号
                  Filenum -- 当前要获取的文件在所有文件中的序号(从 1 开始)，用于在收藏夹中添加文件是判断是否重复添加
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_APP_MEDIA_BROWSER_COMMON_
COMMON API void GetSavedMusicDir(FIND_DATA * pFindData, UINT32 ulFullInfoSectorAddr, UINT32 ulSortSectorAddr, UINT16 uiSortId )
{
    UINT32 temp1;
    UINT8 ucBufTemp[2];

    FW_ReadDataBaseByByte((ulSortSectorAddr<<9)+((UINT32)uiSortId)*2, ucBufTemp, 2);
    temp1 = (UINT16)ucBufTemp[0]+(((UINT16)ucBufTemp[1])<<8);
    FW_ReadDataBaseByByte((ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)DIR_CLUS_SAVE_ADDR_OFFSET, (uint8 *)pFindData, 8);

#ifdef _RK_CUE_
    FW_ReadDataBaseByByte((ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_START_SAVE_ADDR_OFFSET, (uint8 *)&(pFindData->CueStartTime), 4);
    FW_ReadDataBaseByByte((ulFullInfoSectorAddr<<9)+(UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + (UINT32)CUE_END_SAVE_ADDR_OFFSET, (uint8 *)&(pFindData->CueEndTime), 4);

    if(pFindData->CueStartTime != 0 || pFindData->CueEndTime != 0)
    {
        pFindData->IsCue = 1;
    }
    else
    {
        pFindData->IsCue = 0;
    }
#endif
}
#endif

/*
--------------------------------------------------------------------------------
  Function name :
  void GetSavedMusicFileName(unsigned char *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, unsigned int uiSortId, unsigned int uiCharNum, unsigned int uiCurDeep)
  Author        : anzhiguo
  Description   : 获取媒体库显示条目的信息(流派名，歌手名，专辑名，长文件名，ID3Tilte)

  Input         : AddrInfo -- 文件信息存放地址结构体变量
                : uiSortId -- 文件号
                : uiCharNum --- 要读取的字节数
                : uiCurDeep --- 通过该变量确定偏移地址
                : Flag --- 判断是否读取文件名或ID3Title 为1表示是读取文件名或ID3Title，为0表示读取其他ID3信息
  Return        : pFileName 要获取的文件长文件名的指针

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
_APP_MEDIA_BROWSER_COMMON_
COMMON API void GetMediaItemInfo(UINT16 *pFileName, SORT_INFO_ADDR_STRUCT AddrInfo, UINT16 uiSortId, UINT16 uiCharNum, UINT16 uiCurDeep, UINT16 Flag)
{
    UINT16 i;
    UINT16 temp1;
    UINT8   ucBufTemp[8],ucBufTemp1[2];
    UINT32  AddrOffset;
    UINT8     FileInfoBuf[MEDIA_ID3_SAVE_CHAR_NUM *2];//长文件名比ID3的信息要长，故选择长文件的长度开空间不会出现数组越界

   SORTINFO_STRUCT *Subinfo = (SORTINFO_STRUCT *)ucBufTemp;

   if(Flag)
   {
        FW_ReadDataBaseByByte((AddrInfo.ulFileSortInfoSectorAddr<<9)+(unsigned long)(uiSortId*2), ucBufTemp, 2);
        temp1 = (ucBufTemp[0]&0xff)+((ucBufTemp[1]&0xff)<<8); // 获得对应的文件保存号
   }
   else
   {
        //获取一个子项的信息结构 SORTINFO_STRUCT
        FW_ReadDataBaseByByte((AddrInfo.ulSortSubInfoSectorAddr[uiCurDeep]<<9)+(unsigned long)(uiSortId*sizeof(SORTINFO_STRUCT)), ucBufTemp, sizeof(SORTINFO_STRUCT));
        //通过信息结构中的 BaseID 参数获取需要的文件号
        FW_ReadDataBaseByByte((AddrInfo.ulFileSortInfoSectorAddr<<9)+Subinfo->BaseID*2, ucBufTemp1, 2);
        temp1 = (ucBufTemp1[0]&0xff)+((ucBufTemp1[1]&0xff)<<8); // 获得对应的文件保存号
   }

    AddrOffset = (UINT32)(temp1)*BYTE_NUM_SAVE_PER_FILE + AddrInfo.uiSortInfoAddrOffset[uiCurDeep]; // 通过偏移位置来确定读取的是长文件名信息还是ID3Title信息

    FW_ReadDataBaseByByte((AddrInfo.ulFileFullInfoSectorAddr<<9)+AddrOffset, FileInfoBuf, uiCharNum*2);

    for(i=0;i<uiCharNum;i++)
    {
        *pFileName++ = (UINT16)FileInfoBuf[2*i]+((UINT16)FileInfoBuf[2*i+1]<<8);
    }
}


/*
--------------------------------------------------------------------------------
  Function name : unsigned int GetSummaryInfo(unsigned long ulSumSectorAddr, unsigned int uiSumId, unsigned int uiFindSumType, SortInfoAddrOffset, MusicDirDeep)
{
  Author        : anzhiguo
  Description   : 获取ID3归类信息的条目(专辑下的文件个数，或是同一个艺术家下的文件个数)

  Input         : ulSumSectorAddr ID3信息归类存储地址
                : uiSumId  -- 条目id
                : uiFindSumType -- 获取条目信息的类型，(条目总数还是，起始条目序号，还是具体的ID3信息)
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo      2009/06/02         Ver1.0
  desc:         指到对应信息在flash中的sec地址
--------------------------------------------------------------------------------
*/
_APP_MEDIA_BROWSER_COMMON_
COMMON API UINT16 GetSummaryInfo(UINT32 ulSumSectorAddr, UINT16 uiSumId, UINT16 uiFindSumType, UINT16 SortInfoAddrOffset, UINT16 MusicDirDeep)
{
    UINT16 uiSumInfo;
    UINT32 uiSumTypeOffset;
    SORTINFO_STRUCT Subinfo;

    FW_ReadDataBaseByByte((ulSumSectorAddr<<9)+(UINT32)uiSumId*8, (UINT8 *)(&Subinfo), 8);


    if(uiFindSumType==FIND_SUM_STARTFILEID)
    {
        uiSumInfo = Subinfo.BaseID;
    }
    else if(uiFindSumType==FIND_SUM_SORTSTART)
    {
        uiSumInfo = Subinfo.ItemBaseID;
    }
    else if(uiFindSumType==FIND_SUM_ITEMNUM)
    {
        uiSumInfo = Subinfo.ItemNum;

        //Rk Aaron.sun
        if(SortInfoAddrOffset == ID3_ALBUM_SAVE_ADDR_OFFSET &&  MusicDirDeep != 0)
        {
            uiSumInfo++;  // for All Album
        }
    }
    else if (uiFindSumType==FIND_SUM_FILENUM)
    {
        uiSumInfo = Subinfo.FileNum;
    }

    return uiSumInfo;
}

#endif

