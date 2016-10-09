/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\MediaLibrary\FileInfoSave.c
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
#ifdef MEDIA_UPDATE

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "Bsp.h"
#include "global.h"
#include "SysInfoSave.h"
#include "device.h"
#include "FileDevice.h"
#include "ModuleInfoTab.h"
#include "FwAnalysis.h"
#include "AddrSaveMacro.h"
#include "FileInfo.h"
#include "id3.h"

#ifdef _RK_CUE_
#include "cue.h"
#endif

#define   ALL_FILENUM_DEFINE  (SORT_FILENUM_DEFINE * 2)
#define   RECORD_FILE_NUM 999
#define   TREE_BASIC_NODE_NUM_PRE_PAGE (MEDIAINFO_PAGE_SIZE / sizeof(FILE_TREE_BASIC))
#define   TREE_EXTEND_NODE_NUM_PRE_PAGE ((MEDIAINFO_PAGE_SIZE * 4) / sizeof(FILE_TREE_EXTEND))

#ifndef _RK_CUE_
static char MusicFileExtString[] = "MP1MP2MP3WAVAPEFLAAACM4AOGGMP43GPSBCWMA";
#else
static char MusicFileExtString[] = "MP1MP2MP3WAVAPEFLAAACM4AOGGMP43GPSBCCUEWMA";
#endif


typedef  struct _MEDIA_VOLUME_INFO
{
    UINT16 VolumePath[4];   // 'C:\' + '\0'
    UINT16 VolumeName[12];  //Volume Name + '\0'
    UINT32 CurVolumeTotalSubDir;    //当前磁盘的总子目录数
}MEDIA_VOLUME_INFO;

typedef struct _MEDIA_FILE_SAVE_STRUCT
{
    UINT16  LongFileName[MEDIA_ID3_SAVE_CHAR_NUM]; //就是文件系统的长文件名
    UINT16  DirPath[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  id3_title[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  id3_singer[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  id3_album[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  Genre[MEDIA_ID3_SAVE_CHAR_NUM];
    UINT16  TrackID[4];   ////Aaron.sun 2013807
    UINT32  FileOrDir;  // 1:Dir, 0:File
#ifdef _RK_CUE_
    UINT32  StartTime;
    UINT32  EndTime;
#endif
}MEDIA_FILE_SAVE_STRUCT;

MEDIA_FILE_SAVE_STRUCT gFileSaveInfo; // 用于保存文件信息的结构体变量
UINT8    gFileHeaderBuffer[MEDIAINFO_PAGE_SIZE]; //存储基础表所有信息块是否有效标志, 每个bit代表一个信息块
UINT8    gFileInfoBasicBuffer[MEDIAINFO_PAGE_SIZE]; // 存储可播放音频文件NOTE信息
UINT8    gFileInfoExtendBuffer[MEDIAINFO_PAGE_SIZE]; // 存储文件夹NOTE信息
UINT8    gFileTreeBasicBuffer[MEDIAINFO_PAGE_SIZE]; // 存储目录树结构信息，包括所有的子目录
UINT32   gFileTreeExtendBuffer[MEDIAINFO_PAGE_SIZE]; // 存储所有目录的第一个子文件或子目录的指针
/*
--------------------------------------------------------------------------------
  Function name : void FileSaveStructInit(FILE_SAVE_STRUCT  *pFileSaveTemp)
  Author        : anzhiguo
  Description   : initial the structure variable that save file infomation.

  Input         : pFileSaveTemp :structure variable to keeping file
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void FileSaveStructInit(MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp)
{
    UINT32  i;

    for (i=0; i<MEDIA_ID3_SAVE_CHAR_NUM; i++)
    {
        pFileSaveTemp->LongFileName[i] = 0;
        pFileSaveTemp->DirPath[i] = 0;
        pFileSaveTemp->id3_title[i] = 0;
        pFileSaveTemp->id3_singer[i] = 0;
        pFileSaveTemp->id3_album[i] = 0;
        pFileSaveTemp->Genre[i] = 0;
    }
}
/*
--------------------------------------------------------------------------------
  Function name : void PageWriteBufferInit(void)
  Author        : anzhiguo
  Description   : buffer initialization that be used to write flash page.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
void PageWriteBufferInit(void)
{
    memset(gFileHeaderBuffer, 0, MEDIAINFO_PAGE_SIZE);
    memset(gFileInfoBasicBuffer, 0, MEDIAINFO_PAGE_SIZE);
    memset(gFileInfoExtendBuffer, 0, MEDIAINFO_PAGE_SIZE);
    memset(gFileTreeBasicBuffer, 0, MEDIAINFO_PAGE_SIZE);
    memset(gFileTreeExtendBuffer, 0XFF, MEDIAINFO_PAGE_SIZE);
}

/*
--------------------------------------------------------------------------------
  Function name : unsigned char SaveFileInfo(unsigned char *Buffer, FILE_SAVE_STRUCT  *pFileSaveTemp)
  Author        : anzhiguo
  Description   : wirte the file information that need to save to cache buffer,the information include
                  long file name,ID3Title,ID3singer,ID3Album,file path,short file name.

  Input         : Buffer：Buffer to save file information.
                  pFileSaveTemp：the information of current file.
                  bSaveID3: 1 = Media information.

  Return        : return 1

  History:     <author>         <time>         <version>
                anzhiguo     2009/06/02         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT8 SaveFileInfo(UINT8 *Buffer, MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp, UINT8 bSaveID3)
{
    UINT32 i;
    UINT8 *pBuffer,*pPath;

    pBuffer = Buffer + FILE_NAME_SAVE_ADDR_OFFSET;
    for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存长文件名信息
    {

        *pBuffer++ = pFileSaveTemp->LongFileName[i] & 0xff;//低字节
        //printf("%02x ", *(pBuffer - 1));
        *pBuffer++ = ((pFileSaveTemp->LongFileName[i])>>8) & 0xff;//高字节

        if (pFileSaveTemp->LongFileName[i] == 0)
        {
            break;
        }
    }

    pBuffer = Buffer + DIR_PATH_SAVE_ADDR_OFFSET;
    for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存路径
    {
        *pBuffer++ = pFileSaveTemp->DirPath[i] & 0xff;//低字节
        //printf("%02x ", *(pBuffer - 1));
        *pBuffer++ = ((pFileSaveTemp->DirPath[i])>>8) & 0xff;//高字节

        if (pFileSaveTemp->DirPath[i] == 0)
        {
            break;
        }
    }

    pBuffer = Buffer + ATTR_SAVE_ADDR_OFFSET;
    if (pFileSaveTemp->FileOrDir)
    {
        *pBuffer++ = 0;
        *pBuffer++ = 'D';
        *pBuffer++ = 0;
        *pBuffer++ = 'I';
        *pBuffer++ = 0;
        *pBuffer++ = 'R';
        *pBuffer++ = 0;
        *pBuffer++ = 0;
    }
    else
    {
        *pBuffer++ = 0;
        *pBuffer++ = 'F';
        *pBuffer++ = 0;
        *pBuffer++ = 'I';
        *pBuffer++ = 0;
        *pBuffer++ = 'L';
        *pBuffer++ = 0;
        *pBuffer++ = 'E';
    }

#ifdef _RK_CUE_
    pBuffer = Buffer + CUE_START_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = pFileSaveTemp->StartTime;

    pBuffer = Buffer + CUE_END_SAVE_ADDR_OFFSET;
    *((uint32 *)pBuffer) = pFileSaveTemp->EndTime;
#endif

    if (bSaveID3)
    {
        pBuffer = Buffer + ID3_TITLE_SAVE_ADDR_OFFSET;//长文件名保存占用的空间是CHAR_NUM_PER_FILE_NAME的2倍

        for (i = 0; i < MEDIA_ID3_SAVE_CHAR_NUM; i++)//保存id3的title信息
        {

            *pBuffer++ = (pFileSaveTemp->id3_title[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->id3_title[i])>>8;
            if (pFileSaveTemp->id3_title[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer + ID3_SINGLE_SAVE_ADDR_OFFSET;

        for (i=0;i < MEDIA_ID3_SAVE_CHAR_NUM ; i++)//保存id3的singer信息
        {

            *pBuffer++ = (pFileSaveTemp->id3_singer[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->id3_singer[i])>>8;
            if (pFileSaveTemp->id3_singer[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer + ID3_ALBUM_SAVE_ADDR_OFFSET;
        for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存id3的album信息
        {

            *pBuffer++ = (pFileSaveTemp->id3_album[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->id3_album[i])>>8;
            if (pFileSaveTemp->id3_album[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer+ID3_GENRE_SAVE_ADDR_OFFSET;
        for (i=0;i<MEDIA_ID3_SAVE_CHAR_NUM;i++)//保存id3的album信息
        {

            *pBuffer++ = (pFileSaveTemp->Genre[i])&0xff;
            *pBuffer++ = (pFileSaveTemp->Genre[i])>>8;
            if (pFileSaveTemp->Genre[i] == 0)
            {
                break;
            }
        }

        pBuffer = Buffer + TRACKID_SAVE_ADDR_OFFSET;

        for (i = 0; i < 4; i++)
        {
            *(((uint16 *)pBuffer) + i) = pFileSaveTemp->TrackID[i]; //Aaron.sun 201387
        }

    }

    return 1;
}

#ifdef _RK_CUE_
CUE_INFO CUEInfo;
#endif

extern  uint32   FlashSec[3];

static UINT8 GetFileType(UINT16 *path, UINT8 *pStr)
{
    UINT8 Len;
    UINT8 Retval = 0xff;
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

rk_err_t GetPathAndCurFileName(UINT16 *path, UINT16 *dirpath, UINT16 *fileName)
{
    uint16 pathLen;
    uint16 pathTempLen;

    pathLen = StrLenW(path);
    pathTempLen = pathLen;

    if((pathTempLen < 3) || (pathTempLen > MAX_DIRPATH_LEN))
    {
        rk_printf("Pathlen=%d...\n",pathTempLen);
        return RK_ERROR;
    }

    if(pathTempLen == 3)
    {
        rk_printf("c:\\ Pathlen=%d...\n",pathTempLen);
        memcpy(dirpath, path, pathTempLen*2);
        return RK_SUCCESS;
    }

    while(1)
    {
        pathTempLen--;
        if(StrCmpW((path + pathTempLen), (uint16 *)L"\\", 1) == RK_SUCCESS)
        {
            pathTempLen++;
            memcpy(fileName, path + pathTempLen, (pathLen - pathTempLen)*2);
            *(fileName + (pathLen - pathTempLen)) = 0x0000;

            memset(path + pathTempLen, 0, (pathLen - pathTempLen)*2);

            memcpy(dirpath, path, StrLenW(path)*2);
            *(dirpath + StrLenW(path)) = 0x0000;

            break;
        }

        if(pathTempLen < 4)
        {
            memcpy(fileName, path + pathTempLen, (pathLen - pathTempLen)*2);
            *(fileName + (pathLen - pathTempLen)) = 0x0000;

            memset(path + pathTempLen, 0, (pathLen - pathTempLen)*2);

            memcpy(dirpath, path, StrLenW(path)*2);
            *(dirpath + StrLenW(path)) = 0x0000;

            break;
        }
    }

    return RK_SUCCESS;
}

/*
--------------------------------------------------------------------------------
  Function name : void SearchAndSaveMusicInfo(void)
  Author        : anzhiguo
  Description   : check all file information,and save it to specified flash,at the same time to remember
                  the information sort by file name to memory.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
                ctf           2016/04/22         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
void SearchAndSaveMusicInfo(void)
{
    UINT32  i, j;
    HDC hMusicFile;
    MEDIA_FILE_SAVE_STRUCT  *pFileSaveTemp = &gFileSaveInfo; // 用于保存文件信息

    UINT16 uiTotalVolume = 0;   //总磁盘数
    UINT16 uiVolumeTotalSubDir = 0;   //所有磁盘总子目录累计，不包含当前磁盘

    UINT16 uiTotalFile = 0;
    UINT16 uiTotalSubDir = 0;   //目录个数计数
    UINT16 uiTotalMusic = 0;    //音乐文件个数计数

    UINT16 uiFindFileResult = 0 ; // 文件检索结果
    UINT16 uiTotalFileInDir = 0;  //当前目录下的总子文件和子目录数, 不包含子目录的子目录

    UINT16 uiFileInfoBasicIndex = 0;       // Flash Page控制计数     该值等于4时，已经将一个page大小的baffer写满，可以写flash了
    UINT16 uiFileInfoExtendIndex = 0;
    UINT16 uiFileTreeBasicIndex = 0;
    UINT16 uiFileTreeExtendIndex = 0;

    UINT32 ulFileHeaderSectorAddr = 0;
    UINT32 ulFileInfoBasicSectorAddr = 0; // 保存文件详细信息的起始sector地址
    UINT32 ulFileInfoExtendSectorAddr = 0;
    UINT32 ulFileTreeBasicSectorAddr = 0;
    UINT32 ulFileTreeExtendSectorAddr = 0;

    FILE_TREE_BASIC * pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

    ID3V2X_INFO ID3Info;
    MEDIA_VOLUME_INFO MediaVolumeInfo[VOLUME_NUM_MAX];
    VOLUME_INF VolumeInfo;
    HDC hDirDev;

#ifdef _RK_CUE_
    int codec = 0;
    uint32 Index, ret;
#endif

    FW_LoadSegment(SEGMENT_ID_ID3, SEGMENT_OVERLAY_ALL); //调用ID3 解析代码

    //FileSaveStructInit(pFileSaveTemp); // 将用于记录文件保存信息的结构体清0
    PageWriteBufferInit();
    memset(&MediaVolumeInfo, 0, sizeof(MEDIA_VOLUME_INFO) * VOLUME_NUM_MAX);

    ulFileHeaderSectorAddr = gSysConfig.MediaLibrayStartLBA + MUSIC_HEADER_INFO_SECTOR_START;

    ulFileInfoBasicSectorAddr = gSysConfig.MediaLibrayStartLBA + MUSIC_SAVE_INFO_SECTOR_START; //保存详细的文件信息起始地址(sec值)
    ulFileInfoExtendSectorAddr = ulFileInfoBasicSectorAddr + MEDIAINFO_BLOCK_SIZE * 16;

    ulFileTreeBasicSectorAddr = gSysConfig.MediaLibrayStartLBA + MUSIC_TREE_INFO_SECTOR_START;
    ulFileTreeExtendSectorAddr = ulFileTreeBasicSectorAddr + MEDIAINFO_BLOCK_SIZE * 2;

    FileDev_GetVolumeTotalCnt(FileSysHDC, (uint32 *)&uiTotalVolume);
    if(uiTotalVolume > VOLUME_NUM_MAX || uiTotalVolume <= 0)
    {
        rk_printf("Error, not find volume, uiTotalVolume\n", uiTotalVolume);
        gSysConfig.MedialibPara.gMusicFileNum = 0;
        gSysConfig.MedialibPara.gTotalFileNum = 0;
        FW_RemoveSegment(SEGMENT_ID_ID3);
        return;
    }

    hDirDev = RKDev_Open(DEV_CLASS_DIR, 0 ,NOT_CARE);
    if (hDirDev == NULL || hDirDev == (HDC)RK_ERROR || hDirDev == (HDC)RK_PARA_ERR)
    {
        printf("dir device open failure");
        FW_RemoveSegment(SEGMENT_ID_ID3);
        return;
    }

    uiTotalFileInDir = uiTotalVolume;
    for(i = 0; i < uiTotalVolume; i++)  //将各磁盘作为第一层目录存入四张表
    {
        uiTotalFileInDir--;
        uiTotalFile++;
        FileSaveStructInit(pFileSaveTemp);

        FileDev_GetVolumeInfByNum(FileSysHDC, i, &VolumeInfo);

#if 0
        for (j = 0; j < 12; j++)
            MediaVolumeInfo[i].VolumeName[j] = VolumeInfo.VolumeName[j];
#else
        MediaVolumeInfo[i].VolumeName[0] = VolumeInfo.VolumeID;
        MediaVolumeInfo[i].VolumeName[1] = ':';
        MediaVolumeInfo[i].VolumeName[2] = '\0';
#endif

        MediaVolumeInfo[i].VolumePath[0] = VolumeInfo.VolumeID;
        MediaVolumeInfo[i].VolumePath[1] = ':';
        MediaVolumeInfo[i].VolumePath[2] = '\\';
        MediaVolumeInfo[i].VolumePath[3] = '\0';

        if (uiTotalFileInDir)
        {
            pFileTreeBasic->dwNextBrotherID = uiTotalFile;
        }
        else
        {
            pFileTreeBasic->dwNextBrotherID = 0xffffffff;
        }

        pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_DIR;

        gFileHeaderBuffer[(MEDIAINFO_PAGE_SIZE/2) + (uiTotalSubDir/8)] = gFileHeaderBuffer[(MEDIAINFO_PAGE_SIZE/2) + (uiTotalSubDir/8)] | (0x80 >> (uiTotalSubDir % 8));

        uiTotalSubDir++;
        pFileTreeBasic->dwExtendTreeID = uiTotalSubDir - 1;

        for (j = 0; j < 12; j++)
        {
            pFileSaveTemp->LongFileName[j] = MediaVolumeInfo[i].VolumeName[j];
        }

        for (j = 0; j < 4; j++)
        {
            pFileSaveTemp->DirPath[j] = MediaVolumeInfo[i].VolumePath[j];
        }

        pFileSaveTemp->FileOrDir = 1; //dir

        SaveFileInfo(&gFileInfoExtendBuffer[uiFileInfoExtendIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 0);

        uiFileInfoExtendIndex++;
        pFileTreeBasic->dwBasicInfoID = uiTotalSubDir - 1 + (MEDIAINFO_BLOCK_SIZE * 16  * SECTOR_BYTE_SIZE)  / BYTE_NUM_SAVE_PER_FILE;

        if (uiFileInfoExtendIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
        {
            uiFileInfoExtendIndex = 0;//文件序号回零

            FW_WriteDataBaseBySector(ulFileInfoExtendSectorAddr, gFileInfoExtendBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

            for (j = 0; j < MEDIAINFO_PAGE_SIZE; j++)//gFileInfoBasicBuffer 清零
            {
                gFileInfoExtendBuffer[j] = 0;
            }
            ulFileInfoExtendSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
        }

        pFileTreeBasic++;

        if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
        {
            pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

            FW_WriteDataBaseBySector(ulFileTreeBasicSectorAddr, gFileTreeBasicBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

            for (j = 0; j < MEDIAINFO_PAGE_SIZE; j++)//gFileInfoBasicBuffer 清零
            {
                gFileTreeBasicBuffer[j] = 0;
            }
            ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
        }
    }

    for(i = 0; i < uiTotalVolume; i++)
    {
        uint32 dir; //Dir: 1, File: 0
        uint32 TotalFile = 0;
        uint32 TotalSubDir = 0;
        UINT16 CurDirPath[MAX_DIRPATH_LEN]; //DirPath + LongFileName

        if(i > 0)
            uiVolumeTotalSubDir += MediaVolumeInfo[i - 1].CurVolumeTotalSubDir;

        DirDev_BuildDirInfo(hDirDev , MusicFileExtString, MediaVolumeInfo[i].VolumePath);
        DirDev_GetTotalSubDir(hDirDev, &(MediaVolumeInfo[i].CurVolumeTotalSubDir));

        DirDev_GetCurDirTotalFile(hDirDev, &TotalFile);
        DirDev_GetCurDirTotalSubDir(hDirDev, &TotalSubDir);
        uiTotalFileInDir = TotalFile + TotalSubDir;

        if(uiTotalFileInDir == 0)
        {
            gFileTreeExtendBuffer[i] = 0xffffffff;
        }
        else
        {
            gFileTreeExtendBuffer[i] = uiTotalFile;
        }

        uiFindFileResult = DirDev_FindFirst(hDirDev, &dir);
        while (uiTotalFile <= ALL_FILENUM_DEFINE)
        {
            if (uiFindFileResult == RK_SUCCESS)
            {
                uiTotalFileInDir--;
                FileSaveStructInit(pFileSaveTemp);
#ifdef _RK_CUE_
                //DirDev_GetCurPath(hDirDev, CurDirPath);
                //GetPathAndCurFileName(CurDirPath, pFileSaveTemp->DirPath, pFileSaveTemp->LongFileName);
                //codec = GetFileType(pFileSaveTemp->LongFileName,(UINT8 *)MusicFileExtString); //codec = 13: CUE
#endif
                if (dir == 1 && uiTotalSubDir < SORT_FILENUM_DEFINE)
                {
                    uiTotalFile++; //include all media file ,no media file and sub dir

                    if (uiTotalFileInDir)
                    {
                        pFileTreeBasic->dwNextBrotherID = uiTotalFile;
                    }
                    else
                    {
                        pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                    }

                    pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_DIR;

                    gFileHeaderBuffer[(MEDIAINFO_PAGE_SIZE/2) + (uiTotalSubDir/8)] = gFileHeaderBuffer[(MEDIAINFO_PAGE_SIZE/2) + (uiTotalSubDir/8)] | (0x80 >> (uiTotalSubDir % 8));

                    uiTotalSubDir++;
                    pFileTreeBasic->dwExtendTreeID = uiTotalSubDir - 1;

                    DirDev_GetCurPath(hDirDev, CurDirPath);
                    GetPathAndCurFileName(CurDirPath, pFileSaveTemp->DirPath, pFileSaveTemp->LongFileName);

                    pFileSaveTemp->FileOrDir = 1; //dir

                    SaveFileInfo(&gFileInfoExtendBuffer[uiFileInfoExtendIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 0);

                    uiFileInfoExtendIndex++;
                    pFileTreeBasic->dwBasicInfoID = uiTotalSubDir - 1 + (MEDIAINFO_BLOCK_SIZE * 16  * SECTOR_BYTE_SIZE)  / BYTE_NUM_SAVE_PER_FILE;

                    if (uiFileInfoExtendIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
                    {

                        uiFileInfoExtendIndex = 0;//文件序号回零

                        FW_WriteDataBaseBySector(ulFileInfoExtendSectorAddr, gFileInfoExtendBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

                        for (j = 0; j < MEDIAINFO_PAGE_SIZE; j++)//gFileInfoBasicBuffer 清零
                        {
                            gFileInfoExtendBuffer[j] = 0;
                        }
                        ulFileInfoExtendSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                    }

                    pFileTreeBasic++;

                    if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
                    {
                        pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

                        FW_WriteDataBaseBySector(ulFileTreeBasicSectorAddr, gFileTreeBasicBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

                        for (j = 0; j < MEDIAINFO_PAGE_SIZE; j++)//gFileInfoBasicBuffer 清零
                        {
                            gFileTreeBasicBuffer[j] = 0;
                        }
                        ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                    }
                }
#ifdef _RK_CUE_
                else if(codec == 13 && uiTotalMusic  < SORT_FILENUM_DEFINE)
                {

                }
#endif
                else if(uiTotalMusic  < SORT_FILENUM_DEFINE)
                {
                    //save meida file
                    uiTotalFile++; //include all media file ,no media file and sub dir

                    if (uiTotalMusic == (SORT_FILENUM_DEFINE - 1))
                    {
                        pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                    }
                    else if (uiTotalFileInDir)
                    {
                        pFileTreeBasic->dwNextBrotherID = uiTotalFile;
                    }
                    else
                    {
                        pFileTreeBasic->dwNextBrotherID = 0xffffffff;
                    }

                    pFileTreeBasic->dwNodeFlag = MEDIA_FILE_TYPE_FILE;
                    pFileTreeBasic->dwExtendTreeID = 0xffffffff;

                    gFileHeaderBuffer[uiTotalMusic/8] = gFileHeaderBuffer[uiTotalMusic/8] | (0x80 >> (uiTotalMusic % 8));

                    DirDev_GetCurPath(hDirDev, CurDirPath);
                    GetPathAndCurFileName(CurDirPath, pFileSaveTemp->DirPath, pFileSaveTemp->LongFileName);

                    hMusicFile = (HDC)DirDev_OpenCurFile(hDirDev);
                    if(hMusicFile > 0)
                    {
                        memset(&ID3Info, 0, sizeof(ID3V2X_INFO));
#ifdef _RK_ID3_
                        GetAudioId3Info(hMusicFile, &ID3Info, pFileSaveTemp->LongFileName);
#endif
                    }
                    FileDev_CloseFile(hMusicFile);

                    for (j=0; j<MEDIA_ID3_SAVE_CHAR_NUM; j++)
                    {
                        pFileSaveTemp->id3_title[j] = ID3Info.id3_title[j]; // 保存歌手信息
                        pFileSaveTemp->id3_singer[j] = ID3Info.id3_singer[j]; // 保存歌手信息
                        pFileSaveTemp->id3_album[j] = ID3Info.id3_album[j];  // 保存专辑信息
                        pFileSaveTemp->Genre[j] = ID3Info.id3_genre[j];// 保存专辑信息
                    }

                    for (j = 0; j < 4; j++)
                    {
                        pFileSaveTemp->TrackID[j] = ID3Info.nTrack[j]; //Aaron.sun 2013807
                    }

                    pFileSaveTemp->FileOrDir = 0; //dir

#if 0               //if all song show long file name ,pls open this.
                    memcpy(pFileSaveTemp->id3_title,pFileSaveTemp->LongFileName,MEDIA_ID3_SAVE_CHAR_NUM*2);
#else
                    if (pFileSaveTemp->id3_title[0] == 0)
                    {
                        memcpy(pFileSaveTemp->id3_title,pFileSaveTemp->LongFileName,MEDIA_ID3_SAVE_CHAR_NUM*2);
                    }
#endif

#ifdef _RK_CUE_
                    pFileSaveTemp->StartTime = 0;
                    pFileSaveTemp->EndTime = 0;
#endif

                    if (pFileSaveTemp->TrackID[0] == 0x0000)
                    {
                        //pFileSaveTemp->TrackID[0] = 0x39;
                        //pFileSaveTemp->TrackID[1] = 0x39;
                        //pFileSaveTemp->TrackID[2] = 0x39;
                        //pFileSaveTemp->TrackID[3] = 0x39;
                        pFileSaveTemp->TrackID[0] = 0x30;
                        pFileSaveTemp->TrackID[1] = 0x30;
                        pFileSaveTemp->TrackID[2] = 0x30;
                        pFileSaveTemp->TrackID[3] = 0x30;
                    }
                    else
                    {
                        if (pFileSaveTemp->TrackID[3] == 0X39)
                        {
                            pFileSaveTemp->TrackID[3] = 0x30;
                            if (pFileSaveTemp->TrackID[2] == 0X39)
                            {
                                pFileSaveTemp->TrackID[2] = 0x30;
                                if (pFileSaveTemp->TrackID[1] == 0X39)
                                {
                                    pFileSaveTemp->TrackID[1] = 0x30;
                                    if (pFileSaveTemp->TrackID[0] == 0X39)
                                    {
                                        pFileSaveTemp->TrackID[0] = 0x39;
                                        pFileSaveTemp->TrackID[1] = 0x39;
                                        pFileSaveTemp->TrackID[2] = 0x39;
                                        pFileSaveTemp->TrackID[3] = 0x39;
                                    }
                                    else
                                    {
                                        pFileSaveTemp->TrackID[0]++;
                                    }
                                }
                                else
                                {
                                    pFileSaveTemp->TrackID[1]++;
                                }
                            }
                            else
                            {
                                pFileSaveTemp->TrackID[2]++;
                            }

                        }
                        else
                        {
                            pFileSaveTemp->TrackID[3]++;
                        }
                    }

                    SaveFileInfo(&gFileInfoBasicBuffer[uiFileInfoBasicIndex * BYTE_NUM_SAVE_PER_FILE], pFileSaveTemp, 1);

                    uiTotalMusic++;  // all sort media file
                    uiFileInfoBasicIndex++;
                    pFileTreeBasic->dwBasicInfoID = uiTotalMusic - 1;

                    if (uiFileInfoBasicIndex == FILE_SAVE_NUM_PER_PAGE)//gFileInfoBasicBuffer 已经存满信息，大小为8k ，就开始写flash
                    {
                        uiFileInfoBasicIndex = 0;//文件序号回零

                        FW_WriteDataBaseBySector(ulFileInfoBasicSectorAddr, gFileInfoBasicBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

                        for (j = 0; j < MEDIAINFO_PAGE_SIZE; j++)//gFileInfoBasicBuffer 清零
                        {
                            gFileInfoBasicBuffer[j] = 0;
                        }
                        ulFileInfoBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                    }

                    pFileTreeBasic++;

                    if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE == 0)
                    {
                        pFileTreeBasic = (FILE_TREE_BASIC *)gFileTreeBasicBuffer;

                        FW_WriteDataBaseBySector(ulFileTreeBasicSectorAddr, gFileTreeBasicBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

                        for (j = 0; j < MEDIAINFO_PAGE_SIZE; j++)//gFileInfoBasicBuffer 清零
                        {
                            gFileTreeBasicBuffer[j] = 0;
                        }
                        ulFileTreeBasicSectorAddr  +=  (MEDIAINFO_PAGE_SIZE / SECTOR_BYTE_SIZE);//指向下一个page，为下次写做准备
                    }
                }

                //if (uiTotalMusic >= SORT_FILENUM_DEFINE)
                //{
                //    break;
                //}

                uiFindFileResult = DirDev_FindNext(hDirDev, &dir);
            }
            else
            {
                int ret;
                do
                {
                    ret = DirDev_GotoNextDir(hDirDev);       //遍历下一个目录，找完子目录再找同级目录

                    DirDev_GetCurDirTotalFile(hDirDev, &TotalFile);
                    DirDev_GetCurDirTotalSubDir(hDirDev, &TotalSubDir);

                    if (TotalFile + TotalSubDir == 0)
                    {
                        gFileTreeExtendBuffer[uiTotalVolume + uiVolumeTotalSubDir + DirDev_GetFatherDirNum(hDirDev)] = 0xffffffff;
                    }
                    else
                    {
                        break;
                    }

                }while (ret != RK_ERROR);

                if (ret == RK_ERROR)
                {
                    break;
                }

                gFileTreeExtendBuffer[uiTotalVolume + uiVolumeTotalSubDir + DirDev_GetFatherDirNum(hDirDev)] = uiTotalFile;

                uiTotalFileInDir = TotalFile + TotalSubDir;
                uiFindFileResult = DirDev_FindFirst(hDirDev, &dir);
            }
        }
    }

    FW_WriteDataBaseBySector(ulFileHeaderSectorAddr, gFileHeaderBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));

    if (uiFileInfoBasicIndex) // 保存不足2K的文件信息
    {
        FW_WriteDataBaseBySector(ulFileInfoBasicSectorAddr, gFileInfoBasicBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));
    }

    if (uiFileInfoExtendIndex) // 保存不足2K的文件信息
    {
        FW_WriteDataBaseBySector(ulFileInfoExtendSectorAddr, gFileInfoExtendBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));
    }

    if (uiTotalFile % TREE_BASIC_NODE_NUM_PRE_PAGE != 0)
    {
        FW_WriteDataBaseBySector(ulFileTreeBasicSectorAddr, gFileTreeBasicBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE));
    }

    if (uiTotalSubDir)
    {
        FW_WriteDataBaseBySector(ulFileTreeExtendSectorAddr, (UINT8*)gFileTreeExtendBuffer, (MEDIAINFO_PAGE_SIZE/SECTOR_BYTE_SIZE) * 4);
    }

    gSysConfig.MedialibPara.gTotalFileNum = uiTotalFile;
    gSysConfig.MedialibPara.gMusicFileNum = uiTotalMusic; // 得到系统全部文件数目，最大为SORT_FILENUM_DEFINE个

    RKDev_Close(hDirDev);
    FW_RemoveSegment(SEGMENT_ID_ID3);
    rk_printf("---> totalfile = %d, totalmusic = %d <---" , uiTotalFile, uiTotalMusic);
}

#endif

