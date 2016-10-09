/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\File\FileDevice.h
* Owner: Aaron.sun
* Date: 2014.3.31
* Time: 18:03:27
* Desc: File Device Class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.3.31     18:03:27   1.0
********************************************************************************************
*/

#ifndef __DRIVER_FILE_FILEDEVICE_H__
#define __DRIVER_FILE_FILEDEVICE_H__

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
#define _DRIVER_FILE_FILEDEVICE_COMMON_  __attribute__((section("driver_file_filedevice_common")))
#define _DRIVER_FILE_FILEDEVICE_INIT_  __attribute__((section("driver_file_filedevice_init")))
#define _DRIVER_FILE_FILEDEVICE_SHELL_  __attribute__((section("driver_file_filedevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_FILE_FILEDEVICE_DATA_      _DRIVER_FILE_FILEDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_FILE_FILEDEVICE_DATA_      __attribute__((section("driver_file_filedevice_data")))
#else
#error Unknown compiling tools.
#endif


#define VOLUME_NUM_MAX 10

typedef struct _FILE_ATTR
{
    uint16 * FileName; // unicode file name
    uint16 * Path; // unicode path: \ root , . current, .. previous, c:\xxxx\xxxx\xxxx or \xxx\xxx absolute path, xxxx\xxxx relative path
                   //xxx\xxx\ err path,c:\xxx\xxx\ err path etc.

    uint32   CrtTimeTenth;
    uint32   CrtTime;
    uint32  CrtDate;
    uint32  LstAccDate;
    uint32  WrtTime;
    uint32  WrtDate;
    uint32  FileSize;
    uint8   ShortName[12];

}FILE_ATTR;

typedef struct _VOLUME_INF
{
    uint32 VolumeID;
    uint32 VolumeType;
    uint32 VolumeSize;
    uint32 VolumeFreeSize;
    HDC hVolume;
    uint8 VolumeName[12];
}VOLUME_INF;

typedef enum _FILE_DEV_NUM
{
    FILE_DEV_NUM0,
    FILE_DEV_NUM_MAX
}FILE_DEV_NUM;


/* file pointer modify type. */
#define     SEEK_SET                0           //move the file pointer from the file start position.
#define     SEEK_CUR                1           //move the file pointer from the file current position.
#define     SEEK_END                2           //move the file pointer from the file end position.

#define     SYNCH_TOTAL_SIZE        0
#define     SYNCH_ALL_PARA          1


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
extern rk_err_t FileDev_WriteVolumeName(HDC dev, uint32 label, uint8 * name);
extern rk_err_t FileDev_ReadVolumeName(HDC dev, uint32 label, uint8 * name);
extern rk_err_t FileDev_GetFirstPos(HDC dev);
extern rk_err_t FileDev_GetVolumeInfByLabel(HDC dev, uint32 Label, VOLUME_INF * pstVolumeInf);
extern HDC FileDev_CloneFileHandler(HDC hSrc);
extern rk_err_t FileDev_SynchFileHandler(HDC hTarget, HDC hSrc, uint32 Mode);
extern rk_err_t FileDev_ClearVolumeCache(HDC dev, uint32 VolumeID);
extern rk_err_t FileDev_GetFileSize(HDC dev, uint32 * size);
extern rk_err_t FileDev_GetFileOffset(HDC dev, uint32 * offset);
extern rk_err_t FileDev_FileSeek(HDC dev, uint32 pos, uint32 Offset);
extern uint32 DateToString(uint16 date, uint8 * pstr);
extern uint32 TimeToString(uint16 time, uint8 ms, uint8 * pstr);
extern rk_err_t  FileDev_DeleteVolume(HDC dev, VOLUME_INF * pstVolumeInf);
extern rk_err_t FileDev_AddVolume(HDC dev,  VOLUME_INF * pstVolumeInf);
extern rk_err_t FileDev_GetVolumeInfByNum(HDC dev, uint32 VolumeNum, VOLUME_INF * pstVolumeInf);
extern rk_err_t FileDev_GetVolumeTotalCnt(HDC dev, uint32 * pVolumeCnt);
extern rk_err_t FileDev_GetTotalFile(HDC dev, uint8 * ExtName, uint8 Attr);
extern rk_err_t FileDev_GetTotalDir(HDC dev, uint8 * ExtName, uint8 Attr);
extern rk_err_t FileDev_DeleteFile(HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_DeleteDir(HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_CloseDir(HDC dev);
extern rk_err_t FileDev_CloseFile(HDC dev);

extern HDC FileDev_OpenFile(HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr);
extern HDC FileDev_OpenDir(HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr);

extern rk_err_t FileDev_CreateFile(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_CreateDir(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_GetFileName(HDC dev,  uint16 * FileName);
extern rk_err_t FileDev_NextFile(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_PrevFile(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_NextDir(HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr);
extern rk_err_t FileDev_PrevDir(HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr);

extern rk_err_t FileDev_Shell(HDC dev, uint8 * pstr);
extern HDC FileDev_Create(uint32 DevID, void * arg);
extern rk_err_t FileDevDelete(uint32 DevID, void * arg);

extern rk_err_t FileDev_ReadFile(HDC dev, uint8* buffer, uint32 len);
extern rk_err_t FileDev_WriteFile(HDC dev, uint8* buffer, uint32 len);



#endif

