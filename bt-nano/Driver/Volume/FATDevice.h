/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\VOLUME\FATDevice.h
* Owner: Aaron.sun
* Date: 2014.3.10
* Time: 15:20:01
* Desc: fat file sytem device class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.3.10     15:20:01   1.0
********************************************************************************************
*/

#ifndef __DRIVER_VOLUME_FATDEVICE_H__
#define __DRIVER_VOLUME_FATDEVICE_H__

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
#define _DRIVER_VOLUME_FATDEVICE_COMMON_  __attribute__((section("driver_volume_fatdevice_common")))
#define _DRIVER_VOLUME_FATDEVICE_INIT_  __attribute__((section("driver_volume_fatdevice_init")))
#define _DRIVER_VOLUME_FATDEVICE_SHELL_  __attribute__((section("driver_volume_fatdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_VOLUME_FATDEVICE_SHELL_DATA_      _DRIVER_VOLUME_FATDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_VOLUME_FATDEVICE_SHELL_DATA_      __attribute__((section("driver_volume_fatdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif


#define FAT_FILE_MAX 16
#define FAT_DIR_MAX 16


typedef struct _FAT_DEVICE_ARG
{
    HDC hPar;

}FAT_DEVICE_ARG;

//file Attr
#define     ATTR_READ_ONLY          0x01
#define     ATTR_HIDDEN             0x02
#define     ATTR_SYSTEM             0x04
#define     ATTR_DIRECTORY          0x10
#define     ATTR_ARCHIVE            0x20




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
extern rk_err_t FatDev_ReadVolumeName(HDC dev, uint8 * name);
extern rk_err_t FatDev_WriteVolumeName(HDC dev, uint8 * name);
extern rk_err_t FatDev_GetFirstPos(HDC dev);
extern rk_err_t FatDev_GetVolumeSize(HDC dev, uint32 * TotalSize, uint32 * FreeSize);
extern HDC FatDev_CloneFileHandler(HDC hSrc);
extern rk_err_t FatDev_SynchFileHandler(HDC hTarget,  HDC hSrc, uint32 Mode);
extern rk_err_t FatDev_FileSeek(HDC dev , uint32 pos, uint32 SecCnt);
extern rk_err_t FatDev_Shell(HDC dev, uint8 * pstr);
extern HDC FatDev_Create(uint32 DevID, void * arg);
extern rk_err_t FatDev_Delete(uint32 DevID, void * arg);

extern rk_size_t FatDev_ReadFile(HDC dev, uint8* buffer, uint32 len);
extern rk_size_t FatDev_WriteFile(HDC dev, uint8* buffer, uint32 len);

extern rk_err_t FatDev_PrevDir(HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr);
extern rk_err_t FatDev_PrevFile(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr);
extern rk_err_t FatDev_NextDir(HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr);
extern rk_err_t FatDev_NextFile(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr);
extern rk_err_t FatDev_GetFileName(HDC dev,  uint16 * FileName);

extern HDC FatDev_OpenDir(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);
extern HDC FatDev_OpenFile(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);

extern rk_err_t FatDev_CloseFile(HDC dev);
extern rk_err_t FatDev_CloseDir(HDC dev);


extern rk_err_t FatDev_CreateDir(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);
extern rk_err_t FatDev_CreateFile(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);

extern rk_err_t FatDev_DeleteDir(HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr);
extern rk_err_t FatDev_DeleteFile(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr);

extern rk_err_t FatDev_GetTotalDir(HDC dev, uint8 * ExtName, uint8 Attr);
extern rk_err_t FatDev_GetTotalFile(HDC dev, uint8 * ExtName, uint8 Attr);

#endif

