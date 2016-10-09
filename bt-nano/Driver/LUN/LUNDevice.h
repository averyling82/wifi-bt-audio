/*
********************************************************************************************
*
*           Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                                  All rights reserved.
*
* FileName: ..\Driver\LUN\LUNDevice.h
* Owner: Aaron.sun
* Date: 2014.3.6
* Time: 16:05:27
* Desc: Lun Device class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.3.6     16:05:27   1.0
********************************************************************************************
*/

#ifndef __DRIVER_LUN_LUNDEVICE_H__
#define __DRIVER_LUN_LUNDEVICE_H__

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
#define _DRIVER_LUN_LUNDEVICE_COMMON_  __attribute__((section("driver_lun_lundevice_read")))
#define _DRIVER_LUN_LUNDEVICE_INIT_  __attribute__((section("driver_lun_lundevice_init")))
#define _DRIVER_LUN_LUNDEVICE_SHELL_  __attribute__((section("driver_lun_lundevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_LUN_LUNDEVICE_SHELL_DATA_      _DRIVER_LUN_LUNDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_LUN_LUNDEVICE_SHELL_DATA_      __attribute__((section("driver_lun_lundevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

#define LUN_MAX_NUM 10

typedef struct _LUN_DEV_ARG
{
    HDC hStorage;
    uint32 dwStartLBA;
    uint32 dwEndLBA;
    rk_err_t (* pfStorgeRead)(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
    rk_err_t (* pfStorgeWrite)(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
    uint32 dwSupportPar;

}LUN_DEV_ARG;

typedef enum _LUN_DEV_CMD
{
    LUN_DEV_GET_PAR,
    LUN_DEV_CREATE_PAR,
    LUN_DEV_DELETE_PAR,
    LUN_DEV_GET_SIZE,
    LUN_DEV_CMD_NUM

}LUN_DEV_CMD;


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
extern rk_err_t LunDev_GetParTotalCnt(HDC dev, uint32 * pParTotalCnt);
extern rk_err_t LunDev_GetPar(HDC dev, uint32 ParNum, uint32 * pVolumeType, uint32 * pStartLBA, uint32 * pTotalSize);
extern rk_err_t LunDev_GetSize(HDC dev,  uint32 * pSize);
extern rk_err_t LunDev_Shell(HDC dev, uint8 * pstr);
extern HDC LunDev_Create(uint32 DevID, void * arg);
extern rk_err_t LunDev_Delete(uint32 DevID, void * arg);
extern rk_size_t LunDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_size_t LunDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len);



#endif

