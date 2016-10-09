/*
********************************************************************************************
*
*  Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\EMMC\EmmcDevice.h
* Owner: Aaron.sun
* Date: 2014.2.24
* Time: 10:35:46
* Desc:
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.2.24     10:35:46   1.0
********************************************************************************************
*/

#ifndef __DRIVER_EMMC_EMMCDEVICE_H__
#define __DRIVER_EMMC_EMMCDEVICE_H__

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
#define _DRIVER_EMMC_EMMCDEVICE_COMMON_  __attribute__((section("driver_emmc_emmcdevice_common")))
#define _DRIVER_EMMC_EMMCDEVICE_INIT_  __attribute__((section("driver_emmc_emmcdevice_init")))
#define _DRIVER_EMMC_EMMCDEVICE_SHELL_  __attribute__((section("driver_emmc_emmcdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_EMMC_EMMCDEVICE_DATA_      _DRIVER_EMMC_EMMCDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_EMMC_EMMCDEVICE_DATA_      __attribute__((section("driver_emmc_emmcdevice_data")))
#else
#error Unknown compiling tools.
#endif


#define COMMAND_CLASS_7         (0x1 << 7) //Command Class 7:lock card

#define CARD_IS_LOCKED          (0x1 << 25)
#define LOCK_UNLOCK_FAILED      (0x1 << 24)
#define COM_CRC_ERROR           (0x1 << 23)
#define CARD_ECC_FAILED         (0x1 << 21)
#define CC_ERROR                (0x1 << 20) //internal card controller error
#define CARD_UNKNOWN_ERROR      (0x1 << 19) //a general or an unknown error occurred during the operation
#define READY_FOR_DATA          (0x1 << 8)
#define APP_CMD                 (0x1 << 5)


typedef struct _EMMC_DEV_ARG
{
    BUS_WIDTH   BusWidth;
    HDC hSdc;

}EMMC_DEV_ARG;

typedef enum _EMMC_AREA
{
    EMMC_AREA_BOOT1,
    EMMC_AREA_BOOT2,
    EMMC_AREA_RPMB,
    EMMC_AREA_GENRE,
    EMMC_AREA_USER,
    EMMC_AREA_NUM

}EMMC_AREA;


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
extern rk_err_t EmmcDev_GetAreaSize(HDC dev, uint32 Area, uint32 * pSize);
extern rk_err_t EmmcDev_SetArea(HDC dev, uint32 Area);
extern rk_size_t EmmcDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_size_t EmmcDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern HDC EmmcDev_Create(uint32 DevID, void * arg);
extern rk_err_t EmmcDevDelete(uint32 DevID, void * arg);
extern rk_err_t EmmcDev_Shell(HDC dev, uint8 * pstr);



#endif

