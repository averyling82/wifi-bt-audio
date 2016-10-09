/*
********************************************************************************************
*
*           Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                                  All rights reserved.
*
* FileName: Driver\SD\SdDevice.h
* Owner: aaron.sun
* Date: 2014.12.11
* Time: 16:30:28
* Desc: sd /tf driver
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2014.12.11     16:30:28   1.0
********************************************************************************************
*/

#ifndef __DRIVER_SD_SDDEVICE_H__
#define __DRIVER_SD_SDDEVICE_H__

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
#define _DRIVER_SD_SDDEVICE_COMMON_  __attribute__((section("driver_sd_sddevice_common")))
#define _DRIVER_SD_SDDEVICE_INIT_  __attribute__((section("driver_sd_sddevice_init")))
#define _DRIVER_SD_SDDEVICE_SHELL_  __attribute__((section("driver_sd_sddevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_SD_SDDEVICE_SHELL_DATA_      _DRIVER_SD_SDDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_SD_SDDEVICE_SHELL_DATA_      __attribute__((section("driver_sd_sddevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

typedef struct _SD_DEV_ARG
{
    BUS_WIDTH   BusWidth;
    HDC hSdc;

}SD_DEV_ARG;


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
extern rk_err_t SdDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t SdDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_err_t SdDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_err_t SdDev_Delete(uint32 DevID, void * arg);
extern HDC SdDev_Create(uint32 DevID, void * arg);



#endif

