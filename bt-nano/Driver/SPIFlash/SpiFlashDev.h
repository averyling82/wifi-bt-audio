/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SPIFlash\SpiFlashDev.h
* Owner: wangping
* Date: 2015.6.23
* Time: 10:27:22
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.23     10:27:22   1.0
********************************************************************************************
*/


#ifndef __DRIVER_SPIFLASH_SPIFLASHDEV_H__
#define __DRIVER_SPIFLASH_SPIFLASHDEV_H__

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
typedef  struct _SPIFLASH_DEV_ARG
{
    uint32 spirate;
}SPIFLASH_DEV_ARG;

#define _DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_  __attribute__((section("driver_spiflash_spiflashdev_common")))
#define _DRIVER_SPIFLASH_SPIFLASHDEV_INIT_  __attribute__((section("driver_spiflash_spiflashdev_init")))
#define _DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_  __attribute__((section("driver_spiflash_spiflashdev_shell")))
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
extern rk_err_t SpiFlashDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t SpiFlashDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_err_t SpiFlashDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
extern rk_err_t SpiFlashDev_Delete(uint32 DevID, void * arg);
extern HDC SpiFlashDev_Create(uint32 DevID, void * arg);
extern rk_err_t  SpiFlashDev_GetSize(HDC dev, uint32 * Size);
extern void SpiFlashDevIntIsr0(void);


#endif
