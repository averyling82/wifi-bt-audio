/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\syssever\SystemApi.c
* Owner: aaron.sun
* Date: 2016.6.2
* Time: 9:44:12
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.6.2     9:44:12   1.0
********************************************************************************************
*/

#define __SYSTEM_SYSSEVER_SYSTEMAPI_C__
#include "BspConfig.h"
#ifdef __SYSTEM_SYSSEVER_SYSTEMAPI_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "SystemApi.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "hw_sdmmc.h"
#include "emmcdevice.h"
#include "SpiFlashDev.h"
#include "powermanager.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: rkos_get_wifi_ssid
** Input:uint8 * ssid
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 10:01:27
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_wifi_ssid(uint8 * ssid)
{
    uint32 len;

    if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) == NULL)
    {
        return RK_ERROR;
    }
    #ifdef _WIFI_
    rkwifi_get_ssid(ssid, &len);
    #endif
    ssid[len] = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: rkos_get_storage_size
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 9:52:45
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_storage_size(void)
{
    HDC hEmmc, hSpi;
    uint32 StorageSise;


    #ifdef _EMMC_BOOT_
    hEmmc = RKDev_Open(DEV_CLASS_EMMC, 0, NOT_CARE);
    if ((hEmmc == NULL) || (hEmmc == (HDC)RK_ERROR) || (hEmmc == (HDC)RK_PARA_ERR))
    {
        return RK_ERROR;
    }

    if(EmmcDev_GetAreaSize(hEmmc, EMMC_AREA_USER, &StorageSise) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    RKDev_Close(hEmmc);

    return (rk_err_t)StorageSise;
    #endif

    #ifdef _SPI_BOOT_
    hSpi = RKDev_Open(DEV_CLASS_SPIFLASH, 0, NOT_CARE);

    if ((hSpi == NULL) || (hSpi == (HDC)RK_ERROR) || (hSpi == (HDC)RK_PARA_ERR))
    {
        return RK_ERROR;
    }

    if(SpiFlashDev_GetSize(hSpi, &StorageSise) != RK_SUCCESS)
    {
        return RK_ERROR;
    }
    RKDev_Close(hSpi);
    return (rk_err_t)StorageSise;
    #endif
}

/*******************************************************************************
** Name: rkos_get_bt_status
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 9:51:18
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_bt_status(void)
{
#ifdef _BLUETOOTH_
    return gSysConfig.BtOpened;
#else
    return 0;
#endif
}

/*******************************************************************************
** Name: rkos_get_sys_volume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 9:50:01
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_sys_volume(void)
{
    return gSysConfig.OutputVolume;
}

/*******************************************************************************
** Name: rkos_get_battery_level
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 9:48:29
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_battery_level(void)
{
    return gBattery.Batt_Level;
}

/*******************************************************************************
** Name: rkos_get_battery_status
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 9:47:30
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_battery_status(void)
{
    return gBattery.IsBatt_Charge;
}

/*******************************************************************************
** Name: rkos_get_wifi_status
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 9:46:26
*******************************************************************************/
_SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_
COMMON API rk_err_t rkos_get_wifi_status(void)
{
    if(RKTaskFind(TASK_ID_WIFI_APPLICATION, 0) == NULL)
    {
        return RK_ERROR;
    }
    #ifdef _WIFI_
    if(wifi_connect_flag() == TRUE)
    #endif
    {
        return RK_SUCCESS;
    }

    return RK_ERROR;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
