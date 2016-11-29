/*
********************************************************************************************
*
*                  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\OS\Plugin\DevicePlugin.h
* Owner: Aaron.sun
* Date: 2014.4.9
* Time: 8:57:22
* Desc: device manager task
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.4.9     8:57:22   1.0
********************************************************************************************
*/

#ifndef __OS_PLUGIN_DEVICEPLUGIN_H__
#define __OS_PLUGIN_DEVICEPLUGIN_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _OS_DEVICEMANAGER_DEVICEMANAGERTASK_COMMON_  __attribute__((section("os_devicemanager_devicemanagertask_common")))
#define _OS_DEVICEMANAGER_DEVICEMANAGERTASK_INIT_  __attribute__((section("os_devicemanager_devicemanagertask_init")))
#define _OS_DEVICEMANAGER_DEVICEMANAGERTASK_SHELL_  __attribute__((section("os_devicemanager_devicemanagertask_shell")))

typedef enum _DEVICE_LIST
{
#ifdef _EMMC_BOOT_
    DEVICE_LIST_EMMC_BOOT,       //support emmc boot
    DEVICE_LIST_EMMC_DATABASE,   //support data save in emmc boot mode
    DEVICE_LIST_EMMC_FS1,      // support first file sytem in emmc boot mode
#ifdef _HIDDEN_DISK1_
    DEVICE_LIST_EMMC_FS2,      // support second file system in emmc boot mode
#endif
#ifdef _HIDDEN_DISK2_
    DEVICE_LIST_EMMC_FS3,      // support third file system in emmc boot mode
#endif
#endif

#ifdef _SPI_BOOT_
    DEVICE_LIST_SPI_BOOT,       //support spi boot
    DEVICE_LIST_SPI_DATABASE,  //support data save in spi boot mode
    DEVICE_LIST_SPI_FS1,      // support first file system in spi boot mode
#ifdef _HIDDEN_DISK1_
    DEVICE_LIST_SPI_FS2,      // support second file system in spi boot mode
#endif
#ifdef _HIDDEN_DISK2_
    DEVICE_LIST_SPI_FS3,      // support third file system in spi boot mode
#endif
#endif

    DEVICE_LIST_ADUIO_PLAY,   //support audio player
    DEVICE_LIST_DISPLAY,      //support diaplay
    DEVICE_LIST_ADC_KEY,      // support keyboard
    DEVICE_LIST_SD,           //support sd / TF CARD
    DEVICE_LIST_USBDEVICE_MSC,    //support usb device msc
    DEVICE_LIST_SDIO,      //support sdio device

    DEVICE_LIST_FIFO,         //support data stream cache
    DEVICE_LIST_DIR,          //support file system dir tree
    DEVICE_LIST_HOST_USBMSC,  //support usb host

    DEVICE_LIST_FM,    //support Fm play

    DEVICE_LIST_NUM,

}DEVICE_LIST;

typedef enum _DEVICE_CMD
{
    DEVICE_CMD_CREATE_LIST,
    DEVICE_CMD_DELETE_LIST,
    DEVICE_CMD_REMOVE_FS,
    DEVICE_CMD_LOAD_FS,
    DEVICE_CMD_SYS_RESET,
    DEVICE_CMD_SYS_POWER_OFF,
    DEVICE_CMD_IDLE_TICK,
    DEVICE_CMD_CREATE,
    DEVICE_CMD_DELETE,
    DEVICE_CMD_RESUME,
    DEVICE_CMD_NUM

}DEVICE_CMD;

#define MAX_DEV_PER_LIST 10
#define MAX_D_DEV_LIST 0




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
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern uint32 * DeviceList;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t DeviceTask_ResumeDevice(uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode);
extern rk_err_t DeviceTask_DeleteDevice(uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode);
extern rk_err_t DeviceTask_CreateDevice(uint32 ClassID, uint32 ObjectID, void * arg, uint32 Mode);
extern rk_err_t DeviceTask_DevIdleTick(void);
extern rk_err_t DeviceTask_LoadRemoveFs(uint32 DeviceListID, uint32 Mode);
extern rk_err_t DeviceTask_RemoveFs(uint32 DeviceListID, uint32 Mode);
extern rk_err_t DeviceTask_DeleteDeviceList(uint32 DeviceListID, void *arg, uint32 Mode);
extern rk_err_t DeviceTask_System_PowerOff(void);
extern rk_err_t DeviceTask_SystemReset(uint32 maskrom);
extern rk_err_t DeviceTask_CreateDeviceList(uint32 DeviceListID, void *arg, uint32 Mode);
extern void DeviceTask(void);
extern rk_err_t DeviceTaskDeInit(void *pvParameters);
extern rk_err_t DeviceTaskInit(void *pvParameters);



#endif

