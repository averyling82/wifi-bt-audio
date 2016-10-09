/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\File\DirDevice.h
* Owner: aaron.sun
* Date: 2015.6.23
* Time: 13:54:36
* Version: 1.0
* Desc: Dir Device Class
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.23     13:54:36   1.0
********************************************************************************************
*/


#ifndef __DRIVER_FILE_DIRDEVICE_H__
#define __DRIVER_FILE_DIRDEVICE_H__

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
#define _DRIVER_FILE_DIRDEVICE_COMMON_  __attribute__((section("driver_file_dirdevice_common")))
#define _DRIVER_FILE_DIRDEVICE_INIT_  __attribute__((section("driver_file_dirdevice_init")))
#define _DRIVER_FILE_DIRDEVICE_SHELL_  __attribute__((section("driver_file_dirdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_FILE_DIRDEVICE_DATA_      _DRIVER_FILE_DIRDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_FILE_DIRDEVICE_DATA_      __attribute__((section("driver_file_dirdevice_data")))
#else
#error Unknown compiling tools.
#endif


typedef enum
{
   SEARCH_ALL_FS,
   SEARCH_EX_VOICE,
   SEARCH_MAX

}SEARCH_TYPE;


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
extern rk_err_t DirDev_DeleteDir(HDC dev, uint16 * path);
extern rk_err_t DirDev_GetFatherDirNum(HDC dev);
extern HDC DirDev_OpenCurFile(HDC dev);
extern rk_err_t DirDev_GetCurPath(HDC dev, uint16 * path);
extern rk_err_t DirDev_FindNext(HDC dev, uint32 *dir);
extern rk_err_t DirDev_FindFirst(HDC dev, uint32 *dir);
extern rk_err_t DirDev_GotoNextDir(HDC dev);
extern rk_err_t DirDev_GetCurDirTotalFile(HDC dev, uint32 * totalfile);
extern rk_err_t DirDev_GetTotalFile(HDC dev, uint32 * totalfile);
extern rk_err_t DirDev_GetFilePathByGlobalNum(HDC dev, uint16 * path, uint32 curnum);
extern rk_err_t DirDev_GetFilePathByCurNum(HDC dev, uint16 * path, uint32 curnum);
extern rk_err_t DirDev_BuildDirInfo(HDC dev, uint8* ExtName, uint16 * path);
extern rk_err_t DirDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t DirDev_Write(HDC dev);
extern rk_err_t DirDev_Read(HDC dev);
extern rk_err_t DirDev_Delete(uint32 DevID, void * arg);
extern HDC DirDev_Create(uint32 DevID, void * arg);



#endif
