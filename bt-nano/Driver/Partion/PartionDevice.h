/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..call\Driver\Partion\PartionDevice.h
* Owner: Aaron.sun
* Date: 2014.3.12
* Time: 11:49:53
* Desc: Partion device class
* History:
*	 <author>	 <date> 	  <time>	 <version>	   <Desc>
* Aaron.sun     2014.3.12     11:49:53   1.0
********************************************************************************************
*/

#ifndef _CALL_DRIVER_PARTION_PARTIONDEVICE_H__
#define _CALL_DRIVER_PARTION_PARTIONDEVICE_H__

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
#define _DRIVER_PARTION_PARTIONDEVICE_COMMON_  __attribute__((section("driver_partion_partiondevice_common")))
#define _DRIVER_PARTION_PARTIONDEVICE_INIT_  __attribute__((section("driver_partion_partiondevice_init")))
#define _DRIVER_PARTION_PARTIONDEVICE_SHELL_  __attribute__((section("driver_partion_partiondevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_PARTION_PARTIONDEVICE_SHELL_DATA_      _DRIVER_PARTION_PARTIONDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_PARTION_PARTIONDEVICE_SHELL_DATA_      __attribute__((section("driver_partion_partiondevice_shell_data")))
#else
#error Unknown compiling tools.
#endif


#define PARTION_MAX_NUM 5

typedef struct _PAR_DEVICE_ARG
{
    HDC hLun;
    uint32 ParStartLBA;
    uint32 VolumeType;
    uint32 ParTotalSec;

}PAR_DEVICE_ARG;

typedef enum _VOLUME_TYPE
{
   VOLUME_TYPE_FAT12 = 1,
   VOLUME_TYPE_FAT16,
   VOLUME_TYPE_FAT32,
   VOLUME_TYPE_FAT,
   VOLUME_TYPE_EXFAT,
   VOLUME_TYPE_NTFS,
   VOLUME_TYPE_XML,
   VOLUME_TYPE_FTP,
   VOLUME_TYPE_OBEX,
   VOLUME_TYPE_RAW,
   VOLUME_TYPE_NUM

}VOLUME_TYPE;

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
extern rk_err_t ParDev_FormatFat(HDC dev, uint8 * pVolumeName);
extern rk_err_t ParDev_FormatFat32(HDC dev, uint8 * pVolumeName);
extern rk_err_t ParDev_GetVolumeType(HDC dev, uint32 * VolumeType);
extern rk_err_t ParDev_Shell(HDC dev, uint8 * pstr);
extern HDC ParDev_Create(uint32 DevID, void * arg);
extern rk_err_t ParDev_Delete(uint32 DevID, void * arg);
extern rk_size_t ParDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_size_t ParDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len);



#endif

